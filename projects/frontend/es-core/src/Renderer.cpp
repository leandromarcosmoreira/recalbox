#include "Renderer.h"
#include "ImageIO.h"
#include "../data/Resources.h"
#include "ResolutionAdapter.h"
#include "CrtConf.h"
#include <hardware/Board.h>

#ifdef USE_OPENGL_ES
  #define glOrtho glOrthof
#endif

#ifdef USE_OPENGL_ES
typedef GLshort GLType;
#elif defined(USE_OPENGL_DESKTOP)
typedef GLint GLType;
#else
#error "No GL type defined"
#endif

float Renderer::mLastXTranslation = 0.f;
float Renderer::mLastYTranslation = 0.f;
pthread_t Renderer::sGLContextThread = 0;

#ifdef DEBUG
  #define CHECKGLCONTEXT assert(Renderer::sGLContextThread == pthread_self() && "Bad thread accessing GL methods !")
#else
  #define CHECKGLCONTEXT do{ if (Renderer::sGLContextThread != pthread_self()) { LOG(LogError) << "[Renderer] Bad thread accessing GL methods !"; } }while(false)
#endif

#if defined(DEBUG) && !defined(USE_OPENGL_ES)

static void APIENTRY GLDebugCallback(GLenum source,
                                     GLenum type,
                                     GLuint id,
                                     GLenum severity,
                                     GLsizei length,
                                     const GLchar* message,
                                     const void* userParam)
{
  (void)source;
  (void)length;
  (void)userParam;

  const char* typeString = "UNKNOWN";
  switch (type)
  {
    case GL_DEBUG_TYPE_ERROR: typeString = "ERROR"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeString = "DEPRECATED_BEHAVIOR"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: typeString = "UNDEFINED_BEHAVIOR"; break;
    case GL_DEBUG_TYPE_PORTABILITY: typeString = "PORTABILITY"; break;
    case GL_DEBUG_TYPE_PERFORMANCE: typeString = "PERFORMANCE"; break;
    case GL_DEBUG_TYPE_OTHER: typeString = "OTHER"; break;
    default: break;
  }

  const char* severityString = "UNKNOWN";
  switch (severity)
  {
    case GL_DEBUG_SEVERITY_LOW: severityString = "LOW"; break;
    case GL_DEBUG_SEVERITY_MEDIUM: severityString = "MEDIUM"; break;
    case GL_DEBUG_SEVERITY_HIGH: severityString = "HIGH"; break;
    default: break;
  }

  { LOG(LogError) << "[Renderer] GL Error [Type:" << typeString << " - Severity:" << severityString << "] {id:" << id << "} " << message; }
}

void Renderer::ActivateGLDebug()
{
  { LOG(LogInfo) << "[Renderer] GL Debug activated."; }
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(GLDebugCallback, nullptr);
  GLuint unusedIds = 0;
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, 1);
}

#else

void Renderer::ActivateGLDebug()
{
  { LOG(LogInfo) << "[Renderer] GL Debug not activated in GLES."; }
}

#endif

Renderer::Renderer(int width, int height, bool windowed, RotationType rotation)
  : StaticLifeCycleControler<Renderer>("Renderer")
  , mSdlWindow(nullptr)
  , mSdlGLContext(nullptr)
  , mViewportSize(Vector2i(0,0))
  , mVirtualViewportSizeFloat(Vector2f(0,0))
  , mVirtualViewportSize(Vector2i(0,0))
  , mScale(Vector2f(1,1))
  , mRotate(rotation)
  , mViewPortInitialized(false)
  , mInitialCursorState(false)
  , mWindowed(windowed)
{
  #ifdef DEBUG
  ActivateGLDebug();
  #endif

  mViewPortInitialized = Initialize(width, height);
}

Renderer::~Renderer()
{
  Finalize();
}

void Renderer::InformationLogs()
{
  SDL_version linked;
  SDL_GetVersion(&linked);
  LOG(LogInfo) << (_F("[Renderer] SDL2 Version {0}.{1}.{2}") / (int)linked.major / (int)linked.minor/ (int)linked.patch).ToString();

  int numVideoDrivers = SDL_GetNumVideoDrivers();
  String drivers;
  for (int i = 0; i < numVideoDrivers; i++)
  {
    if (!drivers.empty()) drivers.Append(", ");
    drivers.Append(SDL_GetVideoDriver(i));
  }
  LOG(LogInfo) << "[Renderer] Available video drivers: "<< drivers;
  LOG(LogInfo) << (_F("[Renderer] Using video driver: {0}") / SDL_GetCurrentVideoDriver()).ToString();

  int numAudioDrivers = SDL_GetNumAudioDrivers();
  drivers.clear();
  for (int i = 0; i < numAudioDrivers; i++)
  {
    if (!drivers.empty()) drivers.Append(", ");
    drivers.Append(SDL_GetAudioDriver(i));
  }
  LOG(LogInfo) << "[Renderer] Available audio drivers: " << drivers;
  const char* currentAudioDriver = SDL_GetCurrentAudioDriver();
  if (currentAudioDriver != nullptr) LOG(LogInfo) << (_F("[Renderer] Using audio driver: {0}") / currentAudioDriver).ToString();

  LOG(LogInfo) << "[Renderer] GL Vendor: " << (glGetString(GL_VENDOR) != nullptr ? (const char*)glGetString(GL_VENDOR) : "Unknown");
  LOG(LogInfo) << "[Renderer] GL Renderer: " << (glGetString(GL_RENDERER) != nullptr ? (const char*)glGetString(GL_RENDERER) : "Unknown");
  LOG(LogInfo) << "[Renderer] GL Version: " << (glGetString(GL_VERSION) != nullptr ? (const char*)glGetString(GL_VERSION) : "Unknown");
  LOG(LogInfo) << "[Renderer] GL Extensions: " << (glGetString(GL_EXTENSIONS) != nullptr ? (const char*)glGetString(GL_EXTENSIONS) : "Unknown");
}

