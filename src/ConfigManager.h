#ifndef CONFIGMANAGER_CONFIGMANAGER_H
#define CONFIGMANAGER_CONFIGMANAGER_H

#include <list>
#include <typeinfo>
#include <stdlib.h>

using namespace std;

class ConfigManager {
 public:
  template <typename T>
  void add(const char* name, T* var);
  void add(const char* name, char* var, size_t len);
  bool save();

  template <typename T>
  bool is(const char* name);
  template <typename T>
  T as(const char* name);
  template <typename T>
  void set(const char* name, T val);

  void begin();

 private:
  enum ParamType{intT, longT, floatT, uintT, ulongT, ufloatT, stringT};

  class Param {
   public:
    const char* name;
    ParamType type;
    void* ptr;
    size_t size;
  };

  list<Param> params;
};

#endif  // CONFIGMANAGER_CONFIGMANAGER_H
