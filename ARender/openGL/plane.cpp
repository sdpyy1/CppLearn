#include "plane.h"
#include <STBImage/stb_image.h>
unsigned int loadTexture(char const * path);

Plane::Plane(const std::string path)
{
    // 加载纹理
    texture = loadTexture(path.c_str());
    // 设置VAO VBO
    OPENGL_EXTRA_FUNCTIONS->glGenVertexArrays(1, &planeVAO);
    OPENGL_EXTRA_FUNCTIONS->glGenBuffers(1, &planeVBO);
    OPENGL_EXTRA_FUNCTIONS->glBindVertexArray(planeVAO);
    OPENGL_EXTRA_FUNCTIONS->glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    OPENGL_EXTRA_FUNCTIONS->glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    OPENGL_EXTRA_FUNCTIONS->glEnableVertexAttribArray(0);
    OPENGL_EXTRA_FUNCTIONS->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    OPENGL_EXTRA_FUNCTIONS->glEnableVertexAttribArray(1);
    OPENGL_EXTRA_FUNCTIONS->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    OPENGL_EXTRA_FUNCTIONS->glEnableVertexAttribArray(2);
    OPENGL_EXTRA_FUNCTIONS->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    OPENGL_EXTRA_FUNCTIONS->glBindVertexArray(0);
}

void Plane::render()
{
    OPENGL_EXTRA_FUNCTIONS->glBindVertexArray(planeVAO);
    OPENGL_EXTRA_FUNCTIONS->glActiveTexture(GL_TEXTURE0);
    OPENGL_EXTRA_FUNCTIONS->glBindTexture(GL_TEXTURE_2D, texture);
    OPENGL_EXTRA_FUNCTIONS->glDrawArrays(GL_TRIANGLES, 0, 6);
    OPENGL_EXTRA_FUNCTIONS->glBindVertexArray(0);
}
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    OPENGL_FUNCTIONS->glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        OPENGL_EXTRA_FUNCTIONS->glBindTexture(GL_TEXTURE_2D, textureID);
        OPENGL_EXTRA_FUNCTIONS->glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        OPENGL_EXTRA_FUNCTIONS->glGenerateMipmap(GL_TEXTURE_2D);

        OPENGL_EXTRA_FUNCTIONS->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat
        OPENGL_EXTRA_FUNCTIONS->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        OPENGL_EXTRA_FUNCTIONS->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        OPENGL_EXTRA_FUNCTIONS->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
