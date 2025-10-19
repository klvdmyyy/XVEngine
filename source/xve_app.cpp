#include "xve_app.hpp"
#include "config.h"
#include "xve_pipeline.hpp"
#include <memory>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

XveApp::XveApp() {
  createPipelineLayout();
  createPipeline();
  createCommandBuffers();
}

XveApp::~XveApp() { device.getDevice().destroyPipelineLayout(pipelineLayout); }

void XveApp::run() {
  bool quit = false;
  while (!quit) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        quit = true;
      }
    }

    drawFrame();
  }
}

void XveApp::createPipelineLayout() {
  auto createInfo = vk::PipelineLayoutCreateInfo{
      vk::PipelineLayoutCreateFlags(), 0, nullptr, 0, nullptr,
  };

  pipelineLayout = device.getDevice().createPipelineLayout(createInfo);
}

void XveApp::createPipeline() {
  auto extent = swapChain.getSwapChainExtent();
  auto pipelineConfig =
      XvePipeline::defaultPipelineConfigInfo(extent.width, extent.height);
  pipelineConfig.renderPass = swapChain.getRenderPass();
  pipelineConfig.pipelineLayout = pipelineLayout;

  pipeline = std::make_unique<XvePipeline>(device, SIMPLE_SHADER_VERT,
                                           SIMPLE_SHADER_FRAG, pipelineConfig);
}

void XveApp::createCommandBuffers() {
  commandBuffers.resize(swapChain.imageCount());

  auto allocInfo = vk::CommandBufferAllocateInfo{
      device.getCommandPool(),
      vk::CommandBufferLevel::ePrimary,
      static_cast<uint32_t>(commandBuffers.size()),
  };

  try {
    commandBuffers = device.getDevice().allocateCommandBuffers(allocInfo);
  } catch (const vk::SystemError &e) {
    throw std::runtime_error(
        std::format("Failed to allocate command buffers. Error: {}", e.what()));
  }
  for (size_t i = 0; i < commandBuffers.size(); i++) {
    try {
      auto cmd = commandBuffers[i];

      auto beginInfo = vk::CommandBufferBeginInfo{};
      cmd.begin(beginInfo);

      std::array<vk::ClearValue, 2> clearValues{};
      clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
      clearValues[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};

      auto renderPassInfo = vk::RenderPassBeginInfo{
          swapChain.getRenderPass(),
          swapChain.getFramebuffer(i),
          {
              vk::Offset2D{0, 0},
              swapChain.getSwapChainExtent(),
          },
          static_cast<uint32_t>(clearValues.size()),
          clearValues.data(),
      };

      cmd.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

      pipeline->bind(cmd);
      cmd.draw(3, 1, 0, 0);

      cmd.endRenderPass();

      cmd.end();
    } catch (const vk::SystemError &e) {
      throw std::runtime_error(
          std::format("Failed to create command buffers. Error: {}", e.what()));
    }
  }
}

void XveApp::drawFrame() {
  uint32_t imageIndex;
  swapChain.acquireNextImage(&imageIndex);

  auto commandBuffers_ = std::vector<vk::CommandBuffer>{};

  swapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
}
