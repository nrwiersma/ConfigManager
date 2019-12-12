#ifndef CONFIG_STRING_PARAMETER_H__
#define __CONFIG_STRING_PARAMETER_H__

#include <BaseParameter.h>

/**
 * Config String Parameter
 */
class ConfigStringParameter : public BaseParameter {
public:
    ConfigStringParameter(const char *name, char *ptr, size_t length, ParameterMode mode = both) {
        this->name = name;
        this->ptr = ptr;
        this->length = length;
        this->mode = mode;
    }

    ParameterMode getMode() {
        return this->mode;
    }

    void fromJson(JsonObject *json) {
        if (json->containsKey(name) && json->is<char *>(name)) {
            const char * value = json->get<const char *>(name);

            memset(ptr, NULL, length);
            strncpy(ptr, const_cast<char*>(value), length - 1);
        }
    }

    void toJson(JsonObject *json) {
        json->set(name, ptr);
    }

    void clearData() {
        DebugPrint("Clearing: ");
        DebugPrintln(name);
        memset(ptr, NULL, length);
    }
    

private:
    const char *name;
    char *ptr;
    size_t length;
    ParameterMode mode;
};


#endif /* __CONFIG_STRING_PARAMETER_H__ */
