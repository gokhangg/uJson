/*
 * Project: uJson parser
 * Author: Gokhan Gunay, ghngunay@gmail.com
 * Copyright: (C) 2024 by Gokhan Gunay
 * License: GNU GPL v3 (see License.txt)
 */
#pragma once

#include <string_view>
#include <string>
#include <type_traits>

namespace uJson {

using StringView = std::basic_string_view<char>;

template<typename T>
struct TypeCheck {
    using NonConstType = typename std::remove_const<T>::type;
    static constexpr bool isBool = std::is_same<NonConstType, bool>::value;
    static constexpr bool isInteger = std::is_integral<NonConstType>::value && !isBool;
    static constexpr bool isFloatingPoint = std::is_same<NonConstType, float>::value;
    static constexpr bool isDouble = std::is_same<NonConstType, double>::value;
    static constexpr bool isString = std::is_same<NonConstType, std::string>::value;
    static constexpr bool isValid = isBool || isInteger || isFloatingPoint || isDouble || isString;
};

}  // end namespace uJson
