#include "gtest/gtest.h"
#include "core/semantics/genmatrix.h"

namespace {

    TEST(GenMatrices, ConstructAndEvaluate) {
        ltsy::Signature cl_sig {
            {"&", 2},
            {"|", 2},
            {"->", 2},
            {"~", 1},
            {"1", 0},
            {"0", 0},
        };
        auto sig_ptr = std::make_shared<ltsy::Signature>(cl_sig);

        auto tt_or =  ltsy::TruthTable<std::set<int>>(2, 2, std::vector<std::set<int>>{{0}, {1}, {1}, {1}});
        auto tt_and = ltsy::TruthTable<std::set<int>>(2, 2, std::vector<std::set<int>>{{0}, {0}, {0}, {1}});
        auto tt_imp = ltsy::TruthTable<std::set<int>>(2, 2, std::vector<std::set<int>>{{1}, {1}, {0,1,2}, {1}});
        auto tt_neg = ltsy::TruthTable<std::set<int>>(2, 1, std::vector<std::set<int>>{{1}, {0}});
        auto tt_top = ltsy::TruthTable<std::set<int>>(2, 0, std::vector<std::set<int>>{{1}});
        auto tt_bot = ltsy::TruthTable<std::set<int>>(2, 0, std::vector<std::set<int>>{{0}});

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

       ////// the NMatrix
       auto cl_matrix = 
           std::make_shared<ltsy::GenMatrix>(std::set<int>{0,1}, std::vector<std::set<int>>{std::set<int> {1}}, sig_ptr, 
                   std::make_shared<ltsy::SignatureTruthInterp<std::set<int>>>(truth_interp));

       auto p = std::make_shared<ltsy::Prop>("p");
       auto q = std::make_shared<ltsy::Prop>("q");

       //// an M-valuation
       auto val = std::make_shared<ltsy::GenMatrixVarAssignment>(cl_matrix, 
          std::vector<std::pair<ltsy::Prop, int>> {{*p, 1}, {*q, 0}});

       //// an evaluator
       ltsy::GenMatrixPossibleValuesCollector evaluator {val};

       ltsy::Compound fmla ((*sig_ptr)["->"], std::vector<std::shared_ptr<ltsy::Formula>> {p, q});

       auto v = fmla.accept(evaluator);

       std::cout << v << std::endl;
    }

    TEST(GenMatrices, GenerateValuations) {
        ltsy::Signature cl_sig {
            {"&", 2},
            {"|", 2},
            {"->", 2},
            {"~", 1},
            {"1", 0},
            {"0", 0},
        };
        auto sig_ptr = std::make_shared<ltsy::Signature>(cl_sig);

        auto tt_or =  ltsy::TruthTable<std::set<int>>(4, 2, std::vector<std::set<int>>{{0}, {1}, {1}, {1}});
        auto tt_and = ltsy::TruthTable<std::set<int>>(4, 2, std::vector<std::set<int>>{{0}, {0}, {0}, {1}});
        auto tt_imp = ltsy::TruthTable<std::set<int>>(4, 2, std::vector<std::set<int>>{{1}, {1}, {0,1,2}, {1}});
        auto tt_neg = ltsy::TruthTable<std::set<int>>(4, 1, std::vector<std::set<int>>{{1}, {0}});
        auto tt_top = ltsy::TruthTable<std::set<int>>(4, 0, std::vector<std::set<int>>{{1}});
        auto tt_bot = ltsy::TruthTable<std::set<int>>(4, 0, std::vector<std::set<int>>{{0}});

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

       ////// the NMatrix
       auto cl_matrix = 
           std::make_shared<ltsy::GenMatrix>(std::set<int>{0,1,2,3}, std::vector<std::set<int>>{std::set<int> {0,2}, std::set<int>{1,3}}, sig_ptr, 
                   std::make_shared<ltsy::SignatureTruthInterp<std::set<int>>>(truth_interp));

        std::shared_ptr<ltsy::Prop> p = std::make_shared<ltsy::Prop>("p");
        std::shared_ptr<ltsy::Prop> q = std::make_shared<ltsy::Prop>("q");
        ltsy::GenMatrixVarAssignmentGenerator generator {cl_matrix, std::vector<std::shared_ptr<ltsy::Prop>>{p, q}};
        while (generator.has_next()) {
            auto val = generator.next();
        }
    }

