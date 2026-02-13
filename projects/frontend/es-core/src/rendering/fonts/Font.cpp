//
// Created by bkg2k on 10/12/2020.
//

#include <rendering/fonts/Font.h>
#include "rendering/textures/TextureManager.h"
#include "rendering/opengl/Mapper.h"
#include <Renderer.h>

NewFont::NewFont(IFontFaceProvider& faceProvider, float heightInPixel)
  : mFaceProvider(faceProvider)
  , mFace(nullptr)
  , mPath()
  , mRequestedHeightInPixel(heightInPixel)
  , mRealMaxHeightInPixel(0)
  , mTopToBaseline(0)
  , mBottomToBaseline(0)
  , mTextureWidth(0)
  , mTextureHeight(0)
  , mCursorX(0)
  , mCursorY(0)
  , mRowHeight(0)
  , mDefault(true)
{
  assert(heightInPixel > 0 && "Font height <= 0 !");
  // Get font properties
  FT_Face face = FaceForChar(0);
  if (face != nullptr)
  {
    mBottomToBaseline = (int) (-face->size->metrics.descender + 63) >> 6;
    mTopToBaseline = (int) (face->size->metrics.ascender + 63) >> 6;
    mRealMaxHeightInPixel = mTopToBaseline + mBottomToBaseline;
  }
}

NewFont::NewFont(IFontFaceProvider& faceProvider, const Path& fontFile, float heightInPixel)
  : mFaceProvider(faceProvider)
  , mFace(nullptr)
  , mPath(fontFile)
  , mRequestedHeightInPixel(heightInPixel)
  , mRealMaxHeightInPixel(0)
  , mTopToBaseline(0)
  , mBottomToBaseline(0)
  , mTextureWidth(0)
  , mTextureHeight(0)
  , mCursorX(0)
  , mCursorY(0)
  , mRowHeight(0)
  , mDefault(false)
{
  assert(heightInPixel > 0 && "Font heint <= 0 !");
  // Get font properties
  FT_Face face = FaceForChar(0);
  if (face != nullptr)
  {
    mBottomToBaseline = (int) (-face->size->metrics.descender + 63) >> 6;
    mTopToBaseline = (int) (face->size->metrics.ascender + 63) >> 6;
    mRealMaxHeightInPixel = mTopToBaseline + mBottomToBaseline;
  }
}

Glyph* NewFont::Prepare(UnicodeChar unicode, bool embold)
{
  if (!Renderer::IsContextThread() || !Renderer::Instance().HasGLContext()) return nullptr;

  // Bold?
  UnicodeChar unicodeInternal = unicode;
  if (embold) unicodeInternal |= (1 << 31);

  // Already done?
  Glyph* glyph = mGlyphStore.Get(unicodeInternal);
  if (glyph != nullptr)
  {
    if (glyph->Ready()) return glyph;
    // At least one texture of the font has been unloaded
    // Remove all glyph cache to force everything to reload
    LOG(LogWarning) << "[Font] Font texture unloaded - Reset caches!";
    mGlyphStore.CleanUp();
  }

  // Get face for unicode chat
  FT_Face face = FaceForChar(unicode);
  if (face == nullptr) return nullptr;

  // Load the glyph
  if(FT_Load_Char(face, unicode, FT_LOAD_TARGET_NORMAL | FT_LOAD_FORCE_AUTOHINT) != 0)
  {
    LOG(LogError) << "[Font] Error getting character specifications for " << unicode;
    return nullptr;
  }
  // Get the glyph in our own structure
  FT_GlyphSlot faceGlyph = face->glyph;
  FT_Glyph ownGlyph = nullptr;
  FT_Get_Glyph(face->glyph, &ownGlyph);

  if (embold)
    if (ownGlyph->format == FT_GLYPH_FORMAT_OUTLINE)
    {
      FT_OutlineGlyph outGlyph = (FT_OutlineGlyph)ownGlyph;
      FT_Outline_Embolden(&outGlyph->outline, 1 << 6);
    }

  // Render the bitmap
  if (FT_Glyph_To_Bitmap(&ownGlyph, FT_RENDER_MODE_NORMAL, nullptr, 1) != 0)
  {
    LOG(LogError) << "[Font] Error rendering the character " << unicode;
    return nullptr;
  }

  // Get bitmap glyph
  FT_BitmapGlyph bitmapGlyph = (FT_BitmapGlyph)ownGlyph;
  // Make room in the texture
  MakeRoomInTexture(faceGlyph, (int)bitmapGlyph->bitmap.width, (int)bitmapGlyph->bitmap.rows);
  // Render glyph in the texture
  RenderGlyphInTexture(bitmapGlyph);
  // Create our glyph
  glyph = mGlyphStore.New(unicodeInternal)->Set(mCurrentTexture, mTextureWidth, mTextureHeight, mCursorX, mCursorY, faceGlyph, bitmapGlyph, embold);
  // Update cursor
  mCursorX += (int)bitmapGlyph->bitmap.width + 1;

  // Free glyph
  FT_Done_Glyph(ownGlyph);

  return glyph;
}

