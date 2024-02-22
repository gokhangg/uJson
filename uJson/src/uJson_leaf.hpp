/*
 * Project: uJson parser
 * Author: Gokhan Gunay, ghngunay@gmail.com
 * Copyright: (C) 2024 by Gokhan Gunay
 * License: GNU GPL v3 (see License.txt)
 */
#pragma once

#include "uJson/uJson_item_i.hpp"

namespace uJson {

/**
 * @brief Leaf class representing a direct value.
 * 
 */
class Leaf : public ItemI {
 public:
    Leaf() = delete;
    /**
     * @brief Construct a new Leaf object
     * 
     * @param string String containing the data.
     */
    explicit Leaf(const StringView& string) noexcept
        : string_{ string } {}  // NOLINT

    ~Leaf() override = default;
 private:
    const void* GetRootPtr() const noexcept override {
        return &string_;
    }
    StringView string_;
};

}  // end namespace uJson
