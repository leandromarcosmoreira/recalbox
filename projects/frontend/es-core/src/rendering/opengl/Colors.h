//
// Created by bkg2k on 28/09/2020.
//
#pragma once

#include "rendering/GLPlatform.h"

class Colors
{
  public:
    //! Convenient color array for Triangle
    typedef GLubyte TriangleColors[3 * 4];
    //! Convenient color array for Rectangle
    typedef GLubyte RectangleColors[6 * 4];

    //! ARGB
    typedef unsigned int ColorARGB;
    //! ARGB
    typedef unsigned int ColorRGBA;
    //! Component
    typedef unsigned char Component;

    static constexpr ColorARGB None = 0x00000000;

    class Common
    {
      public:
        static constexpr ColorARGB Transparent = 0x00000000;
        static constexpr ColorARGB Black       = 0xFF000000;
        static constexpr ColorARGB White       = 0xFFFFFFFF;
        static constexpr ColorARGB Red         = 0xFFFF0000;
        static constexpr ColorARGB Green       = 0xFF00FF00;
        static constexpr ColorARGB Blue        = 0xFF0000FF;
        static constexpr ColorARGB Purple      = 0xFFFF00FF;
        static constexpr ColorARGB Yellow      = 0xFFFFFF00;
        static constexpr ColorARGB Cyan        = 0xFF00FFFF;
    };

    /*!
     * @brief Build a full opaque RGB color from its three components
     * @param red Red component
     * @param green Green component
     * @param blue Blue component
     * @return ARGB color
     */
    static ColorARGB MakeRGB(Component red, Component green, Component blue) { return (0xFF << 24) | (red << 16) | (green << 8) | blue; }

    /*!
     * @brief Build an ARGB color from its four components
     * @param alpha Alpha component
     * @param red Red component
     * @param green Green component
     * @param blue Blue component
     * @return ARGB color
     */
    static ColorARGB MakeARGB(Component alpha, Component red, Component green, Component blue) { return (alpha << 24) | (red << 16) | (green << 8) | blue; }

    /*!
     * @brief Get Alpha component from an ARGB color
     * @param color Input Color
     * @return Alpha component
     */
    static Component Alpha(ColorARGB color) { return color >> 24; }
    /*!
     * @brief Get Red component from an ARGB color
     * @param color Input Color
     * @return Red component
     */
    static Component Red(ColorARGB color) { return (color >> 16) & 0xFF; }
    /*!
     * @brief Get Green component from an ARGB color
     * @param color Input Color
     * @return Green component
     */
    static Component Green(ColorARGB color) { return (color >> 8) & 0xFF; }
    /*!
     * @brief Get Blue component from an ARGB color
     * @param color Input Color
     * @return Blue component
     */
    static Component Blue(ColorARGB color) { return color & 0xFF; }
    /*!
     * @brief Get pure color RGB, w/o Alpha
     * @param color Input color
     * @return Pure color
     */
    static ColorARGB PureColor(ColorARGB color) { return color & 0xFFFFFF; }

    /*!
     * @brief Replace the given color's alpha component with the given one
     * @param color Source color
     * @param alpha Alpha component
     * @return Source color with the Alpha component replaced
     */
    static ColorARGB SetAlpha(ColorARGB color, Component alpha) { return (color & 0xFFFFFF) | (alpha << 24); }

    /*!
     * @brief Replace the given color's red component with the given one
     * @param color Source color
     * @param red Red component
     * @return Source color with the Red component replaced
     */
    static ColorARGB SetRed(ColorARGB color, Component red) { return (color & 0xFF00FFFF) | (red << 16); }

    /*!
     * @brief Replace the given color's green component with the given one
     * @param color Source color
     * @param green Green component
     * @return Source color with the Green component replaced
     */
    static ColorARGB SetGreen(ColorARGB color, Component green) { return (color & 0xFFFF00FF) | (green << 8); }

    /*!
     * @brief Replace the given color's blue component with the given one
     * @param color Source color
     * @param blue Blue component
     * @return Source color with the Blue component replaced
     */
    static ColorARGB SetBlue(ColorARGB color, Component blue) { return (color & 0xFFFFFF00) | blue; }

    /*!
     * @brief Check if the alpha component of the given color is not zero
     * @param color Color to check
     * @return True if the alpha component is > 0
     */
    static bool IsVisible(ColorARGB color) { return (color >> 24) != 0; }

    /*!
     * @brief Convert ARGB to RGBA
     * @param argb source ARGB
     * @return RGBA conversion
     */
    static ColorRGBA ARGBToABGR(ColorARGB argb)
    {
      unsigned int bgra = __builtin_bswap32(argb);
      return (argb & 0xFF00FF00) | ((bgra & 0xFF00FF00) >> 8);
    }

