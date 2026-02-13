//
// Created by thierry.imbert on 10/01/2020.
//

#include "utils/os/system/Mutex.h"
#include "recalbox/RecalboxSystem.h"
#include "../../../../data/Resources.h"
#include "TextureManager.h"
#include "utils/Sizes.h"
#include <sys/sysinfo.h>

TextureManager::TextureManager()
  : StaticLifeCycleControler<TextureManager>("TextureManager")
  , mPool(this, "Texture", true)
  , mTexturesCache(128)
{
  // Run the thread pool for texture loading
  mPool.Run(-1, true);

  { LOG(LogInfo) << "[TextureManager] Texture manager created."; }
}

TextureManager::~TextureManager()
{
  String::List keys;
  for (const auto& kv : mTexturesCache)
  {
    { LOG(LogTrace) << "[TextureManager] Undestroying texture Texture " << kv.first; }
    keys.push_back(kv.first);
  }
  for (const String& k : keys)
  {
    { LOG(LogTrace) << "[TextureManager] Try to destroy " << k; }
    mTexturesCache.erase(k);
  }
}

RenderingError TextureManager::RequestTextureLoading(Texture& texture, bool noDelay)
{
  if (texture.mTexture->Status() != TextureHolder::Status::Unloaded)
    return RenderingError::NoError;

  // Set the new status to avoid re-entry issues
  texture.mTexture->StartLoading();

  // Load immediately?
  if (noDelay || texture.mTexture->MustLoadImmediately())
    return ThreadPoolRunJob(texture);

  // Delayed load
  mPool.PushFeed(texture);
  return RenderingError::NoError;
}

bool TextureManager::UnloadTheOldest(bool cpu)
{
  //LOG(LogDebug) << "Requested to delete " << (cpu ? "CPU" : "GPU") << " cache of oldest texture";
  unsigned long long oldest = TextureHolder::Tick(); // Initialize to current date time
  TextureHolder* oldestTexture = nullptr;

  { // Critical section: Read/write maps
    Mutex::AutoLock lock(mMutexStorage);

    if (cpu)
    {
      // CPU Loop
      for (TextureHolder* texture : mTextures)
        if (texture->LastUse() < oldest)
          if (texture->CPUConsummed() > 0)
          {
            oldest = texture->LastUse();
            oldestTexture = texture;
            //LOG(LogDebug) << "Candidate: " << oldest << " - " << oldestTexture->CPUConsummed()
            //              << " - " << oldestTexture->FilePath().ToString();
          }
    }
    else
    {
      // GPU Loop
      for (TextureHolder* texture : mTextures)
        if (texture->LastUse() < oldest)
          if (texture->GPUConsummed() > 0)
          {
            oldest = texture->LastUse();
            oldestTexture = texture;
            //LOG(LogDebug) << "Candidate: " << oldest << " - " << oldestTexture->GPUConsummed()
            //              << " - " << oldestTexture->FilePath().ToString();
          }

    }
  }

  // Then unload
  if (oldestTexture != nullptr)
  {
    //LOG(LogDebug) << "BEST Candidate found! " << oldestTexture->FilePath().ToString();
    IUnloadable& unloadable = oldestTexture->AsIUnloadable();
    if (cpu) unloadable.UnloadCaches();
    else unloadable.Unload();
    return true;
  }

  //LOG(LogDebug) << "NO Candidate found";
  return false;
}

RenderingError TextureManager::ThreadPoolRunJob(Texture& feed)
{
  #ifdef BETA
  if (Log::ReportingLevel(LogType::Generic) >= LogLevel::LogDebug)
    ShowStatistics();
  #endif

  //if (mTextures.size() > 10)
  //  UnloadTheOldest(false);

  // Check CPU memory
  WatchCPUMemory();

  // Actually load the texture content and try to bind it
  // either from the thread pool, or from direct calls
  // Check CPU/GPU OOM and process unloading of oldest texture until
  // enough space has been freed
  if (feed.mTexture->Status() < TextureHolder::Status::Loading)
    for(;;)
    {
      // Load is ok?
      switch(feed.mTexture->AsIUnloadable().Load())
      {
        case RenderingError::NoError: return RenderingError::NoError;
        case RenderingError::OutOfMemory:
        {
          FreeAllCPUResources();
          break;
        }
        case RenderingError::OutOfGPUMemory:
        {
          // Try to free some memory
          if (!UnloadTheOldest(false))
            return RenderingError::OutOfGPUMemory; // Nothing more to free
          break;
        }
        case RenderingError::NoResource: return RenderingError::NoResource;
      }

      #ifdef BETA
      if (Log::ReportingLevel(LogType::Generic) >= LogLevel::LogDebug)
        ShowStatistics();
      #endif
    }

  // Already loaded or in loading process
  return RenderingError::NoError;
}

Texture TextureManager::Create(const Path& path, int targetWidth, int targetHeight, TextureHolder::Properties properties)
{
  // Free texture have empty path
  bool noCache = path.IsEmpty() ||
    ((properties & (TextureHolder::Properties::NoCache | TextureHolder::Properties::Mutable | TextureHolder::Properties::Free)) != 0);

  // Cached texture?
  if (!noCache)
  { // Protect storage
    Mutex::AutoLock lock(mMutexStorage);
    Texture* texture = mTexturesCache.try_get(path.ToString());
    if (texture != nullptr) return *texture;
  }

  // Create texture
  Texture newTexture(new TextureHolder(path, targetWidth, targetHeight, properties, *this));
  TextureHolderCreated(newTexture.mTexture); // TODO: usefulness? already done in Texture creation

  // Add to map?
  if (!noCache)
  {
    { // Protect storage
      Mutex::AutoLock lock(mMutexStorage);
      mTexturesCache.insert(path.ToString(), newTexture);
    }
  }

  return newTexture;
}


