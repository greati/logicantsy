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

      ltsy::PNMMultipleConclusionAxiomatizer axiomatizer {discriminator, matrix, 
	      std::make_optional<std::vector<int>>({0,3,1,2}), 
	      std::make_optional<std::vector<std::pair<int,int>>>({{0,3},{2,1}})
      };
      axiomatizer.make_exists_rules();
    };



    TEST(PNMAxiomatization, AxiomatizeCL) {
       ltsy::Signature cl_sig {
           {"and", 2},
           {"or", 2},
           {"->", 2},
           {"neg", 1},
           {"top", 0},
           {"bot", 0},
       };
       auto sig_ptr = std::make_shared<ltsy::Signature>(cl_sig);

        ltsy::BisonFmlaParser parser;
        auto p1 = parser.parse("p1");
        auto p2 = parser.parse("p2");
        auto p1_and_p2 = parser.parse("p1 and p2");
        auto p1_or_p2 = parser.parse("p1 or p2");
        auto p1_to_p2 = parser.parse("p1 -> p2");
        auto neg_p1 = parser.parse("neg p1");
        auto top = parser.parse("top()");
        auto bot = parser.parse("bot()");

	auto tt_or = ltsy::TruthTable<std::set<int>>(2, 
                {
                    {{0,0},{0}},
                    {{0,1},{1}},
                    {{1,0},{1}},
                    {{1,1},{1}},
        }, p1_or_p2);

	auto tt_and = ltsy::TruthTable<std::set<int>>(2, 
                {
                    {{0,0},{0}},
                    {{0,1},{0}},
                    {{1,0},{0}},
                    {{1,1},{1}},
        }, p1_and_p2);

	auto tt_imp = ltsy::TruthTable<std::set<int>>(2, 
                {
                    {{0,0},{1}},
                    {{0,1},{1}},
                    {{1,0},{0}},
                    {{1,1},{1}},
        }, p1_to_p2);

	auto tt_neg = ltsy::TruthTable<std::set<int>>(2, 
                {
                    {{0},{1}},
                    {{1},{0}},
        }, neg_p1);

	auto tt_top = ltsy::TruthTable<std::set<int>>(2, 
                {
                    {{},{1}},
        }, top);

	auto tt_bot = ltsy::TruthTable<std::set<int>>(2, 
                {
                    {{},{0}},
        }, bot);

       auto top_int =  std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["top"], 
                std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_top));
       auto bot_int =  std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["bot"], 
                std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_bot));
       auto or_int =  std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["or"], 
               std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_or));
       auto and_int = std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["and"], 
               std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_and));
       auto imp_int = std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["->"], 
               std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_imp));
       auto neg_int = std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["neg"], 
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


       auto p = std::make_shared<ltsy::Prop>("p");
       ltsy::Discriminator discriminator
       {{
            {0, {{},{p}}},
            {1, {{p},{}}},
       }};


	{
	       //////// the NMatrix
	       auto cl_matrix = 
		   std::make_shared<ltsy::GenMatrix>(std::set<int>{0,1}, std::vector<std::set<int>>{std::set<int> {1}}, sig_ptr, 
			   std::make_shared<ltsy::SignatureTruthInterp<std::set<int>>>(truth_interp));

		std::set<ltsy::MultipleConclusionRule> expected {
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{p1}, ltsy::FmlaSet{p1_and_p2}}), {{1,0}}},
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{p2}, ltsy::FmlaSet{p1_and_p2}}), {{1,0}}},
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{p1_and_p2}, ltsy::FmlaSet{p1, p2}}), {{1,0}}},
		    //
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{p1_or_p2}, ltsy::FmlaSet{p1}}), {{1,0}}},
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{p1_or_p2}, ltsy::FmlaSet{p2}}), {{1,0}}},
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{p1,p2}, ltsy::FmlaSet{p1_or_p2}}), {{1,0}}},
		    //
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{p1_to_p2}, ltsy::FmlaSet{p2}}), {{1,0}}},
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{p1_to_p2,p1}, ltsy::FmlaSet{}}), {{1,0}}},
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{p2}, ltsy::FmlaSet{p1_to_p2, p1}}), {{1,0}}},
		    //
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{top}, ltsy::FmlaSet{}}), {{1,0}}},
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{}, ltsy::FmlaSet{bot}}), {{1,0}}},
		    //
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{neg_p1, p1}, ltsy::FmlaSet{}}), {{1,0}}},
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{}, ltsy::FmlaSet{neg_p1, p1}}), {{1,0}}},
		};
	    
		ltsy::PNMMultipleConclusionAxiomatizer axiomatizer {discriminator, cl_matrix, 
			std::make_optional<std::vector<int>>({1,0}),
			std::make_optional<std::vector<std::pair<int,int>>>({{1,0}})
		};
		auto calc = axiomatizer.make_single_calculus(true, true, true, true);

		ASSERT_EQ(calc.rules_set().size(), expected.size());
		ASSERT_TRUE(calc.rules_set() == expected);
	}

    }


    TEST(PNMAxiomatization, AxiomatizeBKPWK) {
       ltsy::Signature cl_sig {
           {"and", 2},
           {"or", 2},
           {"neg", 1},
       };
       auto sig_ptr = std::make_shared<ltsy::Signature>(cl_sig);



        ltsy::BisonFmlaParser parser;
        auto p1 = parser.parse("p1");
        auto p2 = parser.parse("p2");
        auto p1_and_p2 = parser.parse("p1 and p2");
        auto neg_p1_and_p2 = parser.parse("neg(p1 and p2)");
        auto p1_or_p2 = parser.parse("p1 or p2");
        auto neg_p1_or_p2 = parser.parse("neg(p1 or p2)");
        auto neg_p1 = parser.parse("neg p1");
        auto neg_p2 = parser.parse("neg p2");
        auto neg_neg_p1 = parser.parse("neg (neg p1)");

	auto tt_or = ltsy::TruthTable<std::set<int>>(3, 
                {
                    {{0,0},{0}},
                    {{1,0},{1}},
                    {{2,0},{2}},
                    {{0,1},{1}},
                    {{1,1},{1}},
                    {{2,1},{2}},
                    {{0,2},{2}},
                    {{1,2},{2}},
                    {{2,2},{2}},
        }, p1_or_p2);

	auto tt_and = ltsy::TruthTable<std::set<int>>(3, 
                {
                    {{0,0},{0}},
                    {{1,0},{0}},
                    {{2,0},{2}},
                    {{0,1},{0}},
                    {{1,1},{1}},
                    {{2,1},{2}},
                    {{0,2},{2}},
                    {{1,2},{2}},
                    {{2,2},{2}},
        }, p1_and_p2);

	auto tt_neg = ltsy::TruthTable<std::set<int>>(3, 
                {
                    {{0},{1}},
                    {{1},{0}},
                    {{2},{2}},
        }, neg_p1);

       auto or_int =  std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["or"], 
               std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_or));
       auto and_int = std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["and"], 
               std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_and));
       auto neg_int = std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["neg"], 
              std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_neg));

       auto truth_interp = ltsy::SignatureTruthInterp<std::set<int>>(sig_ptr, 
               {
                   or_int,
                   and_int,
                   neg_int
               });

        auto p = parser.parse("p");
        auto neg_p = parser.parse("neg p");

       ltsy::Discriminator discriminator
       {{
            {0, {{},{p}}},
            {1, {{p},{neg_p}}},
            {2, {{p, neg_p},{}}},
       }};


	{
	       //////// pwk
	       auto cl_matrix = 
		   std::make_shared<ltsy::GenMatrix>(std::set<int>{0,1,2}, std::vector<std::set<int>>{std::set<int> {1,2}}, sig_ptr, 
			   std::make_shared<ltsy::SignatureTruthInterp<std::set<int>>>(truth_interp));

		std::vector<ltsy::MultipleConclusionRule> expected {
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{},ltsy::FmlaSet{p1, neg_p1}}), {{0,1}}},
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{neg_neg_p1},ltsy::FmlaSet{p1}}), {{0,1}}},
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{p1},ltsy::FmlaSet{neg_neg_p1}}), {{0,1}}},
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{p1,p2},ltsy::FmlaSet{p1_and_p2}}), {{0,1}}},
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{p1_and_p2},ltsy::FmlaSet{p1, p2}}), {{0,1}}},
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{p1_and_p2},ltsy::FmlaSet{p1, neg_p2}}), {{0,1}}},
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{p1_and_p2},ltsy::FmlaSet{neg_p1, p2}}), {{0,1}}},
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{neg_p1_and_p2},ltsy::FmlaSet{neg_p1, neg_p2}}), {{0,1}}},
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{p1, neg_p1},ltsy::FmlaSet{p1_and_p2}}), {{0,1}}},
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{p1, neg_p1},ltsy::FmlaSet{neg_p1_and_p2}}), {{0,1}}},
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{p2, neg_p2},ltsy::FmlaSet{p1_and_p2}}), {{0,1}}},
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{p2, neg_p2},ltsy::FmlaSet{neg_p1_and_p2}}), {{0,1}}},
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{p1},ltsy::FmlaSet{p1_or_p2}}), {{0,1}}},
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{p2},ltsy::FmlaSet{p1_or_p2}}), {{0,1}}},
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{p1_or_p2},ltsy::FmlaSet{p1, p2}}), {{0,1}}},
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{neg_p1_or_p2},ltsy::FmlaSet{p1, neg_p2}}), {{0,1}}},
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{neg_p1_or_p2},ltsy::FmlaSet{neg_p1, p2}}), {{0,1}}},
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{neg_p1_or_p2},ltsy::FmlaSet{neg_p1, neg_p2}}), {{0,1}}},
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{p1, neg_p1},ltsy::FmlaSet{neg_p1_or_p2}}), {{0,1}}},
		    {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{p2, neg_p2},ltsy::FmlaSet{neg_p1_or_p2}}), {{0,1}}}
		};

		ltsy::MultipleConclusionCalculus expected_calc {expected};

		//auto calc = monadic_gen_matrix_mult_conc_axiomatizer(cl_matrix, discriminator, {0,1}, {{0,1}}, true, true, 5, 5, true);
	    
		ltsy::PNMMultipleConclusionAxiomatizer axiomatizer {discriminator, cl_matrix, 
			std::make_optional<std::vector<int>>({1,0}),
			std::make_optional<std::vector<std::pair<int,int>>>({{1,0}})
		};
		auto calc = axiomatizer.make_single_calculus(true, true, 3, 3, true);
		for (const auto& r : calc.rules_set())
			std::cout << r.sequent().to_string() << std::endl;

		ASSERT_TRUE(calc.is_equivalent(expected_calc, {p1, neg_p1}, {p1, neg_p1}));
	}

    }

    TEST(PNMAxiomatization, AxiomatizeFreeMat) {
       ltsy::Signature cl_sig {
           {"and", 2},
           {"or", 2},
           {"->", 2},
           {"neg", 1},
           {"top", 0},
           {"bot", 0},
       };
       auto sig_ptr = std::make_shared<ltsy::Signature>(cl_sig);

       auto tt_or =  ltsy::TruthTable<std::set<int>>(2, 2, std::vector<std::set<int>>{{0,1}, {0,1}, {0,1}, {0,1}});
       auto tt_and = ltsy::TruthTable<std::set<int>>(2, 2, std::vector<std::set<int>>{{0,1}, {0,1}, {0,1}, {0,1}});
       auto tt_imp = ltsy::TruthTable<std::set<int>>(2, 2, std::vector<std::set<int>>{{0,1}, {0,1}, {0,1}, {0,1}});
       auto tt_neg = ltsy::TruthTable<std::set<int>>(2, 1, std::vector<std::set<int>>{{0,1}, {0,1}});
       auto tt_top = ltsy::TruthTable<std::set<int>>(2, 0, std::vector<std::set<int>>{{0,1}});
       auto tt_bot = ltsy::TruthTable<std::set<int>>(2, 0, std::vector<std::set<int>>{{0,1}});

       auto top_int =  std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["top"], 
                std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_top));
       auto bot_int =  std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["bot"], 
                std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_bot));
       auto or_int =  std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["or"], 
               std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_or));
       auto and_int = std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["and"], 
               std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_and));
       auto imp_int = std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["->"], 
               std::make_shared<ltsy::TruthTable<std::set<int>>>(tt_imp));
       auto neg_int = std::make_shared<ltsy::TruthInterp<std::set<int>>>((*sig_ptr)["neg"], 
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

        std::set<ltsy::MultipleConclusionRule> expected {
        };
    
        ltsy::PNMMultipleConclusionAxiomatizer axiomatizer {discriminator, cl_matrix, 
		std::make_optional<std::vector<int>>({1,0}),
		std::make_optional<std::vector<std::pair<int,int>>>({{1,0}})
	};
        auto calc = axiomatizer.make_single_calculus(true, true, true, true);

        ASSERT_EQ(calc.rules_set().size(), expected.size());
        ASSERT_TRUE(calc.rules_set() == expected);
    }
}

