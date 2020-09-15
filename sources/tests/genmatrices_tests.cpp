#include "gtest/gtest.h"
#include "core/semantics/genmatrix.h"

namespace {

    TEST(GenMatrices, Construct) {
        ltsy::Signature cl_sig {
            {"&", 2},
            {"|", 2},
            {"->", 2},
            {"~", 1},
            {"1", 0},
            {"0", 0},
        };
        auto sig_ptr = std::make_shared<ltsy::Signature>(cl_sig);

        auto tt_or =  ltsy::TruthTable<std::set<int>>(2, 2, std::vector<std::set<int>> {{0}, {1}, {1}, {1}});
        auto tt_and = ltsy::TruthTable<std::set<int>>(2, 2, std::vector<std::set<int>> {{0}, {0}, {0}, {1}});
        auto tt_imp = ltsy::TruthTable<std::set<int>>(2, 2, std::vector<std::set<int>> {{1}, {1}, {0}, {1}});
        auto tt_neg = ltsy::TruthTable<std::set<int>>(2, 1, std::vector<std::set<int>> {{1}, {0}});
        auto tt_top = ltsy::TruthTable<std::set<int>>(2, 0, std::vector<std::set<int>> {{1}});
        auto tt_bot = ltsy::TruthTable<std::set<int>>(2, 0, std::vector<std::set<int>> {{0}});

        auto top_int =  std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["1"], 
                std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_top));
        auto bot_int =  std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["0"], 
                std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_bot));
        auto or_int =  std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["|"], 
                std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_or));
        auto and_int = std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["&"], 
                std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_and));
        auto imp_int = std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["->"], 
                std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_imp));
        auto neg_int = std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["~"], 
                std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_neg));


        auto truth_interp = ltsy::SignatureTruthInterp<std::set<int>>(sig_ptr, 
                {
                    top_int, 
                    bot_int,
                    or_int,
                    and_int,
                    imp_int,
                    neg_int
                });

        //// the NMatrix
        auto cl_matrix = 
            std::make_shared<ltsy::GenMatrix>(std::set<int>{0,1}, std::vector<std::set<int>>{std::set<int> {1}, std::set<int>{0}}, sig_ptr, 
                    std::make_shared<ltsy::SignatureTruthInterp<std::set<int>>>(truth_interp));
    }
};
