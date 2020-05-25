//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : macro.cpp
//
// Date         : 22. December 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Macro Component
//==================================================================
#include "macro.hpp"

using namespace Placer;
using namespace Placer::Utils;

/**
 * @brief Constructor
 * 
 * @param name      Name of Macro Type
 * @param id        Unique Macro ID
 * @param width     Macro Width
 * @param height    Macro Height
 */
Macro::Macro(std::string const & name,
             std::string const & id,
             size_t const width,
             size_t const height):
    Component(),
    m_encode_pin_macro_frontier(m_encode->get_value(0)),
    m_encode_pins_not_overlapping(m_encode->get_value(0)),
    m_encode_pins_center_of_macro(m_encode->get_value(0)),
    m_encode_pins_relative_to_center(m_encode->get_value(0)),
    m_pin_constraints(m_encode->get_value(0))
{
    m_free = true;
    m_supplement = nullptr;
    m_name = name;
    m_id = id;
    m_lx = m_encode->get_constant(id + "_lx");
    m_ly = m_encode->get_constant(id + "_ly");
    m_width = m_encode->get_value(width);
    m_height = m_encode->get_value(height);
    //m_orientation = m_encode->get_value(eNorth);
    m_orientation = m_encode->get_constant(id + "_orientation");
    m_logger = Logger::getInstance();

    m_logger->add_free_macro(id, width, height);
}

/**
 * @brief Constructor
 * 
 * @param name          Name of Macro Type
 * @param id            Unique Macro ID
 * @param width         Macro Width
 * @param height        Macro Height
 * @param pos_lx        Macro Position X
 * @param pos_ly        Macro Position Y
 * @param orientation   Macro Orientation
 */
Macro::Macro(std::string const & name,
             std::string const & id,
             size_t const width,
             size_t const height,
             size_t const pos_lx,
             size_t const pos_ly,
             size_t const orientation):
    Component(),
    m_encode_pin_macro_frontier(m_encode->get_value(0)),
    m_encode_pins_not_overlapping(m_encode->get_value(0)),
    m_encode_pins_center_of_macro(m_encode->get_value(0)),
    m_pin_constraints(m_encode->get_value(0)),
    m_encode_pins_relative_to_center(m_encode->get_value(0))
{
    m_lx = m_encode->get_value(pos_lx);
    m_ly = m_encode->get_value(pos_ly);
    m_width = m_encode->get_value(width);
    m_height = m_encode->get_value(height);
    m_orientation = m_encode->get_value(orientation);
    m_name = name;
    m_id = id;
    m_free = false;
    m_supplement = nullptr;
    m_logger = Logger::getInstance();

    m_logger->add_fixed_macro(id);
}

/**
 * @brief  Destructor
 */
Macro::~Macro()
{
    for(auto itor: m_pins){
        delete itor.second; itor.second = nullptr;
    }

    m_supplement = nullptr;
    m_logger = nullptr;
}

/**
 * @brief Check Macro Position North Clause
 * 
 * @return z3::expr
 */
z3::expr Macro::is_N()
{
    return m_orientation == m_encode->get_value(eNorth);
}

/**
 * @brief Check West Position North Clause
 * 
 * @return z3::expr
 */
z3::expr Macro::is_W()
{
    return m_orientation == m_encode->get_value(eWest);
}

/**
 * @brief Check Macro Position South Clause
 * 
 * @return z3::expr
 */
z3::expr Macro::is_S()
{
    return m_orientation == m_encode->get_value(eSouth);
}

/**
 * @brief Check Macro Position East Clause
 * 
 * @return z3::expr
 */
z3::expr Macro::is_E()
{
    return m_orientation == m_encode->get_value(eEast);
}

/**
 * @brief 
 * 
 * @param supplement 
 */
void Macro::set_supplement(Supplement* supplement)
{
    nullpointer_check(supplement);
    
    m_supplement = supplement;
    this->handle_supplement();
}

/**
 * @brief 
 * 
 * @param pin 
 */
void Macro::add_pin(Pin *const pin)
{
    assert (pin != nullptr);
    
    m_pins[pin->get_id()] = pin;
}

/**
 * @brief 
 * 
 * @param id 
 * @return Placer::Pin*
 */
Pin* Macro::get_pin(std::string const & id)
{
    return m_pins[id];
}

/**
 * @brief 
 * 
 * @return std::vector< Placer::Pin* >
 */
std::vector<Pin*> Macro::get_pins()
{
    std::vector<Pin*> retval;
    std::transform(m_pins.begin(), m_pins.end(), std::back_inserter(retval),
         [](std::pair<std::string, Pin*> const & p) {
             return p.second; 
        });
    
    return retval;
}

