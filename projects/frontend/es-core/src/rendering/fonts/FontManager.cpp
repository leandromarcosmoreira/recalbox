//
// Created by bkg2k on 10/12/2020.
//

#include <utils/Files.h>
#include <utils/math/Misc.h>
#include <rendering/fonts/FontManager.h>
#include <Renderer.h>
#include "FontProperties.h"
#include <hardware/Board.h>

FontManager::FontManager()
  : StaticLifeCycleControler<FontManager>("FontManager")
  , mLibrary(nullptr)
{
  if (FT_Init_FreeType(&mLibrary) != 0)
  {
    mLibrary = nullptr;
    { LOG(LogFatal) << "[Font] Fatal Error initializing FreeType!"; }
  }
}

FontManager::~FontManager()
{
  // Delete font
  for(auto& font : mFontCollection)
    delete font.second;
  mFontCollection.clear();

  // Dalete font faces
  for(auto& face : mFaceCollection)
    FT_Done_Face(face.second);
  mFaceCollection.clear();

  // Deinit library
  FT_Done_FreeType(mLibrary);
  mLibrary = nullptr;
}

String FontManager::FormatIdentifier(const Path& fontFile, float heightInPixel)
{
  return String(heightInPixel,4)
         .Append('|')
         .Append(fontFile.ToString());
}

NewFont& FontManager::FromTemplate(const Path& fontFile, FontTemplate font, float factor)
{
  if (Board::Instance().CrtBoard().IsCrtAdapterAttached())
  {
    FontProperties properties(fontFile, 7);
    switch(font)
    {
      case FontTemplate::Tiny:
      case FontTemplate::Small:      break;
      case FontTemplate::Medium:     properties = FontProperties(fontFile, properties.HeightFactor() * 2); break;
      case FontTemplate::Large:      properties = FontProperties(fontFile, properties.HeightFactor() * 3); break;
      case FontTemplate::Huge:       properties = FontProperties(fontFile, properties.HeightFactor() * 4); break;
      case FontTemplate::SmallFixed: break;
    }
    return fontFile.IsEmpty() ? FromDefault(properties.HeightFactor() * factor, false) :
           FromFile(properties.GetPath(), properties.HeightFactor() * factor, false);
  }

  FontProperties properties(fontFile, 0.035f);
  switch(font)
  {
    case FontTemplate::Tiny:       properties = FontProperties(fontFile, 0.030f); break;
    case FontTemplate::Small:      break;
    case FontTemplate::Medium:     properties = FontProperties(fontFile, 0.045f); break;
    case FontTemplate::Large:      properties = FontProperties(fontFile, 0.065f); break;
    case FontTemplate::Huge:       properties = FontProperties(fontFile, 0.085f); break;
    case FontTemplate::SmallFixed: properties = FontProperties(Path(sFixedFont), 0.021f); break;
  }
  return fontFile.IsEmpty() ? FromDefault(properties.HeightFactor() * factor, false) :
         FromFile(properties.GetPath(), properties.HeightFactor() * factor, false);
}

NewFont& FontManager::FromTemplateResized(const Path& fontFile, FontTemplate font, float size)
{
  if (Board::Instance().CrtBoard().IsCrtAdapterAttached())
    return FromTemplate(fontFile, font, 1.f);

  static constexpr float defaultSize = 0.030f;

  FontProperties properties(fontFile, size);
  switch(font)
  {
    case FontTemplate::Tiny:       break;
    case FontTemplate::Small:      properties = FontProperties(fontFile, size * (0.035f / defaultSize)); break;
    case FontTemplate::Medium:     properties = FontProperties(fontFile, size * (0.045f / defaultSize)); break;
    case FontTemplate::Large:      properties = FontProperties(fontFile, size * (0.065f / defaultSize)); break;
    case FontTemplate::Huge:       properties = FontProperties(fontFile, size * (0.085f / defaultSize)); break;
    case FontTemplate::SmallFixed: properties = FontProperties(Path(sFixedFont), size * (0.021f / defaultSize)); break;
  }
  return fontFile.IsEmpty() ? FromDefault(properties.HeightFactor(), false) :
         FromFile(properties.GetPath(), properties.HeightFactor(), false);
}

NewFont& FontManager::FromFont(const NewFont& from, float size, bool sizeInPixels)
{
  return FromFile(from.GetPath(), size, sizeInPixels);
}

