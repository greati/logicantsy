#include "core/semantics/judgment_values_corr.h"

using namespace ltsy;

std::set<CognitiveAttitude> BillaticeJudgementValueCorrespondence::_compute_judgs_from_induced_subgraph(const std::set<int>& vals) {
    std::set<CognitiveAttitude> result;
    for (const auto& v : vals)
        for (auto [u, j] : this->_adjs[v])
            if (vals.find(u) != vals.end())
                result.insert(j);
    return result;
}

std::set<CognitiveAttitude> BillaticeJudgementValueCorrespondence::judgements_from_value(int v) {
    std::set<CognitiveAttitude> atts;
    for (const auto ar : _adjs[v])
        atts.insert(ar.second);
    return atts;
}

int BillaticeJudgementValueCorrespondence::value_from_judgements(const std::set<CognitiveAttitude>& judgs) {
    for (const auto& [v, adjlist] : _adjs) {
        if (judgements_from_value(v) == judgs)
           return v; 
    } 
    throw std::logic_error("no value corresponds to those judgements");
}

std::vector<std::set<CognitiveAttitude>> BillaticeJudgementValueCorrespondence::judgements_from_set_values(const std::set<int>& vals) {
    std::vector<std::set<CognitiveAttitude>> atts;
    auto vals_size = vals.size(); 
    // no values
    if (vals_size == 0)
        return atts;
    // induced subgraph
    std::set<CognitiveAttitude> inducedSubgraph = this->_compute_judgs_from_induced_subgraph(vals);;
    if (inducedSubgraph.size() == 0) {
        for (const auto& v : vals) {
            auto js = judgements_from_value(v);
            atts.push_back(js);
        }
    } else {
        for (const auto& a : inducedSubgraph)
            atts.push_back({a});
    }
    return atts;
}

std::shared_ptr<JudgementValueCorrespondence> JudgementValueCorrespondenceFactory::make_judgement_value_correspondence(
        const JudgementValueCorrespondenceType& type) {
    switch(type) {
        case JudgementValueCorrespondenceType::FOUR_BILLATICE:
            return nullptr; 
    }
}
