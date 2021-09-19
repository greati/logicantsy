#ifndef __CLONE_GEN__
#define __CLONE_GEN__

#include "core/semantics/truth_tables.h"
#include "spdlog/spdlog.h"

namespace ltsy {

    class InputFunctionsGenerator {
    
        private:

           std::set<NDTruthTable> _functions;
           int _arity;
           std::vector<decltype(_functions.begin())> _iterators;
           std::vector<NDTruthTable> _current;
           bool finished = false;
           bool first = true;

        public:

            InputFunctionsGenerator(int arity, const decltype(_functions)& functions)
            : _arity {arity}, _functions {functions} {
                if (arity < 0) throw std::invalid_argument("arity in input generator must be >= 1");
                reset();
            } 

            void reset() {
                if (_arity == 0) {
                    finished = true; first = false;
                } else {
                    finished = false;
                    first = true;
                    _iterators.clear();
                    for (int i = 0; i < _arity; ++i) {
                        _iterators.push_back(_functions.begin());
                        _current.push_back(*_iterators[i]);
                    }
                }
            }

            std::vector<NDTruthTable> next() {
                if (_arity == 0) { first = false; return _current; }
                if (first) { first = false; return _current; }
                if (has_next()) {
                    int i = _arity - 1;
                    while (i >= 0 and std::next(_iterators[i]) == _functions.end()) {
                        _iterators[i] = _functions.begin();
                        _current[i] = *_iterators[i];
                        --i;    
                    }
                    _iterators[i]++;
                    _current[i] = *_iterators[i];
                    return _current;
                } else throw std::logic_error("no more inputs to generate");
            }
            bool has_next() {
                if (_arity == 0) return false;
                if (first) return true;
                for (int i = 0; i < _arity; ++i)
                    if (std::next(_iterators[i]) != _functions.end())
                        return true;
                return false;
            }
    };

    class CloneGenerator {
        private:
            std::vector<NDTruthTable> _base;
            int _nvalues;

        public:

            CloneGenerator(int nvalues, const decltype(_base)& base) 
                : _nvalues {nvalues}, _base {base} {}

            std::set<NDTruthTable> make_projections(int arity,
                    const std::vector<std::shared_ptr<Formula>>& props) const {
               std::set<NDTruthTable> projections;
               for (int i = 0; i < arity; ++i) {
                     NDTruthTable proj {_nvalues, arity, props[i]};
                     for (auto det : proj.get_determinants()) {
                         auto inputs = det.get_args();
                         proj.set(det.get_args_pos(), {inputs[i]});
                     }
                     projections.insert(proj);
               }
               return projections;
            }

            /* Generate the arity-ary clone using the functions
             * in the base.
             *
             * @param arity the arity
             * @param the generated clone
             * */
            std::set<NDTruthTable> generate(int arity, 
                    const std::vector<std::shared_ptr<Formula>>& props,
                    std::optional<int> max_depth=std::nullopt,
                    std::optional<std::pair<std::function<bool(NDTruthTable)>, 
                                    int>> predicate = std::nullopt) const {
                if (arity < 0) 
                    throw std::invalid_argument("negative arity in clone generation");
                std::set<NDTruthTable> predicate_satisfied = {};
                std::set<NDTruthTable> previous = {};
                std::set<NDTruthTable> current = make_projections(arity, props);
                for (auto b : _base)
                    if (b.arity() == arity)
                        current.insert(b);
                if (predicate) {
                    for (const auto& c : current)
                        if (predicate->first(c)) predicate_satisfied.insert(c);
                    if (predicate_satisfied.size() >= predicate->second)
                        return predicate_satisfied;
                }
                int depth = 0;
                while (previous != current) {
                    depth += 1;
                    if (max_depth and depth > *max_depth)
                        break;
                    spdlog::info("Current size in clone generation: " + std::to_string(current.size()));
                    previous = current;
                    for (const auto& b : _base) {
                        auto arity_base = b.arity();
                        InputFunctionsGenerator gen {arity_base, current};
                        while (gen.has_next()) {
                            auto gs = gen.next();
                            try {
                                auto composition = b.compose(gs);
                                if (predicate) {
                                    if (predicate->first(composition)) {
                                        predicate_satisfied.insert(composition);
                                        if (predicate_satisfied.size() >= predicate->second)
                                            return predicate_satisfied;
                                    }
                                }
                                current.insert(composition);
                            } catch(std::invalid_argument){}
                        }
                    }
                }
                if (predicate)
                    return predicate_satisfied;
                return current;
            }
    
    };
};


#endif