FT_Face NewFont::FaceForCharFallback(UnicodeChar unicode)
{
  // Lookup in a list of known fonts
  static Path originalPath[] =
    {
      Path(":/Recalbox_icons.ttf"),
      Path(":/droidsansfallback.ttf"),// japanese, chinese, korean
      Path(":/dejavusanscondensed.ttf"),
      Path(":/ubuntu_condensed.ttf"),
    };
  for(const Path& path : originalPath)
  {
    FT_Face face = mFaceProvider.GetFontFace(path, mRequestedHeightInPixel);
    if (face != nullptr)
      if (unicode == 0 || FT_Get_Char_Index(face, unicode) != 0)
        return face;
  }

  // No luck :(
  //{ LOG(LogError) << "[Font] No font face available for unicode character " << unicode; }
  return nullptr;
}

FT_Face NewFont::FaceForChar(UnicodeChar unicode)
{
  if (!mDefault)
  {
    // Non default font may have a cached Face
    if (mFace != nullptr)
      if (unicode == 0 || FT_Get_Char_Index(mFace, unicode) != 0)
        return mFace;

    // Get from face provider and cache the new face
    // Face provider may return nullptr if the font file is not available
    mFace = mFaceProvider.GetFontFace(mPath, mRequestedHeightInPixel);
    if (unicode == 0 || FT_Get_Char_Index(mFace, unicode) != 0)
      return mFace;

    return FaceForCharFallback(unicode);
  }

  // Lookup in default font first
  FT_Face face = mFaceProvider.GetDefaultFontFace(mRequestedHeightInPixel);
  if (face != nullptr)
    if (unicode == 0 || FT_Get_Char_Index(face, unicode) != 0)
      return face;

  return FaceForCharFallback(unicode);
}

void NewFont::MakeRoomInTexture(FT_GlyphSlot glyph, int w, int h)
{
  // Create texture if needed
  if (!mCurrentTexture.Valid())
    MakeNewTexture(glyph->face);

  // No more room on the current row?
  if (mCursorX + w > mTextureWidth)
  {
    // Start a new row
    mCursorX = 0;
    mCursorY += mRowHeight;

    // No more room in the current texture?
    if (mCursorY + h > mTextureHeight)
    {
      MakeNewTexture(glyph->face);
      mCursorX = mCursorY = mRowHeight = 0;
    }
  }

  // Increase row when required
  if (h > mRowHeight)
    mRowHeight = h;
}

