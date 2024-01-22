/*
 * Project: uJson parser
 * Author: Gokhan Gunay, ghngunay@gmail.com
 * Copyright: (C) 2024 by Gokhan Gunay
 * License: GNU GPL v3 (see License.txt)
 */
#pragma once

#include <vector>
#include <memory>
#include <map>
#include <string_view>
#include <optional>
#include <sstream>
#include <algorithm>
#include <utility>

namespace uJson {

using BasicString = std::basic_string_view<char>;

/**
 * @brief Get the Value object inside the given string_view
 * If data type in the string mismatch the function returns empty.
 * @tparam Return type of the value in the string
 * @param string T String view containing the values as string.
 * @return std::optional<T> 
 */
template<typename T>
std::optional<T> GetValue(const BasicString& string) {
    auto str = std::string(std::begin(string), std::end(string));
    auto cstr = str.c_str();
    char* end_ss;
    if constexpr (std::is_integral<T>::value && !std::is_same<T, bool>::value) {
        auto val = std::strtoll(cstr, &end_ss, 10);
        if (end_ss == cstr) return {};
        return static_cast<T>(val);
    } else if constexpr (std::is_same<T, float>::value) {
        auto val = std::strtof(cstr, &end_ss);
        if (end_ss == cstr) return {};
        return val;
    } else if constexpr (std::is_same<T, double>::value) {
        auto val = std::strtod(cstr, &end_ss);
        if (end_ss == cstr) return {};
        return val;
    } else if constexpr (std::is_same<T, std::string>::value || std::is_same<T, const std::string>::value) {
        if (str[0] != '\"') return {};
        str.erase(0, 1);
        auto pos = str.find_last_of("\"");
        str.erase(pos);
        return std::move(str);
    } else if constexpr (std::is_same<T, bool>::value) {
        bool b;
        std::istringstream(str) >> std::boolalpha >> b;
        return b;
    } else {
        static_assert(false, "Unsupported data type expectation.");
    }
    return {};
}


class ItemI;


/**
 * @brief Used to access array item members.
 * 
 */
class ItemArray {
    const std::vector<std::unique_ptr<ItemI>>* vectP_;
 public:
    /**
     * @brief Construct a new Item Array object
     * 
     * @param arr Vector pointer containing the items.
     */
    explicit ItemArray(const void* arr)
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
     * @return std::optional<const ItemI*> 
     */
    virtual std::optional<const ItemI*> Find(const  char* name) const { return {}; }

    /**
     * @brief Get the Value As object given the data type.
     * If there occurs any error returns empty.
     * 
     * @tparam T Expected type.
     * @return std::optional<T> 
     */
    template<typename T>
    std::optional<T> GetValueAs() const {
        if (IsBranch()) return {};
        auto ptr = GetRootPtr();
        if constexpr (!std::is_same<T, ItemArray>::value)
            return GetValue<T>(*reinterpret_cast<const BasicString*>(ptr));
        else
            return ItemArray(ptr);
    }
    virtual ~ItemI() {}

 private:
    /**
     * @brief Get the Root Ptr object
     * 
     * @return const void* pointer to the underlying item.
     */
    virtual const void* GetRootPtr() const = 0;
};


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
    explicit Leaf(const BasicString& string) noexcept
        : string_{ string } {}

    ~Leaf() override = default;
 private:
    const void* GetRootPtr() const noexcept override {
        return &string_;
    }
    BasicString string_;
};


