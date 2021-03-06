#ifndef __COMBINATIONS__
#define __COMBINATIONS__

#include "external/discreture.hpp"

namespace ltsy {

    class CombinationGenerator {
    
        protected:
            int _n;
            int _k = 0;
            std::shared_ptr<std::vector<int>> _current;
            bool _strict_k = false;

        public:

            CombinationGenerator(int n) : _n {n} {/* empty */}
            CombinationGenerator(int n, int k) : _n {n}, _k {k} {/* empty */}
           
            virtual decltype(_current) first() = 0;
            virtual decltype(_current) next() = 0;
            virtual void reset() = 0;
            virtual bool has_next() = 0;
    };

    class DiscretureCombinationGenerator : public CombinationGenerator {

        private:

            discreture::Combinations<int, std::vector<int>> _combinator {0, 0};
            int _current_k = 0;
            decltype(_combinator)::iterator _current_it;
    
        public:

            DiscretureCombinationGenerator(int n) : CombinationGenerator{n} {/* empty */
                reset(); 
            }
            DiscretureCombinationGenerator(int n, int k) : CombinationGenerator{n, k} {/* empty */
                _strict_k = true;
                reset();
            }
            
            decltype(_current) first() override {
                return std::make_shared<std::vector<int>>(_k, 0);
            }

            void reset() {
                _combinator = decltype(_combinator) {_n, _k};
                _current_it = _combinator.begin();
                _current_k = _k;
            }

            decltype(_current) next() override {
                if (has_next()) {
                    _current = std::make_shared<std::vector<int>>(*_current_it);
                    _current_it++;
                    if (_current_it == _combinator.end()) {
                        _current_k++;
                        if (has_next()) {
                            _combinator = decltype(_combinator) {_n, _current_k};
                            _current_it = _combinator.begin();
                        }
                    }
                    return _current;
                } else throw std::logic_error("no next combination available");
            }

            bool has_next() override {
                if (not _strict_k)
                    return this->_current_k <= _n;
                else
                    return this->_current_k == _k;
            }

    };

};

#endif
