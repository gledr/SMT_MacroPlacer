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

/**
 * @brief 
 * 
 * @param name 
 * @param id 
 * @param width
 * @param height
 */
Macro::Macro(std::string const & name,
             std::string const & id,
             size_t const width,
             size_t const height):
    Component(),
    m_encode_pin_macro_frontier(m_encode->get_value(0)),
    m_encode_pins_not_overlapping(m_encode->get_value(0)),
    m_encode_pins_center_of_macro(m_encode->get_value(0)),
    m_bool_orientation(m_encode->get_flag(false)),
    m_grid_coordinates(m_z3_ctx),
    m_cost_distribution(m_z3_ctx)
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
    
    this->get_verbose() && std::cout << "[Info]: Adding Free Macro " 
                       << id << " (" << width << "x" << height << ")"<< std::endl;
}

/**
 * @brief 
 * 
 * @param name
 * @param id
 * @param width
 * @param height
 * @param pos_lx 
 * @param pos_ly 
 * @param orientation 
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
    m_bool_orientation(m_encode->get_flag(false)),
    m_grid_coordinates(m_z3_ctx),
    m_cost_distribution(m_z3_ctx)
{
    m_lx = m_encode->get_value(pos_lx);
    m_ly = m_encode->get_value(pos_ly);
    m_width = m_encode->get_value(width);
    m_height = m_encode->get_value(height);
    m_orientation = m_encode->get_value(orientation);
    m_bool_orientation = m_z3_ctx.bool_const(std::string(name + "_orientation").c_str());
    m_name = name;
    m_id = id;
    m_free = false;
    m_supplement = nullptr;
    
    this->get_verbose() && std::cout << "[Info]: Adding Fixed Macro " << id << std::endl;
}

/**
 * @brief 
 * 
 * @param name
 * @param id 
 * @param width 
 * @param height 
 * @param layout_x
 * @param layout_y 
 */
Macro::Macro(std::string const & name,
             std::string const & id,
             size_t const width,
             size_t const height,
             size_t const layout_x,
             size_t const layout_y,
             CostFunction* lut):
    Component(),
    m_encode_pin_macro_frontier(m_encode->get_value(0)),
    m_encode_pins_not_overlapping(m_encode->get_value(0)),
    m_encode_pins_center_of_macro(m_encode->get_value(0)),
    m_bool_orientation(m_encode->get_flag(false)),
    m_grid_coordinates(m_z3_ctx),
    m_cost_distribution(m_z3_ctx),
    m_lut(lut)
{
    assert (lut != nullptr);

    m_layout_x = layout_x;
    m_layout_y = layout_y;
    m_width = m_encode->get_value(width);
    m_height = m_encode->get_value(height);
    m_bool_orientation = m_z3_ctx.bool_const(std::string(id + "_orientation").c_str());
    m_name = name;
    m_id = id;
    m_free = false;
    m_supplement = nullptr;
    m_init_grid = false;
    
    this->get_verbose() && std::cout << "[Info]: Adding Grid Macro " << id << std::endl;
}

/**
 * @brief 
 */
Macro::~Macro()
{
    for(auto itor: m_pins){
        delete itor.second; itor.second = nullptr;
    }

    m_supplement = nullptr;
}

z3::expr Macro::is_N()
{
    return m_orientation == m_encode->get_value(eNorth);
}

z3::expr Macro::is_W()
{
    return m_orientation == m_encode->get_value(eWest);
}

z3::expr Macro::is_S()
{
    return m_orientation == m_encode->get_value(eSouth);
}

z3::expr Macro::is_E()
{
    return m_orientation == m_encode->get_value(eEast);
}

z3::expr Macro::is_bool_N()
{
    return (m_bool_orientation == m_encode->get_flag(false));
}

z3::expr Macro::is_bool_W()
{
    return (m_bool_orientation == m_encode->get_flag(true));
}

z3::expr Macro::get_bool_orientation()
{
    return m_bool_orientation;
}

/**
 * @brief 
 */
void Macro::init_grid()
{
    for (size_t i = 0; i < m_layout_x; ++i) {
        for (size_t j = 0; j < m_layout_y; ++j) {
            std::string idx_grid = m_id + "_grid_x_" + std::to_string(i) + "_y_" +  std::to_string(j);
            m_cost_distribution.push_back(m_z3_ctx.int_val(m_lut->quadratic_lut(i,j)));
            m_grid_coordinates.push_back(m_z3_ctx.bool_const(idx_grid.c_str()));
        }
    }

    m_init_grid = true;
}

/**
 * @brief 
 * 
 * @return z3::expr
 */
