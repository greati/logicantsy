#include "gtest/gtest.h"
#include "core/parser/fmla/fmla_parser.h"
#include "apps/pnm-axiomatization/multipleconclusion.h"

namespace {

    TEST(PNMAxiomatization, Axiomatize2DExistsCheck) {
        ltsy::Signature cl_sig {
            {"&", 2},
            {"neg", 1},
            {"o", 1},
            {"1", 0},
            {"0", 0},
        };
        auto sig_ptr = std::make_shared<ltsy::Signature>(cl_sig);

        auto tt_and = ltsy::TruthTable<std::set<int>>(6, 2, 
                std::vector<std::set<int>>{
                    {0}, {0}, {0}, {1}, {3}, {2},
                    {0}, {0}, {0}, {1}, {3}, {2},
                    {0}, {0}, {0}, {1}, {3}, {2},
                    {0}, {0}, {0}, {1}, {3}, {2},
                    {0}, {0}, {0}, {1}, {3}, {2},
                    {0}, {0}, {0}, {1}, {3}, {2},
                }
                );
        auto tt_neg = ltsy::TruthTable<std::set<int>>(6, 1, 
                std::vector<std::set<int>>{{5}, {4}, {3}, {2}, {1}, {0}}
                );
        auto tt_circ = ltsy::TruthTable<std::set<int>>(6, 1, 
                std::vector<std::set<int>>{{5}, {0}, {0}, {0}, {0}, {5}}
                );
        auto tt_top = ltsy::TruthTable<std::set<int>>(6, 0, std::vector<std::set<int>>{{5}});
        auto tt_bot = ltsy::TruthTable<std::set<int>>(6, 0, std::vector<std::set<int>>{{0}});

        auto bot_int =  std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["0"], 
                std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_bot));
        auto top_int =  std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["1"], 
                std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_top));
        auto and_int = std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["&"], 
                std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_and));
        auto neg_int = std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["neg"], 
               std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_neg));
        auto circ_int = std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["o"], 
               std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_circ));

       auto truth_interp = ltsy::SignatureTruthInterp<std::set<int>>(sig_ptr, 
               {
                   top_int, 
                   and_int,
                   neg_int,
                   circ_int,
                   bot_int,
               });

       ////// the NMatrix
       auto matrix = 
           std::make_shared<ltsy::GenMatrix>(std::set<int>{0,1,2,3,4,5}, std::vector<std::set<int>>{std::set<int> {3,4,5}, std::set<int>{3,4,5}}, sig_ptr, 
                   std::make_shared<ltsy::SignatureTruthInterp<std::set<int>>>(truth_interp));

       ltsy::BisonFmlaParser parser;
       auto p = parser.parse("p");
       auto neg_p = parser.parse("neg p");
       auto circ_p = parser.parse("o(p)");

      ltsy::Discriminator discriminator
      {{
           {0, {{circ_p},{p},{p},{p}}},
           {1, {{neg_p},{p, circ_p}, {p},{p}}},
           {2, {{},{p, circ_p, neg_p}, {}, {}}},
           {3, {{p, neg_p},{circ_p}, {}, {}}},
           {4, {{p},{circ_p, neg_p}, {}, {}}},
           {5, {{p, circ_p},{}, {}, {}}},
      }};

      ltsy::PNMMultipleConclusionAxiomatizer axiomatizer {discriminator, matrix, {0,1,2,3}, {{0,3},{2,1}}};
      axiomatizer.make_exists_rules();
    };

    TEST(PNMAxiomatization, AxiomatizeCL) {
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
       auto tt_imp = ltsy::TruthTable<std::set<int>>(2, 2, std::vector<std::set<int>>{{1}, {1}, {0}, {1}});
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

      //////// the NMatrix
      auto cl_matrix = 
          std::make_shared<ltsy::GenMatrix>(std::set<int>{0,1}, std::vector<std::set<int>>{std::set<int> {1}}, sig_ptr, 
                  std::make_shared<ltsy::SignatureTruthInterp<std::set<int>>>(truth_interp));

      auto p = std::make_shared<ltsy::Prop>("p");
      ltsy::Discriminator discriminator
      {{
           {0, {{},{p}}},
           {1, {{p},{}}},
      }};
    
       ltsy::PNMMultipleConclusionAxiomatizer axiomatizer {discriminator, cl_matrix, {0,1}, {{0,1}}};
       auto calc = axiomatizer.make_single_calculus(true, true, true, true);
       for (const auto& r : calc.rules()) {
            std::cout << r.sequent().to_string() << std::endl;
       } 
    }
}

