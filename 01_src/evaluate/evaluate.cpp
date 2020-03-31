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

Evaluate::Evaluate(MacroCircuit* mckt):
    m_mckt(mckt)
{
    assert (mckt != nullptr);
}

Evaluate::~Evaluate()
{
}

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

size_t Evaluate::calculate_hpwl(size_t const solution)
{
    Tree* tree = m_mckt->get_tree();
    size_t hpwl = 0;
    std::cout << "Edges: " << tree->get_edges().size() << std::endl;
    for(auto edge: tree->get_edges()){
       
        size_t from_x = 0;
        size_t from_y = 0;
        size_t to_x = 0;
        size_t to_y = 0;
        
        Node* from = edge->get_from();
        if(from->is_node()){
             if(from->has_macro()){
                from_x = from->get_macro()->get_solution_lx(solution);
                from_y = from->get_macro()->get_solution_ly(solution);
            } else if (from->has_cell()){
                continue;
                assert (0 && "Not Implemented");
            } else {
                assert (0);
            }
        } else if (from->is_terminal()){
            from_x = from->get_terminal()->get_solution_pin_pos_x(solution);
            from_y = from->get_terminal()->get_solution_pin_pos_y(solution);
        } else {
            assert (0);
        }
        
        Node* to = edge->get_to();
        if(to->is_node()){
            if(to->has_macro()){
                to_x = to->get_macro()->get_solution_lx(solution);
                to_y = to->get_macro()->get_solution_ly(solution);
            } else if (to->has_cell()){
                continue;
                 assert (0 && "Not Implemented");
            } else {
                assert (0);
            }
        } else if (to->is_terminal()){
            to_x = to->get_terminal()->get_solution_pin_pos_x(solution);
            to_y = to->get_terminal()->get_solution_pin_pos_y(solution);
        } else {
            assert (0);
        }
        
        std::cout << "From_x: " << from_x << " From_y: " << from_y << std::endl;
	std::cout << "To_x: " << to_x << " To_y: " << to_y << std::endl;
        
        hpwl += this->euclidean_distance(std::make_pair(from_x, from_y),
                                              std::make_pair(to_x, to_y));
    }
    std::cout << "HPWL: " << hpwl << std::endl;
    return hpwl;
}

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

    system("gnuplot hpwl_script.plt");
}

size_t Evaluate::euclidean_distance(std::pair<size_t, size_t> const & from,
                                            std::pair<size_t, size_t> const & to)
{
  
    double x = pow(to.first - from.first, 2);
    double y = pow(to.second - from.second, 2);
    
    
    
    double sum = x + y;
    std::cout << "x: " << x << std::endl; 
    std::cout << "y: " << y << std::endl;
    
    std::cout << sum << std::endl;
    
    return sqrt(sum);
}
