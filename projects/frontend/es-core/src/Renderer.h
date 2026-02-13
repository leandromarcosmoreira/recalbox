#pragma once

#include "platform_gl.h"
#include <stack>
#include <SDL.h>
#include <utils/math/Vector2f.h>
#include <utils/math/Vector4i.h>
#include <utils/math/Vector2i.h>
#include <utils/math/Transform4x4f.h>
#include <utils/cplusplus/StaticLifeCycleControler.h>
#include "rendering/opengl/Rectangle.h"
#include "rendering/opengl/Colors.h"
#include "rendering/RenderingError.h"
#include "rendering/opengl/Vertex.h"
#include <utils/storage/Stack.h>
#include <hardware/RotationType.h>
#include <stdint.h>
#include <mutex>

// Forward declaration
class Component;
class Font;

class Renderer : public StaticLifeCycleControler<Renderer>
{
  private:
    //! GL Context thread handle
    static pthread_t sGLContextThread;

    //! New clipping rectangle stack
    Stack<Rectangle> mClippings;
    //! Current clipping rectangle
    Rectangle mCurrentClipping;

    //! SDL Surface
    SDL_Window*   mSdlWindow;
    //! SDL GL context
    SDL_GLContext mSdlGLContext;

    //! Viewport size as ints
    Vector2i   mViewportSize;
    //! Virtual viewport size as floats
    Vector2f   mVirtualViewportSizeFloat;
    //! Virtual viewport size as ints
    Vector2i   mVirtualViewportSize;

    //! Horizontal and vertical scaling
    Vector2f   mScale;
    //! Viewport rotation
    RotationType mRotate;

    //! True if both surface and context have been initialized
    bool mViewPortInitialized;
    //! Initial cursor state
    bool mInitialCursorState;
    //! Windowed mode
    bool mWindowed;

    //! Last X translation
    static float mLastXTranslation;
    //! Last Y translation
    static float mLastYTranslation;

    /*!
     * @brief Create SDL display surface
     * @return True if the surface has been created successfuly
     */
    bool CreateSdlSurface(int width, int height);

    /*!
     * @brief Destroy SDL displaysurface
     */
    void DestroySdlSurface();

    /*!
     * @brief Activate GL debug callback, only in dev/DEBUG mode
     */
    static void ActivateGLDebug();

    /*!
     * @brief Get resolution from configuration file
     * @param w Width to fill
     * @param h Height to fill
     */
    static void GetResolutionFromConfiguration(int& w, int& h);

    /*!
     * @brief Show useful information in logs to help debug remote issues
     */
    static void InformationLogs();

    /*!
     * @brief Rotating the GL viewport of required
     */
    void RotateViewport();

  public:
    static bool IsContextThread() { return sGLContextThread == pthread_self(); }
    bool HasGLContext() { return mSdlGLContext != nullptr; }

    static void ColorToByteArray(GLubyte* array, Colors::ColorARGB color)
    {
      #ifdef LITTLE_ENDIAN
      *(unsigned int*)array = __builtin_bswap32(color);
      #else
      *(unsigned int*)array = color;
      #endif
    }

    static GLuint ColorToGL(Colors::ColorARGB color)
    {
      #ifdef LITTLE_ENDIAN
      return __builtin_bswap32(color);
      #else
      return color;
      #endif
    }

    /*!
     * @brief Constructor
     */
    Renderer(int width, int height, bool windowed, RotationType rotation = RotationType::None);

    /*!
     * @brief Destructor
     */
    ~Renderer();

    /*!
     * @brief Initialize GL viewport
     * @param w Required Width (0 = display width)
     * @param h Required Height (0 = display height)
     * @return true if everything is working fine, false otherwise
     */
    bool Initialize(int w, int h);

    /*!
     * @brief Reinitialize video using previous parameters
     * @return true if everything is working fine, false otherwise
     */
    bool ReInitialize();

    /*!
     * @brief Reinitialize video using previous parameters
     * @return true if everything is working fine, false otherwise
     */
    bool Rotate(RotationType rotation);

