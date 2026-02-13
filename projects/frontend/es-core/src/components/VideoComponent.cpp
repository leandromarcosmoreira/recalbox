#include <components/VideoComponent.h>
#include <audio/AudioManager.h>
#include <VideoEngine.h>
#include <themes/ThemeData.h>
#include <utils/locale/LocaleHelper.h>
#include <Renderer.h>

VideoComponent::VideoComponent(WindowManager& window, IVideoComponentAction* actionInterface, AllowedEffects allowedEffectsd)
  : ThemableComponent(window)
  , mInterface(actionInterface)
  , mState(State::InitializeVideo)
  , mEffect(Effect::BreakingNews)
  , mAllowedEffects(allowedEffectsd)
  , mTargetSize(0)
  , mVertices{ { .Target={ 0, 0 }, .Source={ 0, 0 } } }
  , mColors{ 0 }
  , mColorShift(0xFFFFFFFF)
  , mFadeOpacity(0)
  , mVideoDelay(DEFAULT_VIDEODELAY)
  , mVideoEffect(DEFAULT_VIDEOEFFET)
  , mVideoLoop(DEFAULT_VIDEOLOOP)
  , mTopAlpha(0)
  , mBottomAlpha(0)
  , mDecodeAudio(DEFAULT_VIDEODECODEAUDIO)
  , mKeepRatio(false)
{
  updateColors();
}

void VideoComponent::resize()
{
  VideoEngine::Instance().AquireTexture();
  GLuint texture = VideoEngine::Instance().GetDisplayableFrame();
  const Vector2f textureSize = VideoEngine::Instance().GetDisplayableFrameSize();
  VideoEngine::Instance().ReleaseTexture();

  if (texture == 0) return;

  if (mKeepRatio)
  {
    mSize = textureSize;

    Vector2f resizeScale((mTargetSize.x() / mSize.x()), (mTargetSize.y() / mSize.y()));

    if (resizeScale.x() < resizeScale.y())
    {
      mSize[0] *= resizeScale.x();
      mSize[1] *= resizeScale.x();
    }
    else
    {
      mSize[0] *= resizeScale.y();
      mSize[1] *= resizeScale.y();
    }

    // for SVG rasterization, always calculate width from rounded height (see comment above)
    mSize[1] = Math::round(mSize[1]);
    mSize[0] = (mSize[1] / textureSize.y()) * textureSize.x();
  }
  else
  {
    // if both components are set, we just stretch
    // if no components are set, we don't resize at all
    mSize = mTargetSize.isZero() ? textureSize : mTargetSize;

    // if only one component is set, we resize in a way that maintains aspect ratio
    // for SVG rasterization, we always calculate width from rounded height (see comment above)
    if ((mTargetSize.x() == 0) && (mTargetSize.y() != 0))
    {
      mSize[1] = Math::round(mTargetSize.y());
      mSize[0] = (mSize.y() / textureSize.y()) * textureSize.x();
    }
    else if ((mTargetSize.x() != 0) && (mTargetSize.y() == 0))
    {
      mSize[1] = Math::round((mTargetSize.x() / textureSize.x()) * textureSize.y());
      mSize[0] = (mSize.y() / textureSize.y()) * textureSize.x();
    }
  }
}

void VideoComponent::OnPathChanged()
{
  AudioManager::ResumeMusicIfNecessary();
  VideoEngine::Instance().StopVideo(true);
  ResetAnimations();
}

void VideoComponent::setVideo(const Path& path, int delay, int loops, bool decodeAudio)
{
  if (path != mPath || mVideoDelay != delay || mVideoLoop != loops || mDecodeAudio != decodeAudio)
  {
    mPath = path;
    mVideoDelay = delay;
    mVideoLoop = loops;
    mDecodeAudio = decodeAudio;
    OnPathChanged();
  }
}

void VideoComponent::setVideo(const Path& path, bool decodeAudio)
{
  if (path != mPath || mDecodeAudio != decodeAudio)
  {
    mPath = path;
    mDecodeAudio = decodeAudio;
    OnPathChanged();
  }
}

void VideoComponent::setResize(float width, float height)
{
  if (width != mTargetSize.x() || height != mTargetSize.y())
  {
    mTargetSize.Set(width, height);
    resize();
  }
}

void VideoComponent::setKeepRatio(bool keepRatio)
{
  if (mKeepRatio != keepRatio)
  {
    mKeepRatio = keepRatio;
    resize();
  }
}

