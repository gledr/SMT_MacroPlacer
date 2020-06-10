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
Tree::Tree():
    Object()
{
    m_logger = Utils::Logger::getInstance();
}

/**
 * @brief Destructor
 */
Tree::~Tree()
{
    /* No DTOR Possible due to CCTOR Shallow Copying */
}

/**
 * @brief Release Resources
 */
void Tree::destroy()
{
    for(auto itor: m_nodes){
        delete itor; itor = nullptr;
    }
    m_logger = nullptr;
}

/**
 * @brief Copy Constructor
 * 
 * @param tree Tree to Copy
 */
Tree::Tree(Tree const & tree)
{
    // Copy Pointer --> Shallow Copy
    std::copy(tree.m_edges.begin(), tree.m_edges.end(), std::back_inserter(m_edges));
    std::copy(tree.m_nodes.begin(), tree.m_nodes.end(), std::back_inserter(m_nodes));
    std::copy(tree.m_terminals.begin(), tree.m_terminals.end(), std::back_inserter(m_terminals));
    m_logger = tree.m_logger;
}

/**
 * @brief Assignment Operator
 * 
 * @param tree Tree to Assign
 * @return Placer::Tree&
 */
Tree& Tree::operator=(Tree const & tree)
{
    notimplemented_check();
}

/**
 * @brief Compare Operator
 * 
 * @param tree Tree to Compare
 * @return bool
 */