bool Renderer::CreateSdlSurface(int width, int height)
{
  { LOG(LogInfo) << "[Renderer] Creating surface..."; }

  /*if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
  {
    { LOG(LogError) << "[Renderer] Error initializing SDL!\n	" << SDL_GetError(); }
    return false;
  }*/

  //hide mouse cursor
  mInitialCursorState = SDL_ShowCursor(0) == 1;

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  // multisample anti-aliasing
  //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);

  #ifdef USE_OPENGL_ES
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
  #endif

  mViewportSize.Set(width, height);
  { LOG(LogInfo) << "[Renderer] Trying Resolution: " << width << ',' << height; }

  int flags = SDL_WINDOW_OPENGL;
  if (!mWindowed) flags |= SDL_WINDOW_FULLSCREEN;
  mSdlWindow = SDL_CreateWindow("EmulationStation",
                                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                mViewportSize.x(),
                                mViewportSize.y(),
                                flags | SDL_WINDOW_ALWAYS_ON_TOP);

  if (mSdlWindow == nullptr)
  {
    { LOG(LogError) << "[Renderer] Error creating SDL window!\n\t" << SDL_GetError(); }
    return false;
  }

  { LOG(LogInfo) << "[Renderer] Created window successfully."; }

  //set an icon for the window
  size_t icwidth = 0;
  size_t icheight = 0;
  std::vector<unsigned char> rawData = ImageIO::loadFromMemoryRGBA32(window_icon_256_png_data, window_icon_256_png_size,
                                                                     icwidth, icheight);
  if (!rawData.empty())
  {
    ImageIO::flipPixelsVert(rawData.data(), icwidth, icheight);

    //SDL interprets each pixel as a 32-bit number, so our masks must depend on the endianness (byte order) of the machine
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
    Uint32 rmask = 0xff000000; Uint32 gmask = 0x00ff0000; Uint32 bmask = 0x0000ff00; Uint32 amask = 0x000000ff;
    #else
    Uint32 rmask = 0x000000ff;
    Uint32 gmask = 0x0000ff00;
    Uint32 bmask = 0x00ff0000;
    Uint32 amask = 0xff000000;
    #endif
    //try creating SDL surface from logo data
    SDL_Surface* logoSurface = SDL_CreateRGBSurfaceFrom((void*) rawData.data(), (int)icwidth, (int)icheight, 32, (int)(icwidth * 4),
                                                        rmask, gmask, bmask, amask);
    if (logoSurface != nullptr)
    {
      SDL_SetWindowIcon(mSdlWindow, logoSurface);
      SDL_FreeSurface(logoSurface);
    }
  }

  // Create GL context & store thread id that created this context
  mSdlGLContext = SDL_GL_CreateContext(mSdlWindow);
  sGLContextThread = pthread_self();

  // Log info
  InformationLogs();

  // vsync
  { LOG(LogInfo) << "[Renderer] Activating vertical sync'"; }
  // SDL_GL_SetSwapInterval(0) for immediate updates (no vsync, default),
  // 1 for updates synchronized with the vertical retrace,
  // or -1 for late swap tearing.
  // SDL_GL_SetSwapInterval returns 0 on success, -1 on error.
  // if vsync is requested, try late swap tearing; if that doesn't work, try normal vsync
  // if that doesn't work, report an error
  /*if (SDL_GL_SetSwapInterval(-1) == 0) LOG(LogInfo) << "Adaptative VSync' activated.";
  else*/
  if (SDL_GL_SetSwapInterval(1) == 0) { LOG(LogInfo) << "[Renderer] Normal VSync' activated."; }
  else { LOG(LogWarning) << "[Renderer] Tried to enable vsync, but failed! (" << SDL_GetError() << ')'; }

  return true;
}

