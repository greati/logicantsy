#ifndef __LATEX_PRINTER__
#define __LATEX_PRINTER__

#include "core/printers/base.h"
#include "core/syntax.h"

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

        std::string print(std::shared_ptr<Formula> fmla) const override {
            FormulaPrinter printer {_translation};
            fmla->accept(printer);
            return inline_math(printer.get_string());
        }

        std::vector<std::string> print(const NdSequent<std::set>& seq) const override {
            std::vector<std::string> result;
            for (auto i = 0; i < seq.dimension(); ++i) {
                std::stringstream os;
                const auto seq_fmlas = seq[i];
                for (auto it = seq_fmlas.begin(); it != seq_fmlas.end(); ++it) {
                    os << print(*it);
                    if (std::next(it) != seq_fmlas.end())
                        os << ", ";
                }
                result.push_back(os.str());
            }
            return result;
        }

        std::string print(const NDTruthTable& tt) const override {
            std::stringstream ss;
            ss << "\\begin{table}[H]";
            ss << "\\centering";
            ss << "\\begin{tabular}{";
            std::string header = "";
            for (auto i = 0; i < tt.arity(); ++i) {
               ss << "c|"; 
               header += "&";
            }
            ss << "c}" << std::endl;
            ss << "    \\toprule" << std::endl;
            ss << header << inline_math(get_translation(tt.name(), tt.name())) << "\\\\" << std::endl;
            ss << "    \\midrule" << std::endl;
            auto dets = tt.get_determinants();
            for (auto it = dets.begin(); it != dets.end(); ++it) {
                ss << "    " << print(*it) << "\\\\";
                if (std::next(it) != dets.end())
                    ss << std::endl;    
            }
            ss << std::endl;
            ss << "    \\bottomrule" << std::endl;
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

        std::string print(const MultipleConclusionRule& mcrule) const {
        }

        std::string print(const MultipleConclusionCalculus& mcrule) const {}
    };

};

#endif