    /*!
     * Finalize GL viewport
     */
    void Finalize();

    /*!
     * @brief Applmy the given matrix to openGL context
     * @param transform Matrix
     */
    static void SetMatrix(const Transform4x4f& transform);

    /*!
     * @brief Swap working and dipslayed buffers in double buffering context
     */
    void SwapBuffers();

    /*
     * Clipping
     */

    /*!
     * @brief Initialize clipping stack & stored translations
     */
    void InitializeClipping();

    /*!
     * @brief Push a new clipping rectangle relative to the whole screen
     * @param area Rectangle in abslolute coordinates
     */
    void ClipAbsolute(const Rectangle& area);

    /*!
     * @brief Push a new clipping rectangle relative to the last matrix
     * @param pos Top/Left coordinates
     * @param dim Width/Height
     */
    void Clip(const Rectangle& area);

    /*!
     * @brief Pus a new clip rectangle relative to tranformation and of the given size
     * @param trans Transformation
     * @param size Cliping size
     */
    void Clip(const Transform4x4f& trans, const Vector2f& size);

    /*!
     * @brief Pop latest clipping rectangle
     */
    void Unclip();

    /*!
     * @brief Get current clipping
     * @return Current
     */
    [[nodiscard]] Rectangle CurrentClipping() const
    {
      switch(mRotate)
      {
        case RotationType::Left: return Rectangle(mCurrentClipping.Top() + mCurrentClipping.Height(), mCurrentClipping.Left(), mCurrentClipping.Height(), mCurrentClipping.Width()).Scale(1.f / mScale.x(), 1.f / mScale.y()).Move(-mLastXTranslation, -mLastYTranslation);
        case RotationType::Right: return Rectangle(mCurrentClipping.Top(), (float)mViewportSize.x() - mCurrentClipping.Right(), mCurrentClipping.Height(), mCurrentClipping.Width()).Scale(1.f / mScale.x(), 1.f / mScale.y()).Move(-mLastXTranslation, -mLastYTranslation);
        case RotationType::None:
        case RotationType::Upsidedown:
        default: break;
      }
      return Rectangle(mCurrentClipping).Scale(1.f / mScale.x(), 1.f / mScale.y()).Move(-mLastXTranslation, -mLastYTranslation);
    }

    /*
     * Drawing
     */

    /*!
     * @brief Draw rectangle
     * @param x Left coordinate
     * @param y Top coordinate
     * @param w Width
     * @param h Height
     * @param color Color
     * @param blend_sfactor Source blending
     * @param blend_dfactor Destination blending
     */
    static void DrawRectangle(const Rectangle& area, Colors::ColorARGB color, GLenum blend_sfactor = GL_SRC_ALPHA, GLenum blend_dfactor = GL_ONE_MINUS_SRC_ALPHA);

    /*!
     * @brief Draw rectangle
     * @param x Left coordinate
     * @param y Top coordinate
     * @param w Width
     * @param h Height
     * @param color Color
     * @param blend_sfactor Source blending
     * @param blend_dfactor Destination blending
     */
    static void DrawRectangle(int x, int y, int w, int h, Colors::ColorARGB color, GLenum blend_sfactor = GL_SRC_ALPHA, GLenum blend_dfactor = GL_ONE_MINUS_SRC_ALPHA);

    /*!
     * @brief Draw rectangle
     * @param x Left coordinate
     * @param y Top coordinate
     * @param w Width
     * @param h Height
     * @param color Color
     * @param blend_sfactor Source blending
     * @param blend_dfactor Destination blending
     */
    static void DrawRectangle(float x, float y, float w, float h, Colors::ColorARGB color, GLenum blend_sfactor = GL_SRC_ALPHA, GLenum blend_dfactor = GL_ONE_MINUS_SRC_ALPHA);

