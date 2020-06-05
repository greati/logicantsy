#include "gtest/gtest.h"
#include "core/semantics/nmatrices.h"

namespace {

    TEST(NMatrices, Construct) {
        ltsy::Signature cl_sig {
            {"&", 2},
            {"|", 2},
            {"->", 2},
            {"~", 1},
            {"1", 0},
            {"0", 0},
        };
        auto tt_or =  ltsy::TruthTable<int>(2, 2, {0, 1, 1, 1});
        auto tt_and = ltsy::TruthTable<int>(2, 2, {0, 0, 0, 1});
        auto tt_imp = ltsy::TruthTable<int>(2, 2, {1, 1, 0, 1});
        auto tt_neg = ltsy::TruthTable<int>(2, 1, {1, 0});

        auto top_int = std::make_shared<ltsy::ConstantTruthInterp>(cl_sig["1"], 2, 1);
        auto bot_int = std::make_shared<ltsy::ConstantTruthInterp>(cl_sig["0"], 2, 0);
        auto or_int =  std::make_shared<ltsy::ConnectiveTruthInterp<int>>(cl_sig["|"], 
                std::make_shared<ltsy::TruthTable<int>>(tt_or));
        auto and_int = std::make_shared<ltsy::ConnectiveTruthInterp<int>>(cl_sig["&"], 
                std::make_shared<ltsy::TruthTable<int>>(tt_and));
        auto imp_int = std::make_shared<ltsy::ConnectiveTruthInterp<int>>(cl_sig["->"], 
                std::make_shared<ltsy::TruthTable<int>>(tt_imp));
        auto neg_int = std::make_shared<ltsy::ConnectiveTruthInterp<int>>(cl_sig["~"], 
                std::make_shared<ltsy::TruthTable<int>>(tt_neg));

        auto sig_ptr = std::make_shared<ltsy::Signature>(cl_sig);
        auto truth_interp = ltsy::SignatureTruthInterp(sig_ptr, 
                {
                    top_int, 
                    bot_int,
                    or_int,
                    and_int,
                    imp_int,
                    neg_int
                });
       ltsy::NMatrix cl_matrix {2, 1, sig_ptr, std::make_shared<ltsy::SignatureTruthInterp>(truth_interp)};

    }

};
