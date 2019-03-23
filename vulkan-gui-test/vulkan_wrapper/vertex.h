//
//  vertex.hpp
//  vulkan-gui-test
//
//  Created by Rafael Sabino on 3/23/19.
//  Copyright © 2019 Rafael Sabino. All rights reserved.
//

#pragma once


#include <glm/glm.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vector>

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif

#include <glm/gtx/hash.hpp>

namespace vk {
    
    class vertex
    {
    public:
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 uvCoord;
        glm::vec3 normal;
        
        
        vertex(glm::vec3 pos, glm::vec3 color, glm::vec2 uvCoord, glm::vec3 normal)
        : pos(pos), color(color), uvCoord(uvCoord), normal(normal)
        {}
        
        bool operator==(const vertex& other) const
        {
            return pos == other.pos && color == other.color && uvCoord == other.uvCoord &&
            normal == other.normal;
        }
        
        static VkVertexInputBindingDescription getBindingDescription()
        {
            VkVertexInputBindingDescription vertexInputBindingDescription;
            vertexInputBindingDescription.binding = 0;
            vertexInputBindingDescription.stride = sizeof(vertex);
            vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            
            return vertexInputBindingDescription;
        }
        
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions()
        {
            std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions(4);
            
            vertexInputAttributeDescriptions[0].location = 0;
            vertexInputAttributeDescriptions[0].binding = 0;
            vertexInputAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            vertexInputAttributeDescriptions[0].offset = offsetof(vertex, pos);
            
            vertexInputAttributeDescriptions[1].location = 1;
            vertexInputAttributeDescriptions[1].binding = 0;
            vertexInputAttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            vertexInputAttributeDescriptions[1].offset = offsetof(vertex, color);
            
            vertexInputAttributeDescriptions[2].location = 2;
            vertexInputAttributeDescriptions[2].binding = 0;
            vertexInputAttributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            vertexInputAttributeDescriptions[2].offset = offsetof(vertex, uvCoord);
            
            vertexInputAttributeDescriptions[3].location = 3;
            vertexInputAttributeDescriptions[3].binding = 0;
            vertexInputAttributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
            vertexInputAttributeDescriptions[3].offset = offsetof(vertex, normal);
            
            return vertexInputAttributeDescriptions;
            
        }
    };
}


namespace std
{
    template<> struct hash<vk::vertex>
    {
        size_t operator()(vk::vertex const &vert) const
        {
            size_t h1 = hash<glm::vec3>()(vert.pos);
            size_t h2 = hash<glm::vec3>()(vert.color);
            size_t h3 = hash<glm::vec2>()(vert.uvCoord);
            
            return ((h1 ^ (h2 << 1)) >> 1) ^ h3;
        }
    };
}