    TEST(GenMatrices, NdSequentSoundnessCheck) {
        ltsy::Signature cl_sig {
            {"&", 2},
            {"|", 2},
        };
        auto sig_ptr = std::make_shared<ltsy::Signature>(cl_sig);

        auto tt_or =  ltsy::TruthTable<std::set<int>>(2, 2, std::vector<std::set<int>>{{0}, {1}, {1}, {1}});
        auto tt_and = ltsy::TruthTable<std::set<int>>(2, 2, std::vector<std::set<int>>{{0}, {0}, {0}, {1}});

        auto or_int =  std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["|"], 
                std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_or));
        auto and_int = std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["&"], 
                std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_and));

       auto truth_interp = ltsy::SignatureTruthInterp<std::set<int>>(sig_ptr, 
               {
                   or_int,
                   and_int,
               });

        auto cl_matrix = 
           std::make_shared<ltsy::GenMatrix>(std::set<int>{0,1}, 
                   std::vector<std::set<int>>{std::set<int> {1}}, sig_ptr, 
                   std::make_shared<ltsy::SignatureTruthInterp<std::set<int>>>(truth_interp));


       auto p = std::make_shared<ltsy::Prop>("p");
       auto q = std::make_shared<ltsy::Prop>("q");
       auto conn = std::make_shared<ltsy::Connective>("&", 2);
       auto disj = std::make_shared<ltsy::Connective>("|", 2);
       auto p_conn_q = std::make_shared<ltsy::Compound>(conn, std::vector<std::shared_ptr<ltsy::Formula>>{p, q});
       auto p_disj_q = std::make_shared<ltsy::Compound>(disj, std::vector<std::shared_ptr<ltsy::Formula>>{p, q});

        // validity of a rule
       ltsy::NdSequent<std::set> seq21 ({{p},{p_conn_q}});
       ltsy::NdSequent<std::set> seq31 ({{p},{p_disj_q}});
       ltsy::NdSequent<std::set> seq22 ({{p},{q}});
       ltsy::NdSequentRule<std::set> seqrule1 ({seq21},{seq22});
       ltsy::NdSequentRule<std::set> seqrule2 ({seq31},{seq22});
        ltsy::NdSequentGenMatrixValidator<std::set> sequent_validator {cl_matrix, {0}};
        auto [con_test, con_ce] = sequent_validator.is_rule_satisfiability_preserving(seqrule1);
        std::cout << con_test << std::endl;
        auto [disj_test, disj_ce] = sequent_validator.is_rule_satisfiability_preserving(seqrule2);
        std::cout << disj_test << std::endl;
        std::vector<ltsy::NdSequentGenMatrixValidator<std::set>::CounterExample> ce_list = *disj_ce;
        for (const auto& e : ce_list) {
            std::cout << e.val.print().str() << std::endl;
        }

       //ltsy::NdSequent<std::set> seq1 ({{}, {p_conn_q},{}, {p,q}});
       //ltsy::NdSequent<std::set> seq2 ({{}, {p},{}, {p_conn_q}});
       //ltsy::NdSequent<std::set> seq3 ({{}, {q},{}, {p_conn_q}});
       //ltsy::NdSequent<std::set> seq4 ({{p,q}, {}, {p_conn_q}, {}});
       //ltsy::NdSequent<std::set> seq5 ({{p_conn_q}, {}, {p}, {}});
       //ltsy::NdSequent<std::set> seq6 ({{p_conn_q}, {}, {q}, {}});
    }

    TEST(GenMatrices, TTDeterminizationGenerator) {
        auto tt_or =  ltsy::TruthTable<std::set<int>>(2, 2, std::vector<std::set<int>>{{0, 1}, {0}, {}, {1,0}});
        ltsy::PartialDeterministicTruthTableGenerator generator {std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_or)};
        while (generator.has_next()) {
            auto t = generator.next();
            std::cout << (*t) << std::endl;
        }
        generator.reset();
        while (generator.has_next()) {
            auto t = generator.next();
            std::cout << (*t) << std::endl;
        }

        auto tt_fp =  ltsy::TruthTable<std::set<int>>(2, 2, std::vector<std::set<int>>{{}, {}, {}, {}});
        ltsy::PartialDeterministicTruthTableGenerator fullpartialgen {std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_fp)};
        while (fullpartialgen.has_next()) {
            auto t = fullpartialgen.next();
            std::cout << (*t) << std::endl;
        }
    }


    TEST(GenMatrices, TruthInterpDeterminizationGeneratorBinary) {
        ltsy::Signature cl_sig {
            {"&", 2},
            {"|", 2},
        };
        auto sig_ptr = std::make_shared<ltsy::Signature>(cl_sig);
        auto tt_or =  ltsy::TruthTable<std::set<int>>(2, 2, std::vector<std::set<int>>{{1}, {}, {}, {0,1}});
        auto tt_and =  ltsy::TruthTable<std::set<int>>(2, 2, std::vector<std::set<int>>{{}, {0,1}, {}, {}});

        auto or_int =  std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["|"], 
                std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_or));
        auto and_int = std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["&"], 
                std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_and));
        auto truth_interp = ltsy::SignatureTruthInterp<std::set<int>>(sig_ptr, 
               {or_int,and_int});

        ltsy::PartialDeterministicTruthInterpGenerator gen {std::make_shared<decltype(truth_interp)>(truth_interp)};
        int c = 0;
        while(gen.has_next()) {
            auto si = gen.next();
            c++;
            std::cout << (*si) << std::endl;
        }
        std::cout << c << std::endl;
        gen.reset();
        c = 0;
        while(gen.has_next()) {
            auto si = gen.next();
            c++;
            std::cout << (*si) << std::endl;
        }
        std::cout << c << std::endl;
    }

    TEST(GenMatrices, TruthInterpDeterminizationGeneratorUnary) {
        ltsy::Signature cl_sig {
            {"&", 1},
            {"|", 0},
        };
        auto sig_ptr = std::make_shared<ltsy::Signature>(cl_sig);
        auto tt_and =  ltsy::TruthTable<std::set<int>>(2, 1, std::vector<std::set<int>>{{0},{0,1}});
        auto tt_or =  ltsy::TruthTable<std::set<int>>(2, 0, std::vector<std::set<int>>{{0,1}});

        auto or_int =  std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["|"], 
                std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_or));
        auto and_int = std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["&"], 
                std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_and));
        auto truth_interp = ltsy::SignatureTruthInterp<std::set<int>>(sig_ptr, 
               {or_int,and_int});

        ltsy::PartialDeterministicTruthInterpGenerator gen {std::make_shared<decltype(truth_interp)>(truth_interp)};
        int c = 0;
        while(gen.has_next()) {
            auto si = gen.next();
            c++;
            std::cout << (*si) << std::endl;
        }
        std::cout << c << std::endl;
    }


    TEST(GenMatrices, GenMatrixValuationEvaluator) {
         ltsy::Signature cl_sig {
            {"&", 2},
            {"|", 2},
        };
        auto sig_ptr = std::make_shared<ltsy::Signature>(cl_sig);

        auto tt_or =  ltsy::TruthTable<std::set<int>>(2, 2, std::vector<std::set<int>>{{0}, {0}, {}, {0}});
        auto tt_and = ltsy::TruthTable<std::set<int>>(2, 2, std::vector<std::set<int>>{{0}, {1}, {0}, {1}});

        auto or_int =  std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["|"], 
                std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_or));
        auto and_int = std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["&"], 
                std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_and));

       auto truth_interp = ltsy::SignatureTruthInterp<std::set<int>>(sig_ptr, 
               {
                   or_int,
                   and_int,
               });
       auto truth_interp_ptr = std::make_shared<ltsy::SignatureTruthInterp<std::set<int>>>(truth_interp);

        auto cl_matrix = 
           std::make_shared<ltsy::GenMatrix>(std::set<int>{0,1}, 
                   std::vector<std::set<int>>{std::set<int> {1}}, sig_ptr, 
                   std::make_shared<ltsy::SignatureTruthInterp<std::set<int>>>(truth_interp));   

       auto p = std::make_shared<ltsy::Prop>("p");
       auto q = std::make_shared<ltsy::Prop>("q");
       auto conn = std::make_shared<ltsy::Connective>("&", 2);
       auto disj = std::make_shared<ltsy::Connective>("|", 2);
       auto p_conn_q = std::make_shared<ltsy::Compound>(conn, std::vector<std::shared_ptr<ltsy::Formula>>{p, q});
       auto p_disj_q = std::make_shared<ltsy::Compound>(disj, std::vector<std::shared_ptr<ltsy::Formula>>{p, q});

       auto val = std::make_shared<ltsy::GenMatrixVarAssignment>(cl_matrix, 
          std::vector<std::pair<ltsy::Prop, int>> {{*p, 1}, {*q, 0}});
       ltsy::GenMatrixValuation valuation (val, truth_interp_ptr);
       ltsy::GenMatrixEvaluator evaluator (std::make_shared<decltype(valuation)>(valuation));
       auto res = p_conn_q->accept(evaluator);
       std::cout << res << std::endl;
    }


    TEST(GenMatrices, GenMatrixValuationGenerator) {
         ltsy::Signature cl_sig {
            {"&", 2},
            {"|", 2},
        };
        auto sig_ptr = std::make_shared<ltsy::Signature>(cl_sig);

        auto tt_or =  ltsy::TruthTable<std::set<int>>(2, 2, std::vector<std::set<int>>{{0}, {1}, {1}, {0,1}});
        auto tt_and = ltsy::TruthTable<std::set<int>>(2, 2, std::vector<std::set<int>>{{0}, {1}, {}, {1}});

        auto or_int =  std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["|"], 
                std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_or));
        auto and_int = std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["&"], 
                std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_and));


       auto truth_interp = ltsy::SignatureTruthInterp<std::set<int>>(sig_ptr, 
               {
                   //bot_int,
                   or_int,
                   and_int,
                   //neg_int
               });

        auto cl_matrix = 
           std::make_shared<ltsy::GenMatrix>(std::set<int>{0,1}, 
                   std::vector<std::set<int>>{std::set<int> {1}}, sig_ptr, 
                   std::make_shared<ltsy::SignatureTruthInterp<std::set<int>>>(truth_interp));   

       auto p = std::make_shared<ltsy::Prop>("p");
       auto q = std::make_shared<ltsy::Prop>("q");
       auto conn = (*sig_ptr)["&"];//std::make_shared<ltsy::Connective>("&", 2);
       auto disj = std::make_shared<ltsy::Connective>("|", 2);
       auto p_conn_q = std::make_shared<ltsy::Compound>(conn, std::vector<std::shared_ptr<ltsy::Formula>>{p, q});
       auto p_disj_q = std::make_shared<ltsy::Compound>(disj, std::vector<std::shared_ptr<ltsy::Formula>>{p, q});

       ltsy::GenMatrixValuationGenerator generator {cl_matrix, {p, q}};
       int c = 0;
       while (generator.has_next()) {
           auto v = generator.next();
           ltsy::GenMatrixEvaluator evaluator {v};
           auto resand = p_conn_q->accept(evaluator);
           auto resor = p_disj_q->accept(evaluator);
           std::cout << resand << std::endl;
           std::cout << resor << std::endl;
           c++;
       }
       std::cout << c << std::endl;
    }
};
