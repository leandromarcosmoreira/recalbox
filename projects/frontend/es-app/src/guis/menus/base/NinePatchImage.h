//
// Created by bkg2k on 26/08/24.
//
#pragma once

#include <rendering/opengl/Rectangle.h>
#include <rendering/opengl/Colors.h>
#include "rendering/opengl/Vertex.h"
#include <rendering/textures/Texture.h>

class NinePatchImage
{
  public:
    /*!
     * @brief Constructor
     * @param image Image resource
     */
    explicit NinePatchImage(Texture image)
      : mImage(image)
      , mLastTarget(0, 0)
    {}

    [[nodiscard]] int ChunkWidth() const { return mImage.Width() / 3; }

    [[nodiscard]] int ChunkHeight() const { return mImage.Height() / 3; }

    /*!
     * @brief Draw the nine patch image
     * @param targetArea Target rectangle
     * @param color Color
     */
    void Render(const Rectangle& targetArea, Colors::ColorRGBA color);

  private:
    //! Image
    Texture mImage;
    //! Vertexes
    Vertex mVertices[Vertex::sVertexPerRectangle * 9];
    //! Last rectangle
    Rectangle mLastTarget;

    /*!
     * @brief Prepare drawing to the target area
     * @param targetArea Target area
     */
    void PrepareDrawingTo(const Rectangle& targetArea);
};
