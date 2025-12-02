#include <random>
using namespace std;

#include "model.h"
#include "tree_foliage.h"

TreeFoliage::TreeFoliage(const char *branchModelPath)
{
    std::cout << "Loading tree branch model: " << branchModelPath << std::endl;
    branchModel = new Model(branchModelPath);
    extractBranchVertices();
    std::cout << "  Extracted " << branchVertices.size() << " branch vertices" << std::endl;
    setupQuadMesh();
    std::cout << "  Quad mesh setup complete" << std::endl;
}

TreeFoliage::~TreeFoliage()
{
    delete branchModel;
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteBuffers(1, &quadEBO);
    glDeleteBuffers(1, &instanceVBO);
    for (auto tex : leafTextures)
        glDeleteTextures(1, &tex);
}

void TreeFoliage::setupQuadMesh()
{
    // Single quad (2 triangles) - will be instanced
    float quadVertices[] = {
        // pos           // uv
        -0.5f, -0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.0f, 1.0f};
    unsigned int indices[] = {0, 1, 2, 2, 3, 0};

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glGenBuffers(1, &quadEBO);
    glGenBuffers(1, &instanceVBO);

    glBindVertexArray(quadVAO);

    // Quad geometry
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Attribute 0: position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    // Attribute 1: uv
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

    glBindVertexArray(0);
}

void TreeFoliage::extractBranchVertices()
{
    // Extract vertex positions from branch model for clustering
    for (auto &mesh : branchModel->meshes)
    {
        for (auto &vert : mesh.vertices)
        {
            branchVertices.push_back(vert.Position);
        }
    }
}

void TreeFoliage::GenerateLeafClusters(int clustersPerBranch, int leavesPerCluster)
{
    mt19937 rng(42); // Fixed seed for consistency
    uniform_real_distribution<float> dist01(0.0f, 1.0f);
    uniform_real_distribution<float> distPM1(-1.0f, 1.0f);
    uniform_int_distribution<int> distTex(0, 3);

    clusters.clear();
    allLeaves.clear();

    // Sample branch vertices for cluster attachment points
    int step = max(1, (int)branchVertices.size() / clustersPerBranch);
    for (size_t i = 0; i < branchVertices.size(); i += step)
    {
        LeafCluster cluster;
        cluster.attachPoint = branchVertices[i];
        cluster.radius = 0.8f + dist01(rng) * 0.4f; // 0.8-1.2 units
        cluster.leafCount = leavesPerCluster;

        // Emitter normal (simulate sphere normal at this point)
        cluster.emitterNormal = glm::normalize(cluster.attachPoint);

        clusters.push_back(cluster);

        // Generate leaves in spherical distribution around cluster
        for (int j = 0; j < leavesPerCluster; j++)
        {
            LeafInstance leaf;

            // Random position in sphere
            float theta = dist01(rng) * 2.0f * glm::pi<float>();
            float phi = acos(2.0f * dist01(rng) - 1.0f);
            float r = cluster.radius * pow(dist01(rng), 1.0f / 3.0f); // Uniform sphere distribution

            leaf.offset = r * glm::vec3(
                                  sin(phi) * cos(theta),
                                  sin(phi) * sin(theta),
                                  cos(phi));

            // Scale variation
            leaf.scale = 1.0f + dist01(rng) * 0.5f;
            leaf.textureIndex = distTex(rng);

            // Custom normal - blend between local sphere normal and cluster normal
            glm::vec3 localNormal = glm::normalize(leaf.offset);
            leaf.customNormal = glm::normalize(
                0.7f * cluster.emitterNormal + 0.3f * localNormal);

            allLeaves.push_back(leaf);
        }
    }

    // Upload instance data to GPU
    vector<float> instanceData;
    for (size_t i = 0; i < clusters.size(); i++)
    {
        for (int j = 0; j < clusters[i].leafCount; j++)
        {
            int idx = i * leavesPerCluster + j;
            if (idx >= allLeaves.size())
                break;

            LeafInstance &leaf = allLeaves[idx];
            glm::vec3 worldPos = clusters[i].attachPoint + leaf.offset;

            // Pack: worldPos(3), customNormal(3), scale(1), texIndex(1)
            instanceData.push_back(worldPos.x);
            instanceData.push_back(worldPos.y);
            instanceData.push_back(worldPos.z);
            instanceData.push_back(leaf.customNormal.x);
            instanceData.push_back(leaf.customNormal.y);
            instanceData.push_back(leaf.customNormal.z);
            instanceData.push_back(leaf.scale);
            instanceData.push_back((float)leaf.textureIndex);
        }
    }

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instanceData.size() * sizeof(float),
                 instanceData.data(), GL_STATIC_DRAW);

    // Instance attributes (per-instance, not per-vertex)
    int stride = 8 * sizeof(float);

    // Attribute 2: worldPos
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
    glVertexAttribDivisor(2, 1);

    // Attribute 3: customNormal
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void *)(3 * sizeof(float)));
    glVertexAttribDivisor(3, 1);

    // Attribute 4: scale
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, stride, (void *)(6 * sizeof(float)));
    glVertexAttribDivisor(4, 1);

    // Attribute 5: texIndex
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, stride, (void *)(7 * sizeof(float)));
    glVertexAttribDivisor(5, 1);

    glBindVertexArray(0);

    std::cout << "Generated " << clusters.size() << " leaf clusters with "
              << allLeaves.size() << " total leaves" << std::endl;
}

void TreeFoliage::LoadLeafTextures(const std::vector<const char *> &texturePaths)
{
    for (auto path : texturePaths)
    {
        // Load texture directly (path is already full)
        unsigned int tex;
        glGenTextures(1, &tex);

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

            glBindTexture(GL_TEXTURE_2D, tex);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
            std::cout << "Loaded leaf texture: " << path << " (" << width << "x" << height << ")" << std::endl;
        }
        else
        {
            std::cerr << "Failed to load leaf texture: " << path << std::endl;
        }

        leafTextures.push_back(tex);
    }
}

void TreeFoliage::Draw(Shader &leafShader, Shader &branchShader,
                       const glm::mat4 &model, const glm::mat4 &view,
                       const glm::mat4 &projection, const glm::vec3 &cameraPos)
{
    // === DRAW BRANCHES (solid geometry, opaque) ===
    branchShader.use();
    branchShader.setMat4("model", model);
    branchShader.setMat4("view", view);
    branchShader.setMat4("projection", projection);
    branchShader.setVec3("viewPos", cameraPos);
    branchShader.setVec3("lightDir", glm::vec3(0.3f, -0.7f, 0.5f)); // Match your scene lighting

    branchModel->Draw(branchShader);

    // === DRAW LEAVES (billboarded, transparent) ===
    leafShader.use();
    leafShader.setMat4("model", model);
    leafShader.setMat4("view", view);
    leafShader.setMat4("projection", projection);
    leafShader.setVec3("cameraPos", cameraPos);
    leafShader.setVec3("cameraRight", glm::vec3(view[0][0], view[1][0], view[2][0]));
    leafShader.setVec3("cameraUp", glm::vec3(view[0][1], view[1][1], view[2][1]));

    // Bind all leaf textures
    for (size_t i = 0; i < leafTextures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, leafTextures[i]);
        leafShader.setInt(("leafTexture" + std::to_string(i)).c_str(), i);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(quadVAO);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, allLeaves.size());
    glBindVertexArray(0);

    glDisable(GL_BLEND);
}