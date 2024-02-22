/*
 * Project: uJson parser
 * Author: Gokhan Gunay, ghngunay@gmail.com
 * Copyright: (C) 2024 by Gokhan Gunay
 * License: GNU GPL v3 (see License.txt)
 */
#pragma once

#include "uJson_array.hpp"

namespace uJson {

template<typename T>
std::optional<T> GetValue(const StringView&);

/**
 * @brief Interface for data items.
 * 
 */
class ItemI {
 public:
    /**
     * @brief Used to add an item.
     * 
     * @param name Key of the item to be added.
     * @param item Item to be added.
     */
    virtual void Add(const char* name, std::unique_ptr<ItemI> item) {}

    /**
     * @brief Removes and item in the list given its name.
     * If name is not found does nothing.
     * @param name 
     */
    virtual void Remove(const char* name) {}

    /**
     * @brief Used to check if the item is a branch.
     * 
     * @return true Is branch.
     * @return false Is not branch.
     */
    virtual bool IsBranch() const {
        return false;
    }

    /**
     * @brief Used to find an item given its key.
     * If not found returns empty.
     * 
     * @param name Item key.
     * @return const ItemI* Returns pointer to the empty static instance if the key is not found.
     */
    virtual const ItemI* Find(const  char* name) const { return { &ItemI::empty_ }; }

    /**
     * @brief Get the Value As object given the data type.
     * If there occurs any error returns empty.
     * 
     * @tparam T Expected type.
     * @return std::optional<T> 
     */
    template<typename T>
    std::optional<T> GetValueAs() const {
        static_assert(TypeCheck<T>::isValid || std::is_same<T, Array>::value || \
            std::is_same<T, const Array>::value, "Non suitable type is selected.");
        if (IsBranch()) return {};
        auto ptr = GetRootPtr();
        if (ptr == nullptr) return {};
        if constexpr (TypeCheck<T>::isValid)
            return GetValue<T>(*reinterpret_cast<const StringView*>(ptr));
        else
            return GetArray<T>(ptr);
    }

    virtual ~ItemI() {}

 private:
    /**
     * @brief Get the Root Ptr object
     * 
     * @return const void* pointer to the underlying item.
     */
    virtual const void* GetRootPtr() const { return nullptr; }

    static ItemI empty_;
};

}  // end namespace uJson
