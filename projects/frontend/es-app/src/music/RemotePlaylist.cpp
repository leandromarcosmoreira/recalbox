//
// Created by bkg2k on 29/05/22.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#include "RemotePlaylist.h"
#include "rapidjson/document.h"
#include "recalbox/RecalboxSystem.h"
#include "IRemotePlaylistNotification.h"
#include "network/Networks.h"
#include <patreon/PatronInfo.h>
#include <RecalboxConf.h>
#include <utils/Files.h>

RemotePlaylist::RemotePlaylist(IRemotePlaylistNotification* callback)
  : StaticLifeCycleControler("RemotePlaylist")
  , mSender(*this)
  , mStorageRoot(sStorageRootPath)
  , mNextIndex(-1)
  , mCallback(callback)
{
  Thread::Start("RemotePlaylist");
}

RemotePlaylist::~RemotePlaylist()
{
  mRequest.Cancel();
  mAction.Fire();
  Thread::Stop();
}

void RemotePlaylist::Run()
{
  // Wait for Patreon response
  PatronInfo::Instance().WaitForAuthentication(*this);

  // Check patron status
  if (PatronInfo::Instance().Status() == PatronAuthenticationResult::Patron)
  {
    // Update playlist
    UpdatePlayList();
    // Load & check index
    InitializeIndex();
    // Let's go for action
    mAction.Fire();

    // Main loop
    while(IsRunning())
    {
      // Wait for action
      mAction.WaitSignal();
      if (!IsRunning()) break;
      // Sanitize files
      SanitizeFiles();
      // Download new files
      DownloadFiles();
    }
  }
  else { LOG(LogError) << "[RemotePlaylist] Not a patron"; }
}

bool RemotePlaylist::DeserializePlaylist(const String& jsonString)
{
  // Deserialize
  rapidjson::Document json;
  json.Parse(jsonString.c_str());
  if (!json.HasParseError() && json.IsArray())
  {
    for(const rapidjson::Value& object : json.GetArray())
    {
      if (object.HasMember("name") &&
          object.HasMember("time_seconds") &&
          object.HasMember("size") &&
          object.HasMember("url") &&
          object.HasMember("md5") &&
          object.HasMember("album") &&
          object.HasMember("mixtape") &&
          object.HasMember("artist"))
        mPlaylist.push_back(TrackInfo(object["name"].GetString(),
                                      object["time_seconds"].GetInt(),
                                      object["size"].GetInt(),
                                      object["url"].GetString(),
                                      object["md5"].GetString(),
                                      object["album"].GetString(),
                                      object["mixtape"].GetString(),
                                      object["artist"].GetString()));
    }
    { LOG(LogDebug) << "[RemotePlaylist] Downloaded a playlist with : " << mPlaylist.size() << " musics"; }
    // Cache list
    Files::SaveFile(Path(sStorageList), jsonString);
    return true;
  }
  { LOG(LogError) << "[RemotePlaylist] Error getting playlist. Malformed json: " << jsonString; }
  return false;
}

bool RemotePlaylist::UpdatePlayList()
{
  String jsonString;

  // Remote
  mRequest.SetBearer(RecalboxConf::Instance().GetRecalboxPrivateKey());
  if (mRequest.Execute(sRemotePlayListUrl, jsonString))
  {
    if (mRequest.GetLastHttpResponseCode() == 200)
      return DeserializePlaylist(jsonString);
    { LOG(LogError) << "[RemotePlaylist] Error getting playlist. Error code: " << mRequest.GetLastHttpResponseCode(); }
    return false;
  }
  { LOG(LogError) << "[RemotePlaylist] Error getting playlist."; }

  // Local
  jsonString = Files::LoadFile(Path(sStorageList));
  return DeserializePlaylist(jsonString);
}

void RemotePlaylist::InitializeIndex()
{
  // Initialize main path
  (void)mStorageRoot.CreatePath();

  // Try loading index
  Path indexPath(sStorageIndex);
  String md5Index = Files::LoadFile(indexPath);
  mNextIndex = LookupTrackIndex(md5Index);
  { LOG(LogDebug) << "[RemotePlaylist] Found index to start from : " << mNextIndex << " (" << md5Index << ")"; }
  if (mNextIndex < 0) mNextIndex = 0;
}

void RemotePlaylist::UpdateIndex()
{
  if ((unsigned int)mNextIndex >= (unsigned int)mPlaylist.size()) return;
  Path indexPath(sStorageIndex);
  { LOG(LogDebug) << "[RemotePlaylist] Setting next index file to " << mNextIndex << "(" << mPlaylist[mNextIndex].Name() << ")";}
  Files::SaveFile(indexPath, mPlaylist[mNextIndex].Md5());
}

int RemotePlaylist::LookupTrackIndex(const String& md5)
{
  for(int i = (int)mPlaylist.size(); --i >= 0; )
    if (md5 == mPlaylist[i].Md5())
      return i;
  return -1;
}

