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
            std::set<int> _dvalues;
            Connective _connective;
            std::vector<Prop> _props;
            std::shared_ptr<Compound> _compound;

            enum class IndicatorValue {
                DESIGNATED,
                UNDESIGNATED,
                BOTH,
                NONE
            };

            using DeterminantIndicator = std::vector<std::variant<int, IndicatorValue>>;

            IndicatorValue infer_indicator_value(SetSetSequent& sequent, Formula& p) {
                if (sequent.in_left(p) and sequent.in_right(p))
                    return IndicatorValue::BOTH;
                else if (sequent.in_left(p))
                    return IndicatorValue::DESIGNATED;
                else if (sequent.in_right(p))
                    return IndicatorValue::UNDESIGNATED;
                else
                    return IndicatorValue::NONE;
            }

            DeterminantIndicator process(SetSetSequent& sequent) {
                DeterminantIndicator detindicator; 
                for (auto& p : _props)
                    detindicator.push_back(infer_indicator_value(sequent, p));
                detindicator.push_back(infer_indicator_value(sequent, *_compound));
            }

        public:

            SetSetTruthTableDetermination(int nvalues, 
                    decltype(_dvalues) dvalues, 
                    decltype(_props) props,
                    decltype(_connective) connective) 
                : _nvalues {nvalues}, _dvalues {dvalues}, _props {props}, _connective {connective} {
                std::vector<std::shared_ptr<Formula>> props_args;
                for (auto p : props)
                    props_args.push_back(std::dynamic_pointer_cast<Formula>(std::make_shared<Prop>(p)));
                _compound = std::make_shared<Compound>(std::make_shared<Connective>(_connective), props_args);
            }

            TruthTable<std::unordered_set<int>> determine(const std::set<SetSetSequent>& sequents) {
                std::vector<DeterminantIndicator> determinant_indicators;
                for (auto sequent : sequents)
                    determinant_indicators.push_back(process(sequent));
                TruthTable<std::unordered_set<int>> table = generate_fully_nd_table(_nvalues, _connective.arity());
                ////////// do the magic
                return table;
            }

    };
};

#endif
