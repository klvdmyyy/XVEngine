#include "xve_swap_chain.hpp"
#include "VkBootstrap.h"
#include <limits>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>

XveSwapChain::XveSwapChain(XveDevice &deviceRef, vk::Extent2D windowExtent)
    : device(deviceRef), swapChainExtent(windowExtent) {
  createSwapChain();
  createImageViews();
  createRenderPass();
  createDepthResources();
  createFramebuffers();
  createSyncObjects();
}

void XveSwapChain::createSwapChain() {
  vkb::SwapchainBuilder swapChainBuilder{device.getBDevice()};
  bSwapChain = swapChainBuilder.set_old_swapchain(nullptr).build().value();
}

void XveSwapChain::createImageViews() {
  auto cSwapChainImageViews = bSwapChain.get_image_views().value();
  for (auto &iview : cSwapChainImageViews) {
    swapChainImageViews.push_back(iview);
  }
}

void XveSwapChain::createRenderPass() {
  auto depthAttachment = vk::AttachmentDescription{
      vk::AttachmentDescriptionFlags(),
      findDepthFormat(),
      vk::SampleCountFlagBits::e1,
      vk::AttachmentLoadOp::eClear,
      vk::AttachmentStoreOp::eDontCare,
      vk::AttachmentLoadOp::eDontCare,
      vk::AttachmentStoreOp::eDontCare,
      vk::ImageLayout::eUndefined,
      vk::ImageLayout::eDepthStencilAttachmentOptimal,
  };

  auto depthAttachmentRef = vk::AttachmentReference{
      1,
      vk::ImageLayout::eDepthStencilAttachmentOptimal,
  };

  auto colorAttachment = vk::AttachmentDescription{
      vk::AttachmentDescriptionFlags(), (vk::Format)bSwapChain.image_format,
      vk::SampleCountFlagBits::e1,      vk::AttachmentLoadOp::eClear,
      vk::AttachmentStoreOp::eDontCare, vk::AttachmentLoadOp::eDontCare,
      vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined,
      vk::ImageLayout::ePresentSrcKHR,
  };

  auto colorAttachmentRef = vk::AttachmentReference{
      0,
      vk::ImageLayout::eColorAttachmentOptimal,
  };

  auto subpass = vk::SubpassDescription{
      vk::SubpassDescriptionFlags(),
      vk::PipelineBindPoint::eGraphics,
      {},
      {},
      1,
      &colorAttachmentRef,
      {},
      &depthAttachmentRef,
  };

  auto dependency = vk::SubpassDependency{
      vk::SubpassExternal,
      0,
      vk::PipelineStageFlagBits::eColorAttachmentOutput |
          vk::PipelineStageFlagBits::eEarlyFragmentTests,
      vk::PipelineStageFlagBits::eColorAttachmentOutput |
          vk::PipelineStageFlagBits::eEarlyFragmentTests,
      {},
      vk::AccessFlagBits::eColorAttachmentWrite |
          vk::AccessFlagBits::eDepthStencilAttachmentWrite,
  };

  std::array<vk::AttachmentDescription, 2> attachments = {colorAttachment,
                                                          depthAttachment};

  auto createInfo = vk::RenderPassCreateInfo{
      vk::RenderPassCreateFlags(),
      static_cast<uint32_t>(attachments.size()),
      attachments.data(),
      1,
      &subpass,
      1,
      &dependency,
  };

  try {
    renderPass = device.getDevice().createRenderPass(createInfo);
  } catch (const vk::SystemError &e) {
    log(LogLevel::Error, "{}", e.what());
    throw std::runtime_error("Failed to create render pass");
  }
}

/// Hard zone):

vk::Result XveSwapChain::acquireNextImage(uint32_t *imageIndex) {
  device.getDevice().waitForFences(1, &inFlightFences[currentFrame], vk::True,
                                   std::numeric_limits<uint64_t>::max());
  return device.getDevice().acquireNextImageKHR(
      bSwapChain.swapchain, std::numeric_limits<uint64_t>::max(),
      imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, imageIndex);
}

vk::Result XveSwapChain::submitCommandBuffers(const vk::CommandBuffer *buffers,
                                              uint32_t *imageIndex) {
  if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
    device.getDevice().waitForFences(1, &imagesInFlight[*imageIndex], VK_TRUE,
                                     UINT64_MAX);
  }
  imagesInFlight[*imageIndex] = inFlightFences[currentFrame];

  vk::SubmitInfo submitInfo = {};

  vk::Semaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
  vk::PipelineStageFlags waitStages[] = {
      vk::PipelineStageFlagBits::eColorAttachmentOutput};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = buffers;

  vk::Semaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  device.getDevice().resetFences(1, &inFlightFences[currentFrame]);
  device.getGraphicsQueue().submit(1, &submitInfo,
                                   inFlightFences[currentFrame]);

  vk::PresentInfoKHR presentInfo = {};
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  vk::SwapchainKHR swapChains[] = {bSwapChain.swapchain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;

  presentInfo.pImageIndices = imageIndex;

  auto result = device.getPresentQueue().presentKHR(&presentInfo);

  currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

  return result;
}

