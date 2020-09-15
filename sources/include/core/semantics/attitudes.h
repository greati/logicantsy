#ifndef __ATTITUDES__
#define __ATTITUDES__

#include <set>
#include "core/common.h"
#include <iostream>

namespace ltsy {
    struct CognitiveAttitude {
        Symbol symbol;
        std::set<int> values;

        CognitiveAttitude(const Symbol& symbol, const decltype(values)& values) : values {values}, symbol {symbol} {/*empty*/}

        bool operator<(const CognitiveAttitude& other) const {
            return this->symbol < other.symbol;
        }
        bool operator==(const CognitiveAttitude& other) const {
            return this->symbol == other.symbol;
        }
    };

    /**
     * General correspondence between values and judgements.
     * */
    class JudgementValueCorrespondence {
        public:
            virtual std::set<CognitiveAttitude> judgements_from_value(int) = 0;
            virtual int value_from_judgements(const std::set<CognitiveAttitude>&) = 0;

            /**
             * Produces a set of lists of judgements. Each list
             * represent a configuration of a counter-model
             * in terms of judgements.
             * */
            virtual std::vector<std::set<CognitiveAttitude>> 
                judgements_from_set_values(const std::set<int>&) = 0;
    };

    /**
     * Gives a judgement-value correspondence given
     * when they are represented in a billatice,
     * the judgements occuppying the edges of the
     * underlying graph.
     *
     * For example:
     *
     * {
     *    {
     *     0, {{1, N}, {2, Y}},
     *     1, {{0, N}, {3, nY}},
     *     2, {{0, Y}, {3, nN}},
     *     3, {{1, nY}, {2, nN}}
     *    }
     * }
     *
     * */
    class BillaticeJudgementValueCorrespondence : public JudgementValueCorrespondence {

        private:

            std::map<int, std::vector<std::pair<int, CognitiveAttitude>>> _adjs;

            std::set<CognitiveAttitude> _compute_judgs_from_induced_subgraph(const std::set<int>& vals) {
                std::set<CognitiveAttitude> result;
                for (const auto& v : vals)
                    for (const auto& [u, j] : _adjs[v])
                        if (vals.find(u) != vals.end())
                            result.insert(j);
                return result;
            }

        public:

            BillaticeJudgementValueCorrespondence(const decltype(_adjs)& adjs) : _adjs {adjs} {}

            std::set<CognitiveAttitude> judgements_from_value(int v) override {
                std::set<CognitiveAttitude> atts;
                for (auto& ar : _adjs[v])
                    atts.insert(ar.second);
                return atts;
            }

            int value_from_judgements(const std::set<CognitiveAttitude>& judgs) override {
                for (const auto& [v, adjlist] : _adjs) {
                    if (judgements_from_value(v) == judgs)
                       return v; 
                } 
                throw std::logic_error("no value corresponds to those judgements");
            }

            std::vector<std::set<CognitiveAttitude>> judgements_from_set_values(const std::set<int>& vals) override {
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
    };
}

std::ostream& operator<<(std::ostream& os, const ltsy::CognitiveAttitude& ca) {
    os << ca.symbol;
    return os;
}
#endif