void Renderer::SwapBuffers()
{
  SDL_GL_SwapWindow(mSdlWindow);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::DestroySdlSurface()
{
  SDL_GL_DeleteContext(mSdlGLContext);
  mSdlGLContext = nullptr;

  SDL_DestroyWindow(mSdlWindow);
  mSdlWindow = nullptr;

  //show mouse cursor
  SDL_ShowCursor(mInitialCursorState ? 1 : 0);
  SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void Renderer::GetResolutionFromConfiguration(int& w, int& h)
{
  ResolutionAdapter adapter;
  Resolutions::SimpleResolution output { .Width=0, .Height=0 };
  String resolution = RecalboxConf::Instance().GetEmulationstationVideoMode().LowerCase().Trim();
  { LOG(LogInfo) << "[Renderer] Get resolution from recalbox.conf: " << w << 'x' << h; }

  if (!adapter.AdjustResolution(0, resolution, output))
  {
    resolution = RecalboxConf::Instance().GetGlobalVideoMode().LowerCase();
    adapter.AdjustResolution(0, resolution, output);
  }
  if (output.IsValid())
  {
    w = output.Width;
    h = output.Height;
  }
  { LOG(LogInfo) << "[Renderer] Get resolution from configuration (adjusted): " << w << 'x' << h; }
}

bool Renderer::ReInitialize()
{
  // Reinitialize with real resolution settings
  return Initialize(mViewportSize.x(), mViewportSize.y());
}

bool Renderer::Rotate(RotationType rotation)
{
  this->mRotate = rotation;
  return true;
}

bool Renderer::Initialize(int w, int h)
{
  { LOG(LogInfo) << "[Renderer] Initial resolution: " << w << 'x' << h; }

  bool createdSurface = false;
  bool isCrt = Board::Instance().CrtBoard().IsCrtAdapterAttached();
  // Get resolution from config or crt fixed resolutions if either w or h is nul (not set from command line)
  if (w <= 0 || h <= 0)
  {
    if (isCrt = ResolutionAdapter::GetCRTResolution(w, h); !isCrt)
      GetResolutionFromConfiguration(w, h);
  }
  // Try to create surface if the resolution is valid
  if (w > 0 && h > 0) createdSurface = CreateSdlSurface(w, h);

  // Fallback to SDL2 default resolution
  if (!createdSurface)
  {
    ResolutionAdapter adapter;
    ResolutionAdapter::Resolution defaultRes = adapter.DefaultResolution();
    w = defaultRes.Width;
    h = defaultRes.Height;
    { LOG(LogInfo) << "[Renderer] Get default resolution from Resolution Adapter: " << w << 'x' << h; }
    if (w > 0 && h > 0)
      createdSurface = CreateSdlSurface(defaultRes.Width, defaultRes.Height);
  }
  if (!createdSurface)
    return false;

  mScale.Set(1,1);
  mVirtualViewportSize.Set(mViewportSize.x(), mViewportSize.y());
  mVirtualViewportSizeFloat.Set((float)mViewportSize.x(), (float)mViewportSize.y());

  float scaledWidth = (float)mViewportSize.x();
  if (
    #ifdef OPTION_RECALBOX_SIMULATE_RRGBD
    true
    #else
    RecalboxConf::Instance().GetESForce43()
    #endif
  )
    scaledWidth = (float)mViewportSize.y() * 1.333333334f;
  if (isCrt)
  {
    if(Board::Instance().CrtBoard().ShouldWideScreen())
      scaledWidth = (float)mViewportSize.y() * 1.777777778f;
    else
      scaledWidth = (float)mViewportSize.y() * 1.333333334f;
  }

  mScale.Set((float)mViewportSize.x() / scaledWidth, 1);
  mVirtualViewportSize.Set((int) (scaledWidth), mViewportSize.y());
  mVirtualViewportSizeFloat.Set(scaledWidth, (float)mViewportSize.y());

  glViewport(0, 0, mViewportSize.x(), mViewportSize.y());

  glMatrixMode(GL_PROJECTION);
  glOrtho(0, w, h, 0, -1.0, 1.0);

  RotateViewport();

  glScalef(mScale.x(), mScale.y(), 1.f);
  glMatrixMode(GL_MODELVIEW);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  InitializeClipping();

  return true;
}

void Renderer::RotateViewport()
{
  switch(mRotate)
  {
    case RotationType::Right:
    {
      // Reverse virtual height and width
      mVirtualViewportSize.Set(mVirtualViewportSize.y(), mVirtualViewportSize.x());
      mVirtualViewportSizeFloat.Set(mVirtualViewportSizeFloat.y(), mVirtualViewportSizeFloat.x());
      mScale.Set(mScale.y(), mScale.x());
      glRotatef(90,0,0,1);
      glTranslatef(0, -(float)mViewportSize.x(),0);
      break;
    }
    case RotationType::Left:
    {
      mVirtualViewportSize.Set(mVirtualViewportSize.y(), mVirtualViewportSize.x());
      mVirtualViewportSizeFloat.Set(mVirtualViewportSizeFloat.y(), mVirtualViewportSizeFloat.x());
      mScale.Set(mScale.y(), mScale.x());
      glRotatef(270,0,0,1);
      glTranslatef(-(float)mViewportSize.y(),0,0);
      break;
    }
    case RotationType::Upsidedown:
    {
      glRotatef(180,0,0,1);
      glTranslatef(-(float)mViewportSize.x(),-(float)mViewportSize.y(),0);
      break;
    }
    case RotationType::None:
    default: break;
  }
}

void Renderer::Finalize()
{
  DestroySdlSurface();
}

void Renderer::BuildGLColorArray(GLubyte* ptr, Colors::ColorARGB color, int vertCount)
{
  unsigned int colorGl = 0;
  ColorToByteArray((GLubyte*) &colorGl, color);
  for (int i = vertCount; --i >= 0; )
    ((GLuint*) ptr)[i] = colorGl;
}

void Renderer::InitializeClipping()
{
  // Push a default clipping so that any current clipping call will return a valid clipping
  mClippings.Clear();
  mLastXTranslation = mLastYTranslation = 0;
  Clip(Rectangle(0, 0, mVirtualViewportSize.x(), mVirtualViewportSize.y()));
}

void Renderer::ClipAbsolute(const Rectangle& area)
{
  CHECKGLCONTEXT;
  // Copy
  Rectangle clip(area);
  clip.Scale(mScale.x(), mScale.y());
  // Rotate
  switch(mRotate)
  {
    case RotationType::None: break;
    case RotationType::Right: clip = Rectangle((float)mViewportSize.x() - clip.Bottom(), clip.Left(), clip.Height(), clip.Width()); break;
    case RotationType::Upsidedown: clip = Rectangle((float)mViewportSize.x() - clip.Left() - clip.Right(), (float)mViewportSize.y() - clip.Top() - clip.Bottom(), clip.Right(), clip.Bottom()); break;
    case RotationType::Left: clip = Rectangle(clip.Top(), (float)mViewportSize.y() - clip.Right(), clip.Height(), clip.Width()); break;
  }
  // Reverse the rectangle to match the GL 0,0 in bottom left
  clip.FlipOnScreen(mViewportSize.y());

  // constraint the new clipping whithin the last clipping limits
  if (!mClippings.Empty())
    clip.Intersect(mClippings.Peek());

  // Validate (ensure no negative values)
  clip.Validate();

  // Store new current clipping
  mCurrentClipping = clip;
  mCurrentClipping.FlipOnScreen(mViewportSize.y()); // Revert current clipping

  // Clip!
  mClippings.Push(clip);
  glScissor((int)clip.Left(), (int)clip.Top(), (int)clip.Width(), (int)clip.Height());
  glEnable(GL_SCISSOR_TEST);
}

void Renderer::Clip(const Transform4x4f& trans, const Vector2f& size)
{
  Vector2i clipPos((int)trans.translation().x(), (int)trans.translation().y());
  Vector3f dimScaled = trans * Vector3f(size.x(), size.y(), 0);
  Vector2i clipDim((int)(dimScaled.x() - trans.translation().x()), (int)(dimScaled.y() - trans.translation().y()));

  ClipAbsolute(Rectangle(clipPos, clipDim));
}

void Renderer::Clip(const Rectangle& area)
{
  // Translate clipping rectangle into absolute screen coordinate
  ClipAbsolute(Rectangle(area).Move(mLastXTranslation, mLastYTranslation));
}

void Renderer::Unclip()
{
  CHECKGLCONTEXT;

  // Check
  if (mClippings.Empty()) { LOG(LogError) << "Tried to popClipRect while the stack was empty!"; return; }

  // Pop!
  mClippings.Pop();

  // No more clippings?
  if (mClippings.Empty()) { glDisable(GL_SCISSOR_TEST); return; }

  // Reclip using top clipping
  Rectangle& clip = mClippings.Peek();
  glScissor((int)clip.Left(), (int)clip.Top(), (int)clip.Width(), (int)clip.Height());

  // Store new current clipping
  mCurrentClipping = clip;
  mCurrentClipping.FlipOnScreen(mViewportSize.y()); // Revert current clipping
}

GLuint Renderer::CreateGLTexture(bool nearest)
{
  CHECKGLCONTEXT;

  GLuint id = 0;

  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, nearest ? GL_NEAREST : GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, nearest ? GL_NEAREST : GL_LINEAR);

  return id;
}

void Renderer::DestroyGLTexture(GLuint id)
{
  CHECKGLCONTEXT;

  glDeleteTextures(1, &id);
}

void Renderer::DrawRectangle(const Rectangle& area, Colors::ColorARGB color, GLenum blend_sfactor, GLenum blend_dfactor)
{
  DrawRectangle(Math::roundi(area.Left()), Math::roundi(area.Top()),
                Math::roundi(area.Width()), Math::roundi(area.Height()),
                color, blend_sfactor, blend_dfactor);
}

void
Renderer::DrawRectangle(float x, float y, float w, float h, Colors::ColorARGB color, GLenum blend_sfactor, GLenum blend_dfactor)
{
  DrawRectangle(Math::roundi(x), Math::roundi(y), Math::roundi(w), Math::roundi(h), color, blend_sfactor, blend_dfactor);
}

void Renderer::DrawRectangle(int x, int y, int w, int h, Colors::ColorARGB color, GLenum blend_sfactor, GLenum blend_dfactor)
{
  CHECKGLCONTEXT;

  GLType points[12]
  {
    (GLType)x      , (GLType)y      ,
    (GLType)x      , (GLType)(y + h),
    (GLType)(x + w), (GLType)y      ,
    (GLType)(x + w), (GLType)y      ,
    (GLType)x      , (GLType)(y + h),
    (GLType)(x + w), (GLType)(y + h),
  };

  GLubyte colors[6 * 4];
  BuildGLColorArray(colors, color, 6);

  glEnable(GL_BLEND);
  glBlendFunc(blend_sfactor, blend_dfactor);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  #ifdef USE_OPENGL_ES
  glVertexPointer(2, GL_SHORT, 0, points);
  #else
  glVertexPointer(2, GL_INT, 0, points);
  #endif
  glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);

  glDrawArrays(GL_TRIANGLES, 0, 6);

  glDisable(GL_BLEND);
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
}

