//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : bookshelf.cpp
//
// Date         : 13. January 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Bookshelf Input/Output
//==================================================================
#include "bookshelf.hpp"

using namespace Placer;

/**
 * @brief Constructor
 */
Bookshelf::Bookshelf():
    Object()
{
    m_tree = new Tree();
    m_lut = new CostFunction();
}

/**
 * @brief Destructor
 */
Bookshelf::~Bookshelf()
{
    m_tree = nullptr;
    delete m_lut; m_lut = nullptr;
}

/**
 * @brief Read Bookshelf Format Files
 */
void Bookshelf::read_files()
{
    std::string aux_file = this->get_bookshelf_file();
    
    if(!boost::filesystem::exists(aux_file)){
        throw std::runtime_error("Bookshelf File does not exits!");
    }
    
    std::vector<std::string> aux_content;
    std::string line;
    
    std::ifstream aux_filestream(aux_file);
    while(std::getline(aux_filestream, line)){
        aux_content.push_back(line);
    }
    aux_filestream.close();
    
    if(aux_content.size() != 1){
        throw std::runtime_error("Aux file must contain excatly one line!");
    }
    
    std::vector<std::string> token = Utils::Utils::tokenize(aux_content[0], " ");
    
    m_design_name = Utils::Utils::tokenize(aux_content[0], ".")[0];
    
    for(auto& file : token){
        std::vector<std::string> parts = Utils::Utils::tokenize(file, ".");
        
        if(parts.size() != 2){
            throw std::runtime_error("Bad Syntax used in Aux file!");
        }
        
        if(parts[1] == "nets"){
            m_nets_file = file;
        } else if (parts[1] == "blocks"){
            m_blocks_file = file;
        } else if (parts[1] == "pl"){
            m_pl_file = file;
        } else {
            throw std::runtime_error("Unknown Format used in Aux file!");
        }
    }
    
    this->read_blocks();
    this->calc_estimated_die_area();
    this->locate_biggest_macro();
    this->calculate_gcd();
    this->read_pl();
    //this->read_nets();
}

/**
 * @brief Read Bookshelf Blocks File
 */
void Bookshelf::read_blocks()
{
    this->get_verbose() && std::cout << "[Debug]: Reading Bookshelf Blocks File" << std::endl;
    
    if(!boost::filesystem::exists(m_blocks_file)){
        throw std::runtime_error("Can not open blocks file (" + m_blocks_file + ")");
    }

    std::ifstream blocks_stream(m_blocks_file);
    std::vector<std::string> blocks_content;
    std::string line;

    while(std::getline(blocks_stream, line)){
        blocks_content.push_back(line);
    }
    blocks_stream.close();
    
    if(blocks_content[0] != "UCSC blocks 1.0"){
        throw std::runtime_error("Bookshelf Blocks Header invalid!");
    }

    for(auto line : blocks_content){
        if(line.empty()){
            continue;
        }

        std::vector<std::string> token = Utils::Utils::tokenize(line, " ");

        if(line[0] == '#'){
            continue;

        } else if((token[0].compare("UCSC") == 0)){
            continue;

        }   else if(token[0] == "NumSoftRectangularBlocks"){
            continue;

        } else if (token[0] == "NumHardRectilinearBlocks"){
            m_expected_macros = std::stoi(token[2]);

        } else if(token[0] == "NumTerminals"){
            m_excepted_terminals = std::stoi(token[2]);

        } else if (token[1] == "hardrectilinear"){
            std::string name = token[0];

            std::vector<std::string> macro_token = Utils::Utils::tokenize(line, "(");
            assert (macro_token.size() == 5);
            std::string expr = macro_token[3];

            size_t comma_pos = expr.find(',');
            size_t height = std::stoi(expr.substr(0, comma_pos));
            size_t width = std::stoi(expr.substr(comma_pos+1, expr.length()));

            MacroDefinition macro_def;
            macro_def.width = width;
            macro_def.height = height;
            macro_def.name = name;
            m_macro_definitions.push_back(macro_def);

        } else if (token[1] == "terminal"){
            TerminalDefinition terminal_definition;
            terminal_definition.name = token[0];
            terminal_definition.pos_x = 0;
            terminal_definition.pos_y = 0;
            m_terminal_definitions.push_back(terminal_definition);

        } else {
            std::cout << line << std::endl;
            assert (0);
        }
    }
    
    if(m_excepted_terminals != m_terminal_definitions.size()){
        throw std::runtime_error("Invalid Number of Terminal Specified in Blocks File!");
    }
    
    if(m_expected_macros != m_macro_definitions.size()){
        throw std::runtime_error("Invalid Number of Macros Specified in Blocks File!");
    }
}