    /*!
     * @brief Draw rectangle
     * @param x Left coordinate
     * @param y Top coordinate
     * @param w Width
     * @param h Height
     * @param topleftcolor Top Left Color
     * @param toprightcolor Top Right Color
     * @param bottomleftcolor Bottom Left Color
     * @param bottomrightcolor Bottom Right Color
     */
    static void DrawRectangle(int x, int y, int w, int h,
                              Colors::ColorARGB topleftcolor,
                              Colors::ColorARGB toprightcolor,
                              Colors::ColorARGB bottomrightcolor,
                              Colors::ColorARGB bottomleftcolor);

    /*!
     * @brief Draw rectangle
     * @param x Left coordinate
     * @param y Top coordinate
     * @param w Width
     * @param h Height
     * @param topleftcolor Top Left Color
     * @param toprightcolor Top Right Color
     * @param bottomleftcolor Bottom Left Color
     * @param bottomrightcolor Bottom Right Color
     */
    static void DrawRectangle(float x, float y, float w, float h,
                            Colors::ColorARGB topleftcolor,
                            Colors::ColorARGB toprightcolor,
                            Colors::ColorARGB bottomrightcolor,
                            Colors::ColorARGB bottomleftcolor) { DrawRectangle((int)x, (int)y, (int)w, (int)h, topleftcolor, toprightcolor, bottomrightcolor, bottomleftcolor); }

    /*!
     * @brief Draw polylines
     * @param vertices Vertice list
     * @param colors Color array
     * @param count Vertex count
     */
    static void DrawLines(const Vector2f vertices[], const Colors::ColorARGB colors[], int count);

    /*!
     * @brief Draw textured triangles
     * @param id GL texture id
     * @param vertices Vertice list
     * @param colors Color list
     * @param count Vertice count
     * @param tiled draw tiled texture
     */
    static void DrawTexturedTriangles(GLuint id, const Vertex vertices[], const GLubyte colors[], int count, bool tiled);

    /*!
     * @brief Draw textured triangles using a single color
     * @param id GL texture id
     * @param vertices Vertice list
     * @param color Color
     * @param count Vertice count
     * @param tiled draw tiled texture
     */
    static void DrawTexturedTriangles(GLuint id, const Vertex vertices[], Colors::ColorARGB color, int count, bool tiled);

    /*!
     * @brief Upload Alpha texture data to GPU memory
     * @param id GL Texture id
     * @param width Data width
     * @param height Data height
     * @param data Alpha bytes (1 byte = 1 pixel)
     * @return NoError or OutOfGPUMemory
     */
    static RenderingError UploadAlpha(GLuint id, int width, int height, const void* data);

    /*!
     * @brief Upload RGBA texture data to GPU memory
     * @param id GL Texture id
     * @param width Data width
     * @param height Data height
     * @param data RGBA bytes (4 byte = 1 pixel)
     * @return NoError or OutOfGPUMemory
     */
    static RenderingError UploadRGBA(GLuint id, int width, int height, const void* data);

    /*!
     * @brief Upload Alpha texture data to GPU memory
     * @param id GL Texture id
     * @param width Data width
     * @param height Data height
     * @param data Alpha bytes (1 byte = 1 pixel)
     * @return NoError or OutOfGPUMemory
     */
    static RenderingError UploadAlphaPart(GLuint id, int x, int y, int width, int height, const void* data);

    /*!
     * @brief Upload RGBA texture data to GPU memory
     * @param id GL Texture id
     * @param width Data width
     * @param height Data height
     * @param data RGBA bytes (4 byte = 1 pixel)
     * @return NoError or OutOfGPUMemory
     */
    static RenderingError UploadRGBAPart(GLuint id, int x, int y, int width, int height, const void* data);

    /*!
     * @brief Build a GL color array
     * @param array Array pointer
     * @param color Color
     * @param count Count x Color => array
     */
    static void BuildGLColorArray(GLubyte* array, Colors::ColorARGB color, int Count);

    /*!
     * @brief Create a new texture and return its GL Identifier
     * @return GL Texture identifier
     */
    static GLuint CreateGLTexture(bool nearest = false);

    /*!
     * @brief Destroy the texture associated to the given id
     * @param id GL Texture identifier
     */
    static void DestroyGLTexture(GLuint id);

    /*
     * High level GL
     */

