#include "method.h"
#include "value_type.h"
#include <wizard/method.h>

namespace wizard {
    bool ReadMethod(Method& method, const utils::json::Value& object) {
        if (!object.IsObject()) {
            WIZARD_LOG("Method should be a valid object!", ErrorLevel::ERROR);
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
                    method.varIndex = value.GetUint();
                }
            }
        }

        if (method.name.empty()) {
            WIZARD_LOG("Method: Should contain valid name!", ErrorLevel::ERROR);
            return false;
        }

        if (method.retType == ValueType::Invalid) {
            WIZARD_LOG("Method: '" + method.name + "' Invalid return type!", ErrorLevel::ERROR);
            return false;
        }

        for (size_t i = 0; i < method.paramTypes.size(); ++i) {
            if (method.paramTypes[i] == ValueType::Invalid) {
                WIZARD_LOG("Method: '" + method.name + "' Invalid parameter type - at index: " + std::to_string(i), ErrorLevel::ERROR);
                return false;
            }
        }

        return true;
    }
}