void Renderer::SetMatrix(const Transform4x4f& transform)
{
  CHECKGLCONTEXT;

  mLastXTranslation = transform.r3().x();
  mLastYTranslation = transform.r3().y();
  glLoadMatrixf((float*)&(Transform4x4f(transform).round()));
}

RenderingError Renderer::UploadAlpha(GLuint id, int width, int height, const void* data)
{
  CHECKGLCONTEXT;

  glBindTexture(GL_TEXTURE_2D, id);
  if (glGetError() != GL_NO_ERROR) return RenderingError::NoResource;

  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);
  if (glGetError() == GL_OUT_OF_MEMORY) return RenderingError::OutOfGPUMemory;

  return RenderingError::NoError;
}

RenderingError Renderer::UploadRGBA(GLuint id, int width, int height, const void* data)
{
  CHECKGLCONTEXT;

  glBindTexture(GL_TEXTURE_2D, id);
  if (glGetError() != GL_NO_ERROR) return RenderingError::NoResource;

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  if (glGetError() == GL_OUT_OF_MEMORY) return RenderingError::OutOfGPUMemory;

  return RenderingError::NoError;
}

RenderingError Renderer::UploadAlphaPart(GLuint id, int x, int y, int width, int height, const void* data)
{
  CHECKGLCONTEXT;

  glBindTexture(GL_TEXTURE_2D, id);
  if (glGetError() != GL_NO_ERROR) return RenderingError::NoResource;

  glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, GL_ALPHA, GL_UNSIGNED_BYTE, data);
  if (glGetError() == GL_OUT_OF_MEMORY) return RenderingError::OutOfGPUMemory;

  return RenderingError::NoError;
}

RenderingError Renderer::UploadRGBAPart(GLuint id, int x, int y, int width, int height, const void* data)
{
  CHECKGLCONTEXT;

  glBindTexture(GL_TEXTURE_2D, id);
  if (glGetError() != GL_NO_ERROR) return RenderingError::NoResource;

  glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
  if (glGetError() == GL_OUT_OF_MEMORY) return RenderingError::OutOfGPUMemory;

  return RenderingError::NoError;
}

void Renderer::DrawLines(const Vector2f coordinates[], const Colors::ColorARGB colors[], int count)
{
  CHECKGLCONTEXT;

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  glVertexPointer(2, GL_FLOAT, 0, coordinates);
  glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);

  glDrawArrays(GL_LINES, 0, count);

  glDisable(GL_BLEND);
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
}

void Renderer::DrawRectangle(int x, int y, int w, int h,
                             Colors::ColorARGB topleftcolor,
                             Colors::ColorARGB toprightcolor,
                             Colors::ColorARGB bottomrightcolor,
                             Colors::ColorARGB bottomleftcolor)
{
  CHECKGLCONTEXT;

  GLType points[12]
  {
    (GLType)x      , (GLType)y      ,
    (GLType)x      , (GLType)(y + h),
    (GLType)(x + w), (GLType)y      ,
    (GLType)(x + w), (GLType)y      ,
    (GLType)x      , (GLType)(y + h),
    (GLType)(x + w), (GLType)(y + h),
  };

  GLuint colors[6]
    {
      ColorToGL(topleftcolor),
      ColorToGL(bottomleftcolor),
      ColorToGL(toprightcolor),
      ColorToGL(toprightcolor),
      ColorToGL(bottomleftcolor),
      ColorToGL(bottomrightcolor)
    };

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  #ifdef USE_OPENGL_ES
  glVertexPointer(2, GL_SHORT, 0, points);
  #else
  glVertexPointer(2, GL_INT, 0, points);
  #endif
  glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);

  glDrawArrays(GL_TRIANGLES, 0, 6);

  glDisable(GL_BLEND);
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
}

void Renderer::DrawTexturedTriangles(GLuint id, const Vertex vertices[], const GLubyte colors[], int count, bool tiled)
{
  CHECKGLCONTEXT;

  if (id != 0)
    glBindTexture(GL_TEXTURE_2D, id);

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].Target);
  glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].Source);
  glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);

  glDrawArrays(GL_TRIANGLES, 0, count);

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
}

void Renderer::DrawTexturedTriangles(GLuint id, const Vertex vertices[], Colors::ColorARGB color, int count, bool tiled)
{
  CHECKGLCONTEXT;

  if (id != 0)
    glBindTexture(GL_TEXTURE_2D, id);

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);

  glColor4ub((GLubyte)(color >> 24),
             (GLubyte)(color >> 16),
             (GLubyte)(color >> 8),
             (GLubyte)(color >> 0));

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].Target);
  glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].Source);

  glDrawArrays(GL_TRIANGLES, 0, count);

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
}

