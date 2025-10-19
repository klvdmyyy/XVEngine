#include "xve_pipeline.hpp"

#include <fstream>

std::vector<char> XvePipeline::readFile(const std::string &filepath) {
  std::ifstream file{filepath, std::ios::ate | std::ios::binary};

  if (!file.is_open()) {
    throw std::runtime_error(
        std::format("Can't open shader file: {}", filepath));
  }

  size_t fileSize = static_cast<size_t>(file.tellg());

  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();
  return buffer;
}

vk::ShaderModule
XvePipeline::createShaderModule(const std::vector<char> &code) {
  vk::ShaderModuleCreateInfo createInfo(
      vk::ShaderModuleCreateFlags(), static_cast<uint32_t>(code.size()),
      reinterpret_cast<const uint32_t *>(code.data()));

  return device.getDevice().createShaderModule(createInfo);
}

PipelineConfigInfo XvePipeline::defaultPipelineConfigInfo(uint32_t width,
                                                          uint32_t height) {
  PipelineConfigInfo configInfo;

  configInfo.inputAssemblyInfo = vk::PipelineInputAssemblyStateCreateInfo(
      vk::PipelineInputAssemblyStateCreateFlags(),
      vk::PrimitiveTopology::eTriangleList, false);

  configInfo.viewport.x = 0.0f;
  configInfo.viewport.y = 0.0f;
  configInfo.viewport.width = static_cast<float>(width);
  configInfo.viewport.height = static_cast<float>(height);
  configInfo.viewport.minDepth = 0.0f;
  configInfo.viewport.maxDepth = 1.0f;

  configInfo.scissor.offset = vk::Offset2D{0, 0};
  configInfo.scissor.extent = vk::Extent2D{width, height};

  configInfo.viewportInfo = vk::PipelineViewportStateCreateInfo(
      vk::PipelineViewportStateCreateFlags(), 1, &configInfo.viewport, 1,
      &configInfo.scissor);

  configInfo.rasterizationInfo = vk::PipelineRasterizationStateCreateInfo{
      vk::PipelineRasterizationStateCreateFlags(),
      vk::False,
      vk::False,
      vk::PolygonMode::eFill,
      vk::CullModeFlagBits::eNone,
      vk::FrontFace::eClockwise,
      vk::False,
      0.0f,
      0.0f,
      0.0f,
      1.0f};

  configInfo.multisampleInfo = vk::PipelineMultisampleStateCreateInfo(
      vk::PipelineMultisampleStateCreateFlags(), vk::SampleCountFlagBits::e1,
      vk::False, 1.0f, nullptr, vk::False, vk::False);

  configInfo.colorBlendAttachment = vk::PipelineColorBlendAttachmentState(
      false, vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

  configInfo.colorBlendInfo = vk::PipelineColorBlendStateCreateInfo(
      vk::PipelineColorBlendStateCreateFlags(), vk::False, vk::LogicOp::eCopy,
      1, &configInfo.colorBlendAttachment, {0.0f, 0.0f, 0.0f, 0.0f});

  configInfo.depthStencilInfo = vk::PipelineDepthStencilStateCreateInfo{
      vk::PipelineDepthStencilStateCreateFlags(),
      vk::True,
      vk::True,
      vk::CompareOp::eLess,
      vk::False,
      vk::False,
      {},
      {},
      0.0f,
      1.0f};

  return configInfo;
}

void XvePipeline::bind(vk::CommandBuffer commandBuffer) {
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                             graphicsPipeline);
}

XvePipeline::XvePipeline(XveDevice &device_, const std::string &vertFilepath,
                         const std::string &fragFilepath,
                         const PipelineConfigInfo &configInfo)
    : device(device_) {
  if (configInfo.pipelineLayout == nullptr) {
    throw std::logic_error("Can't create graphics pipeline: no pipelineLayout "
                           "provided in configInfo.");
  }
  if (configInfo.renderPass == nullptr) {
    throw std::logic_error("Can't create graphics pipeline: no renderPass "
                           "provided in configInfo.");
  }

  auto vertCode = readFile(vertFilepath);
  auto fragCode = readFile(fragFilepath);

  log(LogLevel::Info, "Vertex Shader Code Size: {}", vertCode.size());
  log(LogLevel::Info, "Fragment Shader Code Size: {}", fragCode.size());

  vertShaderModule = createShaderModule(vertCode);
  fragShaderModule = createShaderModule(fragCode);

  vk::PipelineShaderStageCreateInfo shaderStages[2] = {
      vk::PipelineShaderStageCreateInfo{vk::PipelineShaderStageCreateFlags(),
                                        vk::ShaderStageFlagBits::eVertex,
                                        vertShaderModule, "main"},
      vk::PipelineShaderStageCreateInfo{vk::PipelineShaderStageCreateFlags(),
                                        vk::ShaderStageFlagBits::eFragment,
                                        fragShaderModule, "main"},
  };

  auto vertexInputInfo = vk::PipelineVertexInputStateCreateInfo{
      vk::PipelineVertexInputStateCreateFlags(),
      0,
      nullptr,
      0,
      nullptr,
      nullptr};

  auto createInfo =
      vk::GraphicsPipelineCreateInfo{{},
                                     2,
                                     shaderStages,
                                     &vertexInputInfo,
                                     &configInfo.inputAssemblyInfo,
                                     nullptr,
                                     &configInfo.viewportInfo,
                                     &configInfo.rasterizationInfo,
                                     &configInfo.multisampleInfo,
                                     &configInfo.depthStencilInfo,
                                     &configInfo.colorBlendInfo,
                                     nullptr,

                                     configInfo.pipelineLayout,
                                     configInfo.renderPass,
                                     configInfo.subpass,

                                     nullptr,
                                     -1,
                                     nullptr};

  auto result = device.getDevice().createGraphicsPipeline(nullptr, createInfo);
  graphicsPipeline = result.value;
}

XvePipeline::~XvePipeline() {
  device.getDevice().destroyShaderModule(vertShaderModule);
  device.getDevice().destroyShaderModule(fragShaderModule);
  device.getDevice().destroyPipeline(graphicsPipeline);
}
