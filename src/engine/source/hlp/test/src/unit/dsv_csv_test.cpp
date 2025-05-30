#include <gtest/gtest.h>

#include "hlp_test.hpp"

auto constexpr NAME = "dsvcsvParser";
static const std::string TARGET = "/TargetField";

/************************************
 *  CSV Parser
 ************************************/
INSTANTIATE_TEST_SUITE_P(
    CSVBuild,
    HlpBuildTest,
    ::testing::Values(BuildT(FAILURE, getCSVParser, {NAME, TARGET, {}, {}}),
                      BuildT(FAILURE, getCSVParser, {NAME, TARGET, {""}, {}}),
                      BuildT(FAILURE, getCSVParser, {NAME, TARGET, {""}, {"out1"}}),
                      BuildT(SUCCESS, getCSVParser, {NAME, TARGET, {""}, {"out1", "out2"}}),
                      BuildT(SUCCESS, getCSVParser, {NAME, TARGET, {""}, {"out1", "out2", "out3"}}),
                      BuildT(FAILURE, getCSVParser, {NAME, TARGET, {}, {"out1", "out2"}})));

INSTANTIATE_TEST_SUITE_P(
    CSVParse,
    HlpParseTest,
    ::testing::Values(

        ParseT(SUCCESS,
               "hi,hi2",
               j(fmt::format(R"({{"{}":{{"field_1":"hi","field_2":"hi2"}}}})", TARGET.substr(1))),
               6,
               getCSVParser,
               {NAME, TARGET, {""}, {"field_1", "field_2"}}),

        ParseT(SUCCESS,
               "hi,hi2 bye",
               j(fmt::format(R"({{"{}":{{"field_1":"hi","field_2":"hi2"}}}})", TARGET.substr(1))),
               6,
               getCSVParser,
               {NAME, TARGET, {" "}, {"field_1", "field_2"}}),
        // TODO Should this case be valid? Stop token issue
        // TestCase {R"(hi,hi2,bye)",
        //   true,
        //   {","},
        //   Options {"field_1", "field_2"},
        //   fn(R"({"field_1":"hi","field_2":"hi2"})"),
        //   6},

        ParseT(SUCCESS,
               R"("v1","v2","v3" - ABC)",
               j(fmt::format(R"({{"{}":{{"f1":"v1","f2":"v2","f3":"v3"}}}})", TARGET.substr(1))),
               14,
               getCSVParser,
               {NAME, TARGET, {" - ABC"}, {"f1", "f2", "f3"}}),
        // TODO Should this case be valid? Stop token issue
        // TestCase {R"("v1","v2 - ABC","v3" - ABC)",
        //   true,
        //   {" -"},
        //   Options {"f1", "f2", "f3"},
        //   fn(R"({"f1":"v1","f2":"v2 - ABC","f3":"v3"})"),
        //   20},

        ParseT(SUCCESS,
               ",",
               j(fmt::format(R"({{"{}":{{"field_1":null,"field_2":null}}}})", TARGET.substr(1))),
               1,
               getCSVParser,
               {NAME, TARGET, {""}, {"field_1", "field_2"}}),

        ParseT(SUCCESS,
               ",,,hi",
               j(fmt::format(R"({{"{}":{{"field_1":null,"field_2":null, "field_3":null, "field_4":"hi"}}}})",
                             TARGET.substr(1))),
               5,
               getCSVParser,
               {NAME, TARGET, {""}, {"field_1", "field_2", "field_3", "field_4"}}),

        ParseT(SUCCESS,
               "hi,,,bye",
               j(fmt::format(R"({{"{}":{{"field_1":"hi","field_2":null, "field_3":null, "field_4":"bye"}}}})",
                             TARGET.substr(1))),
               8,
               getCSVParser,
               {NAME, TARGET, {""}, {"field_1", "field_2", "field_3", "field_4"}}),

        ParseT(FAILURE,
               "hi,  \"wazuh\",,bye",
               {},
               2,
               getCSVParser,
               {NAME, TARGET, {""}, {"field_1", "field_2", "field_3", "field_4"}}),

        ParseT(SUCCESS,
               ",,,",
               j(fmt::format(R"({{"{}":{{"field_1":null,"field_2":null, "field_3":null,"field_4":null}}}})",
                             TARGET.substr(1))),
               3,
               getCSVParser,
               {NAME, TARGET, {""}, {"field_1", "field_2", "field_3", "field_4"}}),

        ParseT(SUCCESS,
               R"("","","","")",
               j(fmt::format(R"({{"{}":{{"field_1":null,"field_2":null, "field_3":null,"field_4":null}}}})",
                             TARGET.substr(1))),
               11,
               getCSVParser,
               {NAME, TARGET, {""}, {"field_1", "field_2", "field_3", "field_4"}}),

        ParseT(SUCCESS,
               ", bye",
               j(fmt::format(R"({{"{}":{{"field_1":null,"field_2":null}}}})", TARGET.substr(1))),
               1,
               getCSVParser,
               {NAME, TARGET, {" "}, {"field_1", "field_2"}}),
        // An empty field must have its delimiter
        // pos != end

        ParseT(FAILURE, "hi1,hi2,hi3,hi4 bye", {}, 7, getCSVParser, {NAME, TARGET, {""}, {"field_1", "field_2"}}),

        ParseT(
            SUCCESS,
            R"(,,hi,"semicolon scaped'"",""' <-- other here <,>",other value,,,value new,,)",
            j(fmt::format(
                R"({{"{}":{{"null_1":null,"null_2":null,"word":"hi","escaped_1":"semicolon scaped'\",\"' <-- other here <,>","no_escape,null_3":"other value","null_4":null,"new":null,"null_5":"value new","null_6":null,"null_7":null}}}})",
                TARGET.substr(1))),
            75,
            getCSVParser,
            {NAME,
             TARGET,
             {""},
             {"null_1",
              "null_2",
              "word",
              "escaped_1",
              "no_escape,null_3",
              "null_4",
              "new",
              "null_5",
              "null_6",
              "null_7"}}),

        ParseT(FAILURE,
               "f1,f2,f3",
               {},
               8,
               getCSVParser,
               {NAME, TARGET, {""}, {"field_1", "field_2", "field_3", "field_4"}}),

        ParseT(FAILURE,
               R"(f1,f2,f3,f4,f5)",
               {},
               11,
               getCSVParser,
               {NAME, TARGET, {""}, {"field_1", "field_2", "field_3", "field_4"}}),

        ParseT(SUCCESS,
               R"(f1,f2,f3,"f4,f5")",
               j(fmt::format(R"({{"{}":{{"field_1":"f1","field_2":"f2","field_3":"f3","field_4":"f4,f5"}}}})",
                             TARGET.substr(1))),
               16,
               getCSVParser,
               {NAME, TARGET, {""}, {"field_1", "field_2", "field_3", "field_4"}}),

        ParseT(FAILURE,
               R"(f1,f2,f3,"f4,f5)",
               {},
               8,
               getCSVParser,
               {NAME, TARGET, {""}, {"field_1", "field_2", "field_3", "field_4"}}),
        // A quote can be escaped using another quote, so " must be encoded as "" in a
        // written CSV
        // if there string contains """, it would be invalid, as there is a single quote

        ParseT(FAILURE,
               R"(f1,f2,f3,f4""")",
               {},
               8,
               getCSVParser,
               {NAME, TARGET, {""}, {"field_1", "field_2", "field_3", "field_4"}}),
        // https://www.rfc-editor.org/rfc/rfc4180 sect 2.5

        ParseT(FAILURE,
               R"(f1,f2,f3,f4"""")",
               {},
               8,
               getCSVParser,
               {NAME, TARGET, {""}, {"field_1", "field_2", "field_3", "field_4"}}),

        ParseT(SUCCESS,
               R"(f1,f2,f3,"--""--")",
               j(fmt::format(R"({{"{}":{{"field_1":"f1","field_2":"f2","field_3":"f3","field_4":"--\"--"}}}})",
                             TARGET.substr(1))),
               17,
               getCSVParser,
               {NAME, TARGET, {""}, {"field_1", "field_2", "field_3", "field_4"}}),

        ParseT(SUCCESS,
               R"(f1,f2,f3,"--""--",)",
               j(fmt::format(
                   R"({{"{}":{{"field_1":"f1","field_2":"f2","field_3":"f3","field_4":"--\"--","field_5":null}}}})",
                   TARGET.substr(1))),
               18,
               getCSVParser,
               {NAME, TARGET, {""}, {"field_1", "field_2", "field_3", "field_4", "field_5"}}),

        ParseT(SUCCESS,
               R"(f1,f2;asd)",
               j(fmt::format(R"({{"{}":{{"field_1":"f1","field_2":"f2"}}}})", TARGET.substr(1))),
               5,
               getCSVParser,
               {NAME, TARGET, {";"}, {"field_1", "field_2"}}),

        ParseT(SUCCESS,
               R"(f1,"f2;wazuh";asd)",
               j(fmt::format(R"({{"{}":{{"field_1":"f1","field_2":"f2;wazuh"}}}})", TARGET.substr(1))),
               13,
               getCSVParser,
               {NAME, TARGET, {";asd"}, {"field_1", "field_2"}}),

        ParseT(FAILURE, R"(f1,f2;a,sd)", {}, 7, getCSVParser, {NAME, TARGET, {""}, {"field_1", "field_2"}}),

        ParseT(SUCCESS,
               R"(0,1.0,,"")",
               j(fmt::format(R"({{"{}":{{"field_1":"0","field_2":"1.0","field_3":null,"field_4":null}}}})",
                             TARGET.substr(1))),
               9,
               getCSVParser,
               {NAME, TARGET, {""}, {"field_1", "field_2", "field_3", "field_4"}}),

        ParseT(SUCCESS,
               R"("v1","v2","v3")",
               j(fmt::format(R"({{"{}":{{"f1":"v1","f2":"v2","f3":"v3"}}}})", TARGET.substr(1))),
               14,
               getCSVParser,
               {NAME, TARGET, {""}, {"f1", "f2", "f3"}}),
        ParseT(SUCCESS,
               R"("v1","v2","v3")",
               j(fmt::format(R"({{"{}":{{"f1":{{"key":"v1"}},"f2":{{"key":"v2"}},"f3":{{"key":{{"subkey":"v3"}}}}}}}})",
                             TARGET.substr(1))),
               14,
               getCSVParser,
               {NAME, TARGET, {""}, {"f1.key", "f2/key", "f3.key/subkey"}})));

/************************************
 *  DSV Parser
 ************************************/
INSTANTIATE_TEST_SUITE_P(
    DSVBuild,
    HlpBuildTest,
    ::testing::Values(BuildT(SUCCESS, getDSVParser, {NAME, TARGET, {""}, {"d", "q", "e", "out1", "out2"}}),
                      BuildT(FAILURE, getDSVParser, {NAME, TARGET, {}, {"d", "q", "e", "out1", "out2"}}),
                      BuildT(SUCCESS, getDSVParser, {NAME, TARGET, {""}, {"d", "q", "e", "out1", "out2", "out3"}}),
                      BuildT(FAILURE, getDSVParser, {NAME, TARGET, {""}, {"d", "q", "e", "out1"}})));

INSTANTIATE_TEST_SUITE_P(
    DSVParse,
    HlpParseTest,
    ::testing::Values(

        ParseT(FAILURE, "val", {}, 3, getDSVParser, {NAME, TARGET, {""}, {"*", "-", " ", "out1", "out2"}}),

        ParseT(SUCCESS,
               "val1|val2",
               j(fmt::format(R"({{"{}":{{"out1":"val1","out2":"val2"}}}})", TARGET.substr(1))),
               strlen("val1|val2"),
               getDSVParser,
               {NAME, TARGET, {""}, {"|", "\"", "\"", "out1", "out2"}}),

        ParseT(FAILURE, "val1,val2", {}, 9, getDSVParser, {NAME, TARGET, {""}, {"|", "\"", "\"", "out1", "out2"}}),

        ParseT(SUCCESS,
               "val1|val2 val3",
               j(fmt::format(R"({{"{}":{{"out1":"val1","out2":"val2"}}}})", TARGET.substr(1))),
               strlen("val1|val2"),
               getDSVParser,
               {NAME, TARGET, {" "}, {"|", "\"", "\"", "out1", "out2"}}),
        // TODO Should this case be valid? Stop token issue
        // TestCase {"val1|val2|val3",
        //   true,
        //   {"|"},
        //   Options {"|", "\"", "\"", "out1", "out2"},
        //   fn(R"({"out1":"val1","out2":"val2"})"),
        //   9},

        ParseT(SUCCESS,
               "'val1'|'val2'|'val3' - something",
               j(fmt::format(R"({{"{}":{{"out1":"val1","out2":"val2","out3":"val3"}}}})", TARGET.substr(1))),
               strlen("'val1'|'val2'|'val3'"),
               getDSVParser,
               {NAME, TARGET, {" - something"}, {"|", "'", "'", "out1", "out2", "out3"}}),
        // TODO Should this case be valid? Stop token issue
        // TestCase {"'val1'|'val2'|'val3 - something'",
        //   true,
        //   {" - something"},
        //   Options {"|", "'", "'", "out1", "out2", "out3"},
        //   fn(R"({"out1":"val1","out2":"val2","out3":"'val3"})"),
        //   31},
        // TestCase {"#val1#$#val2 - something#$#val3# - val4",
        //   true,
        //   {" -"},
        //   Options {"$", "#", "^", "out1", "out2", "out3"},
        //   fn(R"({"out1":"val1","out2":"val2 - something","out3":"val3"})"),
        //   32},

        ParseT(SUCCESS,
               "|",
               j(fmt::format(R"({{"{}":{{"out1":null,"out2":null}}}})", TARGET.substr(1))),
               1,
               getDSVParser,
               {NAME, TARGET, {""}, {"|", "'", "'", "out1", "out2"}}),

        ParseT(SUCCESS,
               "|||",
               j(fmt::format(R"({{"{}":{{"out1":null,"out2":null,"out3":null,"out4":null}}}})", TARGET.substr(1))),
               3,
               getDSVParser,
               {NAME, TARGET, {""}, {"|", "'", "'", "out1", "out2", "out3", "out4"}}),

        ParseT(FAILURE,
               "val1|val2|val3",
               {},
               14,
               getDSVParser,
               {NAME, TARGET, {""}, {"|", "'", "'", "out1", "out2", "out3", "out4"}}),

        ParseT(FAILURE,
               "val1|val2|val3|val4|val5",
               {},
               strlen("val1|val2|val3|val4"),
               getDSVParser,
               {NAME, TARGET, {""}, {"|", "'", "'", "out1", "out2", "out3", "out4"}}),

        ParseT(SUCCESS,
               "val1|val2|val3|'val4|val5'",
               j(fmt::format(R"({{"{}":{{"out1":"val1","out2":"val2","out3":"val3","out4":"val4|val5"}}}})",
                             TARGET.substr(1))),
               strlen("val1|val2|val3|'val4|val5'"),
               getDSVParser,
               {NAME, TARGET, {""}, {"|", "'", "'", "out1", "out2", "out3", "out4"}}),

        ParseT(FAILURE,
               "val1|val2|val3|'val4|val5",
               {},
               14,
               getDSVParser,
               {NAME, TARGET, {""}, {"|", "'", "'", "out1", "out2", "out3", "out4"}}),

        ParseT(FAILURE,
               "val1|val2|val3|val4'''",
               {},
               14,
               getDSVParser,
               {NAME, TARGET, {""}, {"|", "'", "'", "out1", "out2", "out3", "out4"}}),

        ParseT(FAILURE,
               "val1|val2|val3|val4''''",
               {},
               14,
               getDSVParser,
               {NAME, TARGET, {""}, {"|", "'", "'", "out1", "out2", "out3", "out4"}}),

        ParseT(SUCCESS,
               "val1|val2|val3|'--''--'",
               j(fmt::format(R"({{"{}":{{"out1":"val1","out2":"val2","out3":"val3","out4":"--'--"}}}})",
                             TARGET.substr(1))),
               strlen("val1,val2,val3,'--''--'"),
               getDSVParser,
               {NAME, TARGET, {""}, {"|", "'", "'", "out1", "out2", "out3", "out4"}}),
        // should be terminated by ' and not by ,

        ParseT(FAILURE,
               "val1|val2|val3|'--''--',",
               {},
               14,
               getDSVParser,
               {NAME, TARGET, {""}, {"|", "'", "'", "out1", "out2", "out3", "out4", "out5"}}),

        ParseT(SUCCESS,
               "val1|val2;asd",
               j(fmt::format(R"({{"{}":{{"out1":"val1","out2":"val2"}}}})", TARGET.substr(1))),
               strlen("val1|val2"),
               getDSVParser,
               {NAME, TARGET, {";"}, {"|", "'", "'", "out1", "out2"}}),

        ParseT(SUCCESS,
               "val1|'val2;val3';val4",
               j(fmt::format(R"({{"{}":{{"out1":"val1","out2":"val2;val3"}}}})", TARGET.substr(1))),
               strlen("val1|'val2;val3'"),
               getDSVParser,
               {NAME, TARGET, {";val4"}, {"|", "'", "'", "out1", "out2"}}),

        ParseT(FAILURE,
               "val1|val2;x|yz",
               {},
               strlen("val1|val2;x"),
               getDSVParser,
               {NAME, TARGET, {""}, {"|", "'", "'", "out1", "out2"}}),

        ParseT(SUCCESS,
               "|||hi",
               j(fmt::format(R"({{"{}":{{"out1":null,"out2":null, "out3":null, "out4":"hi"}}}})", TARGET.substr(1))),
               5,
               getDSVParser,
               {NAME, TARGET, {""}, {"|", "\"", "\"", "out1", "out2", "out3", "out4"}}),

        ParseT(SUCCESS,
               "|||",
               j(fmt::format(R"({{"{}":{{"out1":null,"out2":null, "out3":null, "out4":null}}}})", TARGET.substr(1))),
               3,
               getDSVParser,
               {NAME, TARGET, {""}, {"|", "\"", "\"", "out1", "out2", "out3", "out4"}}),

        ParseT(SUCCESS,
               "hi|||bye",
               j(fmt::format(R"({{"{}":{{"out1":"hi","out2":null, "out3":null, "out4":"bye"}}}})", TARGET.substr(1))),
               8,
               getDSVParser,
               {NAME, TARGET, {""}, {"|", "\"", "\"", "out1", "out2", "out3", "out4"}}),

        ParseT(FAILURE,
               R"(hi|  "wazuh"||bye)",
               {},
               2,
               getDSVParser,
               {NAME, TARGET, {""}, {"|", "\"", "\"", "out1", "out2", "out3", "out4"}}),

        ParseT(SUCCESS,
               R"(""|""|""|"")",
               j(fmt::format(R"({{"{}":{{"out1":null,"out2":null,"out3":null,"out4":null}}}})", TARGET.substr(1))),
               11,
               getDSVParser,
               {NAME, TARGET, {""}, {"|", "\"", "\"", "out1", "out2", "out3", "out4"}}),

        ParseT(SUCCESS,
               "| bye",
               j(fmt::format(R"({{"{}":{{"out1":null,"out2":null}}}})", TARGET.substr(1))),
               1,
               getDSVParser,
               {NAME, TARGET, {" "}, {"|", "\"", "\"", "out1", "out2"}}),

        ParseT(SUCCESS,
               "0|1.0||''",
               j(fmt::format(R"({{"{}":{{"out1":"0","out2":"1.0","out3":null,"out4":null}}}})", TARGET.substr(1))),
               strlen("0|1.0||''"),
               getDSVParser,
               {NAME, TARGET, {""}, {"|", "'", "'", "out1", "out2", "out3", "out4"}}),

        ParseT(SUCCESS,
               "'val1'|'val2'|'val3'",
               j(fmt::format(R"({{"{}":{{"out1":"val1","out2":"val2","out3":"val3"}}}})", TARGET.substr(1))),
               strlen("'val1','val2','val3'"),
               getDSVParser,
               {NAME, TARGET, {""}, {"|", "'", "'", "out1", "out2", "out3"}}),

        ParseT(
            SUCCESS,
            R"(,,hi,"semicolon scaped'\",\"' <-- other here <,>",other value,,,value new,,)",
            j(fmt::format(
                R"({{"{}":{{"null_1":null,"null_2":null,"word":"hi","escaped_1":"semicolon scaped'\",\"' <-- other here <,>","no_escape,null_3":"other value","null_4":null,"new":null,"null_5":"value new","null_6":null,"null_7":null}}}})",
                TARGET.substr(1))),
            75,
            getDSVParser,
            {NAME,
             TARGET,
             {""},
             {",",
              "\"",
              "\\",
              "null_1",
              "null_2",
              "word",
              "escaped_1",
              "no_escape,null_3",
              "null_4",
              "new",
              "null_5",
              "null_6",
              "null_7"}}),

        ParseT(SUCCESS,
               R"("\"value1\""|value2|value3|valueN)",
               j(fmt::format(R"({{"{}":{{"out1":"\"value1\"","out2":"value2","out3":"value3","outN":"valueN"}}}})",
                             TARGET.substr(1))),
               strlen(R"("\"value1\""|value2|value3|valueN)"),
               getDSVParser,
               {NAME, TARGET, {""}, {"|", "\"", "\\", "out1", "out2", "out3", "outN"}}),
        ParseT(SUCCESS,
               R"("\"v1\""|v2|v3)",
               j(fmt::format(
                   R"({{"{}":{{"f1":{{"key":"\"v1\""}},"f2":{{"key":"v2"}},"f3":{{"key":{{"subkey":"v3"}}}}}}}})",
                   TARGET.substr(1))),
               strlen(R"("\"v1\""|v2|v3)"),
               getDSVParser,
               {NAME, TARGET, {""}, {"|", "\"", "\\", "f1.key", "f2/key", "f3.key/subkey"}})));
