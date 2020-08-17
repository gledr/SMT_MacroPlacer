//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : plotter.cpp
//
// Date         : 04. June 2020
// Compiler     : gcc version 10.1.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : MacroCircuit Plotter
//==================================================================
#include "plotter.hpp"

using namespace Placer;
using namespace Placer::Utils;

/**
 * @brief Constructor
 */
Plotter::Plotter():
    Object()
{
}

/**
 * @brief Destructor
 */
Plotter::~Plotter()
{
}

/**
 * @brief Set Data to Plot
 * 
 * @param terminals Terminals
 * @param components Components
 * @param solution_id Solution to Plot
 * @param layout Layout
 */
void Plotter::set_data(std::vector<Terminal*> const & terminals,
                       std::vector<Component*> const & components,
                       size_t const solution_id,
                       Layout *const layout)
{
    nullpointer_check(layout);
    
    m_layout = layout;
    m_terminals = terminals;
    m_components = components;
    m_solution_id = solution_id;
}

/**
 * @brief Run Plotting
 */
void Plotter::run()
{
    if(!boost::filesystem::exists(this->get_image_directory())){
        boost::filesystem::create_directories(this->get_image_directory());
    }

    matplotlibcpp::backend("Agg"); // Set Agg backend for the case running
                                   // on a workstation without X support
                                   // http://antigrain.com/

    matplotlibcpp::figure_size(800, 600);

    size_t die_lx = m_layout->get_lx().get_numeral_uint();
    size_t die_ly = m_layout->get_ly().get_numeral_uint();

    if(m_layout->is_free_ux()){
        matplotlibcpp::xlim(die_lx-0.5, m_layout->get_solution_ux(m_solution_id)+0.5);
        matplotlibcpp::ylim(die_ly-0.5, m_layout->get_solution_uy(m_solution_id)+0.5);
    } else {
        size_t ux = m_layout->get_ux().get_numeral_uint();
        size_t uy = m_layout->get_uy().get_numeral_uint();
        matplotlibcpp::xlim(die_lx-0.5, ux+0.5);
        matplotlibcpp::ylim(die_ly-0.5, uy+0.5);
    }

    this->draw_layout();

    // Terminals
    for(Terminal* t: m_terminals){
        this->draw_terminal(t);
    }

    for(size_t j = 0; j < m_components.size(); ++j){
        Component* cmp = m_components[j];
        nullpointer_check(cmp);

        size_t width = cmp->get_width().get_numeral_uint();
        size_t height = cmp->get_height().get_numeral_uint();

        size_t o = 0;
        size_t lx = 0;
        size_t ly = 0;
        
        if (cmp->is_free()){
            o  = cmp->get_solution_orientation(m_solution_id);
            lx = cmp->get_solution_lx(m_solution_id);
            ly = cmp->get_solution_ly(m_solution_id);
        } else {
            o  = cmp->get_orientation().get_numeral_uint();
            lx = cmp->get_lx_numeral();
            ly = cmp->get_ly_numeral();
        }
        std::string id = cmp->get_id();

        // North
        if(o == eNorth){
            this->draw_rectangle(lx, ly, lx + width, ly + height, id);

        // West
        } else if (o == eWest){
            this->draw_rectangle(lx - height, ly, lx, ly + width, id);

        // South
        } else if (o == eSouth){
            this->draw_rectangle(lx - width, ly - height, lx ,ly, id);

        // East
        } else if (o == eEast){
            this->draw_rectangle(lx, ly - width, lx + height, ly, id);

        // Flip North
        } else if (o == eFlipNorth){
            this->draw_rectangle(lx - width, ly, lx, ly + height, id);

        // Flip West
        } else if (o == eFlipWest){
            this->draw_rectangle(lx, ly, lx + height, ly + width, id);

        // Flip South
        } else if (o == eFlipSouth){
            this->draw_rectangle(lx, ly - height, lx + width, ly, id);

        // Flip East
        } else if (o == eFlipEast){
            this->draw_rectangle(lx - height, ly - width, lx, ly, id);

        // Error
        } else {
            notsupported_check("Orientation not Supported!");
        }

        for  (Pin* p: cmp->get_pins()){
            this->draw_pin(cmp, p);
        }
    }

    std::stringstream img_name;
    if (m_filename == ""){
        img_name << "placement_" << this->get_def() << "_" << m_solution_id << ".png";
    } else {
        img_name << m_filename << ".png";
    }
    matplotlibcpp::save("./" + img_name.str());
}

