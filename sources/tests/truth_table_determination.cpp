#include "gtest/gtest.h"
#include "core/common.h"
#include "tt_determination/ndsequents.h"
#include "core/semantics/attitudes.h"

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

};
