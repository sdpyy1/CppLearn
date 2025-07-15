//
// Created by Administrator on 2025/7/14.
//

#ifndef OPENGLRENDER_RENDERPASS_H
#define OPENGLRENDER_RENDERPASS_H


class RenderPass {
public:
    virtual void init() = 0;
    virtual void render() = 0;
    virtual ~RenderPass() = default;
};


#endif //OPENGLRENDER_RENDERPASS_H