void VideoComponent::setColorShift(unsigned int color)
{
  mColorShift = color;
  // Grab the opacity from the color shift because we may need to apply it if
  // fading textures in
  mOpacity = color & 0xffU;
  updateColors();
}

void VideoComponent::setOpacity(unsigned char opacity)
{
  if (opacity != mOpacity)
  {
    mOpacity = opacity;
    mColorShift = ((mColorShift >> 8U) << 8U) | mOpacity;
    updateColors();
  }
}

void VideoComponent::updateVertices(double bump)
{
  switch(mEffect)
  {
    case Effect::Bump:
    case Effect::BreakingNews:
    {
      float bumpedWidth = (float)(mSize.x() * bump);
      float bumpedHeight = (float)(mSize.y() * bump);
      float lx = Math::round(mSize.x() / 2.0f - bumpedWidth / 2.0f);
      float ly = Math::round(mSize.y() / 2.0f - bumpedHeight / 2.0f);
      float rx = lx + bumpedWidth;
      float ry = ly + bumpedHeight;

      mVertices[0].Source.Set(lx, ly);
      mVertices[1].Source.Set(lx, ry);
      mVertices[2].Source.Set(rx, ly);

      mVertices[3].Source.Set(rx, ly);
      mVertices[4].Source.Set(lx, ry);
      mVertices[5].Source.Set(rx, ry);
      break;
    }
    case Effect::None:
    case Effect::Fade:
    {
      float x = 0;
      float y = 0;
      float w = Math::round(mSize.x());
      float h = Math::round(mSize.y());

      mVertices[0].Source.Set(x    , y    );
      mVertices[1].Source.Set(x    , y + h);
      mVertices[2].Source.Set(x + w, y    );

      mVertices[3].Source.Set(x + w, y    );
      mVertices[4].Source.Set(x    , y + h);
      mVertices[5].Source.Set(x + w, y + h);
      break;
    }
    case Effect::_LastItem: break;
  }

  for(int i = 6; --i >= 0;)
  {
    mVertices[i + 6].Source = mVertices[i].Source;
    mVertices[i + 6].Source.Y += mSize.y();
  }

  mVertices[0].Target.Set(0.0f, 0.0f);
  mVertices[1].Target.Set(0.0f, 1.0f);
  mVertices[2].Target.Set(1.0f, 0.0f);

  mVertices[3].Target.Set(1.0f, 0.0f);
  mVertices[4].Target.Set(0.0f, 1.0f);
  mVertices[5].Target.Set(1.0f, 1.0f);

  mVertices[6].Target.Set(0.0f, 1.0f);
  mVertices[7].Target.Set(0.0f, 0.0f);
  mVertices[8].Target.Set(1.0f, 1.0f);

  mVertices[9].Target.Set(1.0f, 1.0f);
  mVertices[10].Target.Set(0.0f, 0.0f);
  mVertices[11].Target.Set(1.0f, 0.0f);
}

void VideoComponent::updateColors()
{
  // Regular colors
  Renderer::BuildGLColorArray(mColors, mColorShift, 6);
  // Reflexion top color
  unsigned int color = (mColorShift & 0xFFFFFF00) | (unsigned char)((float)(mColorShift & 0xFF) * mTopAlpha);
  unsigned int colorGl = 0;
  Renderer::ColorToByteArray((GLubyte*)&colorGl, color);
  GLuint* targetColors = (GLuint*)mColors;
  targetColors[6] = targetColors[8] = targetColors[9] = colorGl;
  // Reflexion bottom color
  color = (mColorShift & 0xFFFFFF00) | (unsigned char)((float)(mColorShift & 0xFF) * mBottomAlpha);
  Renderer::ColorToByteArray((GLubyte*)&colorGl, color);
  targetColors[7] = targetColors[10] = targetColors[11] = colorGl;
}

void VideoComponent::ResetAnimations()
{
  { LOG(LogDebug) << "[VideoComponent] Animations reseted!"; }

  mTimer.Initialize(0);
  mState = State::InitializeVideo;
  //{ LOG(LogDebug) << "[VideoComponent] Timer reseted: State::Uninitialized " + DateTime().ToPreciseTimeStamp(); }

  // Stop the video
  if (VideoEngine::Instance().IsPlaying())
    VideoEngine::Instance().StopVideo(false);
}