void NewFont::MakeNewTexture(FT_Face face)
{
  // Get max width/height for this font
  int MaxWidthInPixel = (int)(face->size->metrics.max_advance + 63) >> 6;
  int MaxHeightInPixel = (int)((face->size->metrics.ascender - face->size->metrics.descender) + 63) >> 6;

  // Compute raw texture size clamped to reasonable min size and GPU max size
  int rawTextureWidth  = Math::clampi(MaxWidthInPixel * 8, 256, 1024);
  int rawTextureHeight = Math::clampi(MaxWidthInPixel * 16, 256, 1024);

  // Adjust texture size if they have been clamped
  int horizontalChars = (rawTextureWidth / MaxWidthInPixel);
  int verticalChars   = (rawTextureHeight / MaxWidthInPixel);
  mTextureWidth    = horizontalChars * MaxWidthInPixel;
  mTextureHeight   = verticalChars * MaxHeightInPixel;

  // Let's try to optimize event more.
  // For common usage, we probably don't need more than 128 char maximum
  // So try to reduce the width (to minimize pixel loss) accordingly
  int totalChars = horizontalChars * verticalChars;
  if (totalChars > 128)
    mTextureWidth /= totalChars / 128;

  // Add empty lines

  TextureHolder::Properties properties = TextureHolder::Properties::Free |
                                         TextureHolder::Properties::Alpha |
                                         TextureHolder::Properties::NoFiltering |
                                         TextureHolder::Properties::NoCache;
  mCurrentTexture = TextureManager::Instance().Create(Path::Empty, mTextureWidth, mTextureHeight, properties);
}

void NewFont::RenderGlyphInTexture(FT_BitmapGlyph glyphBitmap)
{
  ITextureCacheControl& cache = TextureManager::Instance();

  while(!mCurrentTexture.UploadAlphaPart(mCursorX, mCursorY,
                                         (int)glyphBitmap->bitmap.width,
                                         (int)glyphBitmap->bitmap.rows,
                                         glyphBitmap->bitmap.buffer))
  {
    { LOG(LogDebug) << "[Font] Cannot upload glyph, remove old textures."; }
    if (!cache.UnloadTheOldest(false))
      break; // Cache cannot be emptied anymore.
  }
}

NewFont::TextChunkList NewFont::SplitText(const String& string, int startInString, int stringLength, int offset, int widthInPixel, const FontEffect& effect)
{
  TextChunkList result;
  result.RefString = string.SubString(startInString, stringLength);
  result.Effect = effect;
  FontEffect::Final final = effect.Finalize();
  offset = Math::clampi(offset, 0, widthInPixel - 1);

  float limit = (float)widthInPixel - .5f;
  int start = startInString;
  int end = start + stringLength;
  for(; start < end; )
  {
    int position = start; // Make position starting from start
    int cut = 0;          // Last cut character encountered
    UnicodeChar c = 0;    // Current char
    if (offset == 0)                                        // Delete dead char only on line start
      do
      {
        if (position >= end) break;                         // out of data
        start = position;                                   // Make start following position to skip dead characters
        c = string.ReadUTF8(position);                      // Extract next character
      } while(c == ' ' || c == '\t' );                      // Repeat while the fetched char is a dead char
    if (c == ' ' || c == '\t' ) break;                      // out of data w/o non-dead characters, exit

    // loop next characters until \n or reaching limits
    float width = (float)offset;                            // Initialize width and width until last cut character
    float cutWidth = 0.f;
    float prevPeak = 0.f;
    position = start;                                       // Restart at last non-dead character
    for(;;)
    {
      int previousPosition = position;                      // Store position before reading next char
      c = string.ReadUTF8(position);                        // Extract next character
      if ((c == ' ' || c == '\t') /*&& (previousPosition > start)*/) // Record any space/tab as cut position
      {
        cut = previousPosition;                             // Record last position before cut character
        cutWidth = width + prevPeak;
      }
      if (c == '\n') { cut = position; break; }             // If we've found a CR, just cut after the CR and exit loop
      Glyph* glyph = Prepare(c, final.Bold);                // Get glyph
      if (glyph == nullptr) continue;                       // Bypass unknown character
      float advance = glyph->mAdvanceX * final.HorizontalScaling;
      float peak = (final.Inclination * glyph->mHP) * final.HorizontalScaling;
      if (width += advance; width + peak > limit)
      {                                                     // Limit reached
        if (cut != 0) width = cutWidth;                     // Cut character has been recorded, take its width
        else
        {
          cut = previousPosition;                           // No cut character have been recorded, just cut
          if (cut == start && offset == 0) cut++;
          width += prevPeak - advance;                      // at the previous position & remove last char
        }
        break;                                              // exit loop
      }
      if (position >= end) { cut = position; break; }       // This is the end
      prevPeak = peak;
    }
    // If cut is still 0, we're at the end
    if (cut == 0) cut = position;

    // Record chunk - cut must have been set when we reach this code
    if (cut - start > 0)
      result.Chunks.Add({ .Start=start, .Length=cut - start, .Width=Math::roundi(width + .5f) - offset });

    // Restart
    start = cut;
    offset = 0;
  }

  return result;
}

