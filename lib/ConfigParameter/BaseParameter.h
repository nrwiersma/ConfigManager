#ifndef __BASE_PARAMETER_H__
#define __BASE_PARAMETER_H__

#include <DebugPrint.h>

enum Mode {ap, api};
enum ParameterMode { get, set, both};

/**
 * Base Parameter
 */
class BaseParameter {
public:
    virtual ParameterMode getMode() = 0;
    virtual void fromJson(JsonObject *json) = 0;
    virtual void toJson(JsonObject *json) = 0;
    virtual void clearData() = 0;
};

#endif /* __BASE_PARAMETER_H__ */
