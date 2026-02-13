//
// Created by Bkg2k on 10/01/2020.
//
#pragma once

#include "utils/os/system/IThreadPoolWorkerInterface.h"
#include "utils/os/system/ThreadPool.h"
#include "utils/storage/HashMap.h"
#include "TextureHolder.h"
#include "Texture.h"
#include "utils/cplusplus/StaticLifeCycleControler.h"
#include "utils/storage/Set.h"
#include "ITextureCacheControl.h"

class TextureManager : public StaticLifeCycleControler<TextureManager>
                     , public IThreadPoolWorkerInterface<Texture, RenderingError>
                     , public ITextureCacheControl
                     , public IHttpConfiguration
{
  private:
    //! Thread pool
    ThreadPool<Texture, RenderingError> mPool;

    //! ALL Textures
    HashSet<TextureHolder*> mTextures;

    //! Path to Texture
    HashMap<std::string, Texture> mTexturesCache;

    //! Storage protection
    Mutex mMutexStorage;

    //! Http filtering/configuration
    HashSet<IHttpConfiguration*> mHttpConfigurationFilter;

    /*!
     * @brief Unload the oldest texture to free memory
     * @param cpu false = unload GPU, true = unload CPU caches
     * @return True if a texture has been unloaded, false otherwise (no more unloadable texture)
     */
    [[nodiscard]] bool UnloadTheOldest(bool cpu) final;

    /*
     * IThreadPoolWorkerInterface implementation
     */

    /*!
     * @brief The main runner. Implement here the task to process a feed object
     * @param feed Feed object to process
     * @return Result Object
     */
    RenderingError ThreadPoolRunJob(Texture& feed) final;

    /*!
     * @brief Select next texture to load
     * @param list List into which to select the next item
     * @return Next item index
     */
    int ThreadPoolNextFeedIndex(const std::vector<IndexedFeed<Texture>>& list) final;

    /*!
     * @brief Called asap by the main thread when a job complete, regarding the tick duration
     * @param completed Currently completed jobs count
     * @param total Total jobs
     */
    void ThreadPoolTick(int completed, int total) override { (void)completed; (void)total; };

    /*
     * Children management
     */

    /*!
     * @brief Called by TextureHolder constructor
     * @param newTexture texture being created
     */
    void TextureHolderCreated(TextureHolder* newTexture);

    /*!
     * @brief Called from TextureHolder destructor
     * @param textureDestroyed texture being destroyed
     */
    void TextureHolderDestroyed(TextureHolder* textureDestroyed);

    /*
     * IHttpProvider
     */

    /*!
     * @brief Configure the given http client
     */
    void ConfigureHttpClient(const String& url, HttpClient& client) final;

    friend class TextureHolder;

  public:
    /*!
     * @brief Default constructor - only ONE instance allowed
     */
    TextureManager();

    /*!
     * @brief Desfault destructor
     */
    ~TextureManager() override;

    /*!
     * @brief Create a new texture
     * @param path File to load
     * @param targetWidth target width (for sizables images only)
     * @param targetHeight target height (for sizables images only)
     * @param properties Texture properties
     * @return Texture
     */
    Texture Create(const Path& path, int targetWidth, int targetHeight, TextureHolder::Properties properties = TextureHolder::Properties::None);

    /*!
     * @brief Create a new texture
     * @param path File to load
     * @param properties Texture properties
     * @return Texture
     */
    Texture Create(const Path& path, TextureHolder::Properties properties = TextureHolder::Properties::None)
    {
      return Create(path, 0, 0, properties);
    }

    /*!
     * @brief Request texture loading, immediate or delayed
     * @param texture Texture to load
     * @param noDelay Load immediately, even if the texture has no immediateLoad property
     */
    RenderingError RequestTextureLoading(Texture& texture, bool noDelay = false);

    /*!:
     * @brief Check if a texture resource exists, either embedded or real file
     * @param path path to resource
     * @return True if the resource is available
     */
    static bool Exists(const Path& path);

    /*
     * Memory global management
     */

    /*!
     * @brief Free all possible CPU caches
     */
    void FreeAllCPUResources();

    /*!
     * @brief Free all possible GL texture
     */
    void FreeAllGPUResources();

    /*!
     * @brief Free both CPU caches and GL textures
     */
    void FreeAllResources();

    /*
     * Statistics
     */

    /*!
     * @brief Return total CPU consumed by texture data
     * @return Total CPU memory in byte
     */
    long long TotalCPUMemory();

    /*!
     * @brief Return total GPU consumed by texture data
     * @return Total GPU memory in byte
     */
    long long TotalGPUMemory();

    /*!
     * @brief Watch CPU memory to prevent free memory to go below 50% or 1Go
     */
    void WatchCPUMemory();

    /*!
     * @brief For debug purpose only
     */
    void ShowStatistics();

    /*!
     * @brief Register a configurator for the given url part
     * @param httpConfigure Http configuration interface
     */
    void RegisterHttpFilter(IHttpConfiguration* httpConfigure) { Mutex::AutoLock locker(mMutexStorage); mHttpConfigurationFilter.insert(httpConfigure); }

    /*!
     * @brief Unregister a previously assigned filter
     * @param httpConfigure Http configuration interface
     */
    void UnregisterHttpFilter(IHttpConfiguration* httpConfigure) { Mutex::AutoLock locker(mMutexStorage); mHttpConfigurationFilter.erase(httpConfigure); }
};

