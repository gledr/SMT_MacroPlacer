//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : encoding.cpp
//
// Date         : 27. December 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Evaluate Placement Result
//==================================================================
#include "evaluate.hpp"

using namespace Placer;
using namespace Placer::Utils;

Evaluate::Evaluate(MacroCircuit* mckt):
    Object(),
    m_mckt(mckt)
{
    nullpointer_check (mckt);
}

Evaluate::~Evaluate()
{
}

/**
 * @brief Find Best HPWL Results
 * 
 * Returns a Pair Holding the Solution ID and the HPWL
 * 
 * @return std::pair< size_t, size_t >
 */
std::pair<size_t, size_t> Evaluate::best_hpwl()
{
    // Solution & Wirelenght
    std::vector<std::pair<size_t, size_t>> hpwl;
    
    std::cout << "Solutions: " << m_mckt->get_solutions() << std::endl;
    
    for(size_t i = 0; i < m_mckt->get_solutions(); ++i){
        size_t result = this->calculate_hpwl(i);
        
        hpwl.push_back(std::make_pair(i, result));
    }
    
    size_t min_hwpl_idx = 0;
    
    for(size_t i = 0; i < hpwl.size(); ++i){
        if(hpwl[i].second < hpwl[min_hwpl_idx].second){
            min_hwpl_idx = i;
        }
    }
    return hpwl[min_hwpl_idx];
}

/**
 * @brief Get HPWL for all Solutions
 * 
 * Returns a Vector of Pairs where each pair holds the Solution ID and
 * the HPWL of the according Solution
 * 
 * @return std::vector< std::pair< size_t, size_t > >
 */
std::vector<std::pair<size_t, size_t>> Evaluate::all_hpwl()
{
    // Solution & Wirelenght
    std::vector<std::pair<size_t, size_t>> hpwl;
    
    for(size_t i = 0; i < m_mckt->get_solutions(); ++i){
        size_t result = this->calculate_hpwl(i);
        
        hpwl.push_back(std::make_pair(i, result));
    }
    
    return hpwl;
}

/**
 * @brief Get the Die Area for all Solutions
 * 
 * Return s Vector of Pairs where each pair hold the Solutiond ID
 * and the Die Area of the according Solution
 * 
 * @return std::vector< std::pair< size_t, size_t > >
 */
std::vector<std::pair<size_t, size_t>> Evaluate::all_area()
{
    std::vector<std::pair<size_t, size_t>> retval;

    for (size_t i = 0; i < m_mckt->get_solutions(); ++i){
        size_t die_area = this->calculate_area(i);
        retval.push_back(std::make_pair(i, die_area));
    }

    return retval;
}

/**
 * @brief Get the Die Area for the Best Solution
 * 
 * Returns a Pair holding the Solution ID and the Die Area
 * 
 * @return std::pair< size_t, size_t >
 */
std::pair<size_t, size_t> Evaluate::best_area()
{
    // Type Element
    typedef std::pair<size_t, size_t> tElem;
    std::vector<tElem> all_area = this->all_area();

    auto min_elem = std::min_element(all_area.begin(), all_area.end(), 
                    [](tElem const & a, tElem const  & b){
                        return a.second < b.second;
    });

    return *min_elem;
}

/**
 * @brief Calculate the HPWL for a given Solution ID
 * 
 * @param solution Solution ID
 * @return size_t
 */
