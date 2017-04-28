//
// Created by Srihari Pratapa on 4/6/17.
//

#ifndef TC_RENDERER_VK_UTILS_H

#include "vulkan_wrapper.h"
#include  <vector>
#include <array>
#define GLM_FORCE_RADIANS
#include <../glm/glm.hpp>
#include <../glm/gtc/matrix_transform.hpp>

struct Vertex {
glm::vec2 pos;
glm::vec3 color;
glm::vec2 tex_coord;
static VkVertexInputBindingDescription GetBindingDescription() {
VkVertexInputBindingDescription binding_description = {};
binding_description.binding = 0;
binding_description.stride = sizeof(Vertex);
binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
return binding_description;
}
static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions() {
std::array<VkVertexInputAttributeDescription, 3> attribute_descriptions = {};
attribute_descriptions[0].binding = 0;
attribute_descriptions[0].location = 0;
attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
attribute_descriptions[0].offset = offsetof(Vertex, pos);
attribute_descriptions[1].binding = 0;
attribute_descriptions[1].location = 1;
attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
attribute_descriptions[1].offset = offsetof(Vertex, color);
attribute_descriptions[2].binding = 0;
attribute_descriptions[2].location = 2;
attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
attribute_descriptions[2].offset = offsetof(Vertex, tex_coord);
return attribute_descriptions;
}
};
const std::vector<Vertex> vertices = {
{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f} },
{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f} },
{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} }
};
/*const std::vector<Vertex> vertices = {*/
//{{-0.5f, -0.5f},{1.0f, 0.0f, 0.0f}},
//{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
//{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
//{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
//{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
//{{-0.5f, -0.5f},{1.0f, 0.0f, 0.0f}}
/*};*/
/*std::vector<Vertex> vertices = {*/
//{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
//{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
//{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
/*};*/
const std::vector<uint16_t> indices = {
0, 1, 2, 2, 3, 0
};
struct UniformBufferObject {
glm::mat4 model;
glm::mat4 view;
glm::mat4 proj;
};



#define TC_RENDERER_VK_UTILS_H
#endif //TC_RENDERER_VK_UTILS_H