void NewFont::MapGlyphTexture(const Glyph& glyph, float x, float y, const FontEffect::Final& final, Vertex::Rectangle& vertex)
{
  x += Math::round(glyph.mBearingX * final.HorizontalScaling);
  y -= Math::round(glyph.mBearingY * final.VerticalScaling);

  // First target triangle and opposite corner
  vertex[0].Target.X = vertex[1].Target.X = x;
  vertex[2].Target.X = vertex[5].Target.X = x + glyph.mWP * final.HorizontalScaling;
  vertex[0].Target.Y = vertex[2].Target.Y = y;
  vertex[1].Target.Y = vertex[5].Target.Y = y + glyph.mHP * final.VerticalScaling;
  vertex[0].Target.X += final.Inclination * glyph.mHP * final.HorizontalScaling;
  vertex[2].Target.X += final.Inclination * glyph.mHP * final.HorizontalScaling;

  // First texture triangle and opposite corner
  vertex[0].Source.X = vertex[1].Source.X = glyph.mX;
  vertex[2].Source.X = vertex[5].Source.X = glyph.mX + glyph.mW;
  vertex[0].Source.Y = vertex[2].Source.Y = glyph.mY;
  vertex[1].Source.Y = vertex[5].Source.Y = glyph.mY + glyph.mH;

  // Copy identical corners
  vertex[3] = vertex[2];
  vertex[4] = vertex[1];
}

void NewFont::DrawChar(UnicodeChar character, int x, int y, Colors::ColorARGB color, ::Alignment origin)
{
  DrawChar(character, x, y, color, origin, mDefaultEffect);
}

void NewFont::DrawChar(UnicodeChar character, int x, int y, Colors::ColorARGB color, ::Alignment origin, const FontEffect& effect)
{
  FontEffect::Final final = effect.Finalize();
  Glyph* glyph = Prepare(character, final.Bold);
  if (glyph == nullptr) return;

  // Adjust coordinates
  float xx = (float)x - (glyph->mBearingX * final.HorizontalScaling);
  float yy = (float)y + (glyph->mBearingY * final.VerticalScaling);
  switch(AlignmentExtractHorizontal(origin))
  {
    case HorizontalAlignment::Left  : break;
    case HorizontalAlignment::Center: xx -= glyph->mWP * final.HorizontalScaling / 2.f; break;
    case HorizontalAlignment::Right : xx -= glyph->mWP * final.HorizontalScaling; break;
  }
  switch(AlignmentExtractVertical(origin))
  {
    case VerticalAlignment::Top   : break;
    case VerticalAlignment::Center: yy -= glyph->mHP * final.VerticalScaling / 2.f; break;
    case VerticalAlignment::Bottom: yy -= glyph->mHP * final.VerticalScaling; break;
  }

  // Build vertexes
  Vertex::Rectangle vertexes;
  MapGlyphTexture(*glyph, xx, yy, final, vertexes);

  // Draw!
  glyph->mTexture.Render(vertexes, color, Vertex::sVertexPerRectangle, false);
}

