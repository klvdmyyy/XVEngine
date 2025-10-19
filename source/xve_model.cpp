#include "xve_model.hpp"
#include <vulkan/vulkan_enums.hpp>

XveModel::XveModel(XveDevice &deviceRef, const std::vector<Vertex> &vertices)
    : device(deviceRef) {
  createVertexBuffer(vertices);
}

XveModel::~XveModel() {
  device.getDevice().destroyBuffer(vertexBuffer);
  device.getDevice().freeMemory(vertexBufferMemory);
}

void XveModel::createVertexBuffer(const std::vector<Vertex> &vertices) {
  vertexCount = static_cast<uint32_t>(vertices.size());
  assert(vertexCount >= 3 && "Vertex count must be at least 3");

  vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

  device.createBuffer(bufferSize, vk::BufferUsageFlagBits::eVertexBuffer,
                      vk::MemoryPropertyFlagBits::eHostVisible |
                          vk::MemoryPropertyFlagBits::eHostCoherent,
                      vertexBuffer, vertexBufferMemory);

  void *data;
  device.getDevice().mapMemory(vertexBufferMemory, {}, bufferSize, {}, &data);
  memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
  device.getDevice().unmapMemory(vertexBufferMemory);
}

void XveModel::draw(vk::CommandBuffer commandBuffer) {
  commandBuffer.draw(vertexCount, 1, 0, 0);
}

void XveModel::bind(vk::CommandBuffer commandBuffer) {
  vk::Buffer buffers[] = {vertexBuffer};
  vk::DeviceSize offsets[] = {0};

  commandBuffer.bindVertexBuffers(0, 1, buffers, offsets);
}

std::vector<vk::VertexInputBindingDescription>
XveModel::Vertex::getBindingDescription() {
  std::vector<vk::VertexInputBindingDescription> bindingDescriptions(1);
  bindingDescriptions[0].binding = 0;
  bindingDescriptions[0].stride = sizeof(Vertex);
  bindingDescriptions[0].inputRate = vk::VertexInputRate::eVertex;
  return bindingDescriptions;
}

std::vector<vk::VertexInputAttributeDescription>
XveModel::Vertex::getAttributeDescription() {
  std::vector<vk::VertexInputAttributeDescription> attributeDescriptions(1);
  attributeDescriptions[0].binding = 0;
  attributeDescriptions[0].location = 0;
  attributeDescriptions[0].format = vk::Format::eR32G32Sfloat;
  attributeDescriptions[0].offset = 0;
  return attributeDescriptions;
}
