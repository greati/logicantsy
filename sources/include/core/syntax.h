#ifndef __SYNTAX__
#define __SYNTAX__

#include <string>
#include <memory>
#include "core/exception.h"
#include "core/common.h"

namespace ltsy {
    
    /**
     * Represents a connective.
     *
     * @author Vitor Greati
     * */
    class Connective {
        private:
            Symbol _symbol;       //< Connective symbol
            Arity _arity;         //< Connective arity
        public:
            Connective() {}
            Connective(const Symbol& _symbol, Arity _arity) : _symbol {_symbol} {
                if (_arity < 0)
                   throw std::invalid_argument(NEGATIVE_ARITY_EXCEPTION); 
                this->_arity = _arity;
            }
            
            inline Symbol symbol() const { return _symbol; }
            inline Arity arity() const { return _arity; }
    };

    /**
     * Signature supporting indexing by arity and connective.
     *
     * @author Vitor Greati
     * */
    class Signature {

        private:

            std::map<Symbol, Connective> _signature;

        public:

            Signature(const std::initializer_list<Connective> connectives) {
                for (const auto& c : connectives)
                    this->add(c);
            }

            void add(const Connective& connective) {
                _signature.insert({connective.symbol(), connective});
            }

            /**
             * Join two signatures, preserving the elements of the
             * left-hand side.
             * */
            void join(const Signature& signature) {
                _signature.insert(signature._signature.begin(), signature._signature.end());
            }

            std::map<Symbol, Connective> operator()(Arity arity) const {
                std::map<Symbol, Connective> connectives_by_arity;
                if (arity < 0)
                   throw std::invalid_argument(NEGATIVE_ARITY_EXCEPTION);
                for (auto& [key,value] : _signature)
                    if (value.arity() == arity)
                        connectives_by_arity.insert({value.symbol(), value});
                return connectives_by_arity;
            }

            Connective operator[](const Symbol& symbol) {
                auto it = _signature.find(symbol);
                if (it == _signature.end())
                    throw ltsy::ConnectiveNotPresentException(symbol);
                return it->second;
            }
    
    };

    class Formula {};

    class Prop : public Formula {
        private:
            Symbol symbol;
    };

    class Compound : public Formula {
        private:
            std::shared_ptr<Connective> _connective;
            std::vector<std::shared_ptr<Formula>> _components;
        public:
            Compound(const decltype(_connective)& _connective, const decltype(_components)& _components) {
                if (_components.size() != _connective->arity())
                    throw std::invalid_argument(WRONG_NUMBER_ARGUMENTS_FMLA_EXCEPTION);
                else {
                    this->_connective = _connective;
                    this->_components = _components;
                }
            }
    };


}

#endif