int NewFont::DrawCharBaseline(UnicodeChar character, int x, int y, Colors::ColorARGB color, const FontEffect& effects)
{
  FontEffect::Final final = effects.Finalize();
  Glyph* glyph = Prepare(character, final.Bold);
  if (glyph == nullptr) return 0;

  // Adjust coordinates
  float xx = (float)x - (glyph->mBearingX * final.HorizontalScaling);
  float yy = (float)y;
  float width = glyph->mWP * final.HorizontalScaling;

  // Build vertexes
  Vertex::Rectangle vertexes;
  MapGlyphTexture(*glyph, xx, yy, final, vertexes);

  // Draw!
  glyph->mTexture.Render(vertexes, color, Vertex::sVertexPerRectangle, false);
  return (int)width;
}

int NewFont::CharWidth(unsigned int unicode, const FontEffect& effect)
{
  float result = 0;
  FontEffect::Final final = effect.Finalize();
  const Glyph* glyph = Prepare(unicode, final.Bold);
  if (glyph != nullptr)
  {
    float width = glyph->mAdvanceX * final.HorizontalScaling;
    float peak = (final.Inclination * glyph->mHP) * final.HorizontalScaling;
    result = width + peak;
  }
  return Math::roundi(result + .5f);
}

int NewFont::TextWidth(const Array<unsigned int>& unicodes, int start, int length, const FontEffect& effect)
{
  float result = 0;
  float width = 0;
  start = Math::clampi(start, 0, unicodes.Count());
  int end = Math::clampi(start + length, start, unicodes.Count());
  FontEffect::Final final = effect.Finalize();
  for(--start; ++start < end; )
  {
    const Glyph* glyph = Prepare(unicodes[start], final.Bold);
    if (glyph != nullptr)
    {
      width += glyph->mAdvanceX * final.HorizontalScaling;
      float peak = (final.Inclination * glyph->mHP) * final.HorizontalScaling;
      if (width + peak > result) result = width + peak;
    }
  }
  return Math::roundi(result + .5f);
}

int NewFont::TextWidth(const String& string, int start, int length, const FontEffect& effect)
{
  float result = 0;
  float width = 0;
  int end = start + length;
  FontEffect::Final final = effect.Finalize();
  for(int position = start; position < end; )
  {
    String::Unicode unicode = string.ReadUTF8(position);
    if (unicode == 0) break;
    const Glyph* glyph = Prepare(unicode, final.Bold);
    if (glyph != nullptr)
    {
      width += glyph->mAdvanceX * final.HorizontalScaling;
      float peak = (final.Inclination * glyph->mHP) * final.HorizontalScaling;
      if (width + peak > result) result = width + peak;
    }
  }
  return Math::roundi(result + .5f);
}

String NewFont::TextEllipsis(const String& string, const FontEffect& effect, int maxWidth)
{
  String result;
  float finalWidth = 0;
  float width = 0;
  int end = string.Count();
  FontEffect::Final final = effect.Finalize();
  const Glyph* ellipsis = Prepare(0x2026, final.Bold);
  float ellipsisWidth = ellipsis != nullptr ?(ellipsis->mAdvanceX * final.HorizontalScaling) + ((final.Inclination * ellipsis->mHP) * final.HorizontalScaling) : 0;
  for(int position = 0; position < end; )
  {
    String::Unicode unicode = string.ReadUTF8(position);
    if (unicode == 0) break;
    const Glyph* glyph = Prepare(unicode, final.Bold);
    if (glyph != nullptr)
    {
      width += glyph->mAdvanceX * final.HorizontalScaling;
      float peak = (final.Inclination * glyph->mHP) * final.HorizontalScaling;
      if (width + peak > finalWidth) finalWidth = width + peak;
      if (finalWidth + ellipsisWidth > (float)maxWidth)
      {
        result.AppendUTF8(0x2026);
        break;
      }
    }
    result.AppendUTF8(unicode);
  }
  return result;
}

