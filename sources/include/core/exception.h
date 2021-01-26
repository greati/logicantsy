#ifndef __CORE_EXCEPTION__
#define __CORE_EXCEPTION__

#include "core/common.h"

namespace ltsy{

    const std::string NEGATIVE_ARITY_EXCEPTION = "provide an arity > 0";
    const std::string CONECTIVE_NOT_IN_SIGNATURE_EXCEPTION = "connective is not in signature";
    const std::string WRONG_NUMBER_ARGUMENTS_FMLA_EXCEPTION = "connective is not in signature";
    const std::string NULL_POINTER_TO_COMPOUND = "a null pointer was passed as a formula component";
    const std::string WRONG_TUPLE_POSITION_EXCEPTION = "invalid tuple position";
    const std::string WRONG_ARITY_INPUT_EXCEPTION = "invalid input tuple arity";
    const std::string INVALID_CONNECTIVE_INTERP_EXCEPTION = "invalid connective interpretation";
    const std::string INVALID_TRUTH_VALUE_EXCEPTION = "invalid truth-value";
    const std::string NULL_IN_CONNECTIVE_INTERP_EXCEPTION = "null value in connective interpretation";
    const std::string CONNECTIVE_ALREADY_INTERP_EXCEPTION = "connective already interpreted";


    class ConnectiveNotPresentException : public std::exception {
        Symbol _symbol;

        public:
            ConnectiveNotPresentException(Symbol symbol) {
                this->_symbol = symbol;
            }
        	const char * what () const throw () {
                return std::string("connective " + _symbol + " not present").c_str();
            }
    };

}

#endif
