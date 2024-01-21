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
    } else if constexpr (std::is_same<T, std::string>::value) {
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

class ItemArray {
    const std::vector<std::unique_ptr<ItemI>>* vectP_;
 public:
    explicit ItemArray(const void* arr)
     : vectP_{ reinterpret_cast<decltype(vectP_)>(arr) } {}
    const ItemI& operator[](const size_t ind) const {
        return *(*vectP_)[ind].get();
    }
    size_t size() const {
        return (*vectP_).size();
    }
};

class ItemI {
 public:
    ItemI() = default;
    ItemI(const ItemI&) = delete;
    const ItemI& operator=(const ItemI&) = delete;
    virtual void Add(const char* name, std::unique_ptr<ItemI> component) {}
    virtual void Remove(const char* name) {}
    virtual bool IsBranch() const {
        return false;
    }
    virtual std::optional<const ItemI*> Find(const  char* name) const { return {}; }
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
 protected:
    virtual const void* GetRootPtr() const = 0;
};


class Leaf : public ItemI {
 public:
    Leaf() = delete;
    explicit Leaf(const BasicString& string)
        : string_{ string } {}
    const void* GetRootPtr() const override {
        return &string_;
    }
    ~Leaf() override = default;
 private:
    BasicString string_;
};


class Branch : public ItemI {
    class Array : public ItemI {
        friend class ItemArray;
     public:
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
        const void* GetRootPtr() const override {
            return &vect_;
        }
        bool empty() const {
            return std::empty(vect_);
        }
     private:
        std::vector<std::unique_ptr<ItemI>> vect_;
    };

 public:
    explicit Branch(const BasicString& basicStr) {
        BasicString string;
        using namespace std::literals;  // NOLINT
        {
            auto frst = basicStr.find_first_not_of(" \n"sv);
            auto last = basicStr.find_last_not_of(" "sv);
            if (basicStr[last] != '}') return;
            string = basicStr.substr(frst, last + 1);
        }
        auto i = std::begin(string);
        const auto const end = std::end(string);  // NOLINT
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


    Branch(const char* str, size_t size)
        : Branch(BasicString{ str, size }) {
    }

    void Add(const char* name, std::unique_ptr<ItemI> component) override {
        map_[name] = std::move(component);
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
            auto j = std::find_if(it, end, [](auto& it) { return it == ']'; });
            if (j == end) return std::unique_ptr<ItemI>();
            auto obj = std::make_unique<Array>(BasicString{ &*it, static_cast<size_t>(std::distance(it, ++j)) });
            if (obj->empty()) return std::unique_ptr<ItemI>();
            it = j;
            Branch::PassSpace(it, end);
            if (*it == ',') ++it;
            return std::move(obj);
        }
        return std::unique_ptr<ItemI>();
    }

    static auto FindBranch(BasicString::iterator& it, \
        const BasicString::iterator& end) -> std::unique_ptr<ItemI> {
        if (*it == '{') {
            auto j = it;
            int i = 0;
            while (*++j != '}' || i > 0) {
                if (*j == '{') ++i;
                if (*j == '}') --i;
                if (j == end) return std::unique_ptr<ItemI>();
            }
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
        if (*it != '\"' && !std::isdigit(*it)) return std::unique_ptr<ItemI>();
        return Branch::FindItem(it, end);
    };

    std::map<BasicString, std::unique_ptr<ItemI>> map_;
};

}  // end namespace uJson