class Branch : public ItemI {
    class Array : public ItemI {
        friend class ItemArray;
     public:
        /**
         * @brief Construct a new Array object
         * 
         * @param string String containing the data.
         */
        explicit Array(const BasicString& string) {
            auto i = ++std::begin(string);
            auto end = std::end(string);
            for (; i != end; ++i) {
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

 public:
    /**
     * @brief Construct a new Branch object
     * 
     * @param string String containing the data.
     */
    explicit Branch(const BasicString& _string) {
        BasicString string;
        using namespace std::literals;  // NOLINT
        {
            auto frst = _string.find_first_not_of(" \n"sv);
            auto last = _string.find_last_not_of(" "sv);
            if (_string[last] != '}') return;
            string = _string.substr(frst, last + 1);
        }
        auto i = std::begin(string);
        const auto end = std::end(string);  // NOLINT
        auto findKey = [&]() {
            Branch::PassSpace(i, end);
            if (*i != '\"') return std::optional<BasicString>{};
            auto j = std::find(++i, end, '\"');
            if (j == end) return std::optional<BasicString>{};
            auto val = std::optional<BasicString>{ BasicString(&*i, static_cast<size_t>(std::distance(i, j))) };
            i = ++j;
            return val;
        };
        if (*i++ != '{') return;
        for (; i != end; ++i) {
            auto item = findKey();
            if (!item) {
                map_.clear();
                return;
            }
            Branch::PassSpace(i, end);
            if (*i++ != ':') {
                map_.clear();
                return;
            }
            auto obj = CheckPresenceObj(i, end);
            if (obj) {
                map_[item.value()] = std::move(obj);
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
        : Branch(BasicString{ str, size }) {
    }

    void Add(const char* name, std::unique_ptr<ItemI> item) override {
        map_[name] = std::move(item);
    }

    void Remove(const char* name) override {
        map_.erase(name);
    }

    bool IsBranch() const override {
        return true;
    }

    std::optional<const ItemI*> Find(const  char* name) const override {
        auto it = map_.find(name);
        if (it == std::end(map_)) return {};
        return std::optional<const ItemI*>{ it->second.get() };
    }

    operator bool() const {
        return !std::empty(map_);
    }

 private:
    const void* GetRootPtr() const override {
        return nullptr;
    }

    static void PassSpace(BasicString::iterator& it, \
        const BasicString::iterator& end) {
        it = std::find_if_not(it, end, [](auto& it) { return it == ' ' || it == '\n'; });
    }

    static auto FindItem(BasicString::iterator& it, \
        const BasicString::iterator& end) -> std::unique_ptr<ItemI> {
        auto j = std::find_if(it, end, [](auto& it) { return it == ',' || it == '}' || it == ']'; });
        if (j == end) return std::unique_ptr<ItemI>();
        auto obj = std::make_unique<Leaf>(BasicString{ &*it, \
            static_cast<size_t>(std::distance(it, j)) });
        it = j;
        return std::move(obj);
    }

    static auto FindArray(BasicString::iterator& it, \
        const BasicString::iterator& end) -> std::unique_ptr<ItemI> {
        if (*it == '[') {
            int i = 0;
            auto j = std::find_if(it, end, [&i](auto& it) { if (it == '[') ++i; if (it == ']') --i;  return i == 0 && it == ']'; });
            if (j == end) return std::unique_ptr<ItemI>();
            auto obj = std::make_unique<Array>(BasicString{ &*it, static_cast<size_t>(std::distance(it, ++j)) });
            if (obj->Empty()) return std::unique_ptr<ItemI>();
            it = j;
            it = find_if_not(it, end, [](auto& it) {return it == ' ' || it == ','; });
            return std::move(obj);
        }
        return std::unique_ptr<ItemI>();
    }

    static auto FindBranch(BasicString::iterator& it, \
        const BasicString::iterator& end) -> std::unique_ptr<ItemI> {
        if (*it == '{') {
            int i = 0;
            auto j = std::find_if(it, end, [&i](auto& it) { if (it == '{') ++i; if (it == '}') --i;  return i == 0 && it == '}'; });
            auto obj = std::make_unique<Branch>(&*it, std::distance(it, ++j));
            if (obj->map_.empty()) return std::unique_ptr<ItemI>();
            it = j;
            PassSpace(it, end);
            if (*it == ',') ++it;
            return std::move(obj);
        }
        return std::unique_ptr<ItemI>();
     }

    static auto CheckPresenceObj(BasicString::iterator& it, \
        const BasicString::iterator& end) -> std::unique_ptr<ItemI> {
        Branch::PassSpace(it, end);
        if (*it == '{') {
            auto value = FindBranch(it, end);
            if (value) return std::move(value);
            return std::unique_ptr<ItemI>();
        }
        if (*it == '[') {
            auto value = Branch::FindArray(it, end);
            if (value) return std::move(value);
            return std::unique_ptr<ItemI>();
        }
        if (*it != '\"' && !std::isalnum(*it)) return std::unique_ptr<ItemI>();
        return Branch::FindItem(it, end);
    };

    std::map<BasicString, std::unique_ptr<ItemI>> map_;
};

}  // end namespace uJson