NewFont& FontManager::FromFile(const Path& fontFile, float size, bool sizeInPixels)
{
  float minScreen = Math::min(Renderer::Instance().DisplayWidthAsFloat(), Renderer::Instance().DisplayHeightAsFloat());
  float heightInPixel = sizeInPixels ? size : (size >= 1.f ? size * Math::ceil(minScreen / 288.f) : minScreen * size);

  // Get font identifier
  String fontIdentifier = FormatIdentifier(fontFile, heightInPixel);

  // Lookup font
  NewFont** font = mFontCollection.try_get(fontIdentifier);
  if (font != nullptr)
    return **font;

  // Create & store new font
  heightInPixel = Math::round(heightInPixel);
  NewFont* newFont = fontFile.IsEmpty() ? new NewFont(*this, heightInPixel) : new NewFont(*this, fontFile, heightInPixel);
  mFontCollection[fontIdentifier] = newFont;
  LOG(LogDebug) << "[Font] Created font " << (fontFile.IsEmpty() ? "default"  : fontFile.ToString()) << ':' << heightInPixel;
  return *newFont;
}

FT_Face FontManager::GetFontFace(const Path& path, float heightInPixel)
{
  // Get font identifier
  String fontIdentifier = FormatIdentifier(path, heightInPixel);

  // Lookup face
  FT_Face* face = mFaceCollection.try_get(fontIdentifier);
  if (face != nullptr)
    return *face;

  // Face is not ready, let's try getting font data
  const String* data = mFaceData.try_get(path.IsEmpty() ? String(sInternalFont) : path.ToString());
  if (data == nullptr)
  {
    String rawdata = Files::LoadFile(path.IsEmpty() ? Path(sInternalFont) : path);
    if (rawdata.empty())
    {
      LOG(LogError) << "[Font] Font file not found " << path.ToString();
      return nullptr;
    }
    mFaceData[path.ToString()] = rawdata;
    data = mFaceData.try_get(path.ToString());
  }
  // Create new Face
  FT_Face newFace = nullptr;
  if (FT_New_Memory_Face(mLibrary, (const FT_Byte*)data->data(), (FT_Long)data->size(), 0, &newFace) != FT_Err_Ok)
  {
    LOG(LogError) << "[Font] Error initializing font face for " << (path.IsEmpty() ? "default" : path.ToChars());
    return nullptr;
  }
  if (FT_Set_Pixel_Sizes(newFace, 0, (unsigned int)Math::round(heightInPixel)) != FT_Err_Ok)
  {
    LOG(LogError) << "[Font] Error initializing font face size for " << (path.IsEmpty() ? "default" : path.ToChars()) << ':' << heightInPixel;
    return nullptr;
  }
  // Store new face
  mFaceCollection[fontIdentifier] = newFace;
  return newFace;
}

void FontManager::ClearFontCaches()
{
  for(auto& font : mFontCollection)
    font.second->ClearCaches();
}

NewFont& FontManager::FromTheme(const ThemeElement& elem, ThemePropertyCategory properties, NewFont* orig)
{
  if (!hasFlag(properties, ThemePropertyCategory::Font) && orig != nullptr) return *orig;

  float size = (orig != nullptr ? (float)orig->RequestedHeight() : FONT_SIZE_MEDIUM);
  Path path = (orig != nullptr ? orig->GetPath() : Path::Empty);

  if (elem.HasProperty(ThemePropertyName::FontSize))
  {
    size = elem.AsFloat(ThemePropertyName::FontSize);
    if (size >= 1.f)
    {
      // As the size is given as an integer as the 240p size reference, we use it as absolute size.
      // and then we adapt to the screen height (x1 up to 288p, x2 up to 576, ....)
      size *= Math::ceil(Renderer::Instance().IsRotatedSide() ? Renderer::Instance().DisplayWidthAsFloat() / 288 : Renderer::Instance().DisplayHeightAsFloat() / 288);
    }
  }
  if (elem.HasProperty(ThemePropertyName::FontPath))
    path = Path(elem.AsString(ThemePropertyName::FontPath));

  if (size >= 1.f)
    size /= Math::min(Renderer::Instance().DisplayWidthAsFloat(), Renderer::Instance().DisplayHeightAsFloat());

  return FromFile(path, size, false);
}
