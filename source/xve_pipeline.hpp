#pragma once

#include "xve_device.hpp"

struct PipelineConfigInfo {
  vk::Viewport viewport;
  vk::Rect2D scissor;
  vk::PipelineViewportStateCreateInfo viewportInfo;
  vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  vk::PipelineRasterizationStateCreateInfo rasterizationInfo;
  vk::PipelineMultisampleStateCreateInfo multisampleInfo;
  vk::PipelineColorBlendAttachmentState colorBlendAttachment;
  vk::PipelineColorBlendStateCreateInfo colorBlendInfo;
  vk::PipelineDepthStencilStateCreateInfo depthStencilInfo;
  vk::PipelineLayout pipelineLayout = nullptr;
  vk::RenderPass renderPass = nullptr;
  uint32_t subpass = 0;
};

class XvePipeline : Logger {
private:
  static std::vector<char> readFile(const std::string &filepath);
  XveDevice &device;

  vk::Pipeline graphicsPipeline;
  vk::ShaderModule vertShaderModule;
  vk::ShaderModule fragShaderModule;

  vk::ShaderModule createShaderModule(const std::vector<char> &code);

public:
  XvePipeline(XveDevice &deviceRef, const std::string &vertFilepath,
              const std::string &fragFilepath,
              const PipelineConfigInfo &configInfo);
  ~XvePipeline();

  static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width,
                                                      uint32_t height);

  void bind(vk::CommandBuffer commandBuffer);
};
