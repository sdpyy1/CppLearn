#pragma once

class RenderPass
{
public:
    virtual void init() = 0;
    virtual void render() = 0;
    virtual ~RenderPass() = default;
};



