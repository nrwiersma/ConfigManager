#ifndef CONFIGMANAGER_PARAMETERS_H
#define CONFIGMANAGER_PARAMETERS_H

#include "Debug.h"

enum ParameterMode { get, set, both };

/**
 * Base Parameter
 */
class BaseParameter {
 public:
  virtual const char* getName() = 0;
  virtual ParameterMode getMode() = 0;
  virtual void fromJson(JsonObject* json) = 0;
  virtual void toJson(JsonObject* json) = 0;
  virtual void clear() = 0;
};

/**
 * Config Parameter
 */
template <typename T>
class ConfigParameter : public BaseParameter {
 public:
  ConfigParameter(const char* name, T* ptr, ParameterMode mode = both) {
    this->name = name;
    this->ptr = ptr;
    this->mode = mode;
  }

  const char* getName() { return this->name; }
  ParameterMode getMode() { return this->mode; }

  void update(T value) { *ptr = value; }

  void fromJson(JsonObject* json) {
    if (json->containsKey(name) && json->getMember(name).is<T>()) {
      this->update(json->getMember(name).as<T>());
    }
  }

  void toJson(JsonObject* json) { json->getOrAddMember(name).set(*ptr); }

  void clear() {
    DebugPrint("Clearing: ");
    DebugPrintln(name);
    *ptr = T();
  }

 private:
  const char* name;
  T* ptr;
  std::function<void(const char*)> cb;
  ParameterMode mode;
};

/**
 * Config String Parameter
 */
class ConfigStringParameter : public BaseParameter {
 public:
  ConfigStringParameter(const char* name,
                        char* ptr,
                        size_t length,
                        ParameterMode mode = both) {
    this->name = name;
    this->ptr = ptr;
    this->length = length;
    this->mode = mode;
  }

  const char* getName() { return this->name; }
  ParameterMode getMode() { return this->mode; }

  void update(const char* value) {
    memset(ptr, 0, length);
    strncpy(ptr, value, length - 1);
  }

  void fromJson(JsonObject* json) {
    if (json->containsKey(name) && json->getMember(name).is<char*>()) {
      const char* value = json->getMember(name).as<const char*>();
      this->update(value);
    }
  }

  void toJson(JsonObject* json) {
    json->getOrAddMember(name).set((const char*)ptr);
  }

  void clear() {
    DebugPrint("Clearing: ");
    DebugPrintln(name);
    memset(ptr, 0, length);
  }

 private:
  const char* name;
  char* ptr;
  size_t length;
  ParameterMode mode;
};

#endif  // CONFIGMANAGER_PARAMETERS_H
