//
// Created by bkg2k on 03/09/25.
//
#pragma once

class ITextureCacheControl
{
  public:
    //! Default destructor
    virtual ~ITextureCacheControl() = default;

    /*!
     * @brief Unload the oldest texture to free memory
     * @param cpu false = unload GPU, true = unload CPU caches
     * @return True if a texture has been unloaded, false otherwise (no more unloadable texture)
     */
    [[nodiscard]] virtual bool UnloadTheOldest(bool cpu) = 0;
};