#ifndef __CORE_EXCEPTION__
#define __CORE_EXCEPTION__

#include "core/common.h"

namespace ltsy{

    const std::string NEGATIVE_ARITY_EXCEPTION = "provide an arity > 0";
    const std::string CONECTIVE_NOT_IN_SIGNATURE_EXCEPTION = "connective is not in signature";
    const std::string WRONG_NUMBER_ARGUMENTS_FMLA_EXCEPTION = "connective is not in signature";

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
