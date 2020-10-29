#include "gtest/gtest.h"
#include "core/syntax.h"
#include "core/parser/fmla/fmla_parser.h"
#include <memory>

namespace {

    TEST(Connective, ValidArity) {
        EXPECT_THROW(
            try {
                auto c = ltsy::Connective("+", -1);
            } catch (const std::invalid_argument& e) {
                EXPECT_STREQ(ltsy::NEGATIVE_ARITY_EXCEPTION.c_str(), e.what());
                throw;
            },
        std::invalid_argument);
    }

    TEST(Connective, WeakOrdering) {
        ltsy::BisonFmlaParser parser;
        ltsy::FmlaSet fmlas;
        fmlas.insert(parser.parse("p and (q or p)"));
        fmlas.insert(parser.parse("p and (r or p)"));
        fmlas.insert(parser.parse("p and (r or p)"));
        fmlas.insert(parser.parse("p and q"));
        fmlas.insert(parser.parse("p"));
        fmlas.insert(parser.parse("q"));
        fmlas.insert(parser.parse("p"));
        fmlas.insert(parser.parse("p or q"));
        fmlas.insert(parser.parse("p or q"));
        ASSERT_EQ(6, fmlas.size());
    }


    TEST(Formula, Compound) {
        auto p = std::make_shared<ltsy::Prop>("p");
        auto q = std::make_shared<ltsy::Prop>("q");
        auto land = std::make_shared<ltsy::Connective>("&", 2);
        ltsy::Compound f {land, {p, q}};
        ltsy::FormulaPrinter printer;
        f.accept(printer);
    }


    TEST(Formula, Substitution) {
        ltsy::BisonFmlaParser parser;
        auto fmla = parser.parse("p and (q or p)");
        auto fmla1 = parser.parse("q or r");
        auto fmla2 = parser.parse("r");
        ltsy::FormulaVarAssignment ass {
            {
                {ltsy::Prop("p"), fmla1}, 
                {ltsy::Prop("q"), fmla2}
            }
        };
        ltsy::SubstitutionEvaluator subs {ass};
        auto subsfmla = fmla->accept(subs);
        std::cout << (*subsfmla) << std::endl;
    }

    TEST(Formula, SubformulaCollect) {
        ltsy::BisonFmlaParser parser;
        auto fmla = parser.parse("p and (q or p)");
        ltsy::SubFormulaCollector subs;
        fmla->accept(subs);
        for (const auto& f : subs.subfmlas()) {
            std::cout << *f << std::endl;
        }
    }

    TEST(Formula, CollectVariables) {
        auto p = std::make_shared<ltsy::Prop>("p");
        auto q = std::make_shared<ltsy::Prop>("q");
        auto land = std::make_shared<ltsy::Connective>("&", 2);
        ltsy::Compound f {land, {p, q}};
        ltsy::VariableCollector var_collector;
        f.accept(var_collector);
        std::set<ltsy::Prop*, ltsy::utils::DeepPointerComp<ltsy::Prop>> expected;
        expected.insert(p.get()); expected.insert(q.get());
        ASSERT_EQ(var_collector.get_collected_variables(), expected);
    }

    TEST(Formula, CollectSignature) {
        auto p = std::make_shared<ltsy::Prop>("p");
        auto q = std::make_shared<ltsy::Prop>("q");
        auto land = std::make_shared<ltsy::Connective>("&", 2);
        ltsy::Compound f {land, {p, q}};
        ltsy::SignatureCollector sig_collector;
        f.accept(sig_collector);
        ltsy::Signature expected;
        expected.add(land);
        ASSERT_EQ(sig_collector.get_collected_signature(), expected);
    }

    TEST(Signature, Specification) {
        ltsy::Signature cl_sig {
            {"&", 2},
            {"|", 2},
            {"->", 2},
            {"~", 1},
            {"1", 0},
            {"0", 0},
        }; 
        ASSERT_EQ(cl_sig["~"]->arity(), 1);
        ASSERT_EQ(cl_sig["|"]->arity(), 2);
        ASSERT_EQ(cl_sig(2)["&"]->arity(), 2);
        ASSERT_EQ(cl_sig(2)["|"]->arity(), 2);
        ASSERT_EQ(cl_sig(2)["->"]->arity(), 2);
    }

};
