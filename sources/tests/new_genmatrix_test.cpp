#include "gtest/gtest.h"
#include "core/semantics/new_genmatrix.h"

namespace{
    TEST(New_GenMatrix, Construct) {
        ltsy::Signature sig {
            {"~", 1},
            {"|", 2},
            {"&", 2}
        };

        std::vector<std::string> domain({ "0", "a", "b", "1" });
        std::vector<std::set<std::string>> d_sets({{"b", "1"}});

        //Construct the truth tables
        std::map<ltsy::Symbol, std::vector<std::set<std::string>>> truth_tables;
        truth_tables["~"] = {{"1"}, {"a"}, {"b"}, {"0"}};
        truth_tables["|"] = {
            {"0"}, {"a"}, {"b"}, {"1"},
            {"a"}, {"a"}, {}, {"1"},
            {"b"}, {}, {"b"}, {"1"},
            {"1"}, {"1"}, {"1"}, {"1"},
        };
        truth_tables["&"] = {
            {"0"}, {"0"}, {"0"}, {"0"},
            {"0"}, {"a"}, {}, {"a"},
            {"0"}, {}, {"b"}, {"b"},
            {"0"}, {"a"}, {"b"}, {"1"},
        };

        ltsy::NewGenMatrix matrix(domain, d_sets, sig, truth_tables);
        
    }
}