#pragma once

#include "xve_window.hpp"
#include <VkBootstrap.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

class XveDevice {
private:
  vk::Instance instance;
  vk::DebugUtilsMessengerEXT debugMessenger;
  vk::SurfaceKHR surface;
  vk::PhysicalDevice physicalDevice;
  vk::Device device;
  vk::Queue graphicsQueue;
  vk::Queue presentQueue;
  vk::CommandPool commandPool;

  vkb::Device bDevice;

  XveWindow &window;

public:
  XveDevice(XveWindow &windowRef);
  ~XveDevice();

  vk::Instance getInstance() const { return instance; }
  vk::PhysicalDevice getPhysicalDevice() const { return physicalDevice; }
  vk::Device getDevice() const { return device; }
  vk::Queue getGraphicsQueue() const { return graphicsQueue; }
  vk::Queue getPresentQueue() const { return presentQueue; }
  vk::CommandPool getCommandPool() const { return commandPool; }

  vkb::Device getBDevice() const { return bDevice; }

  vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates,
                                 vk::ImageTiling tiling,
                                 vk::FormatFeatureFlags features);

  uint32_t findMemoryType(uint32_t bits, vk::MemoryPropertyFlags flags);

  void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                    vk::MemoryPropertyFlags properties, vk::Buffer &buffer,
                    vk::DeviceMemory &bufferMemory);
};
