#ifndef __TRUTH_TABLES__
#define __TRUTH_TABLES__

#include <cmath>
#include <iostream>
#include <vector>
#include <unordered_set>
#include "core/utils.h"
#include "core/common.h"
#include <functional>
#include <stdexcept>
#include <set>
#include <memory>
#include <map>

namespace ltsy {

    template<typename CellType> class TruthTable;

    /**
     * Represents a determinant. 
     *
     * Stores the arguments as an int, from which
     * we can compute the corresponding tuple
     * on demand.
     *
     * @author Vitor Greati
     * */
    template<typename CellType = int>
    class Determinant {
        private:
            int _nvalues;   /*< arg values take values from 0..(_nvalues-1)*/
            int _arity;     /*< arg size is _arity (so the determinant is _arity + 1) */
            std::pair<int, CellType> _data;
            std::map<int, std::string> _values_names;

        public:

            Determinant(int nvalues, int arity, int tuple_pos, CellType last, const decltype(_values_names)& values_names) : 
                _nvalues {nvalues}, _arity {arity}, _values_names {values_names}
            {
                _data = {tuple_pos, last};
            }

            Determinant(int nvalues, int arity, int tuple_pos, CellType last) : 
               Determinant{nvalues, arity, tuple_pos, last, {}} {} 

            Determinant(int nvalues, std::vector<int> args, CellType last, const decltype(_values_names)& values_names) 
                : _nvalues {nvalues}, _arity {static_cast<int>(args.size())}, _values_names {values_names} {
                _data = {utils::position_from_tuple(_nvalues, _arity, args), last};
            }

            Determinant(int nvalues, std::vector<int> args, CellType last) 
                : Determinant{nvalues, args, last, {}} {}

            inline CellType get_last() const { return _data.second; }

            inline CellType& get_last() { return _data.second; }

            inline void set_last(CellType& value) { _data.second = value; }

            inline std::vector<int> get_args() const { 
                return utils::tuple_from_position(_nvalues, _arity, _data.first); 
            }

            inline bool has_only_args(const std::set<int>& vs) const {
                const auto args = get_args();
                for (const auto& arg : args) {
                    if (vs.find(arg) == vs.end())
                        return false;
                }
                return true;
            }

            inline int get_args_pos() const { return _data.first; }

            bool operator==(const Determinant<CellType>& other) const {
                return _data == other._data;
            }

            bool operator<(const Determinant<CellType>& other) const;

            friend std::ostream& operator<<(std::ostream& os, const Determinant<CellType>& det) {
                auto args = det.get_args();
                os << std::string("< ");
                for (auto it = args.begin(); it != args.end(); ++it) {
                    os << (*it) << std::string(" ");
                }
                os << det.get_last();
                os << std::string(" >");
                return os;  
            }


            inline void set_values_names(const decltype(_values_names)& values_names) { _values_names = values_names; }

            inline std::string get_value_name(int value) const { 
                auto f = _values_names.find(value);
                if (f == _values_names.end())
                    return std::to_string(value);
                return f->second;
            }
    };



    /**
     * Represents an `NValues`-valued `Arity`-ary truth table. 
     * Values are taken to be 0,...,`NValues`-1.
     *
     * Only the images are stored. Each input
     * tuple is represented by a natural number
     * corresponding to the position of the tuple
     * in the lexicographical order.
     *
     * @author Vitor Greati
     * */
    template<typename CellType = int>
    class TruthTableBase {
        
        protected:
            
            std::string _name {"#"};  //<! a name to identify the truth-table
            int _nvalues;
            int _number_of_rows;
            int _arity = -1;
            std::vector<CellType> _images;

            std::map<int, std::string> _values_names;

            using TruthTableRow = std::pair<std::vector<int>, CellType>;

            static constexpr auto compute_number_of_rows = [](int nvalues, int arity) { return int(std::pow(nvalues, arity)); };

        public:

            TruthTableBase() {/*empty*/}

            TruthTableBase(int _nvalues, int _arity) : 
                _nvalues {_nvalues},
                _arity {_arity},
                _number_of_rows { this->compute_number_of_rows(_nvalues, _arity) },
                _images {std::vector<CellType>(_number_of_rows)}
                {/* empty */}

            /**
             * Build truth table giving its images array.
             * */
            TruthTableBase(int _nvalues, int _arity, const CellType& default_value) : 
                TruthTableBase {_nvalues, _arity}
                { this->_images = std::vector<CellType>(_number_of_rows, default_value); } 

            /**
             * Build truth table giving its images array.
             * */
            TruthTableBase(int _nvalues, int _arity, const decltype(_images)& _images) : 
                TruthTableBase {_nvalues, _arity}
                { this->_images = _images; } 

            /**
             * Build a truth table from its rows.
             * */
            TruthTableBase(int nvalues, const std::initializer_list<TruthTableRow>& rows);

            inline void set_name(const decltype(_name)& name) { _name = name; }
            inline decltype(_name) name() const { return _name; }

            /* Gives the image at the given position.
             * */
            inline CellType at(int i) const { return _images[i]; }

            inline void set(int i, const CellType& v) { _images[i] = v; }

            /* Return the image at a given input tuple.
             * */
            CellType at(const std::vector<int>& input) const;

            inline int arity() const { return _arity; }

            inline int nvalues() const { return _nvalues; }

            void update(const std::set<Determinant<CellType>>& dets) {
                for (auto e : dets)
                    set(e.get_args_pos(), e.get_last()); 
            }

            void update(const std::set<Determinant<CellType>>& dets,
                    std::function<CellType(const CellType&, const CellType&)> f) {
                for (auto e : dets) {
                    auto current_result = at(e.get_args_pos());
                    set(e.get_args_pos(), f(current_result, e.get_last()));
                } 
            }