void NewFont::DrawTextRaw(const String& string, int start, int length, int x, int y, Colors::ColorARGB color, const FontEffect& effect)
{
  if (length <= 0) return;

  float xf = (float)x;
  float yf = (float)y;

  // Y Clipping according to current clipping area
  Rectangle currentClipping = Renderer::Instance().CurrentClipping();
  if (yf - (float)mTopToBaseline + (float)mRealMaxHeightInPixel < currentClipping.Top() || yf - (float)mTopToBaseline > currentClipping.Bottom()) return;

  // Cache size
  static constexpr int sCacheSize = 512;
  // Prepare uninitialized vertexes
  Vertex::Rectangle vertexes[sCacheSize];

  // Character display loop
  int end = start + length;

  // Character loop
  FontEffect::Final final = effect.Finalize();
  for(int position = start; position < end; )
  {
    // Seek first displayable character
    Glyph* lastValidGlyph = nullptr;
    while (lastValidGlyph == nullptr && position < end)
      lastValidGlyph = Prepare(string.ReadUTF8(position), final.Bold);
    if (lastValidGlyph == nullptr) break; // No more displayable character

    // Store 1st character
    int previousTexture = lastValidGlyph->mTexture.Identifier();
    MapGlyphTexture(*lastValidGlyph, /*Math::round(xf), Math::round(yf)*/ xf, yf, final, vertexes[0]);
    xf += lastValidGlyph->mAdvanceX * final.HorizontalScaling;

    // Seek next characters
    int index = 1;
    while (position < end)
    {
      int previousPosition = position;
      Glyph* glyph = Prepare(string.ReadUTF8(position), final.Bold);
      if (glyph == nullptr) continue;
      if (previousTexture != glyph->mTexture.Identifier())
      {
        position = previousPosition;
        break;
      }
      MapGlyphTexture(*glyph, /*Math::round(xf), Math::round(yf)*/ xf, yf, final, vertexes[index]);
      xf += glyph->mAdvanceX * final.HorizontalScaling;
      if (++index == sCacheSize) break;
    }

    // Draw characters
    if (index != 0)
      lastValidGlyph->mTexture.Render(vertexes[0], color, Vertex::sVertexPerRectangle * index, false);
  }
}

void NewFont::DrawText(const String& string, int start, int length, int x, int y, Colors::ColorARGB color, ::Alignment alignment, const FontEffect& effect)
{
  switch(AlignmentExtractHorizontal(alignment))
  {
    case HorizontalAlignment::Left: break;
    case HorizontalAlignment::Center: x-= TextWidth(string, start, length, effect) / 2; break;
    case HorizontalAlignment::Right: x -= TextWidth(string, start, length, effect); break;
  }

  float yScale = effect.Finalize().VerticalScaling;
  y += (int)((float)mTopToBaseline * yScale);
  switch(AlignmentExtractVertical(alignment))
  {
    case VerticalAlignment::Top: break;
    case VerticalAlignment::Center: y -= mRealMaxHeightInPixel / 2; break;
    case VerticalAlignment::Bottom: y -= mRealMaxHeightInPixel; break;
  }

  DrawTextRaw(string, start, length, x, y, color, effect);
}

void NewFont::DrawText(const String& string, int start, int length, const Rectangle& into, float xoffset, float yoffset, Colors::ColorARGB color, ::Alignment alignment,
                       const FontEffect& effect)
{
  // Position
  float x = into.Left();
  switch(AlignmentExtractHorizontal(alignment))
  {
    case HorizontalAlignment::Left: break;
    case HorizontalAlignment::Center: x+= (into.Width() - (float)TextWidth(string, start, length, effect)) / 2; break;
    case HorizontalAlignment::Right: x+= into.Width() - (float)TextWidth(string, start, length, effect); break;
  }
  float y = into.Top();
  float yScale = effect.Finalize().VerticalScaling;
  switch(AlignmentExtractVertical(alignment))
  {
    case VerticalAlignment::Top: y += (float)mTopToBaseline * yScale; break;
    case VerticalAlignment::Center: y += (float)mTopToBaseline * yScale + (into.Height() - (float)mRealMaxHeightInPixel * yScale) / 2.f; break;
    case VerticalAlignment::Bottom: y += into.Height() - (float)mBottomToBaseline * yScale; break;
  }

  // Clipping + rendering
  Renderer::Instance().Clip(into);
  DrawTextRaw(string, start, length, (int)(x + xoffset), (int)(y + yoffset), color, effect);
  Renderer::Instance().Unclip();
}