/**
 * @brief Read Bookshelf Nets File
 */
void Bookshelf::read_nets()
{
    this->get_verbose() && std::cout << "[Debug]: Reading Bookshelf Nets File" << std::endl;
    
    if(!boost::filesystem::exists(m_nets_file)){
        throw std::runtime_error("Can not open nets file (" + m_nets_file + ")");
    }
    
    std::ifstream nets_stream(m_nets_file);
    std::vector<std::string> nets_content;
    std::string line;
    
    size_t num_nets = 0;
    size_t num_pins = 0;
    
    while(std::getline(nets_stream, line)){
        nets_content.push_back(line);
    }
    nets_stream.close();
    
    if(nets_content[0] != "UCLA nets 1.0"){
        throw std::runtime_error("Bookshelf Nets Header invalid!");
    }
    
    for(size_t i = 0; i < nets_content.size(); ++i){
        // Empty Line
        if (nets_content[i].size () == 0){
            continue;
        }
        
        std::vector<std::string> token = Utils::Utils::tokenize(nets_content[i], " ");
        
        // Comment
        if (token[0] == "#"){
            continue;
        // NumNets : 42
        } else if (token[0] == "NumNets"){
            assert (token[1] == ":");
            num_nets = std::stoi(token[2]);
        // NumPins : 42
        } else if (token[0] == "NumPins"){
            assert (token[1] == ":");
            num_pins = std::stoi(token[2]);
        } else if (token[0] == "NetDegree"){
            num_nets++;

            assert (token[1] == ":");
            size_t sub_net_degree = std::stoi(token[2]);
            size_t j = i+1;
            std::vector<Node*> nodes;
            for(; j < nets_content.size(); ++j){
                std::vector<std::string> sub_token = Utils::Utils::tokenize(nets_content[j], " ");
               
                if (sub_token[0] == "NetDegree"){
                    assert (sub_net_degree == nodes.size());
                    break;
                }

                // Comment
                if (sub_token[0] == "#"){
                    continue;
                }

                std::string id = sub_token[0];
                Terminal* t = nullptr;
                if (this->find_macro(id)){
                    std::string direction = sub_token[1];
                    // Pin has relative position
                    if (sub_token.size() > 2) {
                        assert (sub_token[2] == ":");
                        std::string rel_pos_x = sub_token[3];
                        std::string rel_pos_y = sub_token[4];
                        std::string pin = rel_pos_x + "_" + rel_pos_y;
                        this->add_pin_to_macro(id, pin, direction, rel_pos_x, rel_pos_y);
                    // Pin is center
                    } else {
                        this->add_pin_to_macro(id, "center", direction, "", "");
                    }
                    Node* n = new Node(this->find_macro(id));
                    nodes.push_back(n);
                    num_pins++;
                } else if ((t = this->find_terminal(id))){
                    Node* n = new Node(this->find_terminal(id));
                    nodes.push_back(n);
                    
                } else {
                    assert (0);
                }
            }
            // Sub-State Machine ended -continue global
            i = j-1;

            assert (nodes.size() >= 2);
            char from_case = 0;
            if (nodes[0]->is_terminal()){
                from_case = 't';
            } else if (nodes[0]->has_macro()){
                from_case = 'm';
            } else if (nodes[0]->has_cell()){
                from_case = 'c';
            } else {
                assert (0);
            }

            for(size_t node_index = 1; node_index < nodes.size(); ++node_index){
                char to_case = 0;
                if (nodes[node_index]->is_terminal()){
                    to_case = 't';
                } else if (nodes[node_index]->has_macro()){
                    to_case = 'm';
                } else if (nodes[node_index]->has_cell()){
                    to_case = 'c';
                } else {
                    assert (0);
                }

                if (from_case == 'm' && to_case == 'm'){
                    m_tree->insert_edge<Macro, Macro>(nodes[0]->get_macro(), nodes[node_index]->get_macro(), "", "", "");
                } else if (from_case == 't' && to_case == 'm'){
                    m_tree->insert_edge<Terminal, Macro>(nodes[0]->get_terminal(), nodes[node_index]->get_macro(), "", "", "");
                } else if (from_case == 'm' && to_case == 't'){
                    m_tree->insert_edge<Macro, Terminal>(nodes[0]->get_macro(), nodes[node_index]->get_terminal(), "", "", "");
                } else {
                    assert (0);
                }
            }
            
            for(auto itor: nodes){
                delete itor; itor = nullptr;
            }
        }
    }
}

