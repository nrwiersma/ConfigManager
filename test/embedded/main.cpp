#if defined(ARDUINO) && defined(UNIT_TEST)
#include "Arduino.h"

#include <unity.h>

#include "ConfigParameterTest.h"

void setup() {
  delay(2000);
  UNITY_BEGIN();

  RUN_TEST(test_sets_config_parameter_mode);

  UNITY_END();
}

void loop() {}
#endif