/**
 * @brief Draw Rectangle
 * 
 * @param lx Lower X
 * @param ly Lower Y
 * @param ux Upper X
 * @param uy Upper Y
 * @param label Label to be used
 */
void Plotter::draw_rectangle(size_t const lx,
                             size_t const ly,
                             size_t const ux,
                             size_t const uy,
                             std::string const & label)
{
    matplotlibcpp::plot(std::vector<size_t>({lx, ux}),
                        std::vector<size_t>({ly, ly}),
                        "-k");

    matplotlibcpp::plot(std::vector<size_t>({lx, ux}),
                        std::vector<size_t>({uy, uy}),
                        "-k");

    matplotlibcpp::plot(std::vector<size_t>({lx, lx}),
                        std::vector<size_t>({ly, uy}),
                        "-k");
        
    matplotlibcpp::plot(std::vector<size_t>({ux, ux}),
                        std::vector<size_t>({uy, ly}),
                        "-k");
}

/**
 * @brief Draw Terminal
 * 
 * @param t Pointer to Terminal
 */
void Plotter::draw_terminal(Terminal* t)
{
    nullpointer_check(t);
    
    std::vector<size_t> x;
    std::vector<size_t> y;
    
     if (!t->is_free()){
        x.push_back(t->get_pox_x_numerical());
        y.push_back(t->get_pos_y_numerical());
    } else {
        if (t->has_solution(m_solution_id)){
            x.push_back(t->get_solution_pos_x(m_solution_id));
            y.push_back(t->get_solution_pos_y(m_solution_id));
        }
    }
    matplotlibcpp::scatter(x,y,50);
}

/**
 * @brief Draw Pin
 * 
 * @param parent Pointer to Pin's Parent Component
 * @param pin Pointer to Pin
 */
