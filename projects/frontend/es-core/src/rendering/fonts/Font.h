//
// Created by bkg2k on 10/12/2020.
//
#pragma once

#include <utils/os/fs/Path.h>
#include <rendering/fonts/IFontFaceProvider.h>
#include <rendering/fonts/GlyphStorage.h>
#include <rendering/textures/Texture.h>
#include <rendering/fonts/Alignment.h>
#include "FontEffect.h"
#include "rendering/opengl/Colors.h"
#include "rendering/opengl/Rectangle.h"
#include <Renderer.h>

#define FONT_SIZE_EXTRASMALL (0.030f)
#define FONT_SIZE_SMALL (0.035f)
#define FONT_SIZE_MEDIUM (0.045f)
#define FONT_SIZE_LARGE (0.085f)

#define FONT_PATH_LIGHT ":/ubuntu_condensed.ttf"
#define FONT_PATH_REGULAR ":/ubuntu_condensed.ttf"
#define FONT_PATH_CRT ":/f8bitfortressplus.ttf"

class NewFont : private INoCopy
{
  public:
    //! Default font size, in screen height ratio
    static constexpr const float sDefaultFontSize = .035f;

    //! Chuck of text
    struct TextChunk
    {
      int Start;  //!< Start offset
      int Length; //!< Chunk length
      int Width;  //!< Width in pixel
    };
    //! List of Chunks
    struct TextChunkList
    {
      String RefString;        //!< Reference string
      FontEffect Effect;       //!< Font effect
      Array<TextChunk> Chunks; //!< Chunks in the reference string

      int Width() const { int w; for(const TextChunk& c : Chunks) w = Math::max(w, c.Width); return w; }
      int Height(float lineScale) const { return Math::roundi((float)Chunks.Count() * lineScale); }
    };

    /*!
     * @brief Create default font
     * @param faceProvider Font face provider
     * @param heightInPixel Required height in pixel
     */
    NewFont(IFontFaceProvider& faceProvider, float heightInPixel);

    /*!
     * @brief Create specific font
     * @param faceProvider Font face provider
     * @param fontFile Font path or resource path
     * @param heightInPixel Required height in pixel
     */
    NewFont(IFontFaceProvider& faceProvider, const Path& fontFile, float heightInPixel);

    /*!
     * @brief Get the with in pixel of the given string
     * @param string String to measure
     * @return Width in pixel
     */
    int CharWidth(unsigned int unicode)
    {
      return CharWidth(unicode, mDefaultEffect);
    }

    /*!
     * @brief Get the with in pixel of the given string
     * @param string String to measure
     * @return Width in pixel
     */
    int CharWidth(unsigned int unicode, const FontEffect& effect);

    /*!
     * @brief Get the with in pixel of the given string
     * @param string String to measure
     * @return Width in pixel
     */
    int TextWidth(const String& string)
    {
      return TextWidth(string.data(), 0, (int)string.length(), mDefaultEffect);
    }

    /*!
     * @brief Get the with in pixel of the given string
     * @param string String to measure
     * @return Width in pixel
     */
    int TextWidth(const String& string, const FontEffect& effect)
    {
      return TextWidth(string.data(), 0, (int)string.length(), effect);
    }

    /*!
     * @brief Get the with in pixel of the given string
     * @param string String to measure
     * @param effect Font effect
     * @return Width in pixel
     */
    int TextWidth(const Array<unsigned int>& unicodes, int start, int length, const FontEffect& effect);

    /*!
     * @brief Truncate the string according to the given max width if required, and add ellipsis '…'
     * @param string String to truncate
     * @param maxWidth Maximum allowed width in pixel
     * @return Truncated string or original string if the original string fits into the given width
     */
    String TextEllipsis(const String& string, int maxWidth)
    {
      return TextEllipsis(string, mDefaultEffect, maxWidth);
    }

    /*!
     * @brief Truncate the string according to the given max width if required, and add ellipsis '…'
     * @param string String to truncate
     * @param effect Font effect
     * @param maxWidth Maximum allowed width in pixel
     * @return Truncated string or original string if the original string fits into the given width
     */
    String TextEllipsis(const String& string, const FontEffect& effect, int maxWidth);