double VideoComponent::ProcessEffect(int elapsedms, bool in)
{
  double linear = (double) (in ? elapsedms : (mVideoEffect - elapsedms)) / mVideoEffect;

  switch(mEffect)
  {
    case Effect::Bump:
    {
      double sinus = sin((Pi / 2.0) * linear) + sin(Pi * linear) / 2.0;
      return sinus;
    }
    case Effect::Fade:
    case Effect::BreakingNews:
    {
      if (linear < 0.0) linear = 0.0;
      if (linear > 1.0) linear = 1.0;
      return linear;
    }
    case Effect::None:
    case Effect::_LastItem: break;
  }

  return 1.0;
}

bool VideoComponent::ProcessDisplay(double& effect)
{
  bool video = false;
  effect = 0.0;

  if (mState == State::InitializeVideo)
    if (int elapsed = mTimer.GetMilliSeconds(); elapsed >= mVideoDelay && !mPath.IsEmpty())
    {
      mEffect = (Effect)(((int)mEffect + 1) % (int)Effect::_LastItem);
      if ((mAllowedEffects & AllowedEffects::All) != 0)
        while((mAllowedEffects & (1 << (int)mEffect)) == 0)
          mEffect = (Effect)(((int)mEffect + 1) % (int)Effect::_LastItem);
      else
        mEffect = Effect::None;
      // Start video if it's not started yet
      VideoEngine::Instance().PlayVideo(mPath, mDecodeAudio);
      mState = State::WaitForVideoToStart;
      mTimer.Initialize(0);
      //{  LOG(LogDebug) << "[VideoComponent] Timer reseted: State::InitializeVideo " << DateTime().ToPreciseTimeStamp() << " elapsed: "  << elapsed; }
    }

  if (mState == State::WaitForVideoToStart)
    if (VideoEngine::Instance().IsPlaying())
    {
      resize();
      mState = State::StartVideo;
      mTimer.Initialize(0);
      //{ LOG(LogDebug) << "[VideoComponent] Timer reseted: State::BumpVideo " + DateTime().ToPreciseTimeStamp() << " elapsed: " << elapsed; }
      if (mDecodeAudio) AudioManager::PauseMusicIfNecessary();
      if (mInterface != nullptr) mInterface->VideoComponentRequireAction(this, IVideoComponentAction::Action::FadeOut);
    }

  if (mState == State::StartVideo)
  {
    video = true;
    int elapsed = mTimer.GetMilliSeconds();
    effect = ProcessEffect(elapsed, true);
    if (elapsed >= mVideoEffect || mEffect == Effect::None)
    {
      mState = State::DisplayVideo;
      mTimer.Initialize(0);
      //{ LOG(LogDebug) << "[VideoComponent] Timer reseted: State::DisplayVideo " + DateTime().ToPreciseTimeStamp() << " elapsed: " << elapsed; }
    }
  }

  if (mState == State::DisplayVideo)
  {
    // Video only
    video = true;
    effect = 1.0;
    int elapsed = mTimer.GetMilliSeconds();
    if ((mVideoLoop > 0) && (elapsed >= VideoEngine::Instance().GetVideoDurationMs() * mVideoLoop - mVideoEffect))
    {
      mState = State::StopVideo;
      mTimer.Initialize(0);
      //{ LOG(LogDebug) << "[VideoComponent] Timer reseted: State::FinalizeVideo " + DateTime().ToPreciseTimeStamp() << " elapsed: " << elapsed; }
      if (mInterface != nullptr) mInterface->VideoComponentRequireAction(this, IVideoComponentAction::Action::FadeIn);
    }
  }

  if (mState == State::StopVideo)
  {
    video = true;
    int elapsed = mTimer.GetMilliSeconds();
    effect = ProcessEffect(elapsed, false);
    if (elapsed >= mVideoEffect || mEffect == Effect::None)
    {
      mState = State::InitializeVideo;
      VideoEngine::Instance().StopVideo(false);
      AudioManager::ResumeMusicIfNecessary();
      mTimer.Initialize(0);
      //{ LOG(LogDebug) << "[VideoComponent] Timer reseted: State::DisplayImage " + DateTime().ToPreciseTimeStamp() << " elapsed: " << elapsed; }
      video = false;
    }
  }

  return video;
}

void VideoComponent::Update(int elapsed)
{
  Component::Update(elapsed);
}

