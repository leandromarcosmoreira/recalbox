//
// Created by bkg2k on 07/03/24.
//
#pragma once

#include <utils/storage/HashMap.h>
#include <games/classifications/Regions.h>
#include <games/classifications/Genres.h>
#include <games/SupportSides.h>
#include <games/SupportTypes.h>
#include <rendering/textures/Texture.h>

class PictogramCaches
{
  public:
    /*!
     * @brief Get texture for the given region
     * @param region region to get flag texture from
     * @param height Target height for rasterization
     * @return Flag texture (or empty texture if the flag resource does not exist)
     */
    Texture GetFlag(Regions::GameRegions region, int height);

    /*!
     * @brief Get texture for the given genre
     * @param genre genre to get texture from
     * @param height Target height for rasterization
     * @return Genre texture (or empty texture if the genre resource does not exist)
     */
    Texture GetGenre(GameGenres genres, int height);

    /*!
     * @brief Get texture for the given players
     * @param playermin player min to get best texture from
     * @param playermax player max to get best texture from
     * @param height Target height for rasterization
     * @return Player texture (or empty texture if the player resource does not exist)
     */
    Texture GetPlayers(int playermin, int playermax, int height);

    /*!
     * @brief Get support left part
     * @param height Target height for rasterization
     * @return Left part texture
     */
    Texture GetSupportLeftPart(int height);

    /*!
     * @brief Get support right part
     * @param height Target height for rasterization
     * @return right part texture
     */
    Texture GetSupportRightPart(int height);

    /*!
     * @brief Get support separator
     * @param height Target height for rasterization
     * @return separator texture
     */
    Texture GetSupportSeparator(int height);

    /*!
     * @brief Get support side (A-B only, passing "none" returns an empty texture)
     * @param height Target height for rasterization
     * @return support side texture
     */
    Texture GetSupportSide(SupportSides side, int height);

    /*!
     * @brief Get support number (1-20, out-of-range values returns empty textures)
     * @param height Target height for rasterization
     * @return support side texture
     */
    Texture GetSupportNumber(int number, int height);

    /*!
     * @brief Get support type icon
     * @param type Support type
     * @param height Target height for rasterization
     * @return support type texture
     */
    Texture GetSupportType(SupportTypes type, int height);

    //! Clean all flags to free memory
    void Clear()
    {
      mRegionToTextures.clear();
      mGenreToTextures.clear();
      mPlayersToTextures.clear();
      mSupportNumberToTextures.clear();
      mSupportSideToTextures.clear();
      mSupportTypeToTextures.clear();
      mSupportLeft.Clear();
      mSupportRight.Clear();
      mSupportSeparator.Clear();
    }

  private:
    //! region to flag texture
    HashMap<Regions::GameRegions, Texture> mRegionToTextures;
    //! GameGenre to genre texture
    HashMap<GameGenres, Texture> mGenreToTextures;
    //! GameGenre to genre texture
    HashMap<int, Texture> mPlayersToTextures;
    //! Support size textures
    HashMap<SupportSides, Texture> mSupportSideToTextures;
    //! Support number textures
    HashMap<int, Texture> mSupportNumberToTextures;
    //! Support left part
    Texture mSupportLeft;
    //! Support right part
    Texture mSupportRight;
    //! Support separator
    Texture mSupportSeparator;
    //! Support size textures
    HashMap<SupportTypes, Texture> mSupportTypeToTextures;
    //! Hashmap locker
    Mutex mLocker;
};