size_t Evaluate::calculate_hpwl(size_t const solution)
{
    Tree* tree = m_mckt->get_tree();
    size_t hpwl = 0;
    //std::cout << "Edges: " << tree->get_edges().size() << std::endl;
    for(auto edge: tree->get_edges()){
       
        size_t from_x = 0;
        size_t from_y = 0;
        size_t to_x = 0;
        size_t to_y = 0;
        
        Node* from = edge->get_from();
        if(from->is_node()){
             if(from->has_macro()){
                Macro* m = from->get_macro();
                nullpointer_check(m);
                
                Pin* pin = m->get_pin(edge->get_from_pin());
                nullpointer_check(pin);
                
                if (m->is_part_of_partition()){
                    Partition* p = m->get_parent_partition();
                    nullpointer_check(p);
                    
                    from_x = p->get_solution_lx(solution) + pin->get_solution_pin_pos_x(solution);
                    from_y = p->get_solution_ly(solution) + pin->get_solution_pin_pos_y(solution);
                    
                } else {
                    from_x = m->get_solution_lx(solution) + pin->get_solution_pin_pos_x(solution);
                    from_y = m->get_solution_ly(solution) + pin->get_solution_pin_pos_y(solution);
                }
                
            
            } else if (from->has_cell()){
                continue;
                notimplemented_check();
            } else {
                notsupported_check("Only Macros and Cells are Supported!");
            }
        } else if (from->is_terminal()){
            Terminal* terminal = from->get_terminal();

            if (terminal->is_free()){
                from_x = terminal->get_solution_pos_x(solution);
                from_y = terminal->get_solution_pos_y(solution);
            } else {
                from_x = terminal->get_pos_x().get_numeral_uint();
                from_y = terminal->get_pos_y().get_numeral_uint();
            }
        } else {
            notsupported_check("Only Macros and Terminals are Supported!");
        }

        Node* to = edge->get_to();
        if(to->is_node()){
            if(to->has_macro()){
                Macro* m = to->get_macro();
                nullpointer_check(m);
                
                Pin* pin = m->get_pin(edge->get_to_pin());
                nullpointer_check(pin);
                
                if (m->is_part_of_partition()){
                    Partition* p = m->get_parent_partition();
                    nullpointer_check(p);
                    
                    to_x = p->get_solution_lx(solution) + pin->get_solution_pin_pos_x(solution);
                    to_y = p->get_solution_ly(solution) + pin->get_solution_pin_pos_y(solution);
                    
                } else {
                    to_x = m->get_solution_lx(solution) + pin->get_solution_pin_pos_x(solution);
                    to_y = m->get_solution_ly(solution) + pin->get_solution_pin_pos_y(solution);
                }
            } else if (to->has_cell()){
                continue;
                notimplemented_check();
            } else {
                notsupported_check("Only Macros and Cells are Supported!");
            }
        } else if (to->is_terminal()){
            Terminal* terminal = to->get_terminal();

            if (terminal->is_free()){
                to_x = terminal->get_solution_pos_x(solution);
                to_y = terminal->get_solution_pos_y(solution);
            } else {
                to_x = terminal->get_pos_x().get_numeral_uint();
                to_y = terminal->get_pos_y().get_numeral_uint();
            }
        } else {
            notsupported_check("Only Macros and Terminals are Supported!");
        }

        //std::cout << "From_x: " << from_x << " From_y: " << from_y << std::endl;
        //std::cout << "To_x: " << to_x << " To_y: " << to_y << std::endl;
        
        hpwl += this->euclidean_distance(std::make_pair(from_x, from_y),
                                              std::make_pair(to_x, to_y));
    }
    //std::cout << "HPWL: " << hpwl << std::endl;
    return hpwl;
}

/**
 * @brief Plot the HPWL Distribution using a Bar Chart
 */
void Evaluate::plot_hpwl_distribution()
{
    std::vector<std::pair<size_t, size_t>> hpwl = this->all_hpwl();

    std::ofstream dat_file("hpwl.dat");
    for(auto itor: hpwl){
        dat_file << itor.first << " " << itor.second << std::endl;
    }
    dat_file.close();

    std::ofstream plot_script("hpwl_script.plt");
    plot_script << "set terminal png size 400,300;"  << std::endl;
    plot_script << "set output 'hpwl_distribution.png'" << std::endl;
    plot_script << "set style data histograms" << std::endl;
    plot_script << "plot 'hpwl.dat' using 2:xtic(1)" << std::endl;
    plot_script.close();

    std::vector<std::string> args;
    args.push_back("hpwl_script.plt");

    Utils::Utils::system_execute("gnuplot", args, "", false);
}

/**
 * @brief Calculate the Die Area for a given Solution ID
 * 
 * @param solution Solution ID
 * @return size_t
 */
size_t Evaluate::calculate_area(size_t const solution)
{
    Layout* layout = m_mckt->get_layout();
    
    if (this->get_minimize_die_mode()){
        assertion_check (layout->has_solution(solution));
    }

    size_t lx = 0;
    size_t ly = 0;
    size_t ux = 0;
    size_t uy = 0;

    if (layout->is_free_lx() && layout->is_free_ly()){
        lx = 0;
        ly = 0;
    } else {
        lx = layout->get_lx_numerical();
        ly = layout->get_ly_numerical();
    }
    
    if (layout->is_free_ux() && layout->is_free_uy()){
        ux = layout->get_solution_ux(solution);
        uy = layout->get_solution_uy(solution);
    } else {
        ux = layout->get_ux_numercial();
        uy = layout->get_uy_numerical();
    }

    return (ux - lx) * (uy -ly);
}

/**
 * @brief Calculate the Euclidean Distance between two Points of a 2D Coordinate
 * 
 * @param from First Coordinate Point
 * @param to   Second Coordinate Point
 * @return size_t
 */
size_t Evaluate::euclidean_distance(std::pair<size_t, size_t> const & from,
                                    std::pair<size_t, size_t> const & to)
{
    double x = pow(to.first - from.first, 2);
    double y = pow(to.second - from.second, 2);

    double sum = x + y;
    return sqrt(sum);
}
