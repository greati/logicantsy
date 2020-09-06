#include "gtest/gtest.h"
#include "core/common.h"
#include "core/parser/fmla/fmla_parser.h"

namespace {

    TEST(FmlaParser, BisonFmlaParserSimple) {

        ltsy::BisonFmlaParser parser;

        std::string str1 = "#(p, q, #(s, t))";
        std::cout << *parser.parse(str1) << std::endl;

        std::string str2 = "~(p \/\ q) \\/ r";
        std::cout << *parser.parse(str2) << std::endl;

        std::string str3 = "p `#` q `#` _";
        std::cout << *parser.parse(str3) << std::endl;
    }

}