    /*!
     * @brief Draw a texture using full coordinates, width/height, keeping ratio yes/no
     * @param texture Texture to draw
     * @param x X coordinate
     * @param y Y coordinate
     * @param w Width
     * @param h Height
     * @param keepratio True to keep ratio, false to stretch
     */
//    static void DrawTexture(TextureResource& texture, int x, int y, int w, int h, bool keepratio);

    /*!
     * @brief Draw a texture using full coordinates & texture width/height
     * @param texture Texture to draw
     * @param x X coordinate
     * @param y Y coordinate
     */
//    static void DrawTexture(TextureResource& texture, int x, int y) { DrawTexture(texture, x, y, (int)texture.width(), (int)texture.height(), false); }

    /*!
     * @brief Draw a texture using full coordinates & width/height
     * @param texture Texture to draw
     * @param x X coordinate
     * @param y Y coordinate
     */
//    static void DrawTexture(TextureResource& texture, int x, int y, int w, int h) { DrawTexture(texture, x, y, w, h, false); }

    /*!
     * @brief Draw a texture using full coordinates, width/height, keeping ratio yes/no and a given blending alpha
     * @param texture Texture to draw
     * @param x X coordinate
     * @param y Y coordinate
     * @param w Width
     * @param h Height
     * @param keepratio True to keep ratio, false to stretch
     * @param color Blending alpha
     */
//    static void DrawTexture(TextureResource& texture, int x, int y, int w, int h, bool keepratio, unsigned char alpha);

    /*!
     * @brief Draw a texture using full coordinates, texture width/height and a given blending alpha
     * @param texture Texture to draw
     * @param x X coordinate
     * @param y Y coordinate
     * @param color Blending alpha
     */
//    static void DrawTexture(TextureResource& texture, int x, int y, unsigned char alpha) { DrawTexture(texture, x, y, (int)texture.width(), (int)texture.height(), false, alpha); }

    /*!
     * @brief Draw a texture using full coordinates, width/height and a given blending alpha
     * @param texture Texture to draw
     * @param x X coordinate
     * @param y Y coordinate
     * @param w Width
     * @param h Height
     * @param color Blending alpha
     */
//    static void DrawTexture(TextureResource& texture, int x, int y, int w, int h, unsigned char alpha) { DrawTexture(texture, x, y, w, h, false, alpha); }

    /*!
     * @brief Draw a texture using full coordinates, width/height, keeping ratio yes/no and a given blending color
     * @param texture Texture to draw
     * @param x X coordinate
     * @param y Y coordinate
     * @param w Width
     * @param h Height
     * @param keepratio True to keep ratio, false to stretch
     * @param color Blending color
     */
//    static void DrawTexture(TextureResource& texture, int x, int y, int w, int h, bool keepratio, Colors::ColorARGB color);

    /*!
     * @brief Draw a texture using full coordinates, width/height, keeping ratio yes/no and a given blending color
     * @param texture Texture to draw
     * @param x X coordinate
     * @param y Y coordinate
     * @param w Width
     * @param h Height
     * @param keepratio True to keep ratio, false to stretch
     * @param color Blending color
     */
//    static void DrawTexture(TextureResource& texture, int x, int y, int w, int h, bool flipX, bool flipY, bool keepratio, Colors::ColorARGB color);

    /*!
     * @brief Draw a texture using full coordinates, texture width/height and a given blending color
     * @param texture Texture to draw
     * @param x X coordinate
     * @param y Y coordinate
     * @param color Blending color
     */
//    static void DrawTexture(TextureResource& texture, int x, int y, Colors::ColorARGB color) { DrawTexture(texture, x, y, (int)texture.width(), (int)texture.height(), false, color); }

    /*!
     * @brief Draw a texture using full coordinates, width/height and a given blending color
     * @param texture Texture to draw
     * @param x X coordinate
     * @param y Y coordinate
     * @param w Width
     * @param h Height
     * @param color Blending color
     */
//    static void DrawTexture(TextureResource& texture, int x, int y, int w, int h, Colors::ColorARGB color)  { DrawTexture(texture, x, y, w, h, false, color); }