/**
 * @brief Check if Macro is free o
 * 
 * @return bool
 */
bool Macro::is_free()
{
    return m_free;
}

/**
 * @brief 
 */
void Macro::handle_supplement()
{
    if(!m_supplement->has_macro(m_name)){
        return;
    }
    
    SupplementMacro* sm = m_supplement->get_macro(m_name);
    assert (sm != nullptr);
    
    for(auto pin: m_pins){
        if(sm->has_pin(pin.second->get_id())){
            SupplementPin* p = sm->get_pin(pin.second->get_id());
            assert (p != nullptr);
            
            if(p->has_bitwidth()){
                pin.second->set_bitwidth(p->get_bitwidth());
            }
            if(p->has_frequency()){
                pin.second->set_frequency(p->get_frequency());
            }
        }
    }
}

/**
 * @brief Get Macro Area
 * 
 * @return size_t
 */
size_t Macro::get_area()
{
    assert (m_width.is_numeral());
    assert (m_height.is_numeral());
    
    return m_width.get_numeral_uint() * m_height.get_numeral_uint();
}

/**
 * @brief Dump Macro Information to given stream
 * 
 * @param stream Stream to dump to
 */
void Macro::dump(std::ostream & stream)
{
    stream << std::string(30, '#') << std::endl;
    stream << "Name (" << m_name << ") Id (" << m_id << " )" << std::endl;
    stream << "Size: " << m_width << " x " << m_height << std::endl;
    for(auto itor: m_pins){
        itor.second->dump(stream);
    }
    stream << std::string(30, '#') << std::endl;
    stream << std::endl;
}

/**
 * @brief Encode Macro Pins
 */
void Macro::encode_pins()
{
    z3::expr_vector clauses(m_z3_ctx);
#if 0
    // Pins are at the center of the Macro
    this->encode_pins_center_of_macro();
    
    // Pins are at the macro edge non overlapping
    this->encode_pins_on_macro_frontier();
    this->encode_pins_non_overlapping();
#endif
    // Pins are positioned relative to the macro center
    this->encode_pins_relative_to_center();

    clauses.push_back(m_encode_pins_relative_to_center);
    
    m_pin_constraints =  z3::mk_and(clauses);
}

/**
 * @brief Encode Pins Relative to Center for Bookshelf Data
 */
void Macro::encode_pins_relative_to_center()
{
    z3::expr_vector clauses(m_z3_ctx);

    for (auto itor: m_pins){
        Pin* pin = itor.second;
        z3::expr relative_x = m_encode->get_value(pin->get_offset_x_percentage());
        z3::expr relative_y = m_encode->get_value(pin->get_offset_y_percentage());
        z3::expr offset_x = ((m_width/m_encode->get_value(100))*relative_x);
        z3::expr offset_y = ((m_height/m_encode->get_value(100))*relative_y);

///{{{  Case N
        z3::expr_vector case_n(m_z3_ctx);
        z3::expr center_n_x = m_lx + (m_width/2);
        z3::expr center_n_y = m_ly + (m_height/2);
        case_n.push_back(pin->get_pin_pos_x() == (center_n_x + offset_x));
        case_n.push_back(pin->get_pin_pos_y() == (center_n_y + offset_y));
///}}}
///{{{  Case W
        z3::expr_vector case_w(m_z3_ctx);
        z3::expr center_w_x = m_lx - (m_height/2);
        z3::expr center_w_y = m_ly + (m_width/2);
        case_w.push_back(pin->get_pin_pos_x() == (center_w_x + offset_x));
        case_w.push_back(pin->get_pin_pos_y() == (center_w_y + offset_y));
///}}}
///{{{  Case S
        z3::expr_vector case_s(m_z3_ctx);
        z3::expr center_s_n = m_lx - (m_width/2);
        z3::expr center_s_y = m_ly - (m_height/2);
        case_s.push_back(pin->get_pin_pos_x() == (center_s_n + offset_x));
        case_s.push_back(pin->get_pin_pos_y() == (center_s_y + offset_y));
///}}}
///{{{  Case E
        z3::expr_vector case_e(m_z3_ctx);
        z3::expr center_e_x = m_lx + (m_height/2);
        z3::expr center_e_y = m_ly - (m_width/2);
        case_e.push_back(pin->get_pin_pos_x() == (center_e_x + offset_x));
        case_e.push_back(pin->get_pin_pos_y() == (center_e_y + offset_y));
///}}}
        z3::expr clause = z3::ite(this->is_N(), z3::mk_and(case_n),
                          z3::ite(this->is_W(), z3::mk_and(case_w),
                          z3::ite(this->is_S(), z3::mk_and(case_s),
                          z3::ite(this->is_E(), z3::mk_and(case_e),
                                  m_z3_ctx.bool_val(false)))));

        clauses.push_back(clause);
    }
    m_encode_pins_relative_to_center = z3::mk_and(clauses);
}

