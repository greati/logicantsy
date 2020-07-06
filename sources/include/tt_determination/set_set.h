#ifndef __SETSET_TT_DET__
#define __SETSET_TT_DET__

#include "core/syntax.h"
#include "core/semantics/truth_tables.h"
#include "core/proof-theory/sequents.h"
#include <variant>

namespace ltsy {

    class SetSetTruthTableDetermination {

        private:
            int _nvalues;
            Connective _connective;
            std::vector<Prop> _props;
            std::shared_ptr<Compound> _compound;
            TruthTable<std::set<int>> _table;

            enum class IndicatorValue {
                NONE=0,
                LEFT_ONLY=1,
                RIGHT_ONLY=2,
                BOTH=3,
            };

            using DeterminantIndicator = std::vector<std::variant<int, IndicatorValue>>;

            IndicatorValue infer_indicator_value(SetSetSequent& sequent, Formula& p) {
                if (sequent.in_left(p) and sequent.in_right(p))
                    return IndicatorValue::BOTH;
                else if (sequent.in_left(p))
                    return IndicatorValue::LEFT_ONLY;
                else if (sequent.in_right(p))
                    return IndicatorValue::RIGHT_ONLY;
                else
                    return IndicatorValue::NONE;
            }

            DeterminantIndicator process(SetSetSequent& sequent) {
                DeterminantIndicator detindicator; 
                for (auto& p : _props)
                    detindicator.push_back(infer_indicator_value(sequent, p));
                detindicator.push_back(infer_indicator_value(sequent, *_compound));
                return detindicator;
            }

            void determinant_from_indicator(DeterminantIndicator& indicator,
                    std::set<int>& leftCounterModelValues,
                    std::set<int>& rightCounterModelValues) {
                auto det_set = _table.get_determinants();
                // keep only determinant that would give rise to a counter-model
                for (int i = 0; i < _connective.arity(); ++i) {
                    auto ind_value = std::get<IndicatorValue>(indicator[i]);
                    for (auto it = det_set.begin(); it != det_set.end();) {
                        auto det_args = (*it).get_args();
                        switch(ind_value) {
                            case IndicatorValue::LEFT_ONLY:
                                if (leftCounterModelValues.find(det_args[i]) == leftCounterModelValues.end())
                                    it = det_set.erase(it);
                                else
                                    it++;
                                break;
                            case IndicatorValue::RIGHT_ONLY:
                                if (rightCounterModelValues.find(det_args[i]) == rightCounterModelValues.end())
                                    it = det_set.erase(it);
                                else
                                    it++;
                                break;
                            case IndicatorValue::BOTH:
                                    it = det_set.erase(it);
                                break;
                            case IndicatorValue::NONE:
                                it++;
                                break;
                            default:
                                it++;
                        }
                    }
                }
                // for the candidates, operate on the result column
                decltype(det_set) det_final;
                for (auto e : det_set) {
                    auto det_result = e.get_last();
                    decltype(det_result) new_result;
                    switch(std::get<IndicatorValue>(indicator[indicator.size()-1])) {
                        case IndicatorValue::LEFT_ONLY:
                            // R \ Left
                            new_result = utils::set_difference(det_result, leftCounterModelValues);
                            break;
                        case IndicatorValue::RIGHT_ONLY:
                            // R \ Right
                            new_result = utils::set_difference(det_result, rightCounterModelValues);
                            break;
                        case IndicatorValue::BOTH:
                            // R \ (Left U Right)
                            new_result = utils::set_difference(det_result, leftCounterModelValues);
                            new_result = utils::set_difference(new_result, leftCounterModelValues);
                            break;
                        case IndicatorValue::NONE:
                            // R
                            break;
                    }
                    e.set_last(new_result);
                    det_final.insert(e);
                }
                _table.update(det_final);
            }   

        public:

            SetSetTruthTableDetermination(int nvalues, 
                    decltype(_props) props,
                    decltype(_connective) connective) 
                : _nvalues {nvalues}, _props {props}, _connective {connective} {
                std::vector<std::shared_ptr<Formula>> props_args;
                for (auto p : props)
                    props_args.push_back(std::dynamic_pointer_cast<Formula>(std::make_shared<Prop>(p)));
                _compound = std::make_shared<Compound>(std::make_shared<Connective>(_connective), props_args);
                _table = generate_fully_nd_table(_nvalues, _connective.arity());
            }

            void determine(const std::vector<SetSetSequent>& sequents,
                    std::set<int>& leftCounterModelValues,
                    std::set<int>& rightCounterModelValues) {
                for (auto sequent : sequents) {
                    auto indicator = process(sequent);
                    determinant_from_indicator(indicator, leftCounterModelValues, rightCounterModelValues);
                }
            }

            decltype(_table) table() const { return _table; }

    };
};

#endif
