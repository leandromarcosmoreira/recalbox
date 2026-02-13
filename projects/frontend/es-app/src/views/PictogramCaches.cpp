//
// Created by bkg2k on 07/03/24.
//

#include "PictogramCaches.h"
#include <rendering/textures/TextureManager.h>

Texture PictogramCaches::GetFlag(Regions::GameRegions region, int height)
{
  Mutex::AutoLock lock(mLocker);
  Texture* flag = mRegionToTextures.try_get(region);
  if (flag == nullptr)
  {
    // Load flag
    Texture texture = TextureManager::Instance().Create(Path(":/regions/" + Regions::GameRegionsFromEnum(region) + ".svg"), 0, height, TextureHolder::Properties::NoCache);
    mRegionToTextures.insert(region, texture);
    flag = mRegionToTextures.try_get(region);
  }
  return *flag;
}

Texture PictogramCaches::GetGenre(GameGenres genre, int height)
{
  Mutex::AutoLock lock(mLocker);
  Texture* ptexture = mGenreToTextures.try_get(genre);
  if (ptexture == nullptr)
  {
    // Load genre
    Texture texture = TextureManager::Instance().Create(Path(String(Genres::GetResourcePath(genre).ToString()).Replace("/genre/", "/genreflags/")), 0, height, TextureHolder::Properties::NoCache);
    mGenreToTextures.insert(genre, texture);
    ptexture = mGenreToTextures.try_get(genre);
  }
  return *ptexture;
}

Texture PictogramCaches::GetPlayers(int playermin, int playermax, int height)
{
  // Some control first
  if (playermin == 0xFFFF) { playermin = playermax; playermax = 0; }
  playermin = Math::clampi(playermin, 1, 12);
  playermax = Math::clampi(playermax, 1, 13);
  if (playermax < playermin && playermax != 0) playermax = playermin;

  int playerMinMax = (playermin << 16) | playermax;
  Texture* pPlayers = mPlayersToTextures.try_get(playerMinMax);
  if (pPlayers == nullptr)
  {
    String resourceName(":/players/");
    resourceName.Append(playermin);
    if (playermin != playermax)
    {
      if (playermax <= 12) resourceName.Append("to").Append(playermax);
      else resourceName.Append("plus");
    }
    resourceName.Append(".svg");
    // Load players
    Texture texture = TextureManager::Instance().Create(Path(resourceName), 0, height, TextureHolder::Properties::NoCache);
    mPlayersToTextures.insert(playerMinMax, texture);
    pPlayers = mPlayersToTextures.try_get(playerMinMax);
  }
  return *pPlayers;
}

Texture PictogramCaches::GetSupportLeftPart(int height)
{
  if (!mSupportLeft.Valid())
    mSupportLeft = TextureManager::Instance().Create(Path(":/supports/first.svg"), 0, height, TextureHolder::Properties::NoCache);
  return mSupportLeft;
}

Texture PictogramCaches::GetSupportRightPart(int height)
{
  if (!mSupportRight.Valid())
    mSupportRight = TextureManager::Instance().Create(Path(":/supports/last.svg"), 0, height, TextureHolder::Properties::NoCache);
  return mSupportRight;
}

Texture PictogramCaches::GetSupportSeparator(int height)
{
  if (!mSupportSeparator.Valid())
    mSupportSeparator = TextureManager::Instance().Create(Path(":/supports/separator.svg"), 0, height, TextureHolder::Properties::NoCache);
  return mSupportSeparator;
}

Texture PictogramCaches::GetSupportSide(SupportSides side, int height)
{
  Texture* pSide = mSupportSideToTextures.try_get(side);
  if (pSide == nullptr)
  {
    String name;
    switch(side)
    {
      case SupportSides::None: break;
      case SupportSides::A: name = "A"; break;
      case SupportSides::B: name = "B"; break;
    }
    Texture texture = TextureManager::Instance().Create(Path(String(":/supports/").Append(name).Append(".svg")), 0, height, TextureHolder::Properties::NoCache);
    mSupportSideToTextures[side] = texture;
    pSide = mSupportSideToTextures.try_get(side);
  }
  return *pSide;
}

Texture PictogramCaches::GetSupportNumber(int number, int height)
{
  Texture* pNumber = mSupportNumberToTextures.try_get(number);
  if (pNumber == nullptr)
  {
    Texture texture = TextureManager::Instance().Create(Path(String(":/supports/").Append(number).Append(".svg")), 0, height, TextureHolder::Properties::NoCache);
    mSupportNumberToTextures[number] = texture;
    pNumber = mSupportNumberToTextures.try_get(number);
  }
  return *pNumber;
}

Texture PictogramCaches::GetSupportType(SupportTypes type, int height)
{
  Texture* pSide = mSupportTypeToTextures.try_get(type);
  if (pSide == nullptr)
  {
    String name;
    switch(type)
    {
      case SupportTypes::Unknown: break;
      case SupportTypes::Cartridge: name = "cart"; break;
      case SupportTypes::CD: name = "cd"; break;
      case SupportTypes::HDD: name = "hdd"; break;
      case SupportTypes::FilesAndFolders: name = "file"; break;
      case SupportTypes::Tape: name = "k7"; break;
      case SupportTypes::QuickDisk: name = "qd"; break;
      case SupportTypes::Floppy3: name = "disk3"; break;
      case SupportTypes::Floppy35: name = "disk35"; break;
      case SupportTypes::Floppy525: name = "disk525"; break;
      case SupportTypes::PCB: name = "pcb"; break;
    }
    Texture texture = TextureManager::Instance().Create(Path(String(":/supports/types/").Append(name).Append(".svg")), 0, height, TextureHolder::Properties::NoCache);
    mSupportTypeToTextures[type] = texture;
    pSide = mSupportTypeToTextures.try_get(type);
  }
  return *pSide;
}
