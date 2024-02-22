/*
 * Project: uJson parser
 * Author: Gokhan Gunay, ghngunay@gmail.com
 * Copyright: (C) 2024 by Gokhan Gunay
 * License: GNU GPL v3 (see License.txt)
 */
#pragma once

#include "uJson_type_checks.hpp"

#include <vector>
#include <memory>
#include <optional>

namespace uJson {

class ItemI;
/**
 * @brief Used to access array item members.
 * 
 */
class Array {
    const std::vector<std::unique_ptr<ItemI>>* vectP_;

 public:
    /**
     * @brief Construct a new Item ArrayItem object
     * 
     * @param arr Vector pointer containing the items.
     */
    explicit Array(const void* arr)
     : vectP_{ reinterpret_cast<decltype(vectP_)>(arr) } {}

    /**
     * @brief Used to access items in the array by index.
     * 
     * @param ind 
     * @return const ItemI& 
     */
    const ItemI& operator[](const size_t ind) const {
        return *(*vectP_)[ind].get();
    }

    /**
     * @brief Used to get size of the array.
     * 
     * @return size_t 
     */
    size_t size() const {
        return (*vectP_).size();
    }
};

template<typename T>
std::optional<T> GetArray(const void*);

}  // end namespace uJson
