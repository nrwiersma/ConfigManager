#include <unity.h>

#include <stdio.h>
#include <ConfigParameter.h>

bool DEBUG_MODE = false;

void test_sets_config_parameter_mode() {
  const char *variable = "baphled";
  ConfigParameter<const char*> parameter = ConfigParameter<const char*>("username", &variable, both);

  TEST_ASSERT_EQUAL(both, parameter.getMode());
}

void test_config_parameter_stored_as_json() {
  const char *variable = "baphled";
  const char *expected = "{\"username\":\"baphled\"}";
  DynamicJsonBuffer jsonBuffer;
  JsonObject& obj = jsonBuffer.createObject();

  char body[50] = "\n";

  ConfigParameter<const char*> parameter = ConfigParameter<const char*>("username", &variable, get);

  parameter.toJson(&obj);
  obj.printTo(body);

  TEST_ASSERT_EQUAL_STRING(expected, body);
}

void test_config_parameter_stored_from_json() {
  const char *variable = "foo";
  const char *expected = "{\"username\":\"baphled\"}";
  char body[50] = "\n";

  DynamicJsonBuffer jsonBuffer;
  JsonObject& obj = jsonBuffer.parseObject(expected);

  ConfigParameter<const char*> parameter = ConfigParameter<const char*>("username", &variable, set);

  parameter.fromJson(&obj);

  parameter.toJson(&obj);
  obj.printTo(body);

  TEST_ASSERT_EQUAL_STRING(expected, body);
}

void test_config_parameter_clear_data() {
  const char *variable = "foo";
  const char *expected = "{\"username\":null}";
  char body[50] = "\n";

  DynamicJsonBuffer jsonBuffer;
  JsonObject& obj = jsonBuffer.createObject();

  ConfigParameter<const char*> parameter = ConfigParameter<const char*>("username", &variable, set);

  parameter.clearData();

  parameter.toJson(&obj);
  obj.printTo(body);

  TEST_ASSERT_EQUAL_STRING(expected, body);
}
