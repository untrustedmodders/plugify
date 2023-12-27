#pragma once

#include <wizard/method.h>

namespace wizard {
    std::string_view ValueTypeToString(ValueType valueType);
    ValueType ValueTypeFromString(std::string_view valueType);
}