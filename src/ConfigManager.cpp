#include "ConfigManager.h"

template <typename T>
void ConfigManager::add(const char* name, T* var) {
  // TODO: throw exception as unsupported
}

template <>
void ConfigManager::add<int>(const char* name, int* var) {
  ConfigManager::Param param;
  param.name = name;
  param.type = ConfigManager::intT;
  param.ptr = (void*)var;
  param.size = sizeof(int);
  this->params.push_back(param);

}

void ConfigManager::add(const char* name, char* var, size_t len) {
  ConfigManager::Param param;
  param.name = name;
  param.type = ConfigManager::stringT;
  param.ptr = (void*)var;
  param.size = len;
  this->params.push_back(param);
}

bool ConfigManager::save() {
  // TODO: implement
  return false;
}

template <typename T>
bool ConfigManager::is(const char* name) {
  // TODO: implement
  return false;
}

template <typename T>
T ConfigManager::as(const char* name) {
  // TODO: implement
  return T();
}

template <typename T>
void ConfigManager::set(const char* name, T val) {
  // TODO: implement
}

void ConfigManager::begin() {
  // TODO: implement
}

#ifdef localbuild
// used to compile the project from the project
// and not as a library from another one
void setup() {}
void loop() {}
#endif
