#include <SDL3/SDL.h>

#include <exception>
#include <fstream>
#include <iostream>

#include "xve_app.hpp"

int main() {
  std::ofstream logFile{"log.txt"};
  Logger::setWriteFunction([&logFile](const std::string &msg) {
    logFile << msg << std::endl << std::endl;
  });

  try {
    XveApp app{};
    app.run();
  } catch (const std::exception &e) {
    Logger().log(LogLevel::Error, "{}", e.what());
  }
  logFile.close();
  return 0;
}
