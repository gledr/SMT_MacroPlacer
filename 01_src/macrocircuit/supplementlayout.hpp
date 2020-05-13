#ifndef SUPPLEMENTLAYOUT_HPP
#define SUPPLEMENTLAYOUT_HPP

#include <string>
#include <iostream>

namespace Placer {

class SupplementLayout {
public:
    SupplementLayout(size_t const lx,
                     size_t const ly,
                     size_t const ux,
                     size_t const uy);

    virtual ~SupplementLayout();
    
    size_t get_lx() const;
    size_t get_ly() const;
    size_t get_ux() const;
    size_t get_uy() const;

private:
    size_t m_ly;
    size_t m_lx;
    size_t m_uy;
    size_t m_ux;
};

}

#endif /* SUPPLEMENTLAYOUT_HPP */
