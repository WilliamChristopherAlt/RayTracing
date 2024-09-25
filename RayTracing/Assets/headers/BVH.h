#pragma once

#include <iostream>

#include <glm/glm.hpp>

#include <RayTracing/Assets/headers/mesh.h>

const int MAX_DEPTH = 32;

struct BoundingBox
{
    glm::vec3 min = glm::vec3(1e30f);
    float pad0 = 0.0f;
    glm::vec3 max = glm::vec3(-1e30f);
    float pad1 = 0.0f;

    BoundingBox() = default;
    
    glm::vec3 center() const
    {
        return (min + max) / 2.0f;
    }

    float length(int axis) const
    {
        return max[axis] - min[axis];
    }

    glm::vec3 size() const
    {
        return glm::vec3(max[0] - min[0], max[0] - min[0], max[0] - min[0]);
    }

    void growToInclude(const glm::vec3& point)
    {
        min = glm::min(min, point);
        max = glm::max(max, point);
    }

    void growToInclude(const BVHTriangle& tri)
    {
        min = glm::min(min, tri.min);
        max = glm::max(max, tri.max);
    }

    void expand()
    {
        min -= glm::vec3(1e-4f);
        max += glm::vec3(1e-4f);
    }
};

struct Node
{
    BoundingBox bounds = BoundingBox();
    int triangleIndex = -1;
    int triangleCount = -1;
    int childIndex = -1;
    int pad0 = 0.0f;

    Node() = default;

    Node(const BoundingBox& box, int triIdx, int count, int childIdx) : bounds(box), triangleIndex(triIdx), triangleCount(count), childIndex(childIdx) {}
};


std::string str(const glm::vec3& vec)
{
    std::ostringstream oss;
    oss << "vec3(" 
        << vec.x << ", " 
        << vec.y << ", " 
        << vec.z 
        << ")";
    return oss.str();
}

float nodeCost(const Node& node)
{
    glm::vec3 size = node.bounds.size();
    float halfArea = size.x * (size.y + size.z) + size.y * size.z;
    return halfArea * node.triangleCount;
}

float nodeCost(const glm::vec3& size, int triangleCount)
{
    float halfArea = size.x * (size.y + size.z) + size.y * size.z;
    return halfArea * triangleCount;
}

float evaluateSplit(const Node& node, int splitAxis, float splitPos, const std::vector<BVHTriangle>& triangles)
{
    BoundingBox boundsA = {};
    BoundingBox boundsB = {};
    int numInA = 0;
    int numInB = 0;

    for (int i = node.triangleIndex; i < node.triangleIndex + node.triangleCount; i++)
    {
        const BVHTriangle& tri = triangles[i];
        if (tri.center[splitAxis] < splitPos)
        {
            boundsA.growToInclude(tri);
            numInA++;
        }
        else 
        {
            boundsB.growToInclude(tri);
            numInB++;
        }
    }

    return nodeCost(boundsA.size(), numInA) + nodeCost(boundsB.size(), numInB);
}

void chooseSplit(int& splitAxis, float& splitPos, float& cost, const Node& node, const std::vector<BVHTriangle>& triangles)
{
    const int numTestPerAxis = 10;
    cost = 1e32f;
    splitPos = 0;
    splitAxis = 0;

    for (int axis = 0; axis < 3; axis++)
    {
        float boundsStart = node.bounds.min[axis];
        float boundsEnd = node.bounds.max[axis];

        for (int i = 0; i < numTestPerAxis; i++)
        {
            float t = float(i + 1) / (numTestPerAxis + 1);
            float pos = boundsStart + (boundsEnd - boundsStart) * t;
            float costTmp = evaluateSplit(node, axis, pos, triangles);

            if (costTmp < cost)
            {
                cost = costTmp;
                splitPos = pos;
                splitAxis = axis;
            }
        }
    }
}

class BVH
{
public:
    std::vector<Node> allNodes;

    BVH(std::vector<BVHTriangle>& bvhTriangles, std::vector<RTXTriangle>& rtxTriangles)
    {
        std::cout << "Building BVH..." << std::endl;

        BoundingBox bounds;
        for (const BVHTriangle& tri : bvhTriangles)
            bounds.growToInclude(tri);
        bounds.expand();

        allNodes.push_back(Node(bounds, 0, static_cast<int>(bvhTriangles.size()), -1));
        split(0, 1, bvhTriangles, rtxTriangles);

        std::cout << "Built BVH." << std::endl;
    }

    std::string string(BoundingBox bbox)
    {
        return "Min: " + str(bbox.min) + "\nMax: " + str(bbox.max) + "\n";
    }

    void split(int rootIndex, int depth, std::vector<BVHTriangle>& bvhTriangles, std::vector<RTXTriangle>& rtxTriangles)
    {
        if (depth == MAX_DEPTH || allNodes[rootIndex].triangleCount < 1) 
            return;

        int splitAxis;
        float splitPos;
        float cost;
        chooseSplit(splitAxis, splitPos, cost, allNodes[rootIndex], bvhTriangles);
        if (cost >= nodeCost(allNodes[rootIndex])) return;
        
        Node childA = Node(BoundingBox(), allNodes[rootIndex].triangleIndex, 0, -1);
        Node childB = Node(BoundingBox(), allNodes[rootIndex].triangleIndex, 0, -1);

        for (int i = allNodes[rootIndex].triangleIndex; i < allNodes[rootIndex].triangleIndex + allNodes[rootIndex].triangleCount; i++)
        {
            bool inA = bvhTriangles[i].center[splitAxis] < splitPos;
            Node* child = inA ? &childA : &childB;
            child->bounds.growToInclude(bvhTriangles[i]);
            child->triangleCount += 1;

            if (inA)
            {
                int swap = childA.triangleIndex + childA.triangleCount - 1;
                std::swap(bvhTriangles[i], bvhTriangles[swap]);
                std::swap(rtxTriangles[i], rtxTriangles[swap]);
                childB.triangleIndex += 1;
            }
        }

        childA.bounds.expand();
        childB.bounds.expand();

        if (childA.triangleCount > 0 || childB.triangleCount > 0)
        {
            int childAIndex = allNodes.size();
            allNodes[rootIndex].childIndex = childAIndex;
            allNodes.push_back(childA);
            allNodes.push_back(childB);
            split(childAIndex, depth + 1, bvhTriangles, rtxTriangles);
            split(childAIndex + 1, depth + 1, bvhTriangles, rtxTriangles);
        }

        if (allNodes[rootIndex].childIndex == -1)
        {
            std::cout << "Depth: " << depth << std::endl;
            std::cout << "RTXTriangle index: " << allNodes[rootIndex].triangleIndex << std::endl;
            std::cout << "RTXTriangle counts: " << allNodes[rootIndex].triangleCount << std::endl;
            std::cout << ' ' << std::endl;
        }
    }
};