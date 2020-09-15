#include "gtest/gtest.h"
#include "core/common.h"
#include "tt_determination/ndsequents.h"
#include "tt_determination/ndsequents_normal_form.h"
#include "core/semantics/attitudes.h"
#include "core/semantics/attitude_semantics.h"

namespace {

    TEST(Determinants, BSequentDetermination) {
        auto p = std::make_shared<ltsy::Prop>("p");
        auto q = std::make_shared<ltsy::Prop>("q");
        auto conn = std::make_shared<ltsy::Connective>("&", 2);
        auto p_conn_q = std::make_shared<ltsy::Compound>(conn, std::vector<std::shared_ptr<ltsy::Formula>>{p, q});

        ltsy::NdSequent<std::set> seq1 ({{}, {p_conn_q},{}, {p,q}});
        ltsy::NdSequent<std::set> seq2 ({{}, {p},{}, {p_conn_q}});
        ltsy::NdSequent<std::set> seq3 ({{}, {q},{}, {p_conn_q}});
        ltsy::NdSequent<std::set> seq4 ({{p,q}, {}, {p_conn_q}, {}});
        ltsy::NdSequent<std::set> seq5 ({{p_conn_q}, {}, {p}, {}});
        ltsy::NdSequent<std::set> seq6 ({{p_conn_q}, {}, {q}, {}});
        std::vector<ltsy::NdSequent<std::set>> sequents = {seq1, seq2, seq3, seq4, seq5, seq6};

        ltsy::CognitiveAttitude a {"A", {2, 3}};
        ltsy::CognitiveAttitude na {"NA", {0, 1}};
        ltsy::CognitiveAttitude nr {"NR", {1, 3}};
        ltsy::CognitiveAttitude r {"R", {0, 2}};

        ltsy::NdSequentTruthTableDeterminizer determinizer {4, {*p, *q}, *conn, {nr, na, r, a}};

        determinizer.determine(sequents);

        auto table = determinizer.table();

        std::cout << table << std::endl;
    }

    TEST(Determinants, TonkOverDetermination) {
        auto p = std::make_shared<ltsy::Prop>("p");
        auto q = std::make_shared<ltsy::Prop>("q");
        auto conn = std::make_shared<ltsy::Connective>("TONK", 2);
        auto p_conn_q = std::make_shared<ltsy::Compound>(conn, std::vector<std::shared_ptr<ltsy::Formula>>{p, q});

        ltsy::NdSequent<std::set> seq1 ({{p},{p_conn_q}});
        ltsy::NdSequent<std::set> seq2 ({{p_conn_q},{q}});
        std::vector<ltsy::NdSequent<std::set>> sequents = {seq1, seq2};

        ltsy::CognitiveAttitude a {"A", {1}};
        ltsy::CognitiveAttitude r {"R", {0}};

        ltsy::NdSequentTruthTableDeterminizer determinizer {2, {*p, *q}, *conn, {a, r}};

        determinizer.determine(sequents);

        auto table = determinizer.table();

        std::cout << table << std::endl;
    }

    TEST(Determinants, TopRemoval) {
        auto p = std::make_shared<ltsy::Prop>("p");
        auto q = std::make_shared<ltsy::Prop>("q");
        auto conn = std::make_shared<ltsy::Connective>("&", 2);
        auto p_conn_q = std::make_shared<ltsy::Compound>(conn, std::vector<std::shared_ptr<ltsy::Formula>>{p, q});

        ltsy::NdSequent<std::set> seq1 ({{},{},{p},{p}});
        ltsy::NdSequent<std::set> seq2 ({{},{},{q},{q}});
        std::vector<ltsy::NdSequent<std::set>> sequents = {seq1, seq2};

        ltsy::CognitiveAttitude a {"A", {2, 3}};
        ltsy::CognitiveAttitude na {"NA", {0, 1}};
        ltsy::CognitiveAttitude nr {"NR", {1, 3}};
        ltsy::CognitiveAttitude r {"R", {0, 2}};

        ltsy::NdSequentTruthTableDeterminizer determinizer {4, {*p, *q}, *conn, {nr, na, r, a}};

        determinizer.determine(sequents);

        auto table = determinizer.table();

        std::cout << table << std::endl;
    }


