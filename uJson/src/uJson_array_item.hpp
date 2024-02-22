/*
 * Project: uJson parser
 * Author: Gokhan Gunay, ghngunay@gmail.com
 * Copyright: (C) 2024 by Gokhan Gunay
 * License: GNU GPL v3 (see License.txt)
 */
#pragma once

#include "uJson/uJson_array.hpp"
#include "uJson_internal.hpp"

namespace uJson {


class ArrayItem : public ItemI {
    friend class Array;
 public:
    /**
     * @brief Construct a new ArrayItem object
     *
     * @param string String containing the data.
     */
    explicit ArrayItem(const StringView& string) {
        auto i = std::next(std::begin(string));
        auto end = --std::end(string);
        for (; i != end;) {
            auto obj = CheckPresenceObj(i, end);
            if (obj) {
                vect_.emplace_back(std::move(obj));
                continue;
            }
            vect_.clear();
            return;
        }
    }

    /**
     * @brief Used to check if array is empty.
     *
     * @return true
     * @return false
     */
    bool Empty() const noexcept {
        return std::empty(vect_);
    }

 private:
    const void* GetRootPtr() const noexcept override {
        return &vect_;
    }
    std::vector<std::unique_ptr<ItemI>> vect_;
};

}  // end namespace uJson
