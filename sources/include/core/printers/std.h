#ifndef __STD_PRINTER__
#define __STD_PRINTER__

#include "core/printers/base.h"

namespace ltsy {
    
    /* Print to a standard format.
     * */
    class StdPrinter : public Printer {
        public:

            StdPrinter() : Printer{} {}

            StdPrinter(const decltype(_translation)& translation) :
                Printer {translation} {}

            std::string print(const Formula& fmla) const override {}
            std::string print(const NdSequent<std::set>& sequent) const override {}

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
    };

};

#endif
