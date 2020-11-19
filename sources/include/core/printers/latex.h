#ifndef __LATEX_PRINTER__
#define __LATEX_PRINTER__

#include "core/printers/base.h"

namespace ltsy {
    
    /* Print to a standard format.
     * */
    class LaTeXPrinter : public Printer {
        private:

            std::string inline_math(const std::string s) const {
                return "$" + s + "$";
            }

        public:

        LaTeXPrinter() : Printer{} {}

        LaTeXPrinter(const decltype(_translation)& translation) :
            Printer {translation} {}

        std::string print(const Formula& fmla) const override {}

        std::string print(const NdSequent<std::set>& sequent) const override {}

        std::string print(const NDTruthTable& tt) const override {
            std::stringstream ss;
            ss << "\\begin{table}";
            ss << "\\centering";
            ss << "\\begin{tabular}{";
            std::string header = "";
            for (auto i = 0; i < tt.arity(); ++i) {
               ss << "c|"; 
               header += "&";
            }
            ss << "c}" << std::endl;
            ss << "\\toprule" << std::endl;
            ss << header << inline_math(get_translation(tt.name(), tt.name())) << "\\\\" << std::endl;
            ss << "\\midrule" << std::endl;
            auto dets = tt.get_determinants();
            for (auto it = dets.begin(); it != dets.end(); ++it) {
                ss << print(*it) << "\\\\";
                if (std::next(it) != dets.end())
                    ss << std::endl;    
            }
            ss << std::endl;
            ss << "\\bottomrule" << std::endl;
            ss << "\\end{tabular}";
            ss << "\\end{table}";
            return ss.str();
        }

        std::string print(const Determinant<std::set<int>>& det) const override {
            std::stringstream ss;
            auto args = det.get_args();
            for (auto it = args.begin(); it != args.end(); ++it) {
                ss << std::setw(5) << inline_math(get_translation(det.get_value_name(*it), det.get_value_name(*it)));
                ss << "&";
            }
            auto imgs = det.get_last();
            ss << "\\{";
            for (auto it = imgs.begin(); it != imgs.end(); ++it) {
                ss << inline_math(get_translation(det.get_value_name(*it), det.get_value_name(*it)));
                if (std::next(it) != imgs.end())
                    ss << ",";
            }
            ss << "\\}";
            return ss.str();
        }
    };

};

#endif
