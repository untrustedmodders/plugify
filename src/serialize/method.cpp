#include "method.h"
#include "value_type.h"
#include <wizard/method.h>

namespace wizard {
    bool ReadMethod(Method& method, const utils::json::Value& object) {
        if (!object.IsObject()) {
            WZ_LOG_ERROR("Method should be a valid object!");
            return false;
        }

        method.paramTypes.clear();

        for (auto itr = object.MemberBegin(); itr != object.MemberEnd(); ++itr) {
            const char* blockName = itr->name.GetString();
            const auto& value = itr->value;

            if (!strcmp(blockName, "name")) {
                if (value.IsString()) {
                    method.name = value.GetString();
                }
            }
            else if(!strcmp(blockName, "paramTypes")) {
                if (value.IsArray()) {
                    for (const auto& val : value.GetArray()) {
                        if (val.IsString()) {
                            method.paramTypes.push_back(ValueTypeFromString(val.GetString()));
                        }
                    }
                }
            }
            else if(!strcmp(blockName, "callConv")) {
                if (value.IsString()) {
                    method.callConv = value.GetString();
                }
            }
            else if(!strcmp(blockName, "retType")) {
                if (value.IsString()) {
                    method.retType = ValueTypeFromString(value.GetString());
                }
            }
            else if(!strcmp(blockName, "varIndex")) {
                if (value.IsUint()) {
                    method.varIndex = static_cast<uint8_t>(value.GetUint());
                }
            }
        }

        if (method.name.empty()) {
            WZ_LOG_ERROR("Method: Should contain valid name!");
            return false;
        }

        if (method.retType == ValueType::Invalid) {
            WZ_LOG_ERROR("Method: '{}' Invalid return type!", method.name);
            return false;
        }

        for (size_t i = 0; i < method.paramTypes.size(); ++i) {
            if (method.paramTypes[i] == ValueType::Invalid) {
                WZ_LOG_ERROR("Method: '{}' Invalid parameter type - at index: {}", method.name, i);
                return false;
            }
        }

        return true;
    }
}