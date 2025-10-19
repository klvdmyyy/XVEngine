#include "xve_device.hpp"
#include "VkBootstrap.h"
#include "config.h"
#include "logger.hpp"

#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan_core.h>

class VulkanValidation : public Logger {
public:
};

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
              void *pUserData) {
  VulkanValidation instance;
#define _VK_LOG(LEVEL)                                                         \
  instance.log(LogLevel::LEVEL, "{}", pCallbackData->pMessage);
  if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    _VK_LOG(Error);
  } else if (messageSeverity >=
             VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    _VK_LOG(Warning);
  } else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
    _VK_LOG(Info);
  } else {
    _VK_LOG(Debug);
  }
#undef _VK_LOG

  return VK_SUCCESS;
}

XveDevice::XveDevice(XveWindow &windowRef) : window(windowRef) {
  vkb::InstanceBuilder builder;
  vkb::Instance bInstance =
      builder.set_app_name(APP_NAME)
          .set_app_version(VK_MAKE_VERSION(APP_VMAJOR, APP_VMINOR, APP_VPATCH))
          .set_engine_name(ENGINE_NAME)
          .set_engine_version(
              VK_MAKE_VERSION(ENGINE_VMAJOR, ENGINE_VMINOR, ENGINE_VPATCH))
          .request_validation_layers()
          .add_debug_messenger_severity(
              VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
              VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
              VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
          .add_debug_messenger_type(
              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
              // VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT |
              VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
          .set_debug_callback(debugCallback)
          .enable_extensions(window.getVkInstanceExtensions())
          .build()
          .value();

  // instance = vk::UniqueHandle<vk::Instance,
  // vk::detail::DispatchLoaderStatic>(
  //     bInstance.instance);

  instance = bInstance.instance;
  debugMessenger = bInstance.debug_messenger;

  VkSurfaceKHR cSurface;

  window.createSurface(instance, &cSurface);
  surface = cSurface;

  vkb::PhysicalDeviceSelector physicalDeviceSelector{bInstance};
  vkb::PhysicalDevice bPhysicalDevice =
      physicalDeviceSelector.set_surface(surface).select().value();

  physicalDevice = bPhysicalDevice.physical_device;

  vkb::DeviceBuilder deviceBuilder{bPhysicalDevice};
  bDevice = deviceBuilder.build().value();

  device = bDevice.device;

  graphicsQueue = bDevice.get_queue(vkb::QueueType::graphics).value();
  presentQueue = bDevice.get_queue(vkb::QueueType::present).value();

  auto commandPoolCreateInfo = vk::CommandPoolCreateInfo{
      vk::CommandPoolCreateFlagBits::eTransient |
          vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      bDevice.get_queue_index(vkb::QueueType::graphics).value()};

  commandPool = device.createCommandPool(commandPoolCreateInfo);
}

XveDevice::~XveDevice() {
  device.destroyCommandPool(commandPool);
  device.destroy();
  instance.destroySurfaceKHR(surface);
  vkb::destroy_debug_utils_messenger(instance, debugMessenger);
  instance.destroy();
}

vk::Format
XveDevice::findSupportedFormat(const std::vector<vk::Format> &candidates,
                               vk::ImageTiling tiling,
                               vk::FormatFeatureFlags features) {
  for (auto format : candidates) {
    auto props = physicalDevice.getFormatProperties(format);

    if (tiling == vk::ImageTiling::eLinear &&
        (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == vk::ImageTiling::eOptimal &&
               (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }

  throw std::runtime_error("Failed to find supported format!");
}

uint32_t XveDevice::findMemoryType(uint32_t typeFilter,
                                   vk::MemoryPropertyFlags properties) {
  auto memProperties = physicalDevice.getMemoryProperties();
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) &&
        (memProperties.memoryTypes[i].propertyFlags & properties)) {
      return i;
    }
  }

  throw std::runtime_error("Failed to find suitable memory type.");
}
