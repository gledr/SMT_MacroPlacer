//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : plotter.cpp
//
// Date         : 04. June 2020
// Compiler     : gcc version 10.1.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : LEF/DEF MacroCircuit Plotter
//==================================================================
#include "plotter.hpp"

using namespace Placer;

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

    boost::filesystem::current_path(this->get_image_directory());

    matplotlibcpp::figure_size(800, 600);

    size_t die_lx = m_layout->get_lx().get_numeral_uint();
    size_t die_ly = m_layout->get_ly().get_numeral_uint();
    
    if(m_layout->is_free_ux()){
        matplotlibcpp::xlim(die_lx, m_layout->get_solution_ux(m_solution_id));
        matplotlibcpp::ylim(die_ly, m_layout->get_solution_uy(m_solution_id));
    } else {
        size_t ux = m_layout->get_ux().get_numeral_uint();
        size_t uy = m_layout->get_uy().get_numeral_uint();
        matplotlibcpp::xlim(die_lx, ux);
        matplotlibcpp::ylim(die_ly, uy);
    }
    
    size_t _lx = 0;
    size_t _ly = 0;
    size_t _ux = 0;
    size_t _uy = 0;

    for(size_t j = 0; j < m_components.size(); ++j){
        size_t width = m_components[j]->get_width().get_numeral_uint();
        size_t height = m_components[j]->get_height().get_numeral_uint();

        size_t o  = m_components[j]->get_solution_orientation(m_solution_id);
        size_t lx = m_components[j]->get_solution_lx(m_solution_id);
        size_t ly = m_components[j]->get_solution_ly(m_solution_id);

        // North
        if(o == eNorth){
            _lx = lx;
            _ly = ly;
            _ux = lx + width;
            _uy = ly + height;

        // West
        } else if (o == eWest){
            _lx = lx - height;
            _ly = ly;
            _ux = lx;
            _uy = ly + width;

        // South
        } else if (o == eSouth){
            _lx = lx - width;
            _ly = ly - height;
            _ux = lx;
            _uy = ly;

        // East
        } else if (o == eEast){
            _lx = lx;
            _ly = ly - width;
            _ux = lx + height;
            _uy = ly;

        // Flip North
        } else if (o == eFlipNorth){
            _lx = lx - width;
            _ly = ly;
            _ux = lx;
            _uy = ly + height;

        // Flip West
        } else if (o == eFlipWest){
            _lx = lx;
            _ly = ly;
            _ux = _lx + height;
            _uy = _ly + width;

        // Flip South
        } else if (o == eFlipSouth){
            _lx = lx;
            _ly = ly - height;
            _ux = lx + width;
            _uy = ly;

        // Flip East
        } else if (o == eFlipEast){
            _lx = lx - height;
            _ly = ly - width;
            _ux = lx;
            _uy = ly;

        // Error
        } else {
            notsupported_check("Orientation not Supported!");
        }

        this->draw_rectangle(_lx, _ly, _ux, _uy);
        
      /*
        if (this->get_minimize_hpwl_mode()){
            // Terminals
            for(size_t i = 0 ; i < m_terminals.size(); ++ i){
                size_t x = m_terminals[i]->get_solution_pos_x(m_solution_id);
                size_t y = m_terminals[i]->get_solution_pos_y(m_solution_id);
                gnu_plot_file << "# " << m_terminals[i]->get_id() << std::endl;
                gnu_plot_file  << "set object " << i+100 << " rect from " << std::to_string(x-0.25) << "," << std::to_string(y-0.25) 
                            << " to "  << std::to_string(x+0.25) << ","<< std::to_string(y+0.25) <<" lw 5;"<< std::endl;
            }
            
            // Pin
            size_t id_cnt = 200;
            for (Component* m: m_components){
                for  (Pin* p: m->get_pins()){
                    size_t x = p->get_solution_pin_pos_x(m_solution_id);
                    size_t y = p->get_solution_pin_pos_y(m_solution_id);
                    gnu_plot_file << "# " << m->get_id() << " " << p->get_name() << std::endl;
                    gnu_plot_file  << "set object " << id_cnt << " rect from " << std::to_string(x-0.25) << "," << std::to_string(y-0.25) 
                            << " to "  << std::to_string(x+0.25) << ","<< std::to_string(y+0.25) <<" lw 5;"<< std::endl;
                    id_cnt++;
                }
            }
        }
        gnu_plot_file << "plot x " << std::endl,
        gnu_plot_file.close();

        */
        std::stringstream img_name;
        img_name << "placement_" << this->get_design_name() << "_" << m_solution_id << ".png";
        matplotlibcpp::save("./" + img_name.str());
    }
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