    /*!
     * @brief Draw a texture using full coordinates, width/height, ratio & and blending colors
     * @param texture Texture to draw
     * @param x X coordinate
     * @param y Y coordinate
     * @param w Width
     * @param h Height
     * @param keepratio True to keep ratio
     * @param topleftcolor Top left color blending
     * @param toprightcolor  Top right color blending
     * @param bottomrightcolor Bottom Right color blending
     * @param bottomleftcolor Bottom Left color blending
     */
/*    static void DrawTexture(TextureResource& texture, int x, int y, int w, int h, bool keepratio,
                            Colors::ColorARGB topleftcolor,
                            Colors::ColorARGB toprightcolor,
                            Colors::ColorARGB bottomrightcolor,
                            Colors::ColorARGB bottomleftcolor);
*/
    /*!
     * @brief Draw a texture using full coordinates, width/height and blending colors
     * @param texture Texture to draw
     * @param x X coordinate
     * @param y Y coordinate
     * @param w Width
     * @param h Height
     * @param topleftcolor Top left color blending
     * @param toprightcolor  Top right color blending
     * @param bottomrightcolor Bottom Right color blending
     * @param bottomleftcolor Bottom Left color blending
     */
/*    static void DrawTexture(TextureResource& texture, int x, int y, int w, int h,
                            Colors::ColorARGB topleftcolor,
                            Colors::ColorARGB toprightcolor,
                            Colors::ColorARGB bottomrightcolor,
                            Colors::ColorARGB bottomleftcolor) { DrawTexture(texture, x, y, w, h, false, topleftcolor, toprightcolor, bottomrightcolor, bottomleftcolor); }
*/
    /*!
     * @brief Draw a texture using full coordinates, width/height, ratio,  blending colors & lower reflection
     * @param texture Texture to draw
     * @param x X coordinate
     * @param y Y coordinate
     * @param w Width
     * @param h Height
     * @param keepratio True to keep ratio
     * @param flipOnX True to flip texture around vertical axis
     * @param flipOnY True to flip texture around horizontal axis
     * @param topleftcolor Top left color blending
     * @param toprightcolor  Top right color blending
     * @param bottomrightcolor Bottom Right color blending
     * @param bottomleftcolor Bottom Left color blending
     * @param topAlphaReflection Top alpha reflection
     * @param bottomAlphaReflection Bottom alpha reflection
     */
/*    static void DrawTexture(TextureResource& texture, int x, int y, int w, int h, bool keepratio,
                            bool flipOnX, bool flipOnY,
                            Colors::ColorARGB topleftcolor,
                            Colors::ColorARGB toprightcolor,
                            Colors::ColorARGB bottomrightcolor,
                            Colors::ColorARGB bottomleftcolor,
                            float topAlphaReflection,
                            float bottomAlphaReflection);
*/
    /*!
     * @brief Draw a texture using full coordinates, width/height,  blending colors & lower reflection
     * @param texture Texture to draw
     * @param x X coordinate
     * @param y Y coordinate
     * @param w Width
     * @param h Height
     * @param flipOnX True to flip texture around vertical axis
     * @param flipOnY True to flip texture around horizontal axis
     * @param topleftcolor Top left color blending
     * @param toprightcolor  Top right color blending
     * @param bottomrightcolor Bottom Right color blending
     * @param bottomleftcolor Bottom Left color blending
     * @param topAlphaReflection Top alpha reflection
     * @param bottomAlphaReflection Bottom alpha reflection
     */
/*    static void DrawTexture(TextureResource& texture, int x, int y, int w, int h,
                            bool flipOnX, bool flipOnY,
                            Colors::ColorARGB topleftcolor,
                            Colors::ColorARGB toprightcolor,
                            Colors::ColorARGB bottomrightcolor,
                            Colors::ColorARGB bottomleftcolor,
                            float topAlphaReflection,
                            float bottomAlphaReflection) { DrawTexture(texture, x, y, w, h, false, flipOnX, flipOnY, topleftcolor, toprightcolor, bottomrightcolor, bottomleftcolor, topAlphaReflection, bottomAlphaReflection); }
*/
    /*
     * High level GL with low-level parameters (for use in new textures)
     */

