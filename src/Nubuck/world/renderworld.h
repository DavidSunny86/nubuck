#pragma once

#include <world\entity.h>
#include <world\world.h>
#include <renderer\renderer.h>

struct RenderWorld : W::World::Visitor {
    R::Renderer& renderer;

    RenderWorld(R::Renderer& renderer) : renderer(renderer) { }

    void Visit(W::Entity& entity) const override {
        renderer.Add(entity.GetRenderJob());
    }
};
