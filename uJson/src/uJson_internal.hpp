/*
 * Project: uJson parser
 * Author: Gokhan Gunay, ghngunay@gmail.com
 * Copyright: (C) 2024 by Gokhan Gunay
 * License: GNU GPL v3 (see License.txt)
 */
#pragma once

#include "uJson/uJson_array.hpp"


namespace uJson {

/**
 * @brief Used to find a key in the stream.
 * 
 * @param it stream start point.
 * @param end stream end point.
 * @return std::optional<StringView> 
 */
std::optional<StringView> FindKey(StringView::iterator& it, \
    const StringView::iterator& end);

/**
 * @brief Used to find an object after a key found.
 * 
 * @param it stream start point.
 * @param end stream end point.
 * @return std::unique_ptr<ItemI> 
 */
std::unique_ptr<ItemI> CheckPresenceObj(StringView::iterator& it, \
    const StringView::iterator& end);

}  // end namespace uJson
