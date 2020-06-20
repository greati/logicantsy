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
        auto tt_top = ltsy::TruthTable<int>(2, 0, {1});
        auto tt_bot = ltsy::TruthTable<int>(2, 0, {0});

        auto top_int =  std::make_shared<ltsy::TruthInterp<int>>((*sig_ptr)["1"], 
                std::make_shared<ltsy::TruthTable<int>>(tt_top));
        auto bot_int =  std::make_shared<ltsy::TruthInterp<int>>((*sig_ptr)["0"], 
                std::make_shared<ltsy::TruthTable<int>>(tt_bot));
        auto or_int =  std::make_shared<ltsy::TruthInterp<int>>((*sig_ptr)["|"], 
                std::make_shared<ltsy::TruthTable<int>>(tt_or));
        auto and_int = std::make_shared<ltsy::TruthInterp<int>>((*sig_ptr)["&"], 
                std::make_shared<ltsy::TruthTable<int>>(tt_and));
        auto imp_int = std::make_shared<ltsy::TruthInterp<int>>((*sig_ptr)["->"], 
                std::make_shared<ltsy::TruthTable<int>>(tt_imp));
        auto neg_int = std::make_shared<ltsy::TruthInterp<int>>((*sig_ptr)["~"], 
                std::make_shared<ltsy::TruthTable<int>>(tt_neg));


        auto truth_interp = ltsy::SignatureTruthInterp<int>(sig_ptr, 
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
            std::make_shared<ltsy::NMatrix>(2, std::set<int> {1}, sig_ptr, 
                    std::make_shared<ltsy::SignatureTruthInterp<int>>(truth_interp));

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

    TEST(NMatrices, NMatrixGenerator) {
        ltsy::Signature cl_sig {
            {"&", 2},
            {"|", 2},
            {"->", 2},
            {"~", 1},
            {"1", 0},
            {"0", 0},
        };
        ltsy::NMatrixGenerator gen {2, std::make_shared<ltsy::Signature>(cl_sig)};
        int i = 0;
        while (gen.has_next()) {
            i++;
            auto m = gen.next();
            std::cout << *m << std::endl;
        }
        std::cout << i << std::endl;
    }

    TEST(NMatrices, ValGenerator) {
        ltsy::Signature cl_sig {
            {"&", 2},
            {"|", 2},
            {"->", 2},
            {"~", 1},
            {"1", 0},
            {"0", 0},
        };
        auto sig_ptr = std::make_shared<ltsy::Signature>(cl_sig);

        auto tt_top = ltsy::TruthTable<int>(2, 0, {1});
        auto tt_bot = ltsy::TruthTable<int>(2, 0, {0});
        auto tt_or =  ltsy::TruthTable<int>(2, 2, {0, 1, 1, 1});
        auto tt_and = ltsy::TruthTable<int>(2, 2, {0, 0, 0, 1});
        auto tt_imp = ltsy::TruthTable<int>(2, 2, {1, 1, 0, 1});
        auto tt_neg = ltsy::TruthTable<int>(2, 1, {1, 0});

        auto top_int = std::make_shared<ltsy::TruthInterp<int>>((*sig_ptr)["1"], 
                std::make_shared<ltsy::TruthTable<int>>(tt_top));

        std::cout << (*top_int) << std::endl;

        auto bot_int =  std::make_shared<ltsy::TruthInterp<int>>((*sig_ptr)["0"], 
                std::make_shared<ltsy::TruthTable<int>>(tt_bot));
        auto or_int =  std::make_shared<ltsy::TruthInterp<int>>((*sig_ptr)["|"], 
                std::make_shared<ltsy::TruthTable<int>>(tt_or));
        auto and_int = std::make_shared<ltsy::TruthInterp<int>>((*sig_ptr)["&"], 
                std::make_shared<ltsy::TruthTable<int>>(tt_and));

        std::cout << (*and_int) << std::endl;

        auto imp_int = std::make_shared<ltsy::TruthInterp<int>>((*sig_ptr)["->"], 
                std::make_shared<ltsy::TruthTable<int>>(tt_imp));
        auto neg_int = std::make_shared<ltsy::TruthInterp<int>>((*sig_ptr)["~"], 
                std::make_shared<ltsy::TruthTable<int>>(tt_neg));


        auto truth_interp = ltsy::SignatureTruthInterp<int>(sig_ptr, 
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
            std::make_shared<ltsy::NMatrix>(2, std::set<int> {1}, sig_ptr, 
                    std::make_shared<ltsy::SignatureTruthInterp<int>>(truth_interp));

        ltsy::NMatrixValuationGenerator generator {cl_matrix, {ltsy::Prop{"p"}, ltsy::Prop{"q"}, ltsy::Prop{"r"}}};
        while (generator.has_next()) {
            auto val = generator.next();
            //std::cout << val.print().str() << std::endl;
        }
    };

    TEST(NMatrices, TruthInterpGenerator) {
        ltsy::Signature cl_sig {
            {"&", 2},
            {"|", 2},
            {"->", 2},
            {"~", 1},
            {"1", 0},
            {"0", 0},
        };
        {
            ltsy::TruthInterpGenerator landgen {2, cl_sig["&"]};
            while(landgen.has_next()) {
                auto tt = landgen.next();
                //std::cout << (*tt) << std::endl;
            }
        }
        {
            ltsy::TruthInterpGenerator landgen {2, cl_sig["1"]};
            while(landgen.has_next()) {
                auto tt = landgen.next();
                //std::cout << (*tt) << std::endl;
            }
        }
    };

    TEST(NMatrices, SigTruthInterpGenerator) {
        ltsy::Signature cl_sig {
            {"->", 2},
            {"~", 1},
            {"1", 0},
        };
        auto sig_ptr = std::make_shared<ltsy::Signature>(cl_sig);
        {
            ltsy::SignatureTruthInterpGenerator landgen {2, sig_ptr};
            while(landgen.has_next()) {
                auto tt = landgen.next();
                //std::cout << (*tt) << std::endl;
            }
        }
    };

};