bool Tree::operator==(Tree const & tree)
{
    notimplemented_check();
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

/**
 * @brief Find Node by its ID and Name
 * 
 * @param name Node Name
 * @param id Node ID
 * @return Placer::Node*
 */
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

/**
 * @brief Dump Tree Components to Given Stream
 * 
 * @param stream Stream to Dump to
 */
void Tree::dump(std::ostream& stream)
{
    for(auto itor: m_terminals){
        itor->dump(stream);
    }
    for(auto itor: m_nodes){
        itor->dump(stream);
    }
}

/**
 * @brief Construct Tree from Pre Inserted Edges
 */
void Tree::construct_tree()
{
    try {
        m_logger->construct_tree(m_edges.size());

        for(auto edge: m_edges){
            Node* from = edge->get_from();
            Node* to   = edge->get_to();

          from->insert_edge(edge);
          to->insert_edge(edge);
        }
          this->verify_edges();
    } catch (PlacerException const & exp){
        std::cout << "Could not create connectivity tree (" << exp.what() << ")" << std::endl;
        exit(-1);
    }
}

/**
 * @brief Allocate new Element 
 * 
 * @param _node Node to Add
 */
void Tree::new_element(Node* _node)
{
    nullpointer_check (_node );

    if(_node->is_node()){
        m_nodes.push_back(_node);
    } else if (_node->is_terminal()){
        m_terminals.push_back(_node);
    } else {
        notimplemented_check();
    }
}

/**
 * @brief Iterator over the Tree
 * 
 * @param stream Stream to dump information to
 */
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

/**
 * @brief Recursive Core Function to Iterate over Tree
 * 
 * @param _root Root Node
 * @param current_root Current Root Node
 * @param stream Stream to Dump to
 * @param fringe Nodes in the current fringe left to iterate
 */
void Tree::iterate_tree_recursive(Node* _root,
                                  Node* current_root,
                                  std::ofstream & stream,
                                  std::vector<Node*> & fringe)
{
    nullpointer_check (current_root);

    for(auto itor: current_root->get_edges()){
        if(itor->get_to() != current_root){
            Node* from = itor->get_from();
            Node* to   = itor->get_to();
            nullpointer_check (from);
            nullpointer_check (to);
            std::cout << from->get_id() << " -> " << to->get_id() << ";" << std::endl;
            stream << "\t" << from->get_id() << " -> " << to->get_id() << ";" << std::endl;

            if(itor->get_to()->is_terminal()){
                if (itor->get_to()->get_terminal()->is_output()){
                    return;
                } else if (itor->get_to()->get_terminal()->is_bidirectional()){
                    fringe.push_back(itor->get_to());
                    return;
                } else {
                    notimplemented_check();
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
            throw PlacerException("No dot file created!");
    }

    } catch (std::exception const & exp){
        throw PlacerException(exp.what());
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
        throw Utils::PlacerException("No png file created!");
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

/**
 * @brief Reconstruct Steiner Tree
 * 
 * @return std::map< std::string, std::set< std::string > >
 */
std::map<std::string, std::set<std::string> > Tree::get_steiner_tree()
{
    m_steiner_tree.clear();

    for (Edge* edge: m_edges){
        nullpointer_check(edge);
        
        Node* from = edge->get_from();
        nullpointer_check(from);
        
        std::string from_id = from->get_id();
        std::string from_pin = edge->get_from_pin();

        Node* to = edge->get_to();
        nullpointer_check(to);
        std::string to_id = to->get_id();
        std::string to_pin = edge->get_to_pin();

        std::string id1 = from_id + ":" + from_pin;
        std::string id2 = to_id + ":" + to_pin;
        m_steiner_tree[id1].insert(id2);
    }
    return m_steiner_tree;
}

/**
 * @brief Export Tree as Hypergraph using *.hgr format
 */
void Tree::export_hypergraph()
{
    std::string filename = this->get_design_name() + ".hgr";
    m_logger->export_hypergraph(filename);

    std::ofstream hgr_file (this->get_active_results_directory() +
                            "/" +
                            filename);

    std::map<std::string, std::set<std::string>> steiner_tree = this->get_steiner_tree();

    // Header: Edges Nodes Settings
    // Settings 0  Unweigthed Hypergraph
    //          1  Hypergraph with edge weights
    //          10 Hypergraph with node weights
    //          11 Hypergraph with node and edge weights
    hgr_file << m_steiner_tree.size() << " " << m_nodes.size() + m_terminals.size() << std::endl;
    for (auto edge: steiner_tree){
        std::vector<std::string> root_token = Utils::Utils::tokenize(edge.first, ":");
        Node* root = this->find_node(root_token[0], root_token[0]);
        nullpointer_check(root);
        size_t root_key = 0;
        if (root->is_node()){
            Macro* m = root->get_macro();
            root_key = m->get_key();
        } else {
            Terminal* t = root->get_terminal();
            root_key = t->get_key();
        }

        hgr_file << root_key << " ";

        std::set<size_t> sub_keys;
        for (auto itor2: edge.second){
            std::vector<std::string> token = Utils::Utils::tokenize(itor2, ":");
            Node* node = this->find_node(token[0], token[0]);
            nullpointer_check(node);

            size_t sub_key = 0;
            if (node->is_node()){
                Macro* sub_m = node->get_macro();
                sub_key = sub_m->get_key();
            } else {
                Terminal* t = node->get_terminal();
                sub_key = t->get_key();
            }
            // No Edge can point at itself
            // Use a set to get unique subkeys
            sub_keys.insert(sub_key);
        }

        for (auto itor2: sub_keys){
            if (itor2 != root_key){
                hgr_file << itor2 << " ";
            }
        }
        hgr_file << std::endl;
    }
    hgr_file.close();
}

/**
 * @brief Check if Edges are Valid
 * 
 * Check if Bitwidth and Frequency of the Nodes connected to the edge match
 */
void Tree::verify_edges()
{
    for (Edge* edge: m_edges){
        if (edge->get_bitwidth_from() != edge->get_bitwidth_to()){
            throw PlacerException("Invalid Bitwidth in Edge Detected!");
        }
        if (edge->get_frequency_from() != edge->get_frequency_to()){
            throw PlacerException("Invalid Frequency in Edge Detected!");
        }
    }
}

/**
 * @brief Remove all Edges connected with Terminals
 */
void Tree::strip_terminals()
{
    std::vector<Edge*> next_edges;
    std::vector<Edge*> delete_me;

    for(Edge* edge: m_edges){
        Node* from = edge->get_from();
        Node* to = edge->get_to();

        if (to->is_terminal() || from->is_terminal()){
            delete_me.push_back(edge);
        } else {
            next_edges.push_back(edge);
        }
    }

    for (Edge* edge: delete_me){
        delete edge; edge = nullptr;
    }
    for(auto itor: m_terminals){
        delete itor; itor = nullptr;
    }
    m_terminals.clear();

    m_edges.clear();
    std::copy(next_edges.begin(), next_edges.end(), std::back_inserter(m_edges));
}