    /*!
     * @brief Convert the given ARGB color to ABGR and store RGBA component into a GL byte array
     * @param destination Destination GL byte array
     * @param argb Source ARGB color
     */
    static void ARGBToGLColoArray(GLubyte* destination, ColorARGB argb)
    {
      *((unsigned int*)destination) = ARGBToABGR(argb);
    }

    /*!
     * @brief Convert the given ARGB color to ABGR and store RGBA component into a GL byte array
     * @param destination Destination GL byte array
     * @param argb Source ARGB color
     */
    static void FillToGLColoArray(RectangleColors& destination, ColorARGB argb)
    {
      ColorRGBA* glrgba = (ColorRGBA*)destination;
      ColorRGBA rgba = ARGBToABGR(argb);
      for(int i = 6; --i >= 0; )
        glrgba[i] = rgba;
    }

    /*!
     * @brief Convert the given ARGB color to ABGR and store RGBA component into a GL byte array
     * @param destination Destination GL byte array
     * @param argb Source ARGB color
     */
    static void FillToGLColoArray(TriangleColors& destination, ColorARGB argb)
    {
      ColorRGBA* glrgba = (ColorRGBA*)destination;
      ColorRGBA rgba = ARGBToABGR(argb);
      for(int i = 6; --i >= 0; )
        glrgba[i] = rgba;
    }

    /*!
     * @brief Convert the given ARGB color to ABGR and store the 4 components X times into the destination array
     * @param destination Destination array
     * @param argb Source ARGB color
     * @param count Number of time to repeat the color in the destination array
     */
    static void FillGLColorArray(GLubyte* destination, ColorARGB argb, int count)
    {
      ColorRGBA* glrgba = (ColorRGBA*)destination;
      ColorRGBA rgba = ARGBToABGR(argb);
      for(int i = count; --i >= 0; )
        glrgba[i] = rgba;
    }

    /*!
     * @brief Apply a given color onto another, blending the applied color regarding its alpha component
     * @param appliyedColor Color to apply
     * @param onto Original color
     * @return Blended color
     */
    static ColorARGB ApplyColorOver(ColorARGB appliyedColor, ColorARGB onto)
    {
      unsigned int as = appliyedColor >> 24;
      unsigned int ad = 255 - as;
      unsigned int r = ((((appliyedColor >> 16) & 0xFF) * as) + (((onto >> 16) & 0xFF) * ad)) >> 8;
      unsigned int g = ((((appliyedColor >>  8) & 0xFF) * as) + (((onto >>  8) & 0xFF) * ad)) >> 8;
      unsigned int b = ((((appliyedColor >>  0) & 0xFF) * as) + (((onto >>  0) & 0xFF) * ad)) >> 8;
      return (onto & 0xFF000000) | (r << 16) | (g << 8) | b;
    }

    /*!
     * @brief Apply the alpha component to RGB component and set alpha to full opaque
     * @param source Source color
     * @return Opaque color
     */
    static ColorARGB AlphaToOpaque(ColorARGB source)
    {
      unsigned int a = Alpha(source);
      unsigned int r = (((source >> 16) & 0xFF) * a) >> 8;
      unsigned int g = (((source >>  8) & 0xFF) * a) >> 8;
      unsigned int b = (((source >>  0) & 0xFF) * a) >> 8;
      return 0xFF000000 | (r << 16) | (g << 8) | b;
    }

    /*!
     * @brief Blend a color using the given ratio from 0 to 1
     * @param from Source color
     * @param to Destination color
     * @param ratio Destination ratio (0 = full source, 1 = full destination)
     * @return Blended color
     */
    static ColorARGB Blend(ColorARGB from, ColorARGB to, float ratio)
    {
      int r = (int)(ratio * 255);
      int rs = (int)(from >> 24);
      int gs = (int)((from >> 16) & 0xFF);
      int bs = (int)((from >>  8) & 0xFF);
      int as = (int)((from >>  0) & 0xFF);
      int rd = (int)(to >> 24);
      int gd = (int)((to >> 16) & 0xFF);
      int bd = (int)((to >>  8) & 0xFF);
      int ad = (int)((to >>  0) & 0xFF);
      int rr = rs + (((rd - rs) * r) >> 8);
      int gr = gs + (((gd - gs) * r) >> 8);
      int br = bs + (((bd - bs) * r) >> 8);
      int ar = as + (((ad - as) * r) >> 8);
      return ((unsigned int)rr << 24) | ((unsigned int)gr << 16) | ((unsigned int)br << 8) | (unsigned int)ar;
    }

};