    TEST(Determinants, BillaticeJudgements) {
        ltsy::CognitiveAttitude a {"A", {2, 3}};
        ltsy::CognitiveAttitude na {"NA", {0, 1}};
        ltsy::CognitiveAttitude nr {"NR", {1, 3}};
        ltsy::CognitiveAttitude r {"R", {0, 2}};
        auto billatice = ltsy::BillaticeJudgementValueCorrespondence(
           {
            {0, {{1, r}, {2, a}}},
            {1, {{0, r}, {3, na}}},
            {2, {{0, a}, {3, nr}}},
            {3, {{1, na}, {2, nr}}}
           }
        );
        std::cout << billatice.judgements_from_value(0) << std::endl;
        std::cout << billatice.judgements_from_value(1) << std::endl;
        std::cout << billatice.judgements_from_value(2) << std::endl;
        std::cout << billatice.judgements_from_value(3) << std::endl;
        std::cout << billatice.value_from_judgements({a, r}) << std::endl;
        std::cout << billatice.value_from_judgements({na, r}) << std::endl;
        std::cout << billatice.value_from_judgements({a, nr}) << std::endl;
        std::cout << billatice.value_from_judgements({na, nr}) << std::endl;
        std::cout << billatice.judgements_from_set_values({}) << std::endl;
        std::cout << billatice.judgements_from_set_values({0}) << std::endl;
        std::cout << billatice.judgements_from_set_values({1}) << std::endl;
        std::cout << billatice.judgements_from_set_values({2}) << std::endl;
        std::cout << billatice.judgements_from_set_values({3}) << std::endl;
        std::cout << billatice.judgements_from_set_values({0,1}) << std::endl;
        std::cout << billatice.judgements_from_set_values({0,2}) << std::endl;
        std::cout << billatice.judgements_from_set_values({0,3}) << std::endl;
        std::cout << billatice.judgements_from_set_values({1,2}) << std::endl;
        std::cout << billatice.judgements_from_set_values({1,3}) << std::endl;
        std::cout << billatice.judgements_from_set_values({2,3}) << std::endl;
        std::cout << billatice.judgements_from_set_values({0,1,2}) << std::endl;
        std::cout << billatice.judgements_from_set_values({0,1,3}) << std::endl;
        std::cout << billatice.judgements_from_set_values({1,2,3}) << std::endl;
        std::cout << billatice.judgements_from_set_values({0,2,3}) << std::endl;
        std::cout << billatice.judgements_from_set_values({0,1,2,3}) << std::endl;
    }

    TEST(Determinants, SequentsFromTruthTable) {
        std::set<int> all_values = {0,1,2,3};
        ltsy::CognitiveAttitude a {"A", {0, 2}};
        ltsy::CognitiveAttitude na {"NA", {1, 3}};
        ltsy::CognitiveAttitude nr {"NR", {2, 3}};
        ltsy::CognitiveAttitude r {"R", {0, 1}};
        auto billatice = std::make_shared<ltsy::BillaticeJudgementValueCorrespondence>(
           std::map<int, std::vector<std::pair<int, ltsy::CognitiveAttitude>>>{
            {0, {{1, r}, {2, a}}},
            {1, {{0, r}, {3, na}}},
            {2, {{0, a}, {3, nr}}},
            {3, {{1, na}, {2, nr}}}
           }
        );

        auto p = std::make_shared<ltsy::Prop>("p");
        auto q = std::make_shared<ltsy::Prop>("q");
        auto conn = std::make_shared<ltsy::Connective>("&", 2);
        auto p_conn_q = std::make_shared<ltsy::Compound>(conn, std::vector<std::shared_ptr<ltsy::Formula>>{p, q});

        ltsy::NdSequentsFromTruthTable axiomatizer (all_values, *conn, {p, q}, p_conn_q, {nr, na, a, r}, billatice);

        auto tt_nondet = ltsy::TruthTable<std::set<int>> {4,
            {
                {{0, 0},{0}},
                {{0, 1},{1}},
                {{0, 2},{0}},
                {{0, 3},{1}},
                {{1, 0},{1}},
                {{1, 1},{1}},
                {{1, 2},{1}},
                {{1, 3},{1}},
                {{2, 0},{0}},
                {{2, 1},{1}},
                {{2, 2},{2}},
                {{2, 3},{3}},
                {{3, 0},{1}},
                {{3, 1},{1}},
                {{3, 2},{3}},
                {{3, 3},{3}},
            }
        };

        auto bsequents_for_conj_fde = axiomatizer.axiomatize(tt_nondet);
        std::cout << bsequents_for_conj_fde;
    
    }


    TEST(Determinants, SequentsFromTruthTableFourLattice) {
        ltsy::FourBillatice billatice; 
    }

};
