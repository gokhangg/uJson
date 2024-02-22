/*
 * Project: uJson parser
 * Author: Gokhan Gunay, ghngunay@gmail.com
 * Copyright: (C) 2024 by Gokhan Gunay
 * License: GNU GPL v3 (see License.txt)
 */
#pragma once

#include "uJson_item_i.hpp"


namespace uJson {

/**
 * @brief Used to parse json stream from null terminated c string
 * 
 * @param str buffer
 * @param size stream size
 * @return std::unique_ptr<ItemI> 
 */
std::unique_ptr<ItemI> ParseJsonStream(const char* str);

/**
 * @brief Used to parse json stream from non-null terminated c string
 * 
 * @param str buffer
 * @param size stream size
 * @return std::unique_ptr<ItemI> 
 */
std::unique_ptr<ItemI> ParseJsonStream(const char* str, size_t size);

/**
 * @brief Used to parse json stream from string
 * 
 * @param StringView string_view
 * @return std::unique_ptr<ItemI> 
 */
std::unique_ptr<ItemI> ParseJsonStream(const StringView& str);  // NOLINT

/**
 * @brief Create a Json object
 * 
 * @return std::unique_ptr<ItemI> 
 */
std::unique_ptr<ItemI> CreateJson();  // NOLINT
}  // end namespace uJson