    /*!
     * @brief Get the with in pixel of the given string
     * @param string String to measure
     * @return Width in pixel
     */
    Vector2f TextSize(const String& string)
    {
      return { (float)TextWidth(string.data(), 0, (int)string.length(), mDefaultEffect), (float)mRealMaxHeightInPixel };
    }

    /*!
     * @brief Get the with in pixel of the given string
     * @param string String to measure
     * @return Width in pixel
     */
    Vector2f TextSize(const String& string, const FontEffect& effect)
    {
      return { (float)TextWidth(string.data(), 0, (int)string.length(), effect), (float)mRealMaxHeightInPixel };
    }

    /*!
     * @brief Get the with in pixel of the given string
     * @param string String to measure
     * @param effect Font effect
     * @return Width in pixel
     */
    Vector2f TextSize(const Array<unsigned int>& unicodes, int start, int length, const FontEffect& effect)
    {
      return { (float)TextWidth(unicodes, start, length, effect), (float)mRealMaxHeightInPixel };
    }

    /*!
     * @brief Get the with in pixel of the given string
     * @param string String to measure
     * @return Width in pixel
     */
    TextChunkList SplitText(const String& string, int start, int length, int offsetInPixel, int widthInPixel, const FontEffect& effect);

    /*!
     * @brief Get the with in pixel of the given string
     * @param string String to measure
     * @return Width in pixel
     */
    TextChunkList SplitText(const String& string, int offsetInPixel, int widthInPixel, const FontEffect& effect)
    {
      return SplitText(string, 0, string.Count(), offsetInPixel, widthInPixel, effect);
    }

    /*!
     * @brief Get the with in pixel of the given string
     * @param string String to measure
     * @return Width in pixel
     */
    TextChunkList SplitText(const String& string, int widthInPixel, const FontEffect& effect)
    {
      return SplitText(string, 0, string.Count(), 0, widthInPixel, effect);
    }

    /*!
     * @brief Get the with in pixel of the given string
     * @param string String to measure
     * @return Width in pixel
     */
    TextChunkList SplitText(const String& string, int widthInPixel)
    {
      return SplitText(string, 0, string.Count(), 0, widthInPixel, mDefaultEffect);
    }

    /*!
     * @brief Draw a single char, taking y coordinate as the baseline
     * @param character Unicode character to draw
     * @param x X coordinate
     * @param y Y coordinate (baseline)
     * @return caracter width
     */
    int DrawCharBaseline(UnicodeChar character, int x, int y, Colors::ColorARGB color)
    {
      return DrawCharBaseline(character, x, y, color, mDefaultEffect);
    }

    /*!
     * @brief Draw a single char, taking y coordinate as the baseline
     * @param character Unicode character to draw
     * @param x X coordinate
     * @param y Y coordinate (baseline)
     * @return caracter width
     */
    int DrawCharBaseline(UnicodeChar character, int x, int y, Colors::ColorARGB color, const FontEffect& effects);

    /*!
     * @brief Draw a single char
     * @param character Unicode character to draw
     * @param x X coordinate
     * @param y Y coordinate
     * @param origin Origin within the character box. Using Center will draw the caracter centered from X,Y
     */
    void DrawChar(UnicodeChar character, int x, int y, Colors::ColorARGB, ::Alignment origin);

    /*!
     * @brief Draw a single char
     * @param character Unicode character to draw
     * @param x X coordinate
     * @param y Y coordinate
     * @param origin Origin within the character box. Using Center will draw the caracter centered from X,Y
     * @param effect Font effect
     */
    void DrawChar(UnicodeChar character, int x, int y, Colors::ColorARGB, ::Alignment origin, const FontEffect& effects);

    /*!
     * @brief Draw a string at coordinate X, Y. The Y line is the baseline of the font, not the top!
     * @param string String to draw
     * @param start start byte in the string
     * @param length length byte of the string
     * @param x X coordinate to draw text to
     * @param y Y coordinate to draw text to
     * @param alignment Horizontal alignment applied on X coordinate
     * @param effect Font effect
     */
    void DrawText(const String& string, int start, int length, int x, int y, Colors::ColorARGB color, ::Alignment alignment, const FontEffect& effect);