void NewFont::DrawText(NewFont::TextChunkList& strings, int x, int y, Colors::ColorARGB color, ::HorizontalAlignment alignment, float interline)
{
  int realHeight = Math::roundi((float)mRealMaxHeightInPixel * (1.f + interline));
  y += strings.Chunks.Count() * realHeight;
  for(int i = strings.Chunks.Count(); --i >= 0; )
  {
    const TextChunk& chunk = strings.Chunks[i];
    DrawTextRaw(strings.RefString, chunk.Start, chunk.Length,
                alignment == HorizontalAlignment::Left ? x : x - (alignment == HorizontalAlignment::Center ? chunk.Width / 2 : chunk.Width),
                y -= realHeight, color, strings.Effect);
  }
}

void NewFont::DrawText(NewFont::TextChunkList& strings, const Rectangle& into, float xoffset, float yoffset,
                       Colors::ColorARGB color, ::Alignment alignment, float interline)
{
  // Position
  float x = into.Left();
  HorizontalAlignment hz = AlignmentExtractHorizontal(alignment);
  switch(hz)
  {
    case HorizontalAlignment::Left: break;
    case HorizontalAlignment::Center: x += into.Width() / 2.f; break;
    case HorizontalAlignment::Right: x += into.Width(); break;
  }
  float y = into.Top();
  float totalHeight = ((float)((strings.Chunks.Count() - 1) * mRealMaxHeightInPixel) * (1.f + interline)) + (float)mRealMaxHeightInPixel;
  switch(AlignmentExtractVertical(alignment))
  {
    case VerticalAlignment::Top: break;
    case VerticalAlignment::Center: y += (into.Height() - totalHeight) / 2.f; break;
    case VerticalAlignment::Bottom: y += into.Height() - totalHeight; break;
  }
  y += (float)mTopToBaseline;

  Renderer::Instance().Clip(into);
  for(const TextChunk& chunk :  strings.Chunks)
  {
    // Get real x/y
    float xx = ((hz == HorizontalAlignment::Left ? x : x - (float)(hz == HorizontalAlignment::Center ? chunk.Width / 2 : chunk.Width)) + xoffset);
    float yy = y + yoffset;
    y += (float)mRealMaxHeightInPixel * (1.f + interline);
    // Y clipping
    if (yy + (float)mBottomToBaseline < into.Top()) continue; // Not yet into visible area
    if (yy - (float)mTopToBaseline > into.Bottom()) break;    // Reverse draw => out of area
    // Draw
    DrawTextRaw(strings.RefString, chunk.Start, chunk.Length, (int)xx, (int)yy, color, strings.Effect);
  }
  Renderer::Instance().Unclip();
}

void NewFont::ClearCaches()
{
  // Clear default texture
  mCurrentTexture.Clear();
  // Clear store & free all glyph texture
  mGlyphStore.CleanUp();
  // Reset current texture pointers
  mTextureWidth = 0;
  mTextureHeight = 0;
  mCursorX = 0;
  mCursorY = 0;
  mRowHeight = 0;
}

#ifdef DEBUG
void NewFont::RenderFontCache()
{
  if (mCurrentTexture.Valid())
  {
    Vertex::Rectangle vertexes;
    Mapper::MapTexture(Rectangle(mCurrentTexture.Size().toFloat()), vertexes);
    Renderer::SetMatrix(Transform4x4f::Identity());
    mCurrentTexture.Render(vertexes, Colors::Common::White, Vertex::sVertexPerRectangle, false);
  }
}
#endif
