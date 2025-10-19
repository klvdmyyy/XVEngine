#pragma once

#include "xve_device.hpp"
#include "xve_pipeline.hpp"
#include "xve_swap_chain.hpp"
#include "xve_window.hpp"
#include <memory>

class XveApp {
public:
  XveApp();
  ~XveApp();

  void run();

private:
  void createPipelineLayout();
  void createPipeline();
  void createCommandBuffers();
  void drawFrame();

  static constexpr int WIDTH = 800;
  static constexpr int HEIGHT = 600;

  XveWindow window{"Game", WIDTH, HEIGHT};
  XveDevice device{window};
  XveSwapChain swapChain{device, window.getExtent2D()};
  std::unique_ptr<XvePipeline> pipeline;
  vk::PipelineLayout pipelineLayout;
  std::vector<vk::CommandBuffer> commandBuffers;
};