/**
 * @brief Read Bookshelf Place File
 */
void Bookshelf::read_pl()
{
    this->get_verbose() && std::cout << "[Debug]: Reading Bookshelf Place File" << std::endl;
    
    bool force_free = true;
    
    if(!boost::filesystem::exists(m_pl_file)){
        throw std::runtime_error("Can not open place file (" + m_pl_file + ")");
    }
    
    std::ifstream place_stream(m_pl_file);
    std::vector<std::string> place_content;
    std::string line;
    
    std::vector<std::string> processed_terminals;
    std::vector<std::string> processed_macros;
    
    this->calc_estimated_die_area();
    
    size_t max_size = std::max(m_max_h, m_max_w);
        
    size_t xy = std::ceil(sqrt(m_estimated_area))+max_size;
    m_lut->init_lookup_table(xy, xy);
    
    std::cout << "Using Grid: " << xy/m_gcd_h << std::endl;
    
    while(std::getline(place_stream, line)){
        place_content.push_back(line);
    }
    place_stream.close();
    
    for(auto& line : place_content){
        if(line.empty()){
            continue;
        }

        std::vector<std::string> token = Utils::Utils::tokenize(line, " ");

        if(line[0] == '#'){
            continue;
        } else if((token[0].compare("UCLA") == 0)){
            continue;
        } else if ((token[0].compare("UCSC") == 0)){
            continue;
        }
        
        auto macro = std::find_if(m_macro_definitions.begin(), m_macro_definitions.end(), 
                    [token](MacroDefinition const & def)
                    {
                        return def.name == token[0];
                    });
        
        auto terminal = std::find_if(m_terminal_definitions.begin(), m_terminal_definitions.end(),
                    [token](TerminalDefinition const & def)
                    {
                        return token[0] == def.name;
                    });
        
        if(macro != m_macro_definitions.end()){
            size_t x = std::stoi(token[1]);
            size_t y = std::stoi(token[2]);
            processed_macros.push_back(macro->name);
            
            // Placed Macro
            if(force_free) {
                Macro* m = new Macro(macro->name,
                                     macro->name,
                                     macro->width,
                                     macro->height/*,
                                     xy/m_gcd_w,
                                     xy/m_gcd_h,
                                     m_lut*/);
                m_macros.push_back(m);
            } else if((x != 0) || (y != 0)){
               // m_macros.push_back(new Macro(name,name, width, heigth, x, y, 0));
            // Free Macro
            } else {
                m_macros.push_back(new Macro(macro->name,
                                             macro->name,
                                             macro->width,
                                             macro->height));
            }
            
        } else if(terminal != m_terminal_definitions.end()){
            size_t x = std::stoi(token[1]);
            size_t y = std::stoi(token[2]);
            std::string name = token[0];
            processed_terminals.push_back(name);
            
            // Placed Terminal
            if((x != 0) || (y != 0)){
                Terminal* tmp = new Terminal(name, x, y, e_pin_direction::eUnknown);
                m_terminals.push_back(tmp);
            // Free Terminal
            } else {
                Terminal* tmp = new Terminal(name, e_pin_direction::eUnknown);
                m_terminals.push_back(tmp);
            }
        } else {
            std::cout << line << std::endl;
            assert (0);
        }
    }
   
    if(processed_macros.size() < m_expected_macros){
        throw std::runtime_error("Not all Macros have been processed!");
    } 
    if (processed_terminals.size() < m_excepted_terminals){
        throw std::runtime_error("Not all Terminals have been processed!");
    }
}

/**
 * @brief Get Access to the used Macro in the design
 * 
 * @return std::vector< Placer::Macro* >
 */
std::vector<Macro *> Bookshelf::get_macros()
{
    return m_macros;
}

/**
 * @brief Set Macros for results export
 * 
 * @param macros The placed macros
 */
