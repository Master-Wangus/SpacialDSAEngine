#pragma once

#include "pch.h"
#include "Registry.hpp"
#include "Components.hpp"
#include "Geometry.hpp"

namespace SpatialTreeUtils
{

    inline void ComputeSceneBounds(Registry& registry, Aabb& outBounds)
    {
        glm::vec3 minAll( 1e30f);
        glm::vec3 maxAll(-1e30f);

        auto view = registry.View<TransformComponent, BoundingComponent>();
        for (auto entity : view)
        {
            auto& t  = view.get<TransformComponent>(entity);
            auto& bc = view.get<BoundingComponent>(entity);
            Aabb box = bc.GetAABB();
            box.Transform(t.m_Model);

            minAll = glm::min(minAll, box.min);
            maxAll = glm::max(maxAll, box.max);
        }

        // Fallback if no entities
        if (minAll.x > maxAll.x)
        {
            outBounds = Aabb(glm::vec3(0.0f), 1.0f);
            return;
        }

        glm::vec3 center = (minAll + maxAll) * 0.5f;
        glm::vec3 ext    = (maxAll - minAll) * 0.5f;
        float maxExtent  = glm::compMax(ext);
        outBounds = Aabb(center - glm::vec3(maxExtent), center + glm::vec3(maxExtent));
    }

    inline glm::vec3 LevelColor(int level)
    {
        static const glm::vec3 kColors[] =
        {
            {1.0f, 0.0f, 0.0f}, // red
            {0.0f, 1.0f, 0.0f}, // green
            {0.0f, 0.0f, 1.0f}, // blue
            {1.0f, 1.0f, 0.0f}, // yellow
            {1.0f, 0.0f, 1.0f}, // magenta
            {0.0f, 1.0f, 1.0f}, // cyan
            {1.0f, 1.0f, 1.0f}, // white
            {0.5f, 0.5f, 0.5f}, // gray
            {1.0f, 0.5f, 0.0f}, // orange
            {0.5f, 0.0f, 1.0f}  // purple
        };

        size_t idx = static_cast<size_t>(level) % (sizeof(kColors) / sizeof(kColors[0]));
        return kColors[idx];
    }
} 