/**
 * @brief Encode the Pins of a Macro to be located on its frontier
 * 
 * The Pins may be located on the UPPER, LEFT, LOWER or RIGHT plane
 */
void Macro::encode_pins_on_macro_frontier()
{
    try {
        z3::expr_vector clauses(m_z3_ctx);

        for(auto pin: m_pins){
            z3::expr x = pin.second->get_pin_pos_x();
            z3::expr y = pin.second->get_pin_pos_y();
                
///{{{      Case N
            z3::expr_vector case_n(m_z3_ctx);
{
            //LEFT PLANE
            z3::expr_vector case_n1(m_z3_ctx);
            case_n1.push_back(x == m_lx);
            case_n1.push_back(y > m_ly);
            case_n1.push_back(y < (m_ly + m_height));
            case_n.push_back(z3::mk_and(case_n1));

            //RIGHT PLANE
            z3::expr_vector case_n2(m_z3_ctx);
            case_n2.push_back(x == (m_lx + m_width));
            case_n2.push_back(y > m_ly);
            case_n2.push_back(y < (m_ly + m_height));
            case_n.push_back(z3::mk_and(case_n2));

            //LOWER PLANE
            z3::expr_vector case_n3(m_z3_ctx);
            case_n3.push_back(y == m_ly);
            case_n3.push_back(x > m_lx);
            case_n3.push_back(x < (m_lx + m_width));
            case_n.push_back(z3::mk_and(case_n3));

            //UPPER PLANE
            z3::expr_vector case_n4(m_z3_ctx);
            case_n4.push_back(y == (m_ly + m_height));
            case_n4.push_back(x > m_lx);
            case_n4.push_back(x < (m_lx + m_width));
            case_n.push_back(z3::mk_and(case_n4));
}
///}}}
///{{{      Case W
            z3::expr_vector case_w(m_z3_ctx);
{
            // Case 1 x = moveable, y = ly LEFT PLANE
            z3::expr_vector case_w1(m_z3_ctx);
            case_w1.push_back(y == (m_ly - m_width));
            case_w1.push_back(x < m_lx);
            case_w1.push_back(x > (m_lx - m_height));
            case_w.push_back(z3::mk_and(case_w1));

            // Case 2 x = moveable, y = uy RIGHT PLANE
            z3::expr_vector case_w2(m_z3_ctx);
            case_w2.push_back(y == m_ly);
            case_w2.push_back(x < m_lx);
            case_w2.push_back(x > (m_lx - m_height));
            case_w.push_back(z3::mk_and(case_w2));

            // Case 3 x = lx, y = moveable LOWER PLANE
            z3::expr_vector case_w3(m_z3_ctx);
            case_w3.push_back(x == m_lx);
            case_w3.push_back(y > m_ly);
            case_w3.push_back(y < (m_ly + m_width));
            case_w.push_back(z3::mk_and(case_w3));

            // Case 4 x = ux, y = moveable UPPER PLANE
            z3::expr_vector case_w4(m_z3_ctx);
            case_w4.push_back(x == (m_lx + m_height));
            case_w4.push_back(y > m_ly);
            case_w4.push_back(y < (m_ly + m_width));
            case_w.push_back(z3::mk_and(case_w4));
}
///}}}
///{{{      Case S
            z3::expr_vector case_s(m_z3_ctx);
{
            // Case 1 x = moveable, y = ly LEFT PLANE
            z3::expr_vector case_s1(m_z3_ctx);
            case_s1.push_back(y == (m_ly - m_height));
            case_s1.push_back(x < m_lx);
            case_s1.push_back(x > (m_lx - m_width));
            case_s.push_back(z3::mk_and(case_s1));

            // Case 2 x = moveable, y = uy RIGHT PLANE
            z3::expr_vector case_s2(m_z3_ctx);
            case_s2.push_back(y == (m_ly + m_height)); 
            case_s2.push_back(x < m_lx);
            case_s2.push_back(x > (m_lx - m_width));
            case_s.push_back(z3::mk_and(case_s2));

            // Case 3 x = lx, y = moveable LOWER PLANE
            z3::expr_vector case_s3(m_z3_ctx);
            case_s3.push_back(x == (m_lx - m_width));
            case_s3.push_back(y < m_ly);
            case_s3.push_back(y > (m_ly - m_height));
            case_s.push_back(z3::mk_and(case_s3));
            
            // Case 4 x = ux, y = moveable UPPER PLANE
            z3::expr_vector case_s4(m_z3_ctx);
            case_s4.push_back(x == m_lx);
            case_s4.push_back(y < m_ly);
            case_s4.push_back(y > (m_ly - m_height));
            case_s.push_back(z3::mk_and(case_s4));
}
///}}}
///{{{      Case E
            z3::expr_vector case_e(m_z3_ctx);
{
            // Case 1 x = moveable, y = ly LEFT PLANE
            z3::expr_vector case_e1(m_z3_ctx);
            case_e1.push_back(y == m_ly);
            case_e1.push_back(x > m_lx);
            case_e1.push_back(x < (m_lx + m_height));
            case_e.push_back(z3::mk_and(case_e1));

            // Case 2 x = moveable, y = uy RIGHT PLANE
            z3::expr_vector case_e2(m_z3_ctx);
            case_e2.push_back(y == (m_ly + m_width));
            case_e2.push_back(x > m_lx);
            case_e2.push_back(x < (m_lx + m_height));
            case_e.push_back(z3::mk_and(case_e2));

            // Case 3 x = lx, y = moveable LOWER PLANE
            z3::expr_vector case_e3(m_z3_ctx);
            case_e3.push_back(x == (m_lx - m_height));
            case_e3.push_back(y > m_ly);
            case_e3.push_back(y < (m_ly + m_width));
            case_e.push_back(z3::mk_and(case_e3));

            // Case 4 x = ux, y = moveable UPPER PLANE
            z3::expr_vector case_e4(m_z3_ctx);
            case_e4.push_back(x == (m_lx + m_height));
            case_e4.push_back(y > m_ly);
            case_e4.push_back(y < (m_ly + m_width));
            case_e.push_back(z3::mk_and(case_e4));
}
///}}}
///}}}
            z3::expr clause = z3::ite(this->is_N(), z3::mk_or(case_n),
                              z3::ite(this->is_W(), z3::mk_or(case_w),
                              z3::ite(this->is_S(), z3::mk_or(case_s),
                              z3::ite(this->is_E(), z3::mk_or(case_e), m_z3_ctx.bool_val(false)))));
            clauses.push_back(clause);
        }
        
        m_encode_pin_macro_frontier = z3::mk_and(clauses);
    } catch (z3::exception const & exp){
        std::cout << exp.msg() << std::endl;
        assert (0);
    }
}