void Bookshelf::set_macros(std::vector<Macro *>& macros)
{
    m_macros.clear();
    std::copy(macros.begin(), macros.end(), std::back_inserter(m_macros));
}

/**
 * @brief Get Access to the used Terminal in the design
 * 
 * @return std::vector< Placer::Terminal* >
 */
std::vector<Terminal *> Bookshelf::get_terminals()
{
    return m_terminals;
}

/**
 * @brief Set Terminal for results export
 * 
 * @param terminals The placed terminals
 */
void Bookshelf::set_terminals(std::vector<Terminal *>& terminals)
{
    m_terminals.clear();
    std::copy(terminals.begin(), terminals.end(), std::back_inserter(m_terminals));
}

/**
 * @brief Get access to the connectivity tree
 *
 * @return Placer::Tree*
 */
Tree* Bookshelf::get_tree()
{
    return m_tree;
}

/**
 * @brief Set Tree for results export
 * 
 * @param tree The tree for export
 */
void Bookshelf::set_tree(Tree* tree)
{
    assert (tree != nullptr);
    
    m_tree = tree;
}

/**
 * @brief Calculate the minimum needed die area for the design
 */
void Bookshelf::calc_estimated_die_area ()
{
    m_estimated_area = 0;
  
    for (auto itor: m_macro_definitions){
        size_t x = itor.width;
        size_t y = itor.height;

        m_estimated_area += (x*y);
    }
}

/**
 * @brief Get estimated minimum die area for design
 * 
 * @return size_t
 */
size_t Bookshelf::get_estimated_area ()
{
  return m_estimated_area;
}

/**
 * @brief Get Name of the current design
 * 
 * @return std::string
 */
std::string Bookshelf::get_design_name () const
{
    return m_design_name;
}

/**
 * @brief Search for existing Macro
 * 
 * @param name Macro Identifier
 * @return Placer::Macro*
 */
Macro* Bookshelf::find_macro(std::string const & name)
{
    Macro* ret_val = nullptr;
    
    for (auto itor: m_macros){
        if (itor->get_name() == name){
            ret_val = itor;
            break;
        }
    }
    return ret_val;
}

/**
 * @brief Search for existing Terminal
 * 
 * @param name Terminal Identifier
 * @return Placer::Terminal*
 */
Terminal* Bookshelf::find_terminal(std::string const & name)
{
    Terminal* ret_val = nullptr;
    
    for (auto itor: m_terminals){
        if (itor->get_name() == name){
            ret_val = itor;
            break;
        }
    }
    return ret_val;
}

/**
 * @brief Add Pin to Macro
 * 
 * @param macro Macro Name
 * @param pin Pin Name
 * @param rel_pos_x Relative X Position of Pin to Macro Center
 * @param rel_pos_y Relative Y Position of Pin to Macro Center
 * 
 * @return void
 */
void Bookshelf::add_pin_to_macro(std::string const & macro,
                                 std::string const & pin,
                                 std::string const & direction,
                                 std::string const & rel_pos_x,
                                 std::string const & rel_pos_y)
{
    try {
        Macro* m = this->find_macro(macro);
        Pin* p = nullptr;
        Terminal* t = this->find_terminal(macro);
        e_pin_direction dir = eUnknown;
        
        if (direction == "B"){
            dir = eBidirectional;
        } else {
            assert (0);
        }
        
        if (!((m != nullptr) || (t != nullptr))){
            std::string msg = "Macro/Terminal " + macro + " does not exist";
            throw std::runtime_error(msg);
        }

        if (m){
            // Pin does not yet exist
            if (!m->get_pin(pin)){
                p = new Pin(pin, macro, dir);
                assert (p != nullptr);
                
                if (!rel_pos_x.empty()){
                    std::string x = rel_pos_x.substr(1 ,rel_pos_x.size());
                    int x_i = std::stoi(x);
                    p->set_x_offset(x_i);
                } 
                if (!rel_pos_y.empty()){
                    std::string y = rel_pos_x.substr(1 ,rel_pos_y.size());
                    int y_i = std::stoi(y);
                    p->set_y_offset(y_i);
                }

                m->add_pin(p);
            }
        }

    } catch (std::exception const & exp){
        std::cerr << exp.what() << std::endl;
        exit (0);
    }
}

/**
 * @brief Dump Placement of Macros to Placement File
 */
void Bookshelf::write_placement()
{
    this->write_blocks();
    this->write_nets();
    this->write_pl();
}

