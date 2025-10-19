#pragma once

#include "logger.hpp"
#include "xve_device.hpp"
#include "xve_window.hpp"
#include <VkBootstrap.h>
#include <vector>
#include <vulkan/vulkan.hpp>

class XveSwapChain : Logger {
private:
  static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

  vkb::Swapchain bSwapChain;

  std::vector<vk::Framebuffer> swapChainFramebuffers;
  vk::RenderPass renderPass;

  std::vector<vk::Image> depthImages;
  std::vector<vk::DeviceMemory> depthImageMemorys;
  std::vector<vk::ImageView> depthImageViews;
  std::vector<vk::Image> swapChainImages;
  std::vector<vk::ImageView> swapChainImageViews;

  XveDevice &device;
  vk::Extent2D windowExtent;

  std::vector<vk::Semaphore> imageAvailableSemaphores;
  std::vector<vk::Semaphore> renderFinishedSemaphores;
  std::vector<vk::Fence> inFlightFences;
  std::vector<vk::Fence> imagesInFlight;
  size_t currentFrame = 0;

  vk::Extent2D swapChainExtent;

  void createSwapChain();
  void createImageViews();
  void createDepthResources();
  void createRenderPass();
  void createFramebuffers();
  void createSyncObjects();

public:
  vk::Extent2D getSwapChainExtent() const { return swapChainExtent; }
  vk::RenderPass getRenderPass() const { return renderPass; }
  vk::Framebuffer getFramebuffer(size_t i) const {
    return swapChainFramebuffers[i];
  }
  uint32_t imageCount() const { return bSwapChain.image_count; }

  vk::Format findDepthFormat() {
    return device.findSupportedFormat(
        {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint,
         vk::Format::eD24UnormS8Uint},
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment);
  }

  XveSwapChain(XveDevice &deviceRef, vk::Extent2D windowExtent);
  ~XveSwapChain();

  vk::Result acquireNextImage(uint32_t *imageIndex);
  vk::Result submitCommandBuffers(const vk::CommandBuffer *buffers,
                                  uint32_t *imageIndex);
};
