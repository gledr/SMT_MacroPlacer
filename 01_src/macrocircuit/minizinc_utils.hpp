//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : minizinc_utils.hpp
//
// Date         : 28. July 2020
// Compiler     : gcc version 10.1.0 (GCC)
// Copyright    : Johannes Kepler University
// Description  : Minizinc Encoding Utilities
//==================================================================
#ifndef MINIZINC_HPP
#define MINIZINC_HPP

#include <vector>
#include <string>

#include <exception.hpp>

namespace Placer {
    namespace mzn {

        inline std::string mk_and(std::vector<std::string> const & values);
        inline std::string mk_or(std::vector<std::string> const & values);
        inline std::string mk_eq(std::string const & a, std::string const & b);
        inline std::string mk_lt(std::string const & a, std::string const & b);
        inline std::string mk_gt(std::string const & a, std::string const & b);
        inline std::string mk_le(std::string const & a, std::string const & b);
        inline std::string mk_ge(std::string const & a, std::string const & b);
        inline std::string mk_add(std::string const & a, std::string const & b);
        inline std::string mk_sub(std::string const & a, std::string const & b);
        inline std::string ite(std::string const & cond, 
                               std::string const & a,
                               std::string const & b);
        
        /**
         * @brief Perform And Operation on Vector
         * 
         * @param values Input Values
         * @return std::string
         */
        inline std::string mk_and(std::vector<std::string> const & values)
        {
            std::string retval;

            if (values.size() == 0){
                throw Utils::PlacerException("Can not perform mk_and on empty vector!");
            } else if (values.size() == 1){
                retval = values[0];
            } else {
                std::string and_token = " /\\ ";
                retval + "( ";
                for (size_t i = 0; i < values.size() - 1; ++i){
                    retval + values[i] + and_token;
                }
                retval + values[values.size() - 1] + " )";
            }
            return retval;
        }

        /**
         * @brief Perform Or Operation on Vector
         * 
         * @param values Input Values
         * @return std::string
         */
        inline std::string mk_or(std::vector<std::string> const & values)
        {
            std::string retval;

            if (values.size() == 0){
                throw Utils::PlacerException("Can not perform mk_or on empty vector!");
            } else if (values.size() == 1){
                retval = values[0];
            } else {
                std::string or_token = " \\/ ";
                retval + "( ";
                for (size_t i = 0; i < values.size() - 1; ++i){
                    retval + values[i] + or_token;
                }
                retval + values[values.size() - 1] + " )";
            }
            return retval;
        }

        /**
         * @brief ITE Operator for Minizinc
         * 
         * @param cond Condition
         * @param a First Path
         * @param b Second Path
         * @return std::string
         */
        inline std::string ite(std::string const & cond, 
                               std::string const & a,
                               std::string const & b)
        {
            return "if " + cond + " then " + a + " else " + b + " ;";
        }

        /**
         * @brief Equal operator
         * 
         * @param a First Operand
         * @param b Second Operand
         * @return std::string
         */
        inline std::string mk_eq(std::string const & a, std::string const & b)
        {
            return "( " + a + " == " + b + " )";
        }

        /**
         * @brief .Minizinc Less than Operator
         * 
         * @param a First Operand
         * @param b Second Operand
         * @return std::string
         */
        inline std::string mk_lt(std::string const & a, std::string const & b)
        {
            return " ( " + a + " < " + b + " )";
        }

        /**
         * @brief Minizinc Greater Than Operator
         * 
         * @param a First Operand
         * @param b Second Operand
         * @return std::string
         */
        inline std::string mk_gt(std::string const & a, std::string const & b)
        {
            return " ( " + a + " > " + b + " )";
        }

        /**
         * @brief Minizinc Less Equal Operator
         * 
         * @param a First Operand
         * @param b Second Operand
         * @return std::string
         */
        inline std::string mk_le(std::string const & a, std::string const & b)
        {
            return " ( " + a + " <= " + b + " )";
        }

        /**
         * @brief Minizinc Greater Equal Operator
         * 
         * @param a First Operand
         * @param b Second Operand
         * @return std::string
         */
        inline std::string mk_ge(std::string const & a, std::string const & b)
        {
            return " ( " + a + " >= " + b + " )";
        }

        /**
         * @brief Minizinc Add Operator
         * 
         * @param a First Operand
         * @param b Second Operand
         * @return std::string
         */
        inline std::string mk_add(std::string const & a, std::string const & b)
        {
            return " ( " + a + " + " + b + " )";
        }
        /**
         * @brief Minizinc Sub Operator
         * 
         * @param a First Operand
         * @param b Second Operand
         * @return std::string
         */
        inline std::string mk_sub(std::string const & a, std::string const & b)
        {
            return " ( " + a + " - " + b + " )";
        }
        
    } /* namespace mzn */
} /* namespace Placer */

#endif /* MINIZINC_HPP */
