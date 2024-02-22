/*
 * Project: uJson parser
 * Author: Gokhan Gunay, ghngunay@gmail.com
 * Copyright: (C) 2024 by Gokhan Gunay
 * License: GNU GPL v3 (see License.txt)
 */
#pragma once

#include "uJson/uJson_item_i.hpp"
#include "uJson_internal.hpp"

#include <map>
#include <algorithm>


namespace uJson {

class Branch : public ItemI {
 public:
    Branch() = default;
    /**
     * @brief Construct a new Branch object
     * 
     * @param string String containing the data.
     */
    explicit Branch(const StringView& _string) {
        StringView string;
        using namespace std::literals;  // NOLINT
        {
            auto frst = _string.find_first_not_of(" \n"sv);
            auto last = _string.find_last_not_of(" \n"sv);
            if (_string[last] != '}' || _string[frst] != '{') return;
            string = _string.substr(++frst, last + 1);
        }
        auto i = std::begin(string);
        const auto end = --std::end(string);  // NOLINT

        for (; i != end;) {
            auto item = FindKey(i, end);
            if (!item) {
                map_.clear();
                return;
            }
            if (*i++ != ':') {
                map_.clear();
                return;
            }
            auto obj = CheckPresenceObj(i, end);
            if (obj) {
                map_.emplace(item.value(), std::move(obj));
                continue;
            }
            map_.clear();
            return;
        }
    }

    /**
     * @brief Construct a new Branch object
     * 
     * @param str string.
     * @param size string size.
     */
    Branch(const char* str, size_t size)
        : Branch(StringView{ str, size }) {
    }

    void Add(const char* name, std::unique_ptr<ItemI> item) override {
        if (!item) map_[name] = std::move(item);  // NOLINT
    }

    void Remove(const char* name) override {
        map_.erase(name);
    }

    bool IsBranch() const override {
        return true;
    }

    const ItemI* Find(const  char* name) const override {
        auto it = map_.find(name);
        if (it == std::end(map_)) return ItemI::Find("");
        return it->second.get();
    }

    operator bool() const {
        return !std::empty(map_);
    }

    bool Empty() const {
        return std::empty(map_);
    }

 private:
    const void* GetRootPtr() const override {
        return nullptr;
    }

    std::map<StringView, std::unique_ptr<ItemI>> map_;
};


}  // end namespace uJson