/**
 * @brief Write Blocks File
 */
void Bookshelf::write_blocks()
{
    std::string filename = "export_" + this->get_design_name() + ".blk";

    std::ofstream blkFile(filename);

    std::stringstream feed;
    feed << "UCSC blocks 1.0" << std::endl;
    feed << "# Created " << Utils::Utils::get_current_time();
    feed << "# User " <<  Utils::Utils::get_current_user() << std::endl;
    feed << "# Platform " << Utils::Utils::get_plattform() << std::endl << std::endl;

    feed << "NumSoftRectangularBlocks : 0" << std::endl;
    feed << "NumHardRectilinearBlocks : " << m_macros.size() << std::endl;
    feed << "NumTerminals : " << m_terminals.size() << std::endl << std::endl;

    for(auto& curMacro : m_macros) {
        feed << curMacro->get_id() << " hardrectilinear 4 ";
        feed << "(0, 0) ";
        feed << "(0, " << curMacro->get_width().get_numeral_uint() << ") ";
        feed << "(" << curMacro->get_height().get_numeral_uint() << ", " << curMacro->get_width().get_numeral_uint() << ") ";
        feed << "(" << curMacro->get_height().get_numeral_uint() << ",0) " << std::endl;
    }

    feed << std::endl;

    for(auto terminal: m_terminals){
        feed << terminal->get_name()  << " terminal" << std::endl;
    }

    blkFile << feed.str();
    blkFile.close();
    feed.clear();
}

/**
 * @brief Write Nets File
 */
void Bookshelf::write_nets()
{
     std::string filename = "export_" + this->get_design_name() + ".nets";
    
    std::ofstream netsFile(filename);

    std::stringstream feed;
    feed << "UCSC nets 1.0" << std::endl;
    feed << "# Created " << Utils::Utils::get_current_time() << std::endl;
    feed << "# User " << Utils::Utils::get_current_user() << std::endl;
    feed << "# Platform " << Utils::Utils::get_plattform() << std::endl;
    feed << std::endl;
    feed << "NumNets : " << m_tree->get_edges().size() << std::endl;
    feed << "NumPins: "  << (m_tree->get_edges().size() * 2) << std::endl;
    feed << std::endl;

    for(auto edge: m_tree->get_edges()){
        feed << "NetDegree : 2" << std::endl;
        feed << edge->get_name() << " B\t : %-50.0 %-50.0" << std::endl; 
    }
    netsFile << feed.str();
    netsFile.close();
}

/**
 * @brief Write Placement File
 */
void Bookshelf::write_pl()
{
     std::string filename = "export_" + this->get_design_name() + ".pl";
    
     std::ofstream plFile(filename);

    std::stringstream feed;
    feed << "UCSC pl 1.0" << std::endl;
    feed << "# Created " << Utils::Utils::get_current_time();
    feed << "# User " << Utils::Utils::get_current_user() << std::endl;
    feed << "# Platform " << Utils::Utils::get_plattform() << std::endl;
    feed << std::endl;

    for(auto macro: m_macros){
        feed << macro->get_id() << "\t" << "0" << "\t" << "0" << std::endl; 
    }
    feed << std::endl;

    for(auto terminal: m_terminals){
        feed << terminal->get_name() << "\t 0 \t 0" << std::endl;
    }

    plFile << feed.str();
    plFile.close();
}

void Bookshelf::calculate_gcd()
{
    std::vector<size_t> w;
    std::vector<size_t> h;
    for (auto xy : m_macro_definitions){
        w.push_back(xy.width);
        h.push_back(xy.height);
    }
    
    m_gcd_w = Utils::Utils::gcd(w);
    m_gcd_h = Utils::Utils::gcd(h);
    
    std::cout << "GCD W: " << m_gcd_w << std::endl;
    std::cout << "GCD H: " << m_gcd_h << std::endl;
}


void Bookshelf::locate_biggest_macro()
{
    m_max_w = 0;
    m_max_h = 0;
    
    for (auto m : m_macro_definitions){
        if (m.width > m_max_w){
            m_max_w = m.width;
        }
        if (m.height > m_max_h){
            m_max_h = m.height;
        }
    }
    
    std::cout << "MAX_H: " << m_max_h << std::endl;
    std::cout << "MAX_W: " << m_max_w << std::endl;
}
