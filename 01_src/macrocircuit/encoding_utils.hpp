//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : encoding_utils.hpp
//
// Date         : 08. April 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : SMT Encoding Utilities
//==================================================================
#ifndef ENCODINGUTILS_HPP
#define ENCODINGUTILS_HPP

#include <object.hpp>

#include <z3++.h>

namespace Placer {

/**
 * @class EncodingUtils
 * @brief Class offering inline functions for abstracting Z3 encoding
 */
class EncodingUtils: public virtual Object {
public:

    EncodingUtils():
        Object()
    {
    }

    inline z3::expr add(z3::expr a, z3::expr b)
    {
        z3::expr ret_val(m_z3_ctx);

        if (this->get_logic() == eInt){
            ret_val = a + b;
        } else if (this->get_logic() == eBitVector){
            this->store_constraint(z3::bvadd_no_overflow(a, b, false) == this->get_flag(true));
            //this->store_constraint(z3::bvadd_no_underflow(a,b) == this->get_flag(true));
            ret_val = a + b;
            
        } else {
            assert (0);
        }

        return ret_val;
    }

    inline z3::expr sub(z3::expr const & a, z3::expr const & b)
    {
        z3::expr ret_val(m_z3_ctx);

        if (this->get_logic() == eInt){
            ret_val = a - b;
        } else if (this->get_logic() == eBitVector){
            //this->store_constraint(z3::bvsub_no_overflow(a, b) == this->get_flag(true));
            this->store_constraint(z3::bvsub_no_underflow(a,b, false) == this->get_flag(true));
            ret_val = a - b;
        }
        return ret_val;
    }

   inline z3::expr ge(z3::expr const & a, z3::expr const & b)
    {
        if (this->get_logic() == eInt){
            return a >= b;
        } else {
            return z3::uge(a ,b);
        }
    }

    inline z3::expr le(z3::expr const & a, z3::expr const & b)
    {
        if (this->get_logic() == eInt){
            return a <= b;
        } else {
            return z3::ule(a, b);
        }
    }

    inline z3::expr gt(z3::expr const &a, z3::expr const & b)
    {
        return a > b;
    }

    inline z3::expr lt(z3::expr const & a, z3::expr const & b)
    {
        return a < b;
    }

    inline std::string get_version() 
    {
        return Z3_get_full_version();
    }

    inline z3::expr mk_sum(z3::expr_vector& clauses)
    {
        try {
            if(clauses.size() == 0){
                assert (0);
            } else if (clauses.size() == 1){
                return clauses[0];
            } else {
                z3::expr sum = clauses[0];

                for(size_t i = 1; i < clauses.size(); ++i){
                    sum = sum + clauses[i];
                }
                return sum;
            }
        } catch (z3::exception const & exp){
            std::cout << exp.msg() << std::endl;
            assert (0);
        }
    }
    
    inline z3::expr get_constant(std::string const & id)
    {
        z3::expr ret_val (m_z3_ctx);

        if (this->get_logic() == eInt){
            ret_val = m_z3_ctx.int_const(id.c_str());
        } else if (this->get_logic() == eBitVector){
            ret_val = m_z3_ctx.bv_const(id.c_str(), 16);
        } else {
            assert (0);
        }
        
        return ret_val;
    }

    inline z3::expr get_value(size_t const value)
    {
        z3::expr ret_val(m_z3_ctx);

        if (this->get_logic() == eInt){
            ret_val = m_z3_ctx.int_val(value);
        } else if (this->get_logic() == eBitVector){
            ret_val = m_z3_ctx.bv_val(value, 16);
        } else {
            assert (0);
        }

        return ret_val;
    }

    inline z3::expr get_flag(bool const val)
    {
        return m_z3_ctx.bool_val(val);
    }
};

} /* namespace Placer */

#endif /* ENCODINGUTILS_HPP */
