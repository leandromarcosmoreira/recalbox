//
// Created by bkg2k on 02/08/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <string>
#include "rendering/GLPlatform.h"
#include "rendering/textures/Texture.h"

class Shader
{
  public:
    /*!
     * @brief Constructor
     * @param shader Shader content
     */
    explicit Shader(const String& shader);

    /*!
     * @brief Destructor
     */
    ~Shader();

    void Activate(Texture& input) const;

    void Activate() const;

    void Deactivate() const;

    void SetInt(const char* name, int value) const;

    void SetFloat(const char* name, float value) const;

    void SetVec2(const char* name, float x, float y) const;

    void SetTexture(const char* varPosition, const char* varTexCoord, const char* varTexture, GLuint textureID, Vertex* vertexes);

    GLuint Handle() const { return mProgramObject; }

  private:
    //! Vertex Shader identifier
    GLuint mVertexShader;
    //! Fragment shader identifier
    GLuint mFragmentShader;
    //! Program object
    GLuint mProgramObject;
    //! Texture position attribute handle
    GLint mPositionHandle;
    //! Texture coordinate attribute handle
    GLint mTextCoordHandle;

    /*!
     * @brief Load shader
     * @param shader Shader content
     * @param type Shader type
     * @return True if the shader is loaded, false otherwise
     */
    static GLuint LoadShader(const String& shader, GLenum type);

    static GLuint Link(GLuint vertexShader, GLuint fragmentShader);
};



