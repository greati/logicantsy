#ifndef __SYMM_DUALIZATION__
#define __SYMM_DUALIZATION__

#include "apps/pnm-axiomatization/multipleconclusion.h"
#include "tt_determination/ndsequents.h"

namespace ltsy {


    /**
     * 0 -> f
     * 1 -> n
     * 2 -> b
     * 3 -> t
     */
    class SymmetricalCalculiDualization {

        public:

            enum class WhatMoves {
                COMPONENTS,
                COMPOUND
            };

            struct Movement {
                std::string name = "?";
                WhatMoves what_moves;
                std::vector<std::pair<int,int>> movement;
                Movement() {}
                Movement(const decltype(name)& _name, WhatMoves _what_moves, const decltype(movement)& _movement) 
                : name {_name}, what_moves {_what_moves}, movement {_movement} {}
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
            std::vector<CognitiveAttitude> _judgements {
                {"Y", {2, 3}},
                {"NR", {1, 3}},
                {"R", {2, 0}},
                {"NY", {0, 1}},
            };
        public:
            SymmetricalCalculiDualization(
                const std::map<std::string, Transformation>& transformations) : _transformations {transformations} {
                _base_matrix = std::make_shared<GenMatrix>(
                    std::set<int>{0,1,2,3},
                    std::vector<std::set<int>>{{2,3},{2,0}}
                );
                _transformations["0"] = Transformation {"0", {}};
            };

            struct TransformationResult {
                NDTruthTable tt;
                MultipleConclusionCalculus calculus;
                Transformation transformation;
            };

            std::vector<TransformationResult> create_oppositions(std::shared_ptr<NDTruthTable> tt) const {
                auto fmla = tt->fmla();
                if (fmla == nullptr)
                    throw std::invalid_argument("No representant formula provided for truth table");
                // get connective
                auto connective = fmla->connective();
                if (connective == nullptr)
                    throw std::invalid_argument("No connective provided for truth table");
                // compose matrix
                auto sig = std::make_shared<Signature>();
                sig->add(std::make_shared<Connective>(tt->name(), tt->arity()));
                _base_matrix->set_signature(sig);
                auto interp = std::make_shared<TruthInterp<std::set<int>>>((*sig)[tt->name()], tt);
                auto sigtint = std::make_shared<SignatureTruthInterp<std::set<int>>>(sig);
                _base_matrix->set_interpretation(sigtint);
                // axiomatize input table
                PNMMultipleConclusionAxiomatizer axiomatizer {_discriminant, _base_matrix, 
			std::make_optional<std::vector<int>>({{0, 3, 2, 1}}), 
			std::make_optional<std::vector<std::pair<int,int>>>({{0, 3}, {2, 1}})
		};
                //props
                std::vector<Prop> props;
                auto props_pointers = axiomatizer.make_props(tt->arity());
                for (auto pp : props_pointers)
                    props.push_back(*(std::dynamic_pointer_cast<Prop>(pp)));
                // calculus
                auto original_calculus = 
                   axiomatizer.make_sigma_rules(interp);
                original_calculus = axiomatizer.remove_overlaps(original_calculus);
                axiomatizer.remove_dilutions(original_calculus);
                original_calculus = axiomatizer.simplify_by_cut2(original_calculus);
                axiomatizer.remove_dilutions(original_calculus);
                original_calculus = axiomatizer.remove_overlaps(original_calculus);
                auto calc = MultipleConclusionCalculus {std::vector<MultipleConclusionRule>(original_calculus.begin(),
                original_calculus.end())};
                calc.rename_rules();
                auto calcr = calc.rules();
                original_calculus = std::set<MultipleConclusionRule>(calcr.begin(), calcr.end());
                // start transformations
                std::vector<TransformationResult> transformation_results;
                for (const auto& [name, t] : _transformations) {
                    TransformationResult r;
                    // copy original rules
                    decltype(original_calculus) new_rules;
                    // execute movements
                    for (const auto& r : original_calculus) {
                        NdSequent<std::set> new_sequent = r.sequent();
                        for (const auto& m : t.movements) {
                            auto pred = [m](std::shared_ptr<Formula> fmla) -> bool {
                                if (m.what_moves == WhatMoves::COMPONENTS)
                                    return fmla->type() == Formula::FmlaType::PROP;
                                else
                                    return fmla->type() == Formula::FmlaType::COMPOUND;
                            };
                            new_sequent = new_sequent.transform(m.movement, pred);
                        }
                        MultipleConclusionRule new_rule {
                            r.name() + "(" + name + ")", 
                            new_sequent,
                            r.prem_conc_pos_corresp()
                        };
                        new_rules.insert(new_rule);
                    }
                    NdSequentTruthTableDeterminizer determinizer {static_cast<int>(_base_matrix->values().size()), props, *connective, _judgements, std::nullopt};
                    // collect all sequents from the new rules and determinize
                    std::vector<NdSequent<std::set>> sequents;
                    for (const auto& r : new_rules)
                        sequents.push_back(r.sequent());
                    determinizer.determine(sequents, tt->name(), tt->get_values_names());
                    // perform transformation
                    r.transformation = t;   
                    r.tt = determinizer.table();
                    r.tt.set_values_names(tt->get_values_names());
                    r.calculus = MultipleConclusionCalculus {
                        std::vector<MultipleConclusionRule>(new_rules.begin(), new_rules.end())
                    };
                    transformation_results.push_back(r);
                }
                return transformation_results;
            } 

    };


};

#endif
