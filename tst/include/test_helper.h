#include <gtest/gtest.h>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace maye_sql {

class LoggingEnvironment : public ::testing::Environment {
 public:
  void SetUp() override {

    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_default_logger(console);
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("%^[%T] [%l] %v%$");
    
    spdlog::info("Global Logging Environment Initialized");
  }

  void TearDown() override {
    spdlog::shutdown();
  }
};

}