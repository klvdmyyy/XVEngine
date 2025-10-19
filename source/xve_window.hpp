#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "logger.hpp"

class XveWindow : Logger {
private:
  SDL_Window *window;

public:
  XveWindow(const std::string &title, uint32_t width, uint32_t height);
  ~XveWindow();

  SDL_Window *handle() const { return window; }
  void createSurface(vk::Instance instance, VkSurfaceKHR *surface) {
    log(LogLevel::Debug, "Creating window surface...");
    if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, surface)) {
      log(LogLevel::Error, "{}", SDL_GetError());
      throw std::runtime_error("Failed to create window surface");
    }
  }

  std::vector<const char *> getVkInstanceExtensions() {
    uint32_t extensionsCount;
    const char *const *extensions =
        SDL_Vulkan_GetInstanceExtensions(&extensionsCount);

    std::vector<const char *> extensionsArray(extensionsCount);
    for (uint32_t i = 0; i < extensionsCount; i++) {
      extensionsArray[i] = extensions[i];
    }

    return extensionsArray;
  }

  vk::Extent2D getExtent2D() {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    return vk::Extent2D{
        static_cast<uint32_t>(w),
        static_cast<uint32_t>(h),
    };
  }
};