void RemotePlaylist::SanitizeFiles()
{
  if (mPlaylist.empty()) return;

  // Remove unknown files
  for(const Path& file : Path(sStorageRootPath).GetDirectoryContent())
    if (LookupTrackIndex(file.FilenameWithoutExtension()) < 0)
    {
      (void)file.Delete();
      { LOG(LogDebug) << "[RemotePlaylist] File " << file.ToString() << " not in playlist. Deleted."; }
    }

  // Remove files out index/index+X range
  // Only if network is available
  if (Networks::Instance().HasIP(Networks::IPVersion::Both, Networks::Interfaces::Both))
  {
    mLocker.Lock();
    int first = mNextIndex-1 >= 0 ? mNextIndex - 1 : 0;
    int last = (first + sMaxCachedFile) % (int) mPlaylist.size();
    mLocker.UnLock();
    { LOG(LogDebug) << "[RemotePlaylist] SanitizeFiles: checking files between index " << first << " and " << last; }
    for (int trackIndex = (int) mPlaylist.size(); --trackIndex >= 0 && IsRunning();)
    {
      const Path& file = mPlaylist[trackIndex].LocalPath();
      if (file.Exists())
        if (((first < last) && (trackIndex < first || trackIndex > last)) || ((first > last) && (trackIndex > last && trackIndex < first)) ||
            !mPlaylist[trackIndex].HasValidFile())
        {
          (void)file.Delete();
          { LOG(LogDebug) << "[RemotePlaylist] File " << file.ToString() << "(index " << trackIndex << ") out of index range. Deleted."; }
        }
    }
  }
}

void RemotePlaylist::DownloadFiles()
{
  if (mPlaylist.empty()) return;

  bool notify = false;
  // Loop until there is nothing more to download
  for(bool download = true; download && IsRunning();)
  {
    download = false;
    mLocker.Lock();
    int index = mNextIndex;
    mLocker.UnLock();
    for(int i = sMaxCachedFile; --i >= 0 && IsRunning(); )
    {
      TrackInfo& track = mPlaylist[index];
      if (!track.LocalPath().Exists())
      {
        { LOG(LogDebug) << "[RemotePlaylist] Downloading " << track.LocalPath().ToString() << " at index "<< index; }
        if (!mRequest.Execute(track.Url(), track.LocalPath()))
        {
          if (!IsRunning()) return;
          { LOG(LogError) << "[RemotePlaylist] Failed to download " << track.Url() << ". Retry in a few seconds."; }
          Thread::Sleep(5000);
          download = true;
          break;
        }
        if (!IsRunning()) return;
        if (mRequest.GetLastHttpResponseCode() != 200)
        {
          { LOG(LogError) << "[RemotePlaylist] Failed to download " << track.Url() << ". Http Result code: " << mRequest.GetLastHttpResponseCode() << ". Retry in a few seconds."; }
          Thread::Sleep(5000);
          download = true;
          break;
        }
        if (mRequest.Md5().ToLowerCase() != track.Md5().ToLowerCase())
        {
          { LOG(LogError) << "[RemotePlaylist] Failed to download " << track.Url() << ". MD5 mismatch. Retry."; }
          continue;
        }
        { LOG(LogDebug) << "[RemotePlaylist] Downloaded " << track.LocalPath().ToString(); }
        download = true;
        notify = true;
      }
      index = (index + 1) % (int)mPlaylist.size();
    }
  }
  if (notify) mSender.Send();
}

RemotePlaylist::TrackInfo* RemotePlaylist::GetNextTrack()
{
  if (mPlaylist.empty()) return nullptr;
  if (!RecalboxConf::Instance().GetMusicRemoteEnable()) return nullptr;

  {
    Mutex::AutoLock locker(mLocker);
    for(int i = sMaxCachedFile; --i >= 0 && IsRunning(); mNextIndex = (mNextIndex + 1) % (int)mPlaylist.size())
    {
      if (mPlaylist[mNextIndex].HasValidFile())
      {
        { LOG(LogDebug) << "[RemotePlaylist] Next track found on index  "<< mNextIndex << " : " << mPlaylist[mNextIndex].Name() << " (" << mPlaylist[mNextIndex].Md5() << ")"; }
        int currentIndex = mNextIndex;
        mNextIndex = (mNextIndex + 1) % (int)mPlaylist.size();
        UpdateIndex();
        mAction.Fire();
        return &mPlaylist[currentIndex];
      }
    }
  }
  // No file found, let's go download
  { LOG(LogDebug) << "[RemotePlaylist] No next track found, will download"; }
  mAction.Fire();
  return nullptr;
}

void RemotePlaylist::TrackConsumed()
{
  mAction.Fire();
}

void RemotePlaylist::ReceiveSyncMessage()
{
  if (mCallback != nullptr)
    mCallback->PlaylistReady();
}

