#include <gtest/gtest.h>
#include "test_helper.h"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  
  ::testing::AddGlobalTestEnvironment(new maye_sql::LoggingEnvironment);
  
  return RUN_ALL_TESTS();
}