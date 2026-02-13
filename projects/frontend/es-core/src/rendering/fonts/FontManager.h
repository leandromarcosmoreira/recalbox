//
// Created by bkg2k on 10/12/2020.
//
#pragma once

#include <utils/cplusplus/StaticLifeCycleControler.h>
#include <utils/storage/HashMap.h>
#include <rendering/fonts/Font.h>
#include <rendering/fonts/IFontFaceProvider.h>
#include "FontTemplate.h"
#include <themes/ThemeElement.h>
#include <themes/PropertyCategories.h>

class FontManager : public StaticLifeCycleControler<FontManager>
                  , public IFontFaceProvider
{
  public:
    //! Constructor
    FontManager();

    //! Destructor
    ~FontManager();

    //! Get default font
    NewFont& Default() { return FromFile(Path(), NewFont::sDefaultFontSize, false); }

    //! Get default font
    NewFont& FromDefault(float size, bool sizeInPixels) { return FromFile(Path(), size, sizeInPixels); }

    //! Get font from specified file or resource
    NewFont& FromFile(const Path& fineFile, float size, bool sizeInPixels);

    //! Get font from specified file or resource
    NewFont& FromFont(const NewFont& from, float size, bool sizeInPixels);

    //! Get default font
    //NewFont& FromDefault(const NewFont& sizeTemplate);

    //! Get font from specified file or resource
    //NewFont& FromFile(const Path& fineFile, const NewFont& sizeTemplate);

    //! Get font from specified template with factor
    NewFont& FromTemplate(const Path& fontFile, FontTemplate font, float factor);

    //! Get font from specified template
    NewFont& FromTemplate(const Path& fontFile, FontTemplate font) { return FromTemplate(fontFile, font, 1.f); }

    //! Get font from specified template
    NewFont& FromTemplate(FontTemplate font) { return FromTemplate(Path::Empty, font, 1.f); }

    //! Get font from specified template with factor
    NewFont& FromTemplateResized(const Path& fontFile, FontTemplate font, float factor);

    //! Clear all font's internal caches
    void ClearFontCaches();

    /*!
     * @brief Build new font from theme properties & optional original font
     * @param elem Theme element
     * @param properties Theme property category
     * @param orig Original font
     * @return New font
     */
    NewFont& FromTheme(const ThemeElement& elem, ThemePropertyCategory properties, NewFont* orig);

  private:
    //! Internal font w/ icons
    static constexpr const char* sInternalFont = ":/Recalbox_icons.ttf";
    //! Default fixed font
    static constexpr const char* sFixedFont = ":/UbuntuMonoR.ttf";

    //! Fonts : Identifier/Font instance
    HashMap<String, NewFont*> mFontCollection;
    //! Fonts faces Identifier/Font face
    HashMap<String, FT_Face> mFaceCollection;
    //! Font data Path/Data
    HashMap<String, String> mFaceData;

    //! Free type library handler
    FT_Library mLibrary;

    /*!
     * @brief Get an unique identifier for the given font & size
     * @param fineFile Font path
     * @param heightInPixel Font size
     * @return Unique identifier
     */
    static String FormatIdentifier(const Path& fontFile, float heightInPixel);

    /*
     * IFontFaceProvider implementation
     */

    /*!
     * @brief Get default font face
     * @return Default font face - Never fails, always return a valid face
     */
    FT_Face GetDefaultFontFace(float heightInPixel) override { return GetFontFace(Path(), heightInPixel); }

    /*!
     * @brief Get specific font face
     * @param path Font file path
     * @return Font face - May be nullptr
     */
    FT_Face GetFontFace(const Path& path, float heightInPixel) override;
};
