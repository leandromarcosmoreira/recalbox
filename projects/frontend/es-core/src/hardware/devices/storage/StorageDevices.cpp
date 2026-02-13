//
// Created by bkg2k on 21/02/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#include "utils/locale/LocaleHelper.h"
#include "hardware/Board.h"
#include "RootFolders.h"
#include "StorageDevices.h"
#include "utils/Files.h"
#include <recalbox/BootConf.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/hdreg.h>

StorageDevices::StorageDevices()
  : StaticLifeCycleControler<StorageDevices>("StorageDevs")
  , mBootConfiguration(BootConf::Instance())
  , mShareInRAM(false)
{
  Initialize();
}

void StorageDevices::Initialize()
{
  mShareDevices.clear();
  mStorageDevices.clear();
  mBootRoot.clear();
  mShareInRAM = false;

  AnalyseMounts();
  String current = GetShareDevice();

  // Get storage sizes
  DeviceSizeInfo sizeInfos = GetFileSystemInfo();

  // Link to mount point
  HashMap<String, String> mountPointLinks;
  HashMap<String, bool> readOnlyLinks;
  for(const String& line : GetMountedDeviceList())
  {
    String::List chunks = line.Split(' ', true);
    if (chunks.size() >= 3)
      mountPointLinks[chunks[0]] = chunks[2];
    readOnlyLinks[chunks[0]] = false;
    if (chunks.size() >= 6)
      for(const String& option : chunks[5].ToTrim("()").Split(','))
        if (option == "ro") readOnlyLinks[chunks[0]] = true;
  }

  // Ram?
  if (mShareInRAM)
  {
    current = "RAM";
    mShareDevices.push_back(Device(Types::Ram, "SHARE", "/recalbox/share", sInMemory, "RECALBOX", "tmpfs", _("In Memory!") + " \u26a0", true, sizeInfos, readOnlyLinks["/recalbox/share"]));
    { LOG(LogWarning) << "[Storage] Share is stored in memory!"; }
  }

  // Add Internal
  mShareDevices.push_back(Device(Types::Internal, "SHARE", "/recalbox/share", sInternal, "INTERNAL", "exfat", _("Internal Share Partition"), current == sInternal, sizeInfos, readOnlyLinks["/recalbox/share"]));

  // Network
  if (mBootConfiguration.HasKeyStartingWith("sharenetwork_") || mBootConfiguration.AsString("sharedevice") == "NETWORK")
  {
    mShareDevices.push_back(Device(Types::Internal, "", "/recalbox/share", sNetwork, "NETWORK", "", _("Network Share"), current == sNetwork, sizeInfos, readOnlyLinks["/recalbox/share"]));
    { LOG(LogDebug) << "[Storage] Network share configuration detected"; }
  }

  // Go advance does not support external storage
  if ((Board::Instance().GetBoardType() == BoardType::OdroidAdvanceGo) ||
      (Board::Instance().GetBoardType() == BoardType::OdroidAdvanceGoSuper) ||
      (Board::Instance().GetBoardType() == BoardType::RG351P)) return;

  // Any external
  if (mBootConfiguration.AsString("sharedevice") == "ANYEXTERNAL") {
    mShareDevices.push_back(Device(Types::Internal, "", "/recalbox/share", sAnyExternal, "", "", _("Any External Device").Append(" (").Append(_("Deprecated")).Append(')'), current == sAnyExternal, sizeInfos, readOnlyLinks["/recalbox/share"]));
    { LOG(LogDebug) << "[Storage] Any external share configuration detected"; }
  }

  // External Devices
  String devicePath;
  String propertyLine;
  for(const String& line : GetRawDeviceList())
    if (line.Extract(':', devicePath, propertyLine, true))
    {
      PropertyMap properties = ExtractProperties(propertyLine);
      String uuid = "DEV " + properties.get_or_return_default("UUID");
      String label = properties.get_or_return_default("LABEL");
      String filesystem = properties.get_or_return_default("TYPE");
      if (filesystem.empty()) filesystem = "fs?";
      String displayable = LookupName(devicePath);
      String mountPoint;
      if (String* mp = mountPointLinks.try_get(devicePath); mp != nullptr) mountPoint = *mp;

      // --------- Storage device
      if (!devicePath.StartsWith(mBootRoot) || label == "SHARE")
      {
        if (devicePath.StartsWith(mBootRoot) && label == "SHARE") displayable = "ORIGINAL SHARE";
        // Store
        mStorageDevices.push_back(Device(Types::External, devicePath, mountPoint, uuid, label, filesystem, displayable, current == uuid, sizeInfos, readOnlyLinks[devicePath]));
        { LOG(LogDebug) << "[Storage] External storage: " << devicePath << ' ' << uuid << " \"" << label << "\" " << filesystem; }
      }

      // --------- Share devices
      if (!devicePath.StartsWith(mBootRoot))
      {
        // Extract device properties
        if (displayable.empty())
        {
          displayable = _("Device %d - %l (%f)");
          displayable.Replace("%d", Path(devicePath).Filename()).Replace("%l", label).Replace("%f", filesystem);
        }
        // Store
        mShareDevices.push_back(Device(Types::External, devicePath, mountPoint, uuid, label, filesystem, displayable, current == uuid, sizeInfos, readOnlyLinks[devicePath]));
        { LOG(LogDebug) << "[Storage] External share: " << devicePath << ' ' << uuid << " \"" << label << "\" " << filesystem; }
      }
    }
}

