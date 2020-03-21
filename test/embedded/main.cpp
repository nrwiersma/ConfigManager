#if defined(ARDUINO) && defined(UNIT_TEST)
#include "Arduino.h"

#include <unity.h>

#include "ConfigParameterTest.h"
#include "ConfigStringParameterTest.h"

bool DEBUG_MODE = false;

void setup() {
  delay(2000);

  UNITY_BEGIN();

  // ConfigParameterTest
  RUN_TEST(test_sets_config_parameter_mode);
  RUN_TEST(test_config_parameter_stored_as_json);
  RUN_TEST(test_config_parameter_stored_from_json);
  RUN_TEST(test_config_parameter_clear_data);

  // ConfigStringParameterTest
  RUN_TEST(test_sets_config_string_parameter_mode);
  RUN_TEST(test_config_string_parameter_stored_as_json);
  RUN_TEST(test_config_string_parameter_stored_from_json);
  RUN_TEST(test_config_string_parameter_clear_data);

  UNITY_END();
}

void loop() {}
#endif
