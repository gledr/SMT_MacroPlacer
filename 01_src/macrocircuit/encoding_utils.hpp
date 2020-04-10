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

#include <z3++.h>

namespace Placer {

/**
 * @class EncodingUtils
 * @brief Class offering inline functions for abstracting Z3 encoding
 */
class EncodingUtils {
public:

    EncodingUtils(z3::context * z3_ctx)
    {
        assert (z3_ctx != nullptr);
        m_z3_ctx = z3_ctx;
    }

    inline z3::expr add(z3::expr a, z3::expr b)
    {
        return a + b;
    }

    inline z3::expr sub(z3::expr a, z3::expr b)
    {
        return a - b;
    }

   inline z3::expr ge(z3::expr a, z3::expr b)
    {
        return a >= b;
    }

    inline z3::expr le(z3::expr a, z3::expr b)
    {
        return a <= b;
    }

    inline z3::expr gt(z3::expr a, z3::expr b)
    {
        return a > b;
    }

    inline z3::expr lt(z3::expr a, z3::expr b)
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

private:
    z3::context* m_z3_ctx;
};

}

#endif /* ENCODINGUTILS_HPP */