    /*!
     * @brief Draw a string at coordinate X, Y. The Y line is the baseline of the font, not the top!
     * @param string String to draw
     * @param x X coordinate to draw text to
     * @param y Y coordinate to draw text to
     * @param alignment Horizontal alignment applied on X coordinate
     * @param effect Font effect
     */
    void DrawText(const String& string, int x, int y, Colors::ColorARGB color, ::Alignment alignment, const FontEffect& effect)
    {
      DrawText(string, 0, (int)string.length(), x, y, color, alignment, effect);
    }

    /*!
     * @brief Draw a string at coordinate X, Y. The Y line is the baseline of the font, not the top!
     * @param string String to draw
     * @param x X coordinate to draw text to
     * @param y Y coordinate to draw text to
     * @param alignment Horizontal alignment applied on X coordinate
     */
    void DrawText(const String& string, int x, int y, Colors::ColorARGB color, ::Alignment alignment)
    {
      DrawText(string, x, y, color, alignment, mDefaultEffect);
    }

    /*!
     * @brief Draw a multiline string at coordinate X, Y. The Y line is the baseline of the font, not the top!
     * @param string String to draw
     * @param x X coordinate to draw text to
     * @param y Y coordinate to draw text to
     * @param alignment Horizontal alignment applied on X coordinate
     * @param interline Interline in ratio of font height
     */
    void DrawText(TextChunkList& strings, int x, int y, Colors::ColorARGB color, ::HorizontalAlignment alignment, float interline = 0);

    /*!
     * @brief Draw a single line text in the given rectangle
     * @param string String to draw
     * @param into Rectangle to draw string in (perform clipping)
     * @param xoffset X Offset applied to the text inside the rectangle
     * @param yoffset Y Offset applied to the text inside the rectangle
     * @param color Text color
     * @param alignment Text alignment inside the rectangle
     * @param effect Font effect
     */
    void DrawText(const String& string, int start, int length, const Rectangle& into, float xoffset, float yoffset, Colors::ColorARGB color, ::Alignment alignment, const FontEffect& effect);

    /*!
     * @brief Draw a single line text in the given rectangle
     * @param string String to draw
     * @param into Rectangle to draw string in (perform clipping)
     * @param xoffset X Offset applied to the text inside the rectangle
     * @param yoffset Y Offset applied to the text inside the rectangle
     * @param color Text color
     * @param alignment Text alignment inside the rectangle
     * @param effect Font effect
     */
    void DrawText(const String& string, const Rectangle& into, float xoffset, float yoffset, Colors::ColorARGB color, ::Alignment alignment, const FontEffect& effect)
    {
      DrawText(string, 0, string.Count(), into, xoffset, yoffset, color, alignment, effect);
    }

    /*!
     * @brief Draw a single line text in the given rectangle
     * @param string String to draw
     * @param into Rectangle to draw string in (perform clipping)
     * @param color Text color
     * @param alignment Text alignment inside the rectangle
     * @param effect Font effect
     */
    void DrawText(const String& string, const Rectangle& into, Colors::ColorARGB color, ::Alignment alignment, const FontEffect& effect)
    {
      DrawText(string, 0, string.Count(), into, 0, 0, color, alignment, effect);
    }

    /*!
     * @brief Draw a single line text in the given rectangle
     * @param string String to draw
     * @param into Rectangle to draw string in (perform clipping)
     * @param color Text color
     * @param alignment Text alignment inside the rectangle
     * @param effect Font effect
     */
    void DrawText(const String& string, const Rectangle& into, Colors::ColorARGB color, ::Alignment alignment)
    {
      DrawText(string, 0, string.Count(), into, 0, 0, color, alignment, mDefaultEffect);
    }

    /*!
     * @brief Draw a multi line text in the given rectangle, at the po
     * @param string String to draw
     * @param into Rectangle to draw string in (perform clipping)
     * @param xoffset X Offset applied to the text inside the rectangle
     * @param yoffset Y Offset applied to the text inside the rectangle
     * @param color Text color
     * @param alignment Text alignment inside the rectangle
     * @param interline Interline in ratio of font height
     */
    void DrawText(TextChunkList& strings, const Rectangle& into, float xoffset, float yoffset, Colors::ColorARGB color, ::Alignment alignment, float interline = 0);

