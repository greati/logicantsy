#ifndef __STD_PRINTER__
#define __STD_PRINTER__

#include "core/printers/base.h"
#include "core/syntax.h"

namespace ltsy {
    
    /* Print to a standard format.
     * */
    class StdPrinter : public Printer {
        public:

            StdPrinter() : Printer{} {}

            StdPrinter(const decltype(_translation)& translation) :
                Printer {translation} {}

            std::string print(std::shared_ptr<Formula> fmla) const override {
                FormulaPrinter printer {_translation};
                fmla->accept(printer);
                return printer.get_string();
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

            std::string print(const Determinant<std::set<int>>& det) const override {
                std::stringstream ss;
                auto args = det.get_args();
                for (auto it = args.begin(); it != args.end(); ++it) {
                    ss << std::setw(5) << det.get_value_name(*it);
                    if (std::next(it) != args.cend())
                        ss << " ";
                }
                ss << " -> ";
                auto imgs = det.get_last();
                for (const auto& img : imgs)
                    ss << det.get_value_name(img) << " ";
                return ss.str();
            }

            std::string print(const NDTruthTable& tt) const override {
                std::stringstream ss;
                auto dets = tt.get_determinants();
                ss << tt.name() << std::endl;
                for (auto it = dets.begin(); it != dets.end(); ++it) {
                    ss << print(*it);
                    if (std::next(it) != dets.end())
                        ss << std::endl;    
                }
                return ss.str(); 
            }

            std::string print(const MultipleConclusionRule& mcrule) const {}
            std::string print(const MultipleConclusionCalculus& mcrule) const {}
    };

};

#endif
