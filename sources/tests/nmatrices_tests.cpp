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
        auto sig_ptr = std::make_shared<ltsy::Signature>(cl_sig);

        auto tt_or =  ltsy::TruthTable<int>(2, 2, {0, 1, 1, 1});
        auto tt_and = ltsy::TruthTable<int>(2, 2, {0, 0, 0, 1});
        auto tt_imp = ltsy::TruthTable<int>(2, 2, {1, 1, 0, 1});
        auto tt_neg = ltsy::TruthTable<int>(2, 1, {1, 0});

        auto top_int = std::make_shared<ltsy::ConstantTruthInterp<int>>((*sig_ptr)["1"], 1);
        auto bot_int = std::make_shared<ltsy::ConstantTruthInterp<int>>((*sig_ptr)["0"], 0);
        auto or_int =  std::make_shared<ltsy::ConnectiveTruthInterp<int>>((*sig_ptr)["|"], 
                std::make_shared<ltsy::TruthTable<int>>(tt_or));
        auto and_int = std::make_shared<ltsy::ConnectiveTruthInterp<int>>((*sig_ptr)["&"], 
                std::make_shared<ltsy::TruthTable<int>>(tt_and));
        auto imp_int = std::make_shared<ltsy::ConnectiveTruthInterp<int>>((*sig_ptr)["->"], 
                std::make_shared<ltsy::TruthTable<int>>(tt_imp));
        auto neg_int = std::make_shared<ltsy::ConnectiveTruthInterp<int>>((*sig_ptr)["~"], 
                std::make_shared<ltsy::TruthTable<int>>(tt_neg));


        auto truth_interp = ltsy::SignatureTruthInterp(sig_ptr, 
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
            std::make_shared<ltsy::NMatrix>(2, std::set<int> {1}, sig_ptr, std::make_shared<ltsy::SignatureTruthInterp>(truth_interp));

        auto p = std::make_shared<ltsy::Prop>("p");
        auto q = std::make_shared<ltsy::Prop>("q");

        //// an M-valuation
        auto val = std::make_shared<ltsy::NMatrixValuation>(cl_matrix, 
           std::vector<std::pair<ltsy::Prop, int>> {{*p, 0}, {*q, 1}});

        // an evaluator
        ltsy::NMatrixEvaluator evaluator {val};

        ltsy::Compound fmla ((*sig_ptr)["->"], std::vector<std::shared_ptr<ltsy::Formula>> {p, q});

        auto v = fmla.accept(evaluator);
    }

};