    //! Get requested height
    [[nodiscard]] int RequestedHeight() const { return (int)mRequestedHeightInPixel; }

    //! Get maximum height (height of the higher character)
    [[nodiscard]] int Height() const { return mRealMaxHeightInPixel; }

    //! Get maximum offset from the top to the baseline (from the higher character)
    [[nodiscard]] int TopToBaseline() const { return mTopToBaseline; }

    //! Get maximum offset from the baseline to the bottom (from the higher character)
    [[nodiscard]] int BottomToBaseline() const { return mBottomToBaseline; }

    //! Clear all caches
    void ClearCaches();

    #ifdef DEBUG
    void RenderFontCache();
    #endif

    //! Font path
    [[nodiscard]] const Path& GetPath() const { return mPath; }

  private:
    //! Face provider
    IFontFaceProvider& mFaceProvider;
    //! Font glyph store
    GlyphStorage mGlyphStore;

    //! Default font effect
    const FontEffect mDefaultEffect;

    //! Cached font face for non default fonts
    FT_Face mFace;

    //! Font resource
    Path mPath;
    //! Font height in pixel
    float mRequestedHeightInPixel;
    //! Real max font height
    int mRealMaxHeightInPixel;
    //! Maximum Top to baseline offset in pixel
    int mTopToBaseline;
    //! Maximum Top to baseline offset in pixel
    int mBottomToBaseline;

    //! Current texture in use
    Texture mCurrentTexture;
    //! Current texture width
    int mTextureWidth;
    //! Current texture height
    int mTextureHeight;
    //! Current free position X in the current texture
    int mCursorX;
    //! Current free position Y in the current texture
    int mCursorY;
    //! Row height
    int mRowHeight;

    //! This font is a default font and my lookup character in multiple fonts
    bool mDefault;

    /*!
     * @brief Get the most appropriate font face for the give char
     * @return Pre-configured font face
     */
    FT_Face FaceForChar(UnicodeChar);

    /*!
     * @brief Get the most appropriate font face for the give char
     * If not found, look into default fonts
     * @return Pre-configured font face
     */
    FT_Face FaceForCharFallback(UnicodeChar);

    /*!
     * @brief Make room in current texture to draw the new glyph
     * This may lead to new texture creation if the current one has not enough room
     * @param face Glyph to get metrics from
     */
    void MakeRoomInTexture(FT_GlyphSlot glyph, int w, int h);

    /*!
     * @brief Make new texture regarding font size
     * @param face Font face to get metrics from
     * @return new Texture
     */
    void MakeNewTexture(FT_Face face);

    /*!
     * @brief Copy the given glyph bitmap in the texture
     * @param glyph
     */
    void RenderGlyphInTexture(FT_BitmapGlyph own);

    /*!
     * @brief Prepare the given unicode char. i.e: load the charactere from the font
     * @param unicode Unicode character
     * @param embold Set true to create bold character - Both bold and unbold char may be prepared
     * as they are stored as two sistinct characters
     */
    Glyph* Prepare(UnicodeChar unicode, bool embold);

    /*!
     * @brief Draw a chunk of string at coordinate X, Y. The Y line is the baseline of the font, not the top!
     * @param string String to draw
     * @param start 1 character to draw
     * @param length Length to draw ( != character as the methods decode UTF8 characters)
     * @param x X coordinate to draw text to
     * @param y Y coordinate to draw text to
     * @param effect Font effect
     */
    void DrawTextRaw(const String& string, int start, int length, int x, int y, Colors::ColorARGB, const FontEffect& effect);

    /*!
     * @brief Get the with in pixel of the given string
     * @param string String to measure
     * @param effect Font effect
     * @return Width in pixel
     */
    int TextWidth(const String& string, int start, int length, const FontEffect& effect);

    /*!
     * @brief Create Glyph vertexes from glyph structure
     * @param glyph Glyph structure
     * @param x X target coordinate
     * @param y Y target coordinate
     * @param effect Font effect
     * @param vertex Rectangle vertex array
     */
    static void MapGlyphTexture(const Glyph& glyph, float x, float y, const FontEffect::Final& effect, Vertex::Rectangle& vertex);
};
