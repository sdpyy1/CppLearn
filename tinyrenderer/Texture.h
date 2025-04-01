//
// Created by 刘卓昊 on 2025/4/1.
//

#ifndef TINYRENDERER_TEXTURE_H
#define TINYRENDERER_TEXTURE_H
#include <Eigen/Eigen>
#include "thirdParty/tgaimage.h"
class Texture{
private:
    TGAImage texture;

public:
    Texture(const std::string& name)
    {
        texture.read_tga_file(name);
        width = texture.width();
        height = texture.height();
    }

    int width, height;

    TGAColor getColor(float u, float v)
    {
        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        TGAColor color = texture.get(u_img, v_img);
        return color;
    }

};


#endif //TINYRENDERER_TEXTURE_H
