#pragma once

#include "xve_device.hpp"
#include <vector>
#include <vulkan/vulkan.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

class XveModel {
public:
  struct Vertex {
    glm::vec2 position;

    static std::vector<vk::VertexInputBindingDescription>
    getBindingDescription();
    static std::vector<vk::VertexInputAttributeDescription>
    getAttributeDescription();
  };

  XveModel(XveDevice &deviceRef, const std::vector<Vertex> &vertices);
  ~XveModel();

  XveModel(const XveModel &) = delete;
  XveModel &operator=(const XveModel &) = delete;

  void bind(vk::CommandBuffer commandBuffer);
  void draw(vk::CommandBuffer commandBuffer);

private:
  void createVertexBuffer(const std::vector<Vertex> &vertices);

  XveDevice &device;
  vk::Buffer vertexBuffer;
  vk::DeviceMemory vertexBufferMemory;
  uint32_t vertexCount;
};