/// Potential errors zone:

void XveSwapChain::createDepthResources() {
  vk::Format depthFormat = findDepthFormat();
  vk::Extent2D swapChainExtent = getSwapChainExtent();

  depthImages.resize(bSwapChain.image_count);
  depthImageMemorys.resize(bSwapChain.image_count);
  depthImageViews.resize(bSwapChain.image_count);

  for (size_t i = 0; i < depthImages.size(); i++) {
    auto imageInfo = vk::ImageCreateInfo{
        vk::ImageCreateFlags(),
        vk::ImageType::e2D,
        depthFormat,
        vk::Extent3D{swapChainExtent.width, swapChainExtent.height, 1},
        1,
        1,
        vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment,
        vk::SharingMode::eExclusive,
        {},
        {},
        vk::ImageLayout::eUndefined,
    };

    try {
      depthImages[i] = device.getDevice().createImage(imageInfo);
    } catch (const vk::SystemError &e) {
      throw std::runtime_error(
          std::format("Failed to create image. Error: {}", e.what()));
    }

    auto memoryRequirements =
        device.getDevice().getImageMemoryRequirements(depthImages[i]);

    auto allocInfo = vk::MemoryAllocateInfo{
        memoryRequirements.size,
        device.findMemoryType(memoryRequirements.memoryTypeBits,
                              vk::MemoryPropertyFlagBits::eDeviceLocal)};

    try {
      depthImageMemorys[i] = device.getDevice().allocateMemory(allocInfo);
      device.getDevice().bindImageMemory(depthImages[i], depthImageMemorys[i],
                                         0);
    } catch (const vk::SystemError &e) {
      throw std::runtime_error(std::format(
          "Failed to allocate or bind image memory. Error: {}", e.what()));
    }

    auto viewInfo = vk::ImageViewCreateInfo{
        vk::ImageViewCreateFlags(),
        depthImages[i],
        vk::ImageViewType::e2D,
        depthFormat,
        {},
        {
            vk::ImageAspectFlagBits::eDepth,
            0,
            1,
            0,
            1,
        },
    };

    try {
      depthImageViews[i] = device.getDevice().createImageView(viewInfo);
    } catch (const vk::SystemError &e) {
      throw std::runtime_error(
          std::format("Failed to create image view. Error: {}", e.what()));
    }
  }
}

void XveSwapChain::createFramebuffers() {
  swapChainFramebuffers.resize(bSwapChain.image_count);

  for (size_t i = 0; i < bSwapChain.image_count; i++) {
    std::array<vk::ImageView, 2> attachments = {swapChainImageViews[i],
                                                depthImageViews[i]};

    vk::Extent2D swapChainExtent = getSwapChainExtent();
    auto createInfo = vk::FramebufferCreateInfo{
        vk::FramebufferCreateFlags(),
        renderPass,
        static_cast<uint32_t>(attachments.size()),
        attachments.data(),
        swapChainExtent.width,
        swapChainExtent.height,
        1,
    };

    try {
      swapChainFramebuffers[i] =
          device.getDevice().createFramebuffer(createInfo);
    } catch (const vk::SystemError &e) {
      throw std::runtime_error(
          std::format("Failed to create framebuffer. Error: {}", e.what()));
    }
  }
}

void XveSwapChain::createSyncObjects() {
  imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
  imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

  auto semaphoreInfo = vk::SemaphoreCreateInfo{
      vk::SemaphoreCreateFlags(),
  };

  auto fenceInfo = vk::FenceCreateInfo{
      vk::FenceCreateFlagBits::eSignaled,
  };

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    try {
      imageAvailableSemaphores[i] =
          device.getDevice().createSemaphore(semaphoreInfo);
      renderFinishedSemaphores[i] =
          device.getDevice().createSemaphore(semaphoreInfo);
      inFlightFences[i] = device.getDevice().createFence(fenceInfo);
    } catch (const vk::SystemError &e) {
      throw std::runtime_error(std::format(
          "Failed to create Semaphore or Fence. Error: {}", e.what()));
    }
  }
}

XveSwapChain::~XveSwapChain() {
  for (auto imageView : swapChainImageViews) {
    device.getDevice().destroyImageView(imageView);
  }
  swapChainImageViews.clear();

  if (bSwapChain.swapchain != nullptr) {
    vkb::destroy_swapchain(bSwapChain);
    bSwapChain.swapchain = nullptr;
  }

  for (size_t i = 0; i < depthImages.size(); i++) {
    device.getDevice().destroyImageView(depthImageViews[i]);
    device.getDevice().destroyImage(depthImages[i]);
    device.getDevice().freeMemory(depthImageMemorys[i]);
  }

  for (auto framebuffer : swapChainFramebuffers) {
    device.getDevice().destroyFramebuffer(framebuffer);
  }
  device.getDevice().destroyRenderPass(renderPass);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    device.getDevice().destroySemaphore(renderFinishedSemaphores[i]);
    device.getDevice().destroySemaphore(imageAvailableSemaphores[i]);
    device.getDevice().destroyFence(inFlightFences[i]);
  }
}
