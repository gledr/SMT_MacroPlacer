#ifndef ENCODINGUTILS_HPP
#define ENCODINGUTILS_HPP

#include <z3++.h>

namespace Placer {

class EncodingUtils {
public:

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
};

}

#endif /* ENCODINGUTILS_HPP */
