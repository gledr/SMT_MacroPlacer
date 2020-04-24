#include "costfunction.hpp"

using namespace Placer;

CostFunction::CostFunction()
{
}

CostFunction::~CostFunction()
{
}

void CostFunction::init_lookup_table(int const x, int const y)
{
    for (size_t i = 0; i < x; ++i){
        std::vector<size_t> tmp;
        tmp.resize(y,0);
        
        m_lut.push_back(tmp);
    }
 
    // Quadratic CostFunction from Origin
    for (size_t i = 0; i < x; ++i){
        for(size_t j = 0; j < y; ++j){
           m_lut[i][j] = sqrt((i*i) + (j*j));;
        }
    }
}

size_t CostFunction::quadratic_lut(size_t const x, size_t const y)
{
    return m_lut[x][y];
}
