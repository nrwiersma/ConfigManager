#include <unity.h>

#include <stdio.h>
#include <ConfigStringParameter.h>

void test_sets_config_string_parameter_mode() {
  char *variable = "baphled";
  size_t length = 10;
  ConfigStringParameter parameter = ConfigStringParameter("username", variable, length, both);

  TEST_ASSERT_EQUAL(both, parameter.getMode());
}

void test_config_string_parameter_stored_as_json() {
  char *variable = "baphled";
  const char *expected = "{\"username\":\"baphled\"}";
  DynamicJsonBuffer jsonBuffer;
  JsonObject& obj = jsonBuffer.createObject();

  char body[50] = "\n";
  size_t length = 10;

  ConfigStringParameter parameter = ConfigStringParameter("username", variable, length, set);

  parameter.toJson(&obj);
  obj.printTo(body);

  TEST_ASSERT_EQUAL_STRING(expected, body);
}

void test_config_string_parameter_stored_from_json() {
  char *variable = "foo";
  const char *expected = "{\"username\":\"baphled\"}";
  char body[50] = "\n";
  size_t length = 10;

  DynamicJsonBuffer jsonBuffer;
  JsonObject& obj = jsonBuffer.parseObject(expected);

  ConfigStringParameter parameter = ConfigStringParameter("username", variable, length, set);

  parameter.fromJson(&obj);

  parameter.toJson(&obj);
  obj.printTo(body);

  TEST_ASSERT_EQUAL_STRING(expected, body);
}

void test_config_string_parameter_clear_data() {
  char *variable = "foo";
  const char *expected = "{\"username\":\"\"}";
  char body[50] = "\n";
  size_t length = 10;

  DynamicJsonBuffer jsonBuffer;
  JsonObject& obj = jsonBuffer.createObject();

  ConfigStringParameter parameter = ConfigStringParameter("username", variable, length, set);

  parameter.clearData();

  parameter.toJson(&obj);
  obj.printTo(body);

  TEST_ASSERT_EQUAL_STRING(expected, body);
}
