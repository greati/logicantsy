#ifndef __SYMM_DUALIZATION__
#define __SYMM_DUALIZATION__

#include "apps/pnm-axiomatization/multipleconclusion.h"

namespace ltsy {


    /**
     * 0 -> f
     * 1 -> n
     * 2 -> b
     * 3 -> t
     */
    class SymmetricalCalculiDualization {

        public:

            struct Movement {
                std::string name = "?";
                std::vector<std::pair<int,int>> movement;
                Movement() {}
                Movement(const decltype(name)& _name, const decltype(movement)& _movement) 
                : name {_name}, movement {_movement} {}
            };

            struct Transformation {
                std::string name = "?";
                std::vector<Movement> movements;
                Transformation() {}
                Transformation(const decltype(name)& _name, const decltype(movements)& _movements)
                    : name {_name}, movements {_movements} {}
            };

        private:

            std::map<std::string, Transformation> _transformations;
            std::map<std::string, Movement> _movements;
            std::shared_ptr<GenMatrix> _base_matrix;
            FmlaSet _discriminant_set {std::make_shared<Prop>("p")};
            Discriminator _discriminant {
                {
                    {0, {{},_discriminant_set, _discriminant_set,{}}},
                    {1, {{},{_discriminant_set},{},{_discriminant_set}}},
                    {2, {{_discriminant_set},{},{_discriminant_set},{}}},
                    {3, {{_discriminant_set},{},{},{_discriminant_set}}},
                }
            };
        public:
            SymmetricalCalculiDualization(
                const std::vector<Movement>& movements,
                const std::vector<Transformation>& transformations) {
                for (const auto& m : movements)
                    _movements[m.name] = m;
                for (const auto& t : transformations)
                    _transformations[t.name] = t;
                _base_matrix = std::make_shared<GenMatrix>(
                    std::set<int>{0,1,2,3},
                    std::vector<std::set<int>>{{2,3},{2,0}}
                );
            };

            std::vector<NDTruthTable> create_oppositions(std::shared_ptr<NDTruthTable> tt) const {
                auto sig = std::make_shared<Signature>();
                sig->add(std::make_shared<Connective>(tt->name(), tt->arity()));
                _base_matrix->set_signature(sig);
                auto interp = std::make_shared<TruthInterp<std::set<int>>>((*sig)[tt->name()], tt);
                auto sigtint = std::make_shared<SignatureTruthInterp<std::set<int>>>(sig);
                _base_matrix->set_interpretation(sigtint);
                PNMMultipleConclusionAxiomatizer axiomatizer {_discriminant, _base_matrix, {{1, 0, 3, 2}}, {{1, 0}, {3, 2}}};
                auto original_calculus = 
                   axiomatizer.make_sigma_rules(interp);
                original_calculus = axiomatizer.remove_overlaps(original_calculus);
                axiomatizer.remove_dilutions(original_calculus);
                original_calculus = axiomatizer.simplify_by_cut2(original_calculus);
                axiomatizer.remove_dilutions(original_calculus);
                for (const auto& t : _transformations) {
                    // perform transformation
                }
                return {};
            } 

    };


};

#endif