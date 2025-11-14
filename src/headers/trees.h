#pragma once

#include <glm/glm.hpp>

class Trees
{
public:
    Trees() {}
    ~Trees() {}

    void generateTrees(int count);
    void renderTrees();

private:
    int height;
    int noOfBranches;
    glm::vec3 position;
};