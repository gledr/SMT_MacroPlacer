//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : tree.cpp
//
// Date         : 27. December 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Macro Dependency Tree
//==================================================================
#include "tree.hpp"
#include "edge.hpp"

using namespace Placer;
using namespace Placer::Utils;


/**
 * @brief Constructor
 * 
 * @param ctx Z3 Context
 */
Tree::Tree(z3::context* ctx):
    Object()
{
    assert (ctx != nullptr);
  
    m_z3_ctx = ctx;
    m_logger = Utils::Logger::getInstance();
}

/**
 * @brief Destructor
 */
Tree::~Tree()
{
    for(auto itor: m_nodes){
        delete itor; itor = nullptr;
    }
    for(auto itor: m_terminals){
        delete itor; itor = nullptr;
    }
    for(auto itor: m_edges){
        delete itor; itor = nullptr;
    }
    m_logger = nullptr;
}

/**
 * @brief Get all Edges building up the Tree
 * 
 * @return std::vector<Edge*> 
 */
std::vector<Edge *> Tree::get_edges()
{
    return m_edges;
}

Node* Tree::find_node(std::string const & name,
                      std::string const & id)
{
    Node* retval = nullptr;
    
    for(auto itor: m_nodes){
        if(itor->get_macro() != nullptr){
            if(itor->get_macro()->get_id() == id){
                retval = itor;
                break;
            }
        } else if (itor->get_cell() != nullptr){
            if (itor->get_cell()->get_id() == id){
                retval = itor;
                break;
            }
        }
    }
    
    if(!retval){
        for(auto itor: m_terminals){
            if(itor->get_terminal()->get_name() == id){
                retval = itor;
                break;
            }
        }
    }
    return retval;
}

void Tree::dump(std::ostream& stream)
{
    for(auto itor: m_terminals){
        itor->dump(stream);
    }
    for(auto itor: m_nodes){
        itor->dump(stream);
    }
}

void Tree::construct_tree()
{
    m_logger->construct_tree(m_edges.size());
    
    for(auto edge: m_edges){
        Node* from = edge->get_from();
        Node* to   = edge->get_to();
        
        //assert (from != to);
        
        from->insert_edge(edge);
        to->insert_edge(edge);
    }
}

void Tree::new_element(Node* _node)
{
    assert (_node != nullptr);
    
    if(_node->is_node()){
        m_nodes.push_back(_node);
    } else if (_node->is_terminal()){
        m_terminals.push_back(_node);
    } else {
        assert(0);
    }
}

void Tree::iterate_tree(std::ofstream & stream)
{
    std::vector<Node*> fringe;
    
    for(auto itor: m_terminals){
        if (itor->get_terminal()->is_input()){
            this->iterate_tree_recursive(itor, itor, stream, fringe);
        } else if (itor->get_terminal()->is_bidirectional()){
            if (std::find(fringe.begin(), fringe.end(), itor) == fringe.end()){
                fringe.push_back(itor);
                this->iterate_tree_recursive(itor, itor, stream, fringe);
            } else {
                continue;
            }
        }
    }
}

void Tree::iterate_tree_recursive(Node* _root,
                                  Node* current_root,
                                  std::ofstream & stream,
                                  std::vector<Node*> & fringe)
{
    assert (current_root != nullptr);
    for(auto itor: current_root->get_edges()){
        if(itor->get_to() != current_root){
            Node* from = itor->get_from();
            Node* to   = itor->get_to();
            assert (from != nullptr);
            assert (to != nullptr);
            
            stream << "\t" << from->get_id() << " -> " << to->get_id() << ";" << std::endl;

            if(itor->get_to()->is_terminal()){
                if (itor->get_to()->get_terminal()->is_output()){
                    return;
                } else if (itor->get_to()->get_terminal()->is_bidirectional()){
                    fringe.push_back(itor->get_to());
                    return;
                } else {
                    assert (0);
                }
            } else {
                this->iterate_tree_recursive(_root, itor->get_to(), stream, fringe);
            }
        }
    }
}

/**
 * @brief Visualize generated tree
 */
void Tree::visualize()
{
    this->export_dot_file();
    this->dot_to_png();
    this->show_png();
}

/**
 * @brief Export Tree as dot file
 */
void Tree::export_dot_file()
{
    try {
        std::string img_dir = this->get_image_directory();
        
        if(!boost::filesystem::exists(img_dir)){
            boost::filesystem::create_directories(img_dir);
        }
        
        boost::filesystem::current_path(img_dir);
        
        std::string dot_name = this->get_design_name() + ".dot";
        std::ofstream dot_stream(img_dir + "/" + dot_name);
        
         if(!dot_stream.is_open()){
            throw "Could not create dot stream file!";
        }
        
        dot_stream << "digraph " << this->get_design_name() << std::endl;
        dot_stream << "{" << std::endl;
        this->iterate_tree(dot_stream);
        dot_stream << "}" << std::endl;
        
        dot_stream.close();
        
        if(!boost::filesystem::exists(dot_name)){
            throw "No dot file created!";
    }
        
    } catch (std::exception const & exp){
        std::cout << exp.what() << std::endl;
        assert (0);
    }
}

/**
 * @brief Convert dot file to png
 */
void Tree::dot_to_png()
{
    boost::filesystem::current_path(this->get_image_directory());
    std::string bin = "dot";
    std::string input = this->get_design_name() + ".dot";
    std::string output= this->get_design_name() + ".png";
    bool wait_for_termination = true;
    
    std::vector<std::string> args;
    args.push_back("-Tpng");
    args.push_back(input);
    args.push_back("-o");
    args.push_back(output);
    
    Utils::Utils::system_execute(bin, args, "", wait_for_termination);
    
    if(!boost::filesystem::exists(output)){
        throw "No png file created!";
    }
}

/**
 * @brief Shwow png using system viewer
 */
void Tree::show_png()
{
    boost::filesystem::current_path(this->get_image_directory());
    std::string bin = "kde-open";
    std::string input = this->get_design_name() + ".png";
    bool wait_for_termination = false;
    std::vector<std::string> args;
    args.push_back(input);
    
    Utils::Utils::system_execute(bin, args, "", wait_for_termination);
}
