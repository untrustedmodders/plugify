#pragma once

namespace wizard {
    struct Method;

    bool ReadMethod(Method& method, const utils::json::Value& object);
}