/*
 * Project: uJson parser
 * Author: Gokhan Gunay, ghngunay@gmail.com
 * Copyright: (C) 2024 by Gokhan Gunay
 * License: GNU GPL v3 (see License.txt)
 */

#include "uJson/uJson.hpp"  // NOLINT

#include "gtest/gtest.h"



using namespace std;  // NOLINT
using str = std::string;
using cstr = const std::string;

str GetJsonString(cstr val1, cstr val21, cstr val22, cstr val31, cstr val32) {
    str kJsonStr = "{   \"Key1\" : " + val1 +", \
                        \"Key2\" : {\"Key21\" : " + val21 + ", \
                                    \"Key22\" : " + val22 + "\
                                    }, \
                        \"Key3\" : [1, {\"Key31\" : " + val31 + ", \
                                        \"Key32\" : " + val32 + "\
                                        }, [2, 3, 4]] \
                    }";
    return kJsonStr;
}


TEST(uJson, ValidJson) {
    constexpr auto kVal1 = 77;
    constexpr auto kVal21 = 88.8f;
    constexpr auto kVal22 = "Value22";
    constexpr auto kVal31 = true;
    constexpr auto kVal32 = "Value32";
    cstr dquote = "\"";
    auto json = GetJsonString(to_string(kVal1), to_string(kVal21), dquote + kVal22 + dquote, \
        kVal31 ? "true": "false", dquote + kVal32 + dquote);
    auto branch = uJson::ParseJsonStream(json);
    EXPECT_TRUE(branch);

    auto val1 = branch->Find("Key1");
    EXPECT_EQ(val1->GetValueAs<const int>().value(), kVal1);
    
    auto val2 = branch->Find("Key2");
    EXPECT_NEAR(val2->Find("Key21")->GetValueAs<float>().value(), kVal21, 0.001f);
    EXPECT_EQ(val2->Find("Key22")->GetValueAs<cstr>().value(), kVal22);
    
    auto val3 = branch->Find("Key3")->GetValueAs<const uJson::Array>();
    EXPECT_EQ(val3.value()[0].GetValueAs<int>().value(), 1);
    auto val31 = val3.value()[1].Find("Key31");
    EXPECT_EQ(val31->GetValueAs<bool>().value(), kVal31);
    auto val32 = val3.value()[1].Find("Key32");
    EXPECT_EQ(val32->GetValueAs<cstr>().value(), kVal32);
    auto val33 = val3.value()[2].GetValueAs<uJson::Array>();
    EXPECT_EQ(val33.value()[0].GetValueAs<int>().value(), 2);
}


TEST(uJson, InvalidJson) {
    constexpr auto kVal1 = 77;
    constexpr auto kVal21 = 88.8f;
    constexpr auto kVal22 = "Value22";
    constexpr auto kVal31 = true;
    constexpr auto kVal32 = "Value32";
    cstr dquote = ",";
    auto json = GetJsonString(to_string(kVal1), to_string(kVal21), dquote + kVal22 + dquote, \
        kVal31 ? "true" : "false", dquote + kVal32 + dquote);
    auto branch = uJson::ParseJsonStream(json);
    EXPECT_FALSE(branch);
}