/**
 * @brief Encode Macro Pins not overlapping
 */
void Macro::encode_pins_non_overlapping()
{
    z3::expr_vector clauses(m_z3_ctx);
    
    assert (0);
    
    m_encode_pins_not_overlapping = z3::mk_and(clauses);
}

/**
 * @brief Encode the Pin Position of a Macro to its center
 * 
 * Center Position may vary due to Integer rounding!
 */
void Macro::encode_pins_center_of_macro()
{
    z3::expr_vector clauses(m_z3_ctx);

    for(auto _pin: m_pins){
        Pin* pin = _pin.second;
            
///{{{  Case N
        z3::expr_vector case_n(m_z3_ctx);
        case_n.push_back(pin->get_pin_pos_x() == (m_lx + (m_width/2)));
        case_n.push_back(pin->get_pin_pos_y() == (m_ly + (m_height/2)));
///}}}
///{{{  Case W
        z3::expr_vector case_w(m_z3_ctx);
        case_w.push_back(pin->get_pin_pos_x() == (m_lx - (m_height/2)));
        case_w.push_back(pin->get_pin_pos_y() == (m_ly + (m_width/2)));
///}}}
///{{{  Case S
        z3::expr_vector case_s(m_z3_ctx);
        case_s.push_back(pin->get_pin_pos_x() == (m_lx - (m_width/2)));
        case_s.push_back(pin->get_pin_pos_y() == (m_ly - (m_height/2)));
///}}}
///{{{  Case E
        z3::expr_vector case_e(m_z3_ctx);
        case_e.push_back(pin->get_pin_pos_x() == (m_lx + (m_height/2)));
        case_e.push_back(pin->get_pin_pos_y() == (m_ly - (m_width/2)));
///}}}
        z3::expr clause = z3::ite(this->is_N(), z3::mk_and(case_n),
                          z3::ite(this->is_W(), z3::mk_and(case_w),
                          z3::ite(this->is_S(), z3::mk_and(case_s),
                          z3::ite(this->is_E(), z3::mk_and(case_e),
                                  m_z3_ctx.bool_val(false)))));
            
        clauses.push_back(clause);
    }
    m_encode_pins_center_of_macro = z3::mk_and(clauses);
}

/**
 * @brief Get Clauses for Pins Relative to Macro
 * 
 * @return z3::expr
 */
z3::expr Macro::get_pin_constraints()
{
    return m_pin_constraints;
}