    /*!
     * @brief Draw a texture using full coordinates, width/height, keeping ratio yes/no
     * @param id GL Texture ID
     * @param textureW Texture width
     * @param textureH Texture width
     * @param x X coordinate
     * @param y Y coordinate
     * @param w Width
     * @param h Height
     * @param keepratio True to keep ratio, false to stretch
     */
    static void DrawTexture(GLuint id, int textureW, int textureH, int x, int y, int w, int h, bool keepratio, bool tiled);

    /*!
     * @brief Draw a texture using full coordinates & texture width/height
     * @param id GL Texture ID
     * @param textureW Texture width
     * @param textureH Texture width
     * @param x X coordinate
     * @param y Y coordinate
     */
    static void DrawTexture(GLuint id, int textureW, int textureH, int x, int y, bool tiled) { DrawTexture(id, textureW, textureH, x, y, textureW, textureH, false, tiled); }

    /*!
     * @brief Draw a texture using full coordinates & width/height
     * @param id GL Texture ID
     * @param textureW Texture width
     * @param textureH Texture width
     * @param x X coordinate
     * @param y Y coordinate
     */
    static void DrawTexture(GLuint id, int textureW, int textureH, int x, int y, int w, int h, bool tiled) { DrawTexture(id, textureW, textureH, x, y, w, h, false, tiled); }

    /*!
     * @brief Draw a texture using full coordinates, width/height, keeping ratio yes/no and a given blending alpha
     * @param id GL Texture ID
     * @param textureW Texture width
     * @param textureH Texture width
     * @param x X coordinate
     * @param y Y coordinate
     * @param w Width
     * @param h Height
     * @param keepratio True to keep ratio, false to stretch
     * @param color Blending alpha
     */
    static void DrawTexture(GLuint id, int textureW, int textureH, int x, int y, int w, int h, bool keepratio, bool tiled, unsigned char alpha);

    /*!
     * @brief Draw a texture using full coordinates, texture width/height and a given blending alpha
     * @param id GL Texture ID
     * @param textureW Texture width
     * @param textureH Texture width
     * @param x X coordinate
     * @param y Y coordinate
     * @param color Blending alpha
     */
    static void DrawTexture(GLuint id, int textureW, int textureH, int x, int y, bool tiled, unsigned char alpha) { DrawTexture(id, textureW, textureH, x, y, textureW, textureH, false, tiled, alpha); }

    /*!
     * @brief Draw a texture using full coordinates, width/height and a given blending alpha
     * @param id GL Texture ID
     * @param textureW Texture width
     * @param textureH Texture width
     * @param x X coordinate
     * @param y Y coordinate
     * @param w Width
     * @param h Height
     * @param color Blending alpha
     */
    static void DrawTexture(GLuint id, int textureW, int textureH, int x, int y, int w, int h, bool tiled, unsigned char alpha) { DrawTexture(id, textureW, textureH, x, y, w, h, false, tiled, alpha); }

    /*!
     * @brief Draw a texture using full coordinates, width/height, keeping ratio yes/no and a given blending color
     * @param id GL Texture ID
     * @param textureW Texture width
     * @param textureH Texture width
     * @param x X coordinate
     * @param y Y coordinate
     * @param w Width
     * @param h Height
     * @param keepratio True to keep ratio, false to stretch
     * @param color Blending color
     */
    static void DrawTexture(GLuint id, int textureW, int textureH, int x, int y, int w, int h, bool keepratio, bool tiled, Colors::ColorARGB color);

    /*!
     * @brief Draw a texture using full coordinates, width/height, keeping ratio yes/no and a given blending color
     * @param id GL Texture ID
     * @param textureW Texture width
     * @param textureH Texture width
     * @param x X coordinate
     * @param y Y coordinate
     * @param w Width
     * @param h Height
     * @param keepratio True to keep ratio, false to stretch
     * @param color Blending color
     */
    static void DrawTexture(GLuint id, int textureW, int textureH, int x, int y, int w, int h, bool flipX, bool flipY, bool keepratio, bool tiled, Colors::ColorARGB color);