String::List StorageDevices::GetCommandOutput(const String& command)
{
  String output;
  char buffer[4096];
  FILE* pipe = popen(command.data(), "r");
  if (pipe != nullptr)
  {
    while (feof(pipe) == 0)
      if (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        output.Append(buffer);
    pclose(pipe);
  }
  return output.Split('\n');
}

String::List StorageDevices::GetRawDeviceList()
{
  return GetCommandOutput("blkid");
}

String::List StorageDevices::GetMountedDeviceList()
{
  return GetCommandOutput("mount");
}

StorageDevices::DeviceSizeInfo StorageDevices::GetFileSystemInfo()
{
  DeviceSizeInfo result;
  for(const String& line : GetCommandOutput("df -kP"))
  {
    String::List items = line.Split(' ', true);
    if (items.size() >= 6)
    {
      long long size = items[1].AsInt64();
      long long free = items[3].AsInt64();
      result[items[0]] = SizeInfo(size, free);
      // Special cases
      if (items[5] == RootFolders::DataRootFolder.ToString())
        result["SHARE"] = SizeInfo(size, free);
    }
  }
  return result;
}

StorageDevices::PropertyMap StorageDevices::ExtractProperties(const String& properties)
{
  PropertyMap map;

  String key;
  String value;
  for(const String& kv : properties.SplitQuoted(' '))
    if (kv.Extract('=', key, value, true))
      map[key] = value.Trim('"');

  return map;
}

void StorageDevices::SetShareDevice(const Device& device)
{
  mBootConfiguration.SetString(sShareDevice, device.UUID);
  mBootConfiguration.Save();
}

String StorageDevices::GetShareDevice()
{
  return mBootConfiguration.AsString(sShareDevice, sInternal);
}

void StorageDevices::AnalyseMounts()
{
  mBootRoot = "/dev/bootdevice";
  for(const String& line : GetMountedDeviceList())
  {
    String::List items = line.Split(' ', true);
    if (items.size() < 6)
    {
      { LOG(LogError) << "[Storage] Incomplete mount line: " << line; }
      continue;
    }
    if (items[2] == "/recalbox/share") mShareInRAM =  (items[4] == "tmpfs");
    else if (items[2] == "/boot") mBootRoot = items[0].Trim("0123456789");
  }
  if (mBootRoot.empty()) mBootRoot = "/dev/boot"; // for testing purpose only :)
  { LOG(LogDebug) << "[Storage] BootRoot: " << mBootRoot << " - Is In Ram: " << mShareInRAM; }
}

String StorageDevices::LookupName(const String& originalPath)
{
  int partition = -1;
  String device = Path(originalPath).Filename();
  if (int l = device.Count() - 1; l > 1 && device[l - 1] == 'p' && device[l] >= '0' && device[l] <= '9')
  {
    partition = device[l] - 0x30;
    device.Delete(l - 1, 2);
  }

  Path base = Path(String("/sys/block/").Append(device).Append("/device"));
  String result = Files::LoadFile(base / "vendor").Trim();
  if (!result.empty()) result.Append(' ');
  result.Append(Files::LoadFile(base / "model").Trim());
  if (!result.empty()) result.Append(' ');
  result.Append(Files::LoadFile(base / "name").Trim());

  if (!result.Trim().empty() && partition >= 0)
    result.Append(", part. #").Append(partition);

  return result;
}
