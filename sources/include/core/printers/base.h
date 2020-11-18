#ifndef __BASE_PRINTER__
#define __BASE_PRINTER__

#include "core/common.h"

namespace ltsy {

    /* Interface for printer objects.
     * */
    class Printer {
        protected:
            std::map<std::string, std::string> _translation;
        public:

            enum class PrinterType : int {
                PLAIN,
                LATEX,
            };

            Printer() {}

            Printer(const decltype(_translation)& translation) :
                _translation {translation} {}

            /* Set the tex translation map.
             * */
            void set_translation(const decltype(_translation)& translation) {
                this->_translation = translation;
            }

            /* Get the translation of a symbol to a tex.
             * Asks the user to provide a default value if a translation
             * is not available.
             *
             * @param symb the symbol
             * @param def the default string value
             * @return the tex representation in string format
             * */
            std::string get_translation(const std::string& symb, const std::string def) const {
                auto f = this->_translation.find(symb);
                if (f == _translation.end())
                    return def;
                return f->second;
            }
        
            virtual std::string print(const Formula& fmla) const = 0;
            virtual std::string print(const NdSequent<std::set>& sequent) const = 0;
            virtual std::string print(const NDTruthTable& tt) const = 0;
            virtual std::string print(const Determinant<std::set<int>>& nddeterminant) const = 0;
    };
}

#endif