    /*!
     * @brief Draw a texture using full coordinates, texture width/height and a given blending color
     * @param id GL Texture ID
     * @param textureW Texture width
     * @param textureH Texture width
     * @param x X coordinate
     * @param y Y coordinate
     * @param color Blending color
     */
    static void DrawTexture(GLuint id, int textureW, int textureH, int x, int y, bool tiled, Colors::ColorARGB color) { DrawTexture(id, textureW, textureH, x, y, textureW, textureH, false, tiled, color); }

    /*!
     * @brief Draw a texture using full coordinates, width/height and a given blending color
     * @param id GL Texture ID
     * @param textureW Texture width
     * @param textureH Texture width
     * @param x X coordinate
     * @param y Y coordinate
     * @param w Width
     * @param h Height
     * @param color Blending color
     */
    static void DrawTexture(GLuint id, int textureW, int textureH, int x, int y, int w, int h, bool tiled, Colors::ColorARGB color)  { DrawTexture(id, textureW, textureH, x, y, w, h, false, tiled, color); }

    /*!
     * @brief Draw a texture using full coordinates, width/height, ratio & and blending colors
     * @param id GL Texture ID
     * @param textureW Texture width
     * @param textureH Texture width
     * @param x X coordinate
     * @param y Y coordinate
     * @param w Width
     * @param h Height
     * @param keepratio True to keep ratio
     * @param topleftcolor Top left color blending
     * @param toprightcolor  Top right color blending
     * @param bottomrightcolor Bottom Right color blending
     * @param bottomleftcolor Bottom Left color blending
     */
    static void DrawTexture(GLuint id, int textureW, int textureH, int x, int y, int w, int h, bool keepratio, bool tiled,
                            Colors::ColorARGB topleftcolor,
                            Colors::ColorARGB toprightcolor,
                            Colors::ColorARGB bottomrightcolor,
                            Colors::ColorARGB bottomleftcolor);

    /*!
     * @brief Draw a texture using full coordinates, width/height and blending colors
     * @param id GL Texture ID
     * @param textureW Texture width
     * @param textureH Texture width
     * @param x X coordinate
     * @param y Y coordinate
     * @param w Width
     * @param h Height
     * @param topleftcolor Top left color blending
     * @param toprightcolor  Top right color blending
     * @param bottomrightcolor Bottom Right color blending
     * @param bottomleftcolor Bottom Left color blending
     */
    static void DrawTexture(GLuint id, int textureW, int textureH, int x, int y, int w, int h, bool tiled,
                            Colors::ColorARGB topleftcolor,
                            Colors::ColorARGB toprightcolor,
                            Colors::ColorARGB bottomrightcolor,
                            Colors::ColorARGB bottomleftcolor) { DrawTexture(id, textureW, textureH, x, y, w, h, false, tiled, topleftcolor, toprightcolor, bottomrightcolor, bottomleftcolor); }

    /*!
     * @brief Draw a texture using full coordinates, width/height, ratio,  blending colors & lower reflection
     * @param id GL Texture ID
     * @param textureW Texture width
     * @param textureH Texture width
     * @param x X coordinate
     * @param y Y coordinate
     * @param w Width
     * @param h Height
     * @param keepratio True to keep ratio
     * @param flipOnX True to flip texture around vertical axis
     * @param flipOnY True to flip texture around horizontal axis
     * @param topleftcolor Top left color blending
     * @param toprightcolor  Top right color blending
     * @param bottomrightcolor Bottom Right color blending
     * @param bottomleftcolor Bottom Left color blending
     * @param topAlphaReflection Top alpha reflection
     * @param bottomAlphaReflection Bottom alpha reflection
     */
    static void DrawTexture(GLuint id, int textureW, int textureH, int x, int y, int w, int h, bool keepratio, bool tiled,
                            bool flipOnX, bool flipOnY,
                            Colors::ColorARGB topleftcolor,
                            Colors::ColorARGB toprightcolor,
                            Colors::ColorARGB bottomrightcolor,
                            Colors::ColorARGB bottomleftcolor,
                            float topAlphaReflection,
                            float bottomAlphaReflection);