/*void Renderer::DrawTexture(TextureResource& texture, int x, int y, int w, int h, bool keepratio)
{
  CHECKGLCONTEXT;

  if (keepratio && texture.realWidth() != 0 && texture.realHeight() != 0)
  {
    float areaRatio = (float)w / (float)h;
    float textureRatio = texture.realWidth() / texture.realHeight();
    if (areaRatio < textureRatio)
    {
      double height = (float)w / textureRatio;
      y += (h - (int) height) / 2;
      h = (int)height;
    }
    else
    {
      double width = (float)h * textureRatio;
      x += (w - (int) width) / 2;
      w = (int)width;
    }
  }

  if (texture.bind())
  {
    bool tiled = texture.isTiled();
    float tx = tiled ? (float)w / texture.realWidth() : 1.f;
    float ty = tiled ? (float)h / texture.realHeight() : 1.f;
    Vertex vertices[Vertex::sVertexPerRectangle]
    {
      { .Target={ x    , y     }, .Source={ 0.f,  ty } },
      { .Target={ x    , y + h }, .Source={ 0.f, 0.f } },
      { .Target={ x + w, y     }, .Source={  tx,  ty } },
      { .Target={ x + w, y     }, .Source={  tx,  ty } },
      { .Target={ x    , y + h }, .Source={ 0.f, 0.f } },
      { .Target={ x + w, y + h }, .Source={  tx, 0.f } }
    };

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].Target);
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].Source);

    glDrawArrays(GL_TRIANGLES, 0, Vertex::sVertexPerRectangle);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
  }
}*/

void Renderer::DrawTexture(GLuint id, int textureW, int textureH, int x, int y, int w, int h, bool keepratio, bool tiled)
{
  CHECKGLCONTEXT;

  if (id != 0) glBindTexture(GL_TEXTURE_2D, id);

  if (keepratio && textureW != 0 && textureH != 0)
  {
    float areaRatio = (float)w / (float)h;
    float textureRatio = (float)textureW / (float)textureH;
    if (areaRatio < textureRatio)
    {
      double height = (float)w / textureRatio;
      y += (h - (int) height) / 2;
      h = (int)height;
    }
    else
    {
      double width = (float)h * textureRatio;
      x += (w - (int) width) / 2;
      w = (int)width;
    }
  }

  float tx = tiled ? (float)w / (float)textureW : 1.f;
  float ty = tiled ? (float)h / (float)textureH : 1.f;
  Vertex vertices[Vertex::sVertexPerRectangle]
    {
      { .Target={ x    , y     }, .Source={ 0.f,  ty } },
      { .Target={ x    , y + h }, .Source={ 0.f, 0.f } },
      { .Target={ x + w, y     }, .Source={  tx,  ty } },
      { .Target={ x + w, y     }, .Source={  tx,  ty } },
      { .Target={ x    , y + h }, .Source={ 0.f, 0.f } },
      { .Target={ x + w, y + h }, .Source={  tx, 0.f } }
    };

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].Target);
  glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].Source);

  glDrawArrays(GL_TRIANGLES, 0, Vertex::sVertexPerRectangle);

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
}

/*void Renderer::DrawTexture(TextureResource& texture, int x, int y, int w, int h, bool keepratio, unsigned char alpha)
{
  CHECKGLCONTEXT;

  if (keepratio && texture.realWidth() != 0 && texture.realHeight() != 0)
  {
    float areaRatio = (float)w / (float)h;
    float textureRatio = texture.realWidth() / texture.realHeight();
    if (areaRatio < textureRatio)
    {
      double height = (float)w / textureRatio;
      y += (h - (int) height) / 2;
      h = (int)height;
    }
    else
    {
      double width = (float)h * textureRatio;
      x += (w - (int) width) / 2;
      w = (int)width;
    }
  }

  if (texture.bind())
  {
    bool tiled = texture.isTiled();
    float tx = tiled ? (float)w / texture.realWidth() : 1.f;
    float ty = tiled ? (float)h / texture.realHeight() : 1.f;
    Vertex vertices[Vertex::sVertexPerRectangle]
    {
      { .Target={ x    , y     }, .Source={ 0.f,  ty } },
      { .Target={ x    , y + h }, .Source={ 0.f, 0.f } },
      { .Target={ x + w, y     }, .Source={  tx,  ty } },
      { .Target={ x + w, y     }, .Source={  tx,  ty } },
      { .Target={ x    , y + h }, .Source={ 0.f, 0.f } },
      { .Target={ x + w, y + h }, .Source={  tx, 0.f } }
    };

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);

    glColor4ub(0xFF, 0xFF, 0xFF, alpha);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].Target);
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].Source);

    glDrawArrays(GL_TRIANGLES, 0, Vertex::sVertexPerRectangle);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
  }
}*/

void Renderer::DrawTexture(GLuint id, int textureW, int textureH, int x, int y, int w, int h, bool keepratio, bool tiled, unsigned char alpha)
{
  CHECKGLCONTEXT;

  if (id != 0) glBindTexture(GL_TEXTURE_2D, id);

  if (keepratio && textureW != 0 && textureH != 0)
  {
    float areaRatio = (float)w / (float)h;
    float textureRatio = (float)textureW / (float)textureH;
    if (areaRatio < textureRatio)
    {
      double height = (float)w / textureRatio;
      y += (h - (int) height) / 2;
      h = (int)height;
    }
    else
    {
      double width = (float)h * textureRatio;
      x += (w - (int) width) / 2;
      w = (int)width;
    }
  }

  float tx = tiled ? (float)w / (float)textureW : 1.f;
  float ty = tiled ? (float)h / (float)textureH : 1.f;
  Vertex vertices[Vertex::sVertexPerRectangle]
    {
      { .Target={ x    , y     }, .Source={ 0.f,  ty } },
      { .Target={ x    , y + h }, .Source={ 0.f, 0.f } },
      { .Target={ x + w, y     }, .Source={  tx,  ty } },
      { .Target={ x + w, y     }, .Source={  tx,  ty } },
      { .Target={ x    , y + h }, .Source={ 0.f, 0.f } },
      { .Target={ x + w, y + h }, .Source={  tx, 0.f } }
    };

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);

  glColor4ub(0xFF, 0xFF, 0xFF, alpha);

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].Target);
  glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].Source);

  glDrawArrays(GL_TRIANGLES, 0, Vertex::sVertexPerRectangle);

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
}