void Plotter::draw_pin(Component* parent, Pin* pin)
{
    nullpointer_check(pin);
    nullpointer_check(parent);
    
    size_t x_pos = 0;
    size_t y_pos = 0;
    size_t lx = 0;
    size_t ly = 0;
    size_t ux = 0;
    size_t uy = 0;

    if (!pin->is_free()){
        x_pos = pin->get_pin_pos_x_numeral();
        y_pos = pin->get_pin_pos_y_numeral();
    } else {
        if (pin->has_solution(m_solution_id)){
            x_pos = pin->get_solution_pin_pos_x(m_solution_id);
            y_pos = pin->get_solution_pin_pos_y(m_solution_id);
        } else {
            return;
        }
    }
    
    eOrientation o = eOrientation::eNorth;
    
    if (parent->is_free()){
        o = parent->get_solution_orientation(m_solution_id);
    } else {
        o = static_cast<eOrientation>(parent->get_orientation().get_numeral_uint());
    }
    
   
    if (o == eNorth){
        if (parent->is_free()){
            lx = parent->get_solution_lx(m_solution_id);
            ly = parent->get_solution_ly(m_solution_id);
        } else {
            lx = parent->get_lx_numeral();
            ly = parent->get_ly_numeral();
        }
        ux = lx + parent->get_width_numeral();
        uy = ly + parent->get_height_numeral();
    } else if (o == eWest){
        if (parent->is_free()){
            lx = parent->get_solution_lx(m_solution_id) - parent->get_height_numeral();
            ly = parent->get_solution_ly(m_solution_id);
            ux = parent->get_solution_lx(m_solution_id);
            uy = parent->get_solution_ly(m_solution_id) + parent->get_width_numeral();
        } else {
            lx = parent->get_lx_numeral()- parent->get_height_numeral();
            ly = parent->get_ly_numeral();
            ux = parent->get_lx_numeral();
            uy = parent->get_ly_numeral() + parent->get_width_numeral();
        }
    } else {
        notimplemented_check();
    }

    enum ePlane {ePlaneLeft, ePlaneRight, ePlaneUpper, ePlaneLower, eCenter, ePlaneInit};
    ePlane pin_location = ePlaneInit;

    // Left Plane
    if ((x_pos == lx) && ((y_pos >= ly) && (y_pos <= uy))){
        pin_location = ePlaneLeft;
    // Right Plane
    } else if ((x_pos == ux) && ((y_pos >= ly) && (y_pos <= uy))){
        pin_location = ePlaneRight;
    // Lower Plane
    } else if ((y_pos == ly) && ((x_pos >= lx) && (x_pos <= ux))){
        pin_location = ePlaneLower;
    // Upper Plane 
    } else if ((y_pos == uy) && ((x_pos >= lx) && (x_pos <= ux))){
        pin_location = ePlaneUpper;
    //} else if ((y_pos == uy/2) && (x_pos == ux/2)){
    //    pin_location = eCenter;
    } else {
        pin_location = eCenter;
        //throw PlacerException("Pin Position Not Supported!");
    }
    double pin_offset = 0.2;
    std::vector<double> x;
    std::vector<double> y;

    if (pin_location == ePlaneLeft){
        x.push_back(x_pos + pin_offset);
        y.push_back(y_pos);
    } else if (pin_location == ePlaneLower){
        x.push_back(x_pos);
        y.push_back(y_pos + pin_offset);
    } else if (pin_location == ePlaneRight){
        x.push_back(x_pos - pin_offset);
        y.push_back(y_pos);
    } else if (pin_location == ePlaneUpper){
        x.push_back(x_pos);
        y.push_back(y_pos - pin_offset);
    } else if (pin_location == eCenter){
        x.push_back(x_pos);
        y.push_back(y_pos);
    } else {
        throw PlacerException("Invalid Pin Position Detected!");
    }

    matplotlibcpp::scatter(x,y,50);
}

/**
 * @brief Draw Layout Shape
 */
void Plotter::draw_layout()
{
    size_t lx = 0;
    size_t ly = 0;
    size_t ux = 0;
    size_t uy = 0;

    if (m_layout->is_free_lx()){
        notimplemented_check();
    } else {
        lx = m_layout->get_lx_numerical();
    }
    if (m_layout->is_free_ly()){
        notimplemented_check();
    } else {
        ly = m_layout->get_ly_numerical();
    }
    if (m_layout->is_free_ux()){
        ux = m_layout->get_solution_ux(m_solution_id);
    } else {
        ux = m_layout->get_uy_numerical();
    }
    if (m_layout->is_free_uy()){
        uy = m_layout->get_solution_uy(m_solution_id);
    } else {
        uy = m_layout->get_uy_numerical();
    }

    matplotlibcpp::plot(std::vector<size_t>({lx, ux}),
                        std::vector<size_t>({ly, ly}),
                        "--b");

    matplotlibcpp::plot(std::vector<size_t>({lx, ux}),
                        std::vector<size_t>({uy, uy}),
                        "--b");

    matplotlibcpp::plot(std::vector<size_t>({lx, lx}),
                        std::vector<size_t>({ly, uy}),
                       "--b");

    matplotlibcpp::plot(std::vector<size_t>({ux, ux}),
                        std::vector<size_t>({uy, ly}),
                        "--b");
}

/**
 * @brief Set Name for Export
 *
 * @param name Name to Apply
 */
void Plotter::set_filename(std::string const & name)
{
    m_filename = name;
}
