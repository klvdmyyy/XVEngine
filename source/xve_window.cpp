#include "xve_window.hpp"

#include <stdexcept>

XveWindow::XveWindow(const std::string &title, uint32_t width,
                     uint32_t height) {
  log(LogLevel::Debug, "Creating window...", title);
  window = SDL_CreateWindow(title.c_str(), width, height, SDL_WINDOW_VULKAN);
  if (window == nullptr) {
    log(LogLevel::Error, "{}", SDL_GetError());
    throw std::runtime_error(std::format("Failed to create window: {}", title));
  }
}

XveWindow::~XveWindow() {
  log(LogLevel::Debug, "Destroying window...");
  SDL_DestroyWindow(window);
}