/*void Renderer::DrawTexture(TextureResource& texture, int x, int y, int w, int h, bool keepratio, Colors::ColorARGB color)
{
  CHECKGLCONTEXT;

  //DrawRectangle(x, y, w, h, 0xFF000040);
  if (keepratio && texture.realWidth() != 0 && texture.realHeight() != 0)
  {
    float areaRatio = (float)w / (float)h;
    float textureRatio = texture.realWidth() / texture.realHeight();
    if (areaRatio < textureRatio)
    {
      double height = (float)w / textureRatio;
      y += (h - (int) height) / 2;
      h = (int)height;
    }
    else
    {
      double width = (float)h * textureRatio;
      x += (w - (int) width) / 2;
      w = (int)width;
    }
  }

  if (texture.bind())
  {
    bool tiled = texture.isTiled();
    float tx = tiled ? (float)w / texture.realWidth() : 1.f;
    float ty = tiled ? (float)h / texture.realHeight() : 1.f;
    Vertex vertices[Vertex::sVertexPerRectangle]
    {
      { .Target={ x    , y     }, .Source={ 0.f,  ty } },
      { .Target={ x    , y + h }, .Source={ 0.f, 0.f } },
      { .Target={ x + w, y     }, .Source={  tx,  ty } },
      { .Target={ x + w, y     }, .Source={  tx,  ty } },
      { .Target={ x    , y + h }, .Source={ 0.f, 0.f } },
      { .Target={ x + w, y + h }, .Source={  tx, 0.f } }
    };

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);

    glColor4ub((GLubyte)(color >> 24),
               (GLubyte)(color >> 16),
               (GLubyte)(color >> 8),
               (GLubyte)(color >> 0));

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].Target);
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].Source);

    glDrawArrays(GL_TRIANGLES, 0, Vertex::sVertexPerRectangle);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
  }
}*/

void Renderer::DrawTexture(GLuint id, int textureW, int textureH, int x, int y, int w, int h, bool keepratio, bool tiled, Colors::ColorARGB color)
{
  CHECKGLCONTEXT;

  if (id != 0) glBindTexture(GL_TEXTURE_2D, id);

  if (keepratio && textureW != 0 && textureH != 0)
  {
    float areaRatio = (float)w / (float)h;
    float textureRatio = (float)textureW / (float)textureH;
    if (areaRatio < textureRatio)
    {
      double height = (float)w / textureRatio;
      y += (h - (int) height) / 2;
      h = (int)height;
    }
    else
    {
      double width = (float)h * textureRatio;
      x += (w - (int) width) / 2;
      w = (int)width;
    }
  }

  float tx = tiled ? (float)w / (float)textureW : 1.f;
  float ty = tiled ? (float)h / (float)textureH : 1.f;
  Vertex vertices[Vertex::sVertexPerRectangle]
    {
      { .Target={ x    , y     }, .Source={ 0.f,  ty } },
      { .Target={ x    , y + h }, .Source={ 0.f, 0.f } },
      { .Target={ x + w, y     }, .Source={  tx,  ty } },
      { .Target={ x + w, y     }, .Source={  tx,  ty } },
      { .Target={ x    , y + h }, .Source={ 0.f, 0.f } },
      { .Target={ x + w, y + h }, .Source={  tx, 0.f } }
    };

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);

  glColor4ub((GLubyte)(color >> 24),
             (GLubyte)(color >> 16),
             (GLubyte)(color >> 8),
             (GLubyte)(color >> 0));

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].Target);
  glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].Source);

  glDrawArrays(GL_TRIANGLES, 0, Vertex::sVertexPerRectangle);

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
}

/*void Renderer::DrawTexture(TextureResource& texture, int x, int y, int w, int h, bool flipX, bool flipY, bool keepratio, Colors::ColorARGB color)
{
  CHECKGLCONTEXT;

  //DrawRectangle(x, y, w, h, 0xFF000040);
  if (keepratio && texture.realWidth() != 0 && texture.realHeight() != 0)
  {
    float areaRatio = (float)w / (float)h;
    float textureRatio = texture.realWidth() / texture.realHeight();
    if (areaRatio < textureRatio)
    {
      double height = (float)w / textureRatio;
      y += (h - (int) height) / 2;
      h = (int)height;
    }
    else
    {
      double width = (float)h * textureRatio;
      x += (w - (int) width) / 2;
      w = (int)width;
    }
  }

  if (texture.bind())
  {
    int x1 = x;
    int x2 = x;
    if (flipX) x1 += w; else x2 += w;
    int y1 = y;
    int y2 = y;
    if (flipY) y1 += h; else y2 += h;
    bool tiled = texture.isTiled();
    float tx = tiled ? (float)w / texture.realWidth() : 1.f;
    float ty = tiled ? (float)h / texture.realHeight() : 1.f;
    Vertex vertices[Vertex::sVertexPerRectangle]
      {
        { .Target={ x1, y1 }, .Source={ 0.f,  ty } },
        { .Target={ x1, y2 }, .Source={ 0.f, 0.f } },
        { .Target={ x2, y1 }, .Source={  tx,  ty } },
        { .Target={ x2, y1 }, .Source={  tx,  ty } },
        { .Target={ x1, y2 }, .Source={ 0.f, 0.f } },
        { .Target={ x2, y2 }, .Source={  tx, 0.f } }
      };

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);

    glColor4ub((GLubyte)(color >> 24),
               (GLubyte)(color >> 16),
               (GLubyte)(color >> 8),
               (GLubyte)(color >> 0));

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].Target);
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].Source);

    glDrawArrays(GL_TRIANGLES, 0, Vertex::sVertexPerRectangle);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
  }
}*/

void Renderer::DrawTexture(GLuint id, int textureW, int textureH, int x, int y, int w, int h, bool flipX, bool flipY, bool keepratio, bool tiled, Colors::ColorARGB color)
{
  CHECKGLCONTEXT;

  if (id != 0) glBindTexture(GL_TEXTURE_2D, id);

  if (keepratio && textureW != 0 && textureH != 0)
  {
    float areaRatio = (float)w / (float)h;
    float textureRatio = (float)textureW / (float)textureH;
    if (areaRatio < textureRatio)
    {
      double height = (float)w / textureRatio;
      y += (h - (int) height) / 2;
      h = (int)height;
    }
    else
    {
      double width = (float)h * textureRatio;
      x += (w - (int) width) / 2;
      w = (int)width;
    }
  }

  int x1 = x;
  int x2 = x;
  if (flipX) x1 += w; else x2 += w;
  int y1 = y;
  int y2 = y;
  if (flipY) y1 += h; else y2 += h;
  float tx = tiled ? (float)w / (float)textureW : 1.f;
  float ty = tiled ? (float)h / (float)textureH : 1.f;
  Vertex vertices[Vertex::sVertexPerRectangle]
    {
      { .Target={ x1, y1 }, .Source={ 0.f,  ty } },
      { .Target={ x1, y2 }, .Source={ 0.f, 0.f } },
      { .Target={ x2, y1 }, .Source={  tx,  ty } },
      { .Target={ x2, y1 }, .Source={  tx,  ty } },
      { .Target={ x1, y2 }, .Source={ 0.f, 0.f } },
      { .Target={ x2, y2 }, .Source={  tx, 0.f } }
    };

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);

  glColor4ub((GLubyte)(color >> 24),
             (GLubyte)(color >> 16),
             (GLubyte)(color >> 8),
             (GLubyte)(color >> 0));

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].Target);
  glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].Source);

  glDrawArrays(GL_TRIANGLES, 0, Vertex::sVertexPerRectangle);

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
}

