#include <unity.h>

#include <stdio.h>
#include <ConfigParameter.h>

bool DEBUG_MODE = false;

void test_sets_config_parameter_mode() {
  const char *variable = "baphled";

  ConfigParameter<const char*> configParameters = ConfigParameter<const char*>("username", &variable, both);

  TEST_ASSERT_EQUAL(configParameters.getMode(), both);
}
