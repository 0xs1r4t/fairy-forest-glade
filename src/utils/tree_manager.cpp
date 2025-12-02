#include <iostream>
using namespace std;

#include "tree_manager.h"

TreeManager::TreeManager(Terrain *terrain, TreeFoliage *normalType, TreeFoliage *thickType,
                         int count, const LODConfig &lodConfig)
    : terrain(terrain), normalTree(normalType), thickTree(thickType),
      lodConfig(lodConfig), visibleCount(0)
{
    std::cout << "Generating tree positions..." << std::endl;
    generateTreePositions(count);
    std::cout << "Placed " << trees.size() << " trees on terrain" << std::endl;
}

void TreeManager::generateTreePositions(int desiredCount)
{
    mt19937 rng(42);
    uniform_real_distribution<float> distX(-50.0f, 50.0f);
    uniform_real_distribution<float> distZ(-50.0f, 50.0f);
    uniform_real_distribution<float> distScale(2.0f, 4.0f);
    uniform_real_distribution<float> distRot(0.0f, 360.0f);
    uniform_real_distribution<float> distType(0.0f, 1.0f);

    int attempts = desiredCount * 3;

    for (int i = 0; i < attempts && trees.size() < desiredCount; i++)
    {
        float x = distX(rng);
        float z = distZ(rng);
        float y = terrain->getHeight(x, z);
        glm::vec3 normal = terrain->getNormal(x, z);

        // Tree placement rules
        if (normal.y > 0.85f && y > -1.0f && y < 8.0f)
        {
            // Check spacing
            bool tooClose = false;
            float minSpacing = 4.0f;

            for (const auto &existing : trees)
            {
                float dist = glm::distance(glm::vec2(x, z),
                                           glm::vec2(existing.position.x, existing.position.z));
                if (dist < minSpacing)
                {
                    tooClose = true;
                    break;
                }
            }

            if (!tooClose)
            {
                TreeInstance tree;
                tree.position = glm::vec3(x, y, z);
                tree.scale = distScale(rng);
                tree.rotation = distRot(rng);
                tree.useThickType = distType(rng) > 0.5f;
                tree.boundingRadius = tree.scale * 3.0f; // Approximate tree size

                trees.push_back(tree);
            }
        }
    }
}

void TreeManager::Draw(Shader &leafShader, Shader &branchShader,
                       const glm::mat4 &view, const glm::mat4 &projection,
                       const Camera::Frustum &frustum, const Camera &camera)
{
    visibleCount = 0;
    int nearCount = 0, midCount = 0, farCount = 0;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (const auto &tree : trees)
    {
        // Frustum culling
        if (!camera.IsSphereInFrustum(frustum, tree.position, tree.boundingRadius))
        {
            continue;
        }

        // LOD distance check
        float distance = glm::distance(camera.Position, tree.position);
        int lodLevel = lodConfig.GetLODLevel(tree.position, camera.Position);

        if (lodLevel >= 3)
            continue; // Too far, cull

        visibleCount++;

        // Build model matrix
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, tree.position);
        model = glm::scale(model, glm::vec3(tree.scale));
        model = glm::rotate(model, glm::radians(tree.rotation), glm::vec3(0, 1, 0));

        // LOD: Adjust rendering based on distance
        // For now, we draw all visible trees fully
        // TODO: Could dynamically reduce leaf cluster count here

        if (lodLevel == 0)
            nearCount++;
        else if (lodLevel == 1)
            midCount++;
        else
            farCount++;

        // Draw tree
        if (tree.useThickType)
        {
            thickTree->Draw(leafShader, branchShader, model, view, projection, camera.Position);
        }
        else
        {
            normalTree->Draw(leafShader, branchShader, model, view, projection, camera.Position);
        }
    }

    glDisable(GL_BLEND);

    // Debug output
    static int frameCount = 0;
    if (++frameCount % 60 == 0)
    {
        std::cout << "Trees: " << visibleCount << " / " << trees.size()
                  << " (Near: " << nearCount << ", Mid: " << midCount << ", Far: " << farCount << ")"
                  << std::endl;
    }
}