    /*!
     * @brief Draw a texture using full coordinates, width/height,  blending colors & lower reflection
     * @param id GL Texture ID
     * @param textureW Texture width
     * @param textureH Texture width
     * @param x X coordinate
     * @param y Y coordinate
     * @param w Width
     * @param h Height
     * @param flipOnX True to flip texture around vertical axis
     * @param flipOnY True to flip texture around horizontal axis
     * @param topleftcolor Top left color blending
     * @param toprightcolor  Top right color blending
     * @param bottomrightcolor Bottom Right color blending
     * @param bottomleftcolor Bottom Left color blending
     * @param topAlphaReflection Top alpha reflection
     * @param bottomAlphaReflection Bottom alpha reflection
     */
    static void DrawTexture(GLuint id, int textureW, int textureH, int x, int y, int w, int h, bool tiled,
                            bool flipOnX, bool flipOnY,
                            Colors::ColorARGB topleftcolor,
                            Colors::ColorARGB toprightcolor,
                            Colors::ColorARGB bottomrightcolor,
                            Colors::ColorARGB bottomleftcolor,
                            float topAlphaReflection,
                            float bottomAlphaReflection) { DrawTexture(id, textureW, textureH, x, y, w, h, false, tiled, flipOnX, flipOnY, topleftcolor, toprightcolor, bottomrightcolor, bottomleftcolor, topAlphaReflection, bottomAlphaReflection); }

    /*
     * Accessors
     */

    //! Get display Width as integer
    [[nodiscard]] int DisplayWidthAsInt() const { return mVirtualViewportSize.x(); }
    //! Get display Height as integer
    [[nodiscard]] int DisplayHeightAsInt() const { return mVirtualViewportSize.y(); }
    //! Get display Width as float
    [[nodiscard]] float DisplayWidthAsFloat() const { return mVirtualViewportSizeFloat.x(); }
    //! Get display Height as float
    [[nodiscard]] float DisplayHeightAsFloat() const { return mVirtualViewportSizeFloat.y(); }

    //! Is small resolution?
    [[nodiscard]] bool Is240p() const { return IsRotatedSide() ? mVirtualViewportSize.y() <= 480 || mViewportSize.x() <= 320 : mVirtualViewportSize.x() <= 480 || mViewportSize.y() <= 320; }
    //! Is middle resolution?
    [[nodiscard]] bool Is480pOrLower() const { return mViewportSize.y() <= 576; }
    //! Return true window width (not scaled, not rotated)
    [[nodiscard]] int RealDisplayWidthAsInt() const { return mViewportSize.x(); }
    //! Return true window height
    [[nodiscard]] int RealDisplayHeightAsInt() const { return mViewportSize.y(); }
    //! Return true if the screen is rotated either left or right
    [[nodiscard]] bool IsRotatedSide() const { return mRotate == RotationType::Left || mRotate == RotationType::Right; }
    //! Return the screen rotation
    [[nodiscard]] RotationType Rotation() const { return mRotate; }
    //! Is resolution QVGA or less?
    [[nodiscard]] bool IsQVGA() const { return RealDisplayHeightAsInt() <= 288; }
    //! Is Resolution between QVGA (excluded) & VGA (included)?
    [[nodiscard]] bool IsVGA() const { int h = RealDisplayHeightAsInt(); return h > 288 && h <= 576; }
    //! Is Resolution between VGA (excluded) & HD (included)?
    [[nodiscard]] bool IsHD() const { int h = RealDisplayHeightAsInt(); return h > 576 && h <= 920; }
    //! Is Resolution higher than HD?
    [[nodiscard]] bool IsFHD() const { return RealDisplayHeightAsInt() > 920; }


    //! Check if the Renderer is properly initialized
    [[nodiscard]] bool Initialized() const { return mViewPortInitialized; }
};