z3::expr Macro::encode_grid()
{
    try {
        assert (m_init_grid);
        
        z3::expr_vector clauses(m_z3_ctx);
        size_t elements = 0;
        
        size_t height = m_height.get_numeral_uint();
        size_t width = m_width.get_numeral_uint();
        
        // Macro Covers Certain Area
        ///{{{
        z3::expr_vector covering_n(m_z3_ctx);
        z3::expr_vector covering_w(m_z3_ctx);
        for (size_t i = 0; i < m_layout_x; ++i) {
            for (size_t j = 0; j < m_layout_y; ++j) {
                // North
                if (((i + width) < m_layout_x) &&
                    ((j + height) < m_layout_y)) {
                    if (((i + 1) * m_layout_x) + j < m_grid_coordinates.size()) {
                        z3::expr_vector area(m_z3_ctx);

                        for (size_t h = 0; h < height; ++h){
                            for(size_t w = 0; w < width; ++w){
                                int idx = (i * m_layout_x) + j + (w*m_layout_y) + h;
                                area.push_back(m_grid_coordinates[idx]);
                            }
                        }
                        elements = area.size();
                        covering_n.push_back(mk_and(area));
                    }
                }
                
                // West
                // In the Grid
                if (i >= height &&
                    (j + width- 1 ) < m_layout_x){
                    // In the Vector
                    if (((i*width) + j) < m_grid_coordinates.size()){
                        z3::expr_vector area(m_z3_ctx);
                        size_t base_cell = (i-1)*m_layout_y + j;
                        //std::cout << base_cell << std::endl;
                        for (size_t h = 0; h < width; ++h){
                            size_t current_row = base_cell + h;
                            //std::cout << current_row << std::endl;
                            for(int w = 0; w < height; ++w){
                                int current_cell = current_row - (w*m_layout_x);
                                //std::cout << current_cell << std::endl;
                                area.push_back(m_grid_coordinates[current_cell]);
                            }
                        }
                        covering_w.push_back(z3::mk_and(area));
                    }
                }
            }
        }
    ///}}}
    
        clauses.push_back(z3::ite(this->is_bool_N(), z3::mk_or(covering_n), 
                        z3::ite(this->is_bool_W(), z3::mk_or(covering_w), m_encode->get_flag(false))));

        // Macro has to cover a particular number of cells
        std::vector<int> val_m1_grid(m_grid_coordinates.size(), 1);
        int _val_m1_arr[val_m1_grid.size()];
        std::copy(val_m1_grid.begin(), val_m1_grid.end(), _val_m1_arr);
        z3::expr sum_m1_grid = z3::pbeq(m_grid_coordinates, _val_m1_arr, m_height.get_numeral_uint() * m_width.get_numeral_uint());
        clauses.push_back(sum_m1_grid);

        return z3::mk_and(clauses);
    } catch (z3::exception const & exp){
        std::cout << exp.msg() << std::endl;
        assert (0);
    }
}

/**
 * @brief 
 * 
 * @return z3::expr_vector
 */
z3::expr_vector Macro::get_grid_coordinates()
{
    return m_grid_coordinates;
}

/**
 * @brief 
 * 
 * @return z3::expr_vector
 */
z3::expr_vector Macro::get_grid_costs()
{
    z3::expr_vector ret_val(m_z3_ctx);
    
    for (size_t i = 0; i < m_grid_coordinates.size(); ++i){
        ret_val.push_back(z3::ite(m_grid_coordinates[i], m_cost_distribution[i], m_z3_ctx.int_val(0)));
    }
    
    return ret_val;
}

/**
 * @brief 
 * 
 * @param supplement 
 */
void Macro::set_supplement(Supplement* supplement)
{
    assert (supplement != nullptr);
    
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
 * @brief 
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
 * @brief 
 * 
 * @return size_t
 */
size_t Macro::get_area()
{
    return m_width.get_numeral_uint() * m_height.get_numeral_uint();
}

/**
 * @brief 
 * 
 * @param stream 
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
 * @brief 
 */
void Macro::encode_pins()
{
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
 * @brief 
 * 
 * @return z3::expr
 */
z3::expr Macro::get_pin_constraints()
{
}

/**
 * @brief 
 * 
 * @param x 
 * @param y 
 */
void Macro::add_solution_root(const size_t x, const size_t y)
{
    std::cout << "Set Macro Root to: " << x << ":" << y << std::endl;
    
    m_root_solution = std::make_pair(x,y);
}

/**
 * @brief 
 * 
 * @return std::pair< long unsigned int, long unsigned int >
 */
std::pair<size_t, size_t> Macro::get_solution_root() const
{
    return m_root_solution;
}

void Macro::add_solution_grid(size_t const x, size_t const y)
{
    m_grid_solutions.push_back(std::make_pair(x,y));
}

void Macro::calculate_root()
{
    // North
    if (m_bool_orientation == m_encode->get_flag(false)){
        m_root_solution = *std::min_element(m_grid_solutions.begin(), m_grid_solutions.end(), [](std::pair<size_t, size_t> a, std::pair<size_t, size_t> b){
            return a.first <= b.first && a.second <= b.second;
        });
    // West
    } else if (m_bool_orientation == m_encode->get_flag(true)){
        m_root_solution = *std::min_element(m_grid_solutions.begin(), m_grid_solutions.end(), [](std::pair<size_t, size_t> a, std::pair<size_t, size_t> b){
            return a.first >= b.first && a.second <= b.second;
        });
    }

    std::cout << m_id << " Setting Root: " << m_root_solution.first << ":" << m_root_solution.second  << " Orientation: " << m_sol_orientation.front() << std::endl;
}
