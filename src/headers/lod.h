#pragma once

#include <glm/glm.hpp>

// to manage the LOD (Level-Of-Distance)
struct LODConfig
{
    // Distance thresholds
    float nearDistance = 15.0f; // Full detail
    float midDistance = 35.0f;  // Medium detail
    float farDistance = 60.0f;  // Low detail (or cull)

    // Density multipliers per LOD level
    float nearDensity = 1.0f; // 100% instances
    float midDensity = 0.5f;  // 50% instances
    float farDensity = 0.2f;  // 20% instances

    // Calculate LOD level for a position
    int GetLODLevel(const glm::vec3 &position, const glm::vec3 &cameraPos) const
    {
        float dist = glm::distance(position, cameraPos);

        if (dist < nearDistance)
            return 0; // Near (full detail)
        else if (dist < midDistance)
            return 1; // Mid
        else if (dist < farDistance)
            return 2; // Far
        else
            return 3; // Culled
    }

    // Get density multiplier for distance
    float GetDensityMultiplier(float distance) const
    {
        if (distance < nearDistance)
            return nearDensity;
        else if (distance < midDistance)
            return midDensity;
        else if (distance < farDistance)
            return farDensity;
        else
            return 0.0f; // Cull
    }
};