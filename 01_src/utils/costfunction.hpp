#ifndef COSTFUNCTION_HPP
#define COSTFUNCTION_HPP

#include <object.hpp>

#include <vector>
#include <cmath>

namespace Placer {

class CostFunction: public virtual Object {
public:
    CostFunction();
    
    virtual ~CostFunction();
    
    void init_lookup_table(int const x, int const y);
    
    size_t quadratic_lut(size_t const x, size_t const y);
private:
    std::vector<std::vector<size_t>> m_lut;
};

}

#endif /* COSTFUNCTION_HPP */
