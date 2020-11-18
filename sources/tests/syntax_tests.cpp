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
        std::set<std::shared_ptr<ltsy::Prop>, ltsy::utils::DeepSharedPointerComp<ltsy::Prop>> expected;
        expected.insert(p); expected.insert(q);
        ASSERT_EQ(var_collector.get_collected_variables().size(), 2);
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

    TEST(Formula, FormulaAssignmentGenerator3props) {
        ltsy::BisonFmlaParser parser;
        auto fmla1 = parser.parse("p and (q or p)");
        auto fmla2 = parser.parse("p or r");
        auto fmla3 = parser.parse("neg t");
        ltsy::FmlaSet s {fmla1, fmla2, fmla3};
        std::vector<std::shared_ptr<ltsy::Prop>> props
            {
                std::make_shared<ltsy::Prop>("p"),
                std::make_shared<ltsy::Prop>("q"),
                std::make_shared<ltsy::Prop>("r"),
            };
        ltsy::FormulaVarAssignmentGenerator gen {props, s};
        int c = 0;
        while (gen.has_next()) {
            auto v = gen.next();
            std::cout << v->print().str() << std::endl;
            c++;
        }
        std::cout << c << std::endl;
    }

    TEST(Formula, FormulaAssignmentGenerator1prop) {
        ltsy::BisonFmlaParser parser;
        auto fmla1 = parser.parse("p and (q or p)");
        auto fmla2 = parser.parse("p or r");
        auto fmla3 = parser.parse("neg t");
        ltsy::FmlaSet s {fmla1, fmla2, fmla3};
        std::vector<std::shared_ptr<ltsy::Prop>> props
            {
                std::make_shared<ltsy::Prop>("p"),
            };
        ltsy::FormulaVarAssignmentGenerator gen {props, s};
        int c = 0;
        while (gen.has_next()) {
            auto v = gen.next();
            std::cout << v->print().str() << std::endl;
            c++;
        }
        std::cout << c << std::endl;
    }

    TEST(Formula, FormulaAssignmentGenerator0prop) {
        ltsy::BisonFmlaParser parser;
        auto fmla1 = parser.parse("p and (q or p)");
        auto fmla2 = parser.parse("p or r");
        auto fmla3 = parser.parse("neg t");
        ltsy::FmlaSet s {fmla1, fmla2, fmla3};
        std::vector<std::shared_ptr<ltsy::Prop>> props {};
        ltsy::FormulaVarAssignmentGenerator gen {props, s};
        int c = 0;
        while (gen.has_next()) {
            auto v = gen.next();
            std::cout << v->print().str() << std::endl;
            c++;
        }
        std::cout << c << std::endl;
    }

    TEST(Formula, FmlaSetIntersect) {
        ltsy::BisonFmlaParser parser;
        auto fmla1 = parser.parse("p and (q or p)");
        auto fmla2 = parser.parse("p or r");
        auto fmla3 = parser.parse("neg t");
        ltsy::FmlaSet f1 {fmla1, fmla2};
        ltsy::FmlaSet f2 {fmla2, fmla3};
        ltsy::FmlaSet inters;
        std::set_intersection(f1.begin(), f1.end(),
                f2.begin(), f2.end(), std::inserter(inters, inters.begin()));
        for (auto f : inters)
            std::cout << *f << std::endl;
    }

};