/*void Renderer::DrawTexture(TextureResource& texture, int x, int y, int w, int h, bool keepratio,
                           Colors::ColorARGB topleftcolor, Colors::ColorARGB toprightcolor,
                           Colors::ColorARGB bottomrightcolor, Colors::ColorARGB bottomleftcolor)
{
  CHECKGLCONTEXT;

  if (keepratio && texture.realWidth() != 0 && texture.realHeight() != 0)
  {
    float areaRatio = (float)w / (float)h;
    float textureRatio = texture.realWidth() / texture.realHeight();
    if (areaRatio < textureRatio)
    {
      double height = (float)w / textureRatio;
      y += (h - (int) height) / 2;
      h = (int)height;
    }
    else
    {
      double width = (float)h * textureRatio;
      x += (w - (int) width) / 2;
      w = (int)width;
    }
  }

  if (texture.bind())
  {
    bool tiled = texture.isTiled();
    float tx = tiled ? (float)w / texture.realWidth() : 1.f;
    float ty = tiled ? (float)h / texture.realHeight() : 1.f;
    Vertex vertices[Vertex::sVertexPerRectangle]
    {
      { .Target={ x    , y     }, .Source={ 0.f,  ty } },
      { .Target={ x    , y + h }, .Source={ 0.f, 0.f } },
      { .Target={ x + w, y     }, .Source={  tx,  ty } },
      { .Target={ x + w, y     }, .Source={  tx,  ty } },
      { .Target={ x    , y + h }, .Source={ 0.f, 0.f } },
      { .Target={ x + w, y + h }, .Source={  tx, 0.f } }
    };

    GLuint colors[Vertex::sVertexPerRectangle];
    ColorToByteArray((GLubyte*)&colors[0], topleftcolor);
    ColorToByteArray((GLubyte*)&colors[1], bottomleftcolor);
    ColorToByteArray((GLubyte*)&colors[2], toprightcolor);
    colors[3] = colors[2];
    colors[4] = colors[1];
    ColorToByteArray((GLubyte*)&colors[5], bottomrightcolor);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].Target);
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].Source);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);

    glDrawArrays(GL_TRIANGLES, 0, Vertex::sVertexPerRectangle);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
  }
}*/

void Renderer::DrawTexture(GLuint id, int textureW, int textureH, int x, int y, int w, int h, bool keepratio, bool tiled,
                           Colors::ColorARGB topleftcolor, Colors::ColorARGB toprightcolor,
                           Colors::ColorARGB bottomrightcolor, Colors::ColorARGB bottomleftcolor)
{
  CHECKGLCONTEXT;

  if (id != 0) glBindTexture(GL_TEXTURE_2D, id);

  if (keepratio && textureW != 0 && textureH != 0)
  {
    float areaRatio = (float)w / (float)h;
    float textureRatio = (float)textureW / (float)textureH;
    if (areaRatio < textureRatio)
    {
      double height = (float)w / textureRatio;
      y += (h - (int) height) / 2;
      h = (int)height;
    }
    else
    {
      double width = (float)h * textureRatio;
      x += (w - (int) width) / 2;
      w = (int)width;
    }
  }

  float tx = tiled ? (float)w / (float)textureW : 1.f;
  float ty = tiled ? (float)h / (float)textureH : 1.f;
  Vertex vertices[Vertex::sVertexPerRectangle]
    {
      { .Target={ x    , y     }, .Source={ 0.f,  ty } },
      { .Target={ x    , y + h }, .Source={ 0.f, 0.f } },
      { .Target={ x + w, y     }, .Source={  tx,  ty } },
      { .Target={ x + w, y     }, .Source={  tx,  ty } },
      { .Target={ x    , y + h }, .Source={ 0.f, 0.f } },
      { .Target={ x + w, y + h }, .Source={  tx, 0.f } }
    };

  GLuint colors[Vertex::sVertexPerRectangle];
  ColorToByteArray((GLubyte*)&colors[0], topleftcolor);
  ColorToByteArray((GLubyte*)&colors[1], bottomleftcolor);
  ColorToByteArray((GLubyte*)&colors[2], toprightcolor);
  colors[3] = colors[2];
  colors[4] = colors[1];
  ColorToByteArray((GLubyte*)&colors[5], bottomrightcolor);

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].Target);
  glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].Source);
  glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);

  glDrawArrays(GL_TRIANGLES, 0, Vertex::sVertexPerRectangle);

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);

  glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
}