void VideoComponent::Render(const Transform4x4f& parentTrans)
{
  if (mThemeDisabled) return;

  double effect = 0.0;
  bool display = ProcessDisplay(effect);

  if (display)
  {
    VideoEngine::Instance().AquireTexture();
    GLuint videoFrame = VideoEngine::Instance().GetDisplayableFrame();
    glBindTexture(GL_TEXTURE_2D, videoFrame);
    VideoEngine::Instance().ReleaseTexture();

    // Bounds
    updateVertices(effect);
    // Opacity
    setOpacity((mEffect == Effect::Fade) ? (int)(effect * 255.0) : 255);
    updateColors();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &mVertices[0].Source);

    // Anti-Rotation
    setRotationOrigin(0.5f, 0.5f);
    if (mTopAlpha != 0.f || mBottomAlpha != 0.f)
    {
      setRotation(mEffect == Effect::BreakingNews ? -(float) (Pi * 4.0) * (float) effect : 0.0f);
      mPosition.translateY(mSize.y());
      Transform4x4f trans = parentTrans * getTransform();
      Renderer::SetMatrix(trans);
      mPosition.translateY(-mSize.y());
      glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &mVertices[6].Target);
      glColorPointer(4, GL_UNSIGNED_BYTE, 0, &mColors[6 * 4]);
      glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    // Rotation
    setRotation(mEffect == Effect::BreakingNews ? (float)(Pi * 4.0) * (float)effect : 0.0f);
    Transform4x4f trans = parentTrans * getTransform();
    Renderer::SetMatrix(trans);
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &mVertices[0].Target);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, &mColors[0]);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
  }
}

void VideoComponent::OnApplyThemeElement(const ThemeElement& element, ThemePropertyCategory properties)
{
  if (hasFlag(properties, ThemePropertyCategory::Size))
  {
    Vector2f scale = getParent() != nullptr ? getParent()->getSize() : Vector2f(
      Renderer::Instance().DisplayWidthAsFloat(), Renderer::Instance().DisplayHeightAsFloat());
    if (element.HasProperty(ThemePropertyName::Size))
    {
      setKeepRatio(false);
      setResize(element.AsVector(ThemePropertyName::Size) * scale);
    }
    else if (element.HasProperty(ThemePropertyName::MaxSize))
    {
      setKeepRatio(true);
      setSize(element.AsVector(ThemePropertyName::MaxSize) * scale);
      setResize(element.AsVector(ThemePropertyName::MaxSize) * scale);
    }
    if (element.HasProperty(ThemePropertyName::KeepRatio))
      setKeepRatio(element.AsBool(ThemePropertyName::KeepRatio));
  }

  if (hasFlag(properties, ThemePropertyCategory::Path))
    setVideo(element.HasProperty(ThemePropertyName::Path) ? element.AsPath(ThemePropertyName::Path) : Path::Empty, mDecodeAudio);

  if (hasFlag(properties, ThemePropertyCategory::Color))
    setColorShift(element.HasProperty(ThemePropertyName::Color) ? (unsigned int)element.AsInt(ThemePropertyName::Color) : 0xFFFFFFFF);

  if (hasFlag(properties, ThemePropertyCategory::Effects))
  {
    SetReflection(element.HasProperty(ThemePropertyName::Reflection) ? element.AsVector(ThemePropertyName::Reflection) : Vector2f());
    if (element.HasProperty(ThemePropertyName::Animations))
    {
      mAllowedEffects = AllowedEffects::None;
      for (String& animation: element.AsString(ThemePropertyName::Animations).Split(','))
        if (animation.Trim() == "none") mAllowedEffects = AllowedEffects::None;
        else if (animation.Trim() == "bump") mAllowedEffects |= AllowedEffects::Bump;
        else if (animation.Trim() == "fade") mAllowedEffects |= AllowedEffects::Fade;
        else if (animation.Trim() == "breakingnews") mAllowedEffects |= AllowedEffects::BreakingNews;
    }
    else mAllowedEffects = AllowedEffects::All;
    mVideoLoop = element.HasProperty(ThemePropertyName::Loops) ? element.AsInt(ThemePropertyName::Loops) : DEFAULT_VIDEOLOOP;
    mVideoDelay = element.HasProperty(ThemePropertyName::Delay) ? element.AsInt(ThemePropertyName::Delay) : DEFAULT_VIDEODELAY;
  }
  else
  {
    mAllowedEffects = AllowedEffects::All;
    mVideoLoop = DEFAULT_VIDEOLOOP;
    mVideoDelay = DEFAULT_VIDEODELAY;
  }
  //if (mVideoLoop < 0) mVideoLoop = INT32_MAX;
}

bool VideoComponent::CollectHelpItems(Help& help)
{
  help.Set(Help::Valid(), _("SELECT"));
  return true;
}


