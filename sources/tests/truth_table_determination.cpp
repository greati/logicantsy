#include "gtest/gtest.h"
#include "core/common.h"
#include "tt_determination/set_set.h"
#include "tt_determination/ndsequents.h"
#include "core/semantics/attitudes.h"

namespace {

    TEST(Determinants, TwoValuedDetermination) {
        auto p = std::make_shared<ltsy::Prop>("p");
        auto q = std::make_shared<ltsy::Prop>("q");
        auto conn = std::make_shared<ltsy::Connective>("&", 2);
        auto p_conn_q = std::make_shared<ltsy::Compound>(conn, std::vector<std::shared_ptr<ltsy::Formula>>{p, q});
        std::unordered_set<int> leftCounterModel {1};
        std::unordered_set<int> rightCounterModel {0};
        
        // Land
        ltsy::SetSetSequent landI {{p, q}, {p_conn_q}}; 
        ltsy::SetSetSequent landE1 {{p_conn_q}, {p}}; 
        ltsy::SetSetSequent landE2 {{p_conn_q}, {q}}; 

        ltsy::SetSetTruthTableDetermination determinator {2, {*p, *q}, *conn};
        determinator.determine({landI, landE1, landE2}, leftCounterModel, rightCounterModel);

        auto land_table = determinator.table();
        std::cout << land_table << std::endl;
        
        // Lor
        ltsy::SetSetSequent lorE {{p_conn_q}, {p, q}}; 
        ltsy::SetSetSequent lorI1 {{p}, {p_conn_q}}; 
        ltsy::SetSetSequent lorI2 {{q}, {p_conn_q}}; 

        ltsy::SetSetTruthTableDetermination lordeterminator {2, {*p, *q}, *conn};
        lordeterminator.determine({lorE, lorI1, lorI2}, leftCounterModel, rightCounterModel);

        auto lor_table = lordeterminator.table();
        std::cout << lor_table << std::endl;

        // Impl
        ltsy::SetSetSequent impE {{p_conn_q, p}, {q}}; 
        ltsy::SetSetSequent impH1 {{}, {p_conn_q, p}}; 
        ltsy::SetSetSequent impH2 {{q}, {p_conn_q}}; 

        ltsy::SetSetTruthTableDetermination impdeterminator {2, {*p, *q}, *conn};
        impdeterminator.determine({impE, impH1, impH2}, leftCounterModel, rightCounterModel);

        auto imp_table = impdeterminator.table();
        std::cout << imp_table << std::endl;
    }

    TEST(Determinants, FourValuedDetermination) {
        auto p = std::make_shared<ltsy::Prop>("p");
        auto q = std::make_shared<ltsy::Prop>("q");
        auto conn = std::make_shared<ltsy::Connective>("&", 2);
        auto p_conn_q = std::make_shared<ltsy::Compound>(conn, std::vector<std::shared_ptr<ltsy::Formula>>{p, q});
        
        // Land
        ltsy::SetSetSequent landI {{p, q}, {p_conn_q}}; 
        ltsy::SetSetSequent landE1 {{p_conn_q}, {p}}; 
        ltsy::SetSetSequent landE2 {{p_conn_q}, {q}}; 
        std::vector<ltsy::SetSetSequent> sequents = {landI, landE1, landE2};

        ltsy::SetSetTruthTableDetermination determinator {4, {*p, *q}, *conn};

        std::unordered_set<int> acceptValues {2, 3};
        std::unordered_set<int> nonAcceptValues {0, 1};
        determinator.determine(sequents, acceptValues, nonAcceptValues);

        std::unordered_set<int> nonRejectValues {1, 3};
        std::unordered_set<int> rejectValues {0, 2};
        determinator.determine(sequents, nonRejectValues, rejectValues);

        auto land_table = determinator.table();
        std::cout << land_table << std::endl;   
    }


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

};