/*void Renderer::DrawTexture(TextureResource& texture, int x, int y, int w, int h, bool keepratio,
                           bool flipOnX, bool flipOnY,
                           Colors::ColorARGB topleftcolor, Colors::ColorARGB toprightcolor,
                           Colors::ColorARGB bottomrightcolor, Colors::ColorARGB bottomleftcolor,
                           float topAlphaReflection, float bottomAlphaReflection)
{
  CHECKGLCONTEXT;

  if (keepratio && texture.realWidth() != 0 && texture.realHeight() != 0)
  {
    float areaRatio = (float)w / (float)h;
    float textureRatio = texture.realWidth() / texture.realHeight();
    if (areaRatio < textureRatio)
    {
      double height = (float)w / textureRatio;
      y += (h - (int) height) / 2;
      h = (int)height;
    }
    else
    {
      double width = (float)h * textureRatio;
      x += (w - (int) width) / 2;
      w = (int)width;
    }
  }
  //DrawRectangle(x, y, w, h, 0x00FF0040);

  if (texture.bind())
  {
    int x1 = x;
    int x2 = x;
    if (flipOnX) x1 += w; else x2 += w;
    int y1 = y;
    int y2 = y;
    if (flipOnY) y1 += h; else y2 += h;
    bool tiled = texture.isTiled();
    float tx = tiled ? (float)w / texture.realWidth() : 1.f;
    float ty = tiled ? (float)h / texture.realHeight() : 1.f;
    Vertex vertices[Vertex::sVertexPerRectangle]
    {
      { .Target={ x1, y1 }, .Source={ 0.f,  ty } },
      { .Target={ x1, y2 }, .Source={ 0.f, 0.f } },
      { .Target={ x2, y1 }, .Source={  tx,  ty } },
      { .Target={ x2, y1 }, .Source={  tx,  ty } },
      { .Target={ x1, y2 }, .Source={ 0.f, 0.f } },
      { .Target={ x2, y2 }, .Source={  tx, 0.f } }
    };

    GLuint colors[Vertex::sVertexPerRectangle];
    ColorToByteArray((GLubyte*)&colors[0], topleftcolor);
    ColorToByteArray((GLubyte*)&colors[1], bottomleftcolor);
    ColorToByteArray((GLubyte*)&colors[2], toprightcolor);
    colors[3] = colors[2];
    colors[4] = colors[1];
    ColorToByteArray((GLubyte*)&colors[5], bottomrightcolor);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].Target);
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].Source);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);

    glDrawArrays(GL_TRIANGLES, 0, Vertex::sVertexPerRectangle);

    if (topAlphaReflection != 0 || bottomAlphaReflection != 0)
    {
      for(int i = Vertex::sVertexPerRectangle; --i >= 0;)
      {
        vertices[i].Target.Y += (float)h;
        vertices[i].Source.Y = vertices[i].Source.Y == 0 ? 1 : 0;
      }
      ColorToByteArray((GLubyte*)&colors[0], (topleftcolor & 0xFFFFFF00) | (int)((float)(topleftcolor & 0xFF) * topAlphaReflection));
      ColorToByteArray((GLubyte*)&colors[1], (bottomleftcolor & 0xFFFFFF00) | (int)((float)(bottomleftcolor & 0xFF) * bottomAlphaReflection));
      ColorToByteArray((GLubyte*)&colors[2], (toprightcolor & 0xFFFFFF00) | (int)((float)(toprightcolor & 0xFF) * topAlphaReflection));
      colors[3] = colors[2];
      colors[4] = colors[1];
      ColorToByteArray((GLubyte*)&colors[5], (bottomrightcolor & 0xFFFFFF00) | (int)((float)(bottomrightcolor & 0xFF) * bottomAlphaReflection));

      glDrawArrays(GL_TRIANGLES, 0, Vertex::sVertexPerRectangle);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
  }
}*/

void Renderer::DrawTexture(GLuint id, int textureW, int textureH, int x, int y, int w, int h, bool keepratio, bool tiled,
                           bool flipOnX, bool flipOnY,
                           Colors::ColorARGB topleftcolor, Colors::ColorARGB toprightcolor,
                           Colors::ColorARGB bottomrightcolor, Colors::ColorARGB bottomleftcolor,
                           float topAlphaReflection, float bottomAlphaReflection)
{
  CHECKGLCONTEXT;

  if (id != 0) glBindTexture(GL_TEXTURE_2D, id);

  if (keepratio && textureW != 0 && textureH != 0)
  {
    float areaRatio = (float)w / (float)h;
    float textureRatio = (float)textureW / (float)textureH;
    if (areaRatio < textureRatio)
    {
      double height = (float)w / textureRatio;
      y += (h - (int) height) / 2;
      h = (int)height;
    }
    else
    {
      double width = (float)h * textureRatio;
      x += (w - (int) width) / 2;
      w = (int)width;
    }
  }

  int x1 = x;
  int x2 = x;
  if (flipOnX) x1 += w; else x2 += w;
  int y1 = y;
  int y2 = y;
  if (flipOnY) y1 += h; else y2 += h;
  float tx = tiled ? (float)w / (float)textureW : 1.f;
  float ty = tiled ? (float)h / (float)textureH : 1.f;
  Vertex vertices[Vertex::sVertexPerRectangle]
    {
      { { x1, y1 }, { 0.f,  ty } },
      { { x1, y2 }, { 0.f, 0.f } },
      { { x2, y1 }, {  tx,  ty } },
      { { x2, y1 }, {  tx,  ty } },
      { { x1, y2 }, { 0.f, 0.f } },
      { { x2, y2 }, {  tx, 0.f } }
    };

  GLuint colors[Vertex::sVertexPerRectangle];
  ColorToByteArray((GLubyte*)&colors[0], topleftcolor);
  ColorToByteArray((GLubyte*)&colors[1], bottomleftcolor);
  ColorToByteArray((GLubyte*)&colors[2], toprightcolor);
  colors[3] = colors[2];
  colors[4] = colors[1];
  ColorToByteArray((GLubyte*)&colors[5], bottomrightcolor);

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].Target);
  glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].Source);
  glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);

  glDrawArrays(GL_TRIANGLES, 0, Vertex::sVertexPerRectangle);

  if (topAlphaReflection != 0 || bottomAlphaReflection != 0)
  {
    for(int i = Vertex::sVertexPerRectangle; --i >= 0;)
    {
      vertices[i].Target.Y += (float)h;
      vertices[i].Source.Y = vertices[i].Source.Y == 0 ? 1 : 0;
    }
    ColorToByteArray((GLubyte*)&colors[0], (topleftcolor & 0xFFFFFF00) | (int)((float)(topleftcolor & 0xFF) * topAlphaReflection));
    ColorToByteArray((GLubyte*)&colors[1], (bottomleftcolor & 0xFFFFFF00) | (int)((float)(bottomleftcolor & 0xFF) * bottomAlphaReflection));
    ColorToByteArray((GLubyte*)&colors[2], (toprightcolor & 0xFFFFFF00) | (int)((float)(toprightcolor & 0xFF) * topAlphaReflection));
    colors[3] = colors[2];
    colors[4] = colors[1];
    ColorToByteArray((GLubyte*)&colors[5], (bottomrightcolor & 0xFFFFFF00) | (int)((float)(bottomrightcolor & 0xFF) * bottomAlphaReflection));

    glDrawArrays(GL_TRIANGLES, 0, Vertex::sVertexPerRectangle);
  }

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);

  glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
}