            std::set<Determinant<CellType>> get_determinants() const {
                std::set<Determinant<CellType>> result;
                for (auto i {0}; i < _images.size(); ++i){
                    Determinant<CellType> d (_nvalues, _arity, i, at(i), _values_names);
                    result.insert(d);
                }
                return result;
            }

            friend std::ostream& operator<<(std::ostream& os, const TruthTableBase<CellType>& tt) {
                for (auto i = 0; i < tt._images.size(); ++i) {
                    auto row = utils::tuple_from_position(tt._nvalues, tt._arity, i);
                    for (auto it = row.cbegin(); it != row.cend(); it++) {
                        os << (*it);
                        if (std::next(it) != row.cend())
                            os << std::string(" ");
                    }
                    os << std::string(" -> ");
                    os << tt._images[i];
                    os << std::endl;
                }
                return os;  
            }

            inline void set_values_names(const decltype(_values_names)& values_names) { _values_names = values_names; }
            inline std::string get_value_name(int value) const { 
                auto f = _values_names.find(value);
                if (f == _values_names.end())
                    return std::to_string(value);
                return f->second;
            }

            std::stringstream print(std::function<void(std::stringstream&, const CellType&)> cell_printer) const;
            std::stringstream print(const std::map<int, std::string>& values_map) const;
            std::stringstream print() const;
    };

    template class TruthTableBase<std::set<int>>;
    template class TruthTableBase<int>;

    template<typename CellType = int>
    class TruthTable : public TruthTableBase<CellType> {
    
        using TruthTableRow = std::pair<std::vector<int>, CellType>;
        public:
            using TruthTableBase<CellType>::TruthTableBase;
    
    
    };

    template class TruthTable<int>;

    template<>
    class TruthTable<std::set<int>> : public TruthTableBase<std::set<int>> {
        public:
            using TruthTableBase<std::set<int>>::TruthTableBase;
            /* Return a set of all sets X = {x1,...,xn} s.t. *(x1,...,xn) = empty.
             * */
            std::set<std::set<int>> partial_inputs() const {
                std::set<std::set<int>> result;
                for (const auto& det : get_determinants()) {
                    if (det.get_last().empty()) {
                        auto args = det.get_args();
                        result.insert(std::set<int>{args.begin(), args.end()}); 
                    }
                } 
                return result;
            }   

            /* Check if a sub table is total.
             *
             * @return true if the sub table is total
             * */
            bool
            is_sub_table_total(const std::set<int>& subvalues) const {
                auto dets = this->get_determinants();
                for (const auto det : dets) {
                    if (not det.has_only_args(subvalues))
                        continue;
                    auto adjusted_det = det;
                    std::set<int> inters;
                    auto det_last = det.get_last();
                    std::set_intersection(det_last.begin(), det_last.end(),
                            subvalues.begin(), subvalues.end(), std::inserter(inters, inters.begin()));
                    if (inters.empty())
                        return false;
                }
                return true;
            }

            /* Return a set of determinants corresponding to the
             * given subvalues.
             *
             * @return the set of determinants after filtering and the set
             * of determinants after filtering having empty outputs
             * */
            std::pair<std::set<Determinant<std::set<int>>>, std::set<Determinant<std::set<int>>>>
            get_sub_table_determinants(const std::set<int>& subvalues) const {
                std::set<Determinant<std::set<int>>> result;
                std::set<Determinant<std::set<int>>> empty_dets;
                auto dets = this->get_determinants();
                for (const auto det : dets) {
                    if (not det.has_only_args(subvalues))
                        continue;
                    auto adjusted_det = det;
                    std::set<int> inters;
                    auto det_last = det.get_last();
                    std::set_intersection(det_last.begin(), det_last.end(),
                            inters.begin(), inters.end(), std::inserter(inters, inters.begin()));
                    adjusted_det.set_last(inters);
                    result.insert(adjusted_det);
                    if (inters.empty())
                        empty_dets.insert(adjusted_det);
                }
                return {result, empty_dets};
            }
    };

    using NDTruthTable = TruthTable<std::set<int>>;

    NDTruthTable generate_fully_nd_table(int nvalues, int arity);
    NDTruthTable generate_fully_partial_table(int nvalues, int arity);
        
    class TruthTableGenerator {
        
        private:
            int _current_index = 0;
            int _nvalues;
            int _arity;
            int _quantity;
            int _number_of_rows;
            std::shared_ptr<TruthTable<int>> _current;

        public:

            TruthTableGenerator() {}

            TruthTableGenerator(int nvalues, int arity) 
                : _nvalues {nvalues}, _arity {arity} {
                _number_of_rows = utils::compute_number_of_rows(nvalues, arity);
                _quantity = _arity > 0 ? utils::compute_number_of_functions(nvalues, arity) : _nvalues;
                reset();
            }

            decltype(_current) current() {
                return _current;
            }

            decltype(_current) next() {
                if (has_next()) {
                    auto images = utils::tuple_from_position(_nvalues, _number_of_rows, _current_index);
                    _current = std::make_shared<TruthTable<int>>(_nvalues, _arity, images);
                    ++_current_index;
                    return _current;
                } else {
                    throw std::logic_error("no truth table available to generate");
                }
            }

            void reset() {
                _current_index = 0;
                auto images = utils::tuple_from_position(_nvalues, _number_of_rows, _current_index);
                _current = std::make_shared<TruthTable<int>>(_nvalues, _arity, images);
            }

            bool has_next() {
                return _current_index < _quantity;
            }

    };
};

#endif