void TextureManager::FreeAllCPUResources()
{
  Mutex::AutoLock lock(mMutexStorage);
  for (TextureHolder* texture : mTextures)
    texture->FreeCPUCaches();
}

void TextureManager::FreeAllGPUResources()
{
  Mutex::AutoLock lock(mMutexStorage);
  for (TextureHolder* texture : mTextures)
    texture->AsIUnloadable().Unload();
}

void TextureManager::FreeAllResources()
{
  FreeAllCPUResources();
  FreeAllGPUResources();
}

bool TextureManager::Exists(const Path& path)
{
  // Internal resource?
  if (path.ToChars()[0] == ':')
  {
    // Embedded
    const auto& it = res2hMap.find(path.ToString());
    return (it != res2hMap.end());
  }
  if (path.StartWidth("http://") || path.StartWidth("https://")) return true;
  return path.Exists();
}

void TextureManager::ShowStatistics()
{
  long long estimatedGPUMemory = 0;
  long long estimatedCPUMemory = 0;
  int countLoaded = 0;
  int countLoading = 0;
  int countUnloaded = 0;
  int countSVG = 0;
  int countInvalid = 0;
  { // Critical section: Read/write maps
    Mutex::AutoLock lock(mMutexStorage);

    for (auto& texture : mTexturesCache)
      if (!texture.second.Valid())
        countInvalid++;

    for (TextureHolder* texture : mTextures)
    {
      countLoaded   += texture->Status() == TextureHolder::Status::Loaded   ? 1 : 0;
      countUnloaded += texture->Status() == TextureHolder::Status::Unloaded ? 1 : 0;
      countLoading  += texture->Status() == TextureHolder::Status::Loading || texture->Status() == TextureHolder::Status::Queued  ? 1 : 0;
      countSVG      += texture->IsSVG() ? 1 : 0;
      estimatedCPUMemory += texture->CPUConsummed();
      estimatedGPUMemory += texture->GPUConsummed();
    }
  }
  LOG(LogInfo) << "[Texture Statistics] Total: " << mTextures.size()
               << " Cached/Invalids: " << mTexturesCache.size() << '/' << countInvalid
               << " Loaded/Loading/Unloaded: " << countLoaded << '/' << countLoading << '/' << countUnloaded
               << " SVG: " << countSVG
               << " CPU: " << Sizes(estimatedCPUMemory).ToHumanSize()
               << " GPU: " << Sizes(estimatedGPUMemory).ToHumanSize();
}

void TextureManager::TextureHolderCreated(TextureHolder* newTexture)
{
  // Critical section: Read/write maps
  Mutex::AutoLock lock(mMutexStorage);
  mTextures.insert(newTexture);
}

void TextureManager::TextureHolderDestroyed(TextureHolder* textureDestroyed)
{
  // Critical section: Read/write maps
  Mutex::AutoLock lock(mMutexStorage);
  if (!textureDestroyed->IsNotCached())
    if (!mTexturesCache.erase(textureDestroyed->FilePath().ToString()))
      { LOG(LogTrace) << "Texture path " << textureDestroyed->FilePath() << " not found in cache !"; }
  mTextures.erase(textureDestroyed);
}

long long TextureManager::TotalCPUMemory()
{
  long long result = 0;
  // Critical section: Read/write maps
  Mutex::AutoLock lock(mMutexStorage);
  for (TextureHolder* texture : mTextures)
    result += texture->CPUConsummed();
  return result;
}

long long TextureManager::TotalGPUMemory()
{
  long long result = 0;
  // Critical section: Read/write maps
  Mutex::AutoLock lock(mMutexStorage);
  for (TextureHolder* texture : mTextures)
    result += texture->GPUConsummed();
  return result;
}

#include <sys/resource.h>

void TextureManager::WatchCPUMemory()
{
  rusage usage {};
  getrusage(RUSAGE_SELF, &usage);
  long long consummedMemory = (long long)usage.ru_maxrss << 10LL;

  struct sysinfo sysInfos {};
  sysinfo(& sysInfos);
  long long halfMemory = ((long long)(sysInfos.totalram) * (long long)sysInfos.mem_unit) >> 1;
  long long freeMemory = consummedMemory != 0 ? ((long long)sysInfos.totalram * (long long)sysInfos.mem_unit) - consummedMemory : 0;
  long long oneGigabyte = (1 << 30);

  //LOG(LogDebug) << "Half memory: " << halfMemory;
  //LOG(LogDebug) << "Free memory: " << freeMemory;

  if (freeMemory < halfMemory && freeMemory < oneGigabyte)
    (void)UnloadTheOldest(true);
}

int TextureManager::ThreadPoolNextFeedIndex(const std::vector<IndexedFeed<Texture>>& list)
{
  int selectedIndex = 0;
  unsigned long long int mostRecentUse = 0;
  for(int i = (int)list.size(); --i >= 0;)
    if (unsigned long long int lastUse = list[i].Feed.mTexture->LastUse(); lastUse > mostRecentUse)
    {
      mostRecentUse = lastUse;
      selectedIndex = i;
    }
  return selectedIndex;
}

void TextureManager::ConfigureHttpClient(const String& url, HttpClient& client)
{
  Mutex::AutoLock locker(mMutexStorage);
  for(IHttpConfiguration* configurator : mHttpConfigurationFilter)
    if (configurator != nullptr)
      configurator->ConfigureHttpClient(url, client);
}
