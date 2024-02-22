/*
 * Project: uJson parser
 * Author: Gokhan Gunay, ghngunay@gmail.com
 * Copyright: (C) 2024 by Gokhan Gunay
 * License: GNU GPL v3 (see License.txt)
 */

#include "uJson/uJson.hpp"

#include "uJson_leaf.hpp"
#include "uJson_array_item.hpp"
#include "uJson_branch.hpp"


#include <algorithm>
#include <sstream>

namespace uJson {

/**
 * @brief Get the Value object inside the given string_view
 * If data type in the string mismatch the function returns empty.
 * @tparam Return type of the value in the string
 * @param string T String view containing the values as string.
 * @return std::optional<T> 
 */
template<typename T>
std::optional<T> GetValue(const StringView& string) {
    static_assert(TypeCheck<T>::isValid, "Unsupported data type expectation.");
    auto str = std::string(std::begin(string), std::end(string));
    auto cstr = str.c_str();
    char* end_ss;
    if constexpr (TypeCheck<T>::isInteger) {
        auto val = std::strtoll(cstr, &end_ss, 10);
        if (end_ss == cstr) return {};
        return static_cast<T>(val);
    } else if constexpr (TypeCheck<T>::isFloatingPoint) {
        auto val = std::strtof(cstr, &end_ss);
        if (end_ss == cstr) return {};
        return val;
    } else if constexpr (TypeCheck<T>::isDouble) {
        auto val = std::strtod(cstr, &end_ss);
        if (end_ss == cstr) return {};
        return val;
    } else if constexpr (TypeCheck<T>::isString) {
        if (str[0] != '\"') return {};
        str.erase(0, 1);
        auto pos = str.find_last_of("\"");
        str.erase(pos);
        return str;
    } else if constexpr (TypeCheck<T>::isBool) {
        bool b;
        std::istringstream(str) >> std::boolalpha >> b;
        return b;
    }
    return {};
}

#define ActivateConversion(TYPE) template std::optional<const TYPE> GetValue<const TYPE>(const StringView&); \
    template std::optional<TYPE> GetValue<TYPE>(const StringView&)
ActivateConversion(int);
ActivateConversion(float);
ActivateConversion(std::string);
ActivateConversion(bool);
ActivateConversion(double);
ActivateConversion(size_t);

template<typename T>
std::optional<T> GetArray<T>(const void* ptr) {
    static_assert(std::is_same<T, Array>::value || std::is_same<T, const Array>::value, "Non array type is selected.");
    return T(ptr);
}

template std::optional<Array> GetArray<Array>(const void* ptr);
template std::optional<const Array> GetArray<const Array>(const void* ptr);

ItemI ItemI::empty_;

static void PassSpace(StringView::iterator& it, \
    const StringView::iterator& end) {
    it = std::find_if_not(it, end, [](auto& it) { return it == ' ' || it == '\n'; });
}

std::optional<StringView> FindKey(StringView::iterator& it, \
    const StringView::iterator& end) {
    PassSpace(it, end);
    if (*it != '\"') return std::optional<StringView>{};
    auto j = std::find(++it, end, '\"');
    if (j == end) return std::optional<StringView>{};
    auto val = std::optional<StringView>{ StringView(&*it, static_cast<size_t>(std::distance(it, j))) };
    it = ++j;
    PassSpace(it, end);
    return val;
}

auto FindItem(StringView::iterator& it, \
    const StringView::iterator& end) -> std::unique_ptr<ItemI> {
    auto j = std::find_if(it, end, [](auto& it) { return it == ',' || it == '}' || it == ']'; });
    if (j > end) return std::unique_ptr<ItemI>();
    auto obj = std::make_unique<Leaf>(StringView{ &*it, \
        static_cast<size_t>(std::distance(it, j)) });
    it = j;
    return obj;
}

auto FindArray(StringView::iterator& it, \
    const StringView::iterator& end) -> std::unique_ptr<ItemI> {
    if (*it == '[') {
        int i = 0;
        auto j = std::find_if(it, end, [&i](auto& it) {
            if (it == '[') ++i;
            if (it == ']') --i;
            return i == 0 && it == ']'; });
        if (j == end) return std::unique_ptr<ItemI>();
        auto obj = std::make_unique<ArrayItem>(StringView{ &*it, static_cast<size_t>(std::distance(it, ++j)) });
        if (obj->Empty()) return std::unique_ptr<ItemI>();
        it = j;
        return obj;
    }
    return std::unique_ptr<ItemI>();
}

auto FindBranch(StringView::iterator& it, \
    const StringView::iterator& end) -> std::unique_ptr<ItemI> {
    if (*it == '{') {
        int i = 0;
        auto j = std::find_if(it, end, [&i](auto& it) {
            if (it == '{') ++i;
            if (it == '}') --i;
            return i == 0 && it == '}'; });
        auto obj = std::make_unique<Branch>(&*it, std::distance(it, ++j));
        if (obj->Empty()) return std::unique_ptr<ItemI>();
        it = j;
        return obj;
    }
    return std::unique_ptr<ItemI>();
}

auto CheckPresenceObj(StringView::iterator& it, \
    const StringView::iterator& end) -> std::unique_ptr<ItemI> {
    PassSpace(it, end);
    auto value = std::unique_ptr<ItemI>();
    if (*it == '{') {
        value = FindBranch(it, end);
    }
    else if (*it == '[') {
        value = FindArray(it, end);
    }
    else {
        if (*it != '\"' && !std::isalnum(*it)) return std::unique_ptr<ItemI>();
        value = FindItem(it, end);
    }
    PassSpace(it, end);
    if (*it == ',') ++it;
    PassSpace(it, end);
    return value;
};



std::unique_ptr<ItemI> ParseJsonStream(const char* str) {
    auto branch = std::make_unique<Branch>(str, std::strlen(str));
    if (branch->Empty()) return nullptr;
    return branch;
}

/**
 * @brief Used to parse json stream from non-null terminated c string
 * 
 * @param str buffer
 * @param size stream size
 * @return std::unique_ptr<ItemI> 
 */
std::unique_ptr<ItemI> ParseJsonStream(const char* str, size_t size) {
    auto branch = std::make_unique<Branch>(str, size);
    if (branch->Empty()) return nullptr;
    return branch;
}
/**
 * @brief Used to parse json stream from string
 * 
 * @param string string
 * @return std::unique_ptr<ItemI> 
 */
std::unique_ptr<ItemI> ParseJsonStream(const StringView& str) {
    auto branch = std::make_unique<Branch>(str);
    if (branch->Empty()) return nullptr;
    return branch;
}

std::unique_ptr<ItemI> CreateJson() {
    return std::make_unique<Branch>();
}

}  // end namespace uJson
