#ifndef __CONFIG_PARAMETER_H__
#define __CONFIG_PARAMETER_H_

#include "BaseParameter.h"

/**
 * Config Parameter
 */
template<typename T>
class ConfigParameter : public BaseParameter {
public:
    ConfigParameter(const char *name, T *ptr, ParameterMode mode = both) {
        this->name = name;
        this->ptr = ptr;
        this->mode = mode;
    }

    ParameterMode getMode() {
        return this->mode;
    }

    void fromJson(JsonObject *json) {
        if (json->containsKey(name) && json->is<T>(name)) {
            *ptr = json->get<T>(name);
        }
    }

    void toJson(JsonObject *json) {
        json->set(name, *ptr);
    }

    void clearData() {
        DebugPrint("Clearing: ");
        DebugPrintln(name);
        *ptr = T();
    }

private:
    const char *name;
    T *ptr;
    std::function<void(const char*)> cb;
    ParameterMode mode;
};

#endif /* __CONFIG_PARAMETER_H__ */
