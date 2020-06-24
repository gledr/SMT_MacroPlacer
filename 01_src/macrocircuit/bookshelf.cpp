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
using namespace Placer::Utils;

/**
 * @brief Constructor
 */
Bookshelf::Bookshelf():
    Object()
{
    m_tree = new Tree();
    m_logger = Logger::getInstance();
    m_design_read = false;
}

/**
 * @brief Destructor
 */
Bookshelf::~Bookshelf()
{
    m_tree = nullptr;
    m_logger = nullptr;
}

/**
 * @brief Read Bookshelf Format Files
 */
void Bookshelf::read_files()
{
    this->read_aux();
    this->read_blocks();
    this->read_pl();
    this->read_nets();

    m_design_read = true;
}

/**
 * @brief Read Bookshelf Aux File
 */
void Bookshelf::read_aux()
{
    std::string aux_file = this->get_bookshelf_file();

    m_logger->bookshelf_read_aux(aux_file);

    if(!boost::filesystem::exists(aux_file)){
        throw PlacerException("Bookshelf Aux File does not exits!");
    }

    std::vector<std::string> aux_content;
    std::string line;

    std::ifstream aux_filestream(aux_file);
    while(std::getline(aux_filestream, line)){
        aux_content.push_back(line);
    }
    aux_filestream.close();

    if(aux_content.size() != 1){
        throw PlacerException("Aux file must contain excatly one line!");
    }

    std::vector<std::string> token = Utils::Utils::tokenize(aux_content[0], " ");

    m_design_name = Utils::Utils::tokenize(aux_content[0], ".")[0];

    for(auto& file : token){
        std::vector<std::string> parts = Utils::Utils::tokenize(file, ".");
        
        if(parts.size() != 2){
            throw PlacerException("Bad Syntax used in Aux file!");
        }

        if(parts[1] == "nets"){
            m_nets_file = file;
        } else if (parts[1] == "blocks"){
            m_blocks_file = file;
        } else if (parts[1] == "pl"){
            m_pl_file = file;
        } else {
            std::cout << parts[1] << std::endl;
            throw PlacerException("Unknown Format used in Aux file!");
        }
    }
}

/**
 * @brief Read Bookshelf Blocks File
 */
void Bookshelf::read_blocks()
{
    m_logger->bookshelf_read_blocks(m_blocks_file);

    if(!boost::filesystem::exists(m_blocks_file)){
        throw PlacerException("Can not open blocks file (" + m_blocks_file + ")");
    }

    std::ifstream blocks_stream(m_blocks_file);
    std::vector<std::string> blocks_content;
    std::string line;

    while(std::getline(blocks_stream, line)){
        blocks_content.push_back(line);
    }
    blocks_stream.close();

    if(blocks_content[0] != "UCSC blocks 1.0"){
        throw PlacerException("Bookshelf Blocks Header invalid!");
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
            assertion_check (macro_token.size() == 5);
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
            notsupported_check(line);
        }
    }

    if(m_excepted_terminals != m_terminal_definitions.size()){
        throw PlacerException("Invalid Number of Terminal Specified in Blocks File!");
    }

    if(m_expected_macros != m_macro_definitions.size()){
        throw PlacerException
        ("Invalid Number of Macros Specified in Blocks File!");
    }

    this->calc_estimated_die_area();
}

/**
 * @brief Read Bookshelf Nets File
 */
void Bookshelf::read_nets()
{
    m_logger->bookshelf_read_nets(m_nets_file);

    if(!boost::filesystem::exists(m_nets_file)){
        throw PlacerException("Can not open nets file (" + m_nets_file + ")");
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
        throw PlacerException("Bookshelf Nets Header invalid!");
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
            assertion_check (token[1] == ":");
            num_nets = std::stoi(token[2]);
        // NumPins : 42
        } else if (token[0] == "NumPins"){
            assertion_check (token[1] == ":");
            num_pins = std::stoi(token[2]);
        } else if (token[0] == "NetDegree"){
            num_nets++;

            assertion_check (token[1] == ":");
            size_t sub_net_degree = std::stoi(token[2]);
            size_t j = i+1;
            std::vector<Node*> nodes;
            std::vector<std::string> pins;
            for(; j < nets_content.size(); ++j){
                std::vector<std::string> sub_token = Utils::Utils::tokenize(nets_content[j], " ");

                if (sub_token[0] == "NetDegree"){
                    assertion_check (sub_net_degree == nodes.size());
                    break;
                }

                // Comment
                if (sub_token[0] == "#"){
                    continue;
                }

                std::string id = sub_token[0];
                if (this->has_macro(id)){
                    std::string direction = sub_token[1];
                    // Pin has relative position
                    if (!this->get_minimize_die_mode()){
                        if (sub_token.size() > 2) {
                            assertion_check (sub_token[2] == ":");
                            std::string rel_pos_x = sub_token[3];
                            std::string rel_pos_y = sub_token[4];
                            std::string pin = rel_pos_x + "_" + rel_pos_y;
                            this->add_pin_to_macro(id, pin, direction, rel_pos_x, rel_pos_y);
                            pins.push_back(pin);
                        // Pin is free
                        } else {
                            std::string pin =  "%0.0_%0.0";
                            this->add_pin_to_macro(id, pin, direction, "", "");
                            pins.push_back(pin);
                        }
                    }
                    Node* n = new Node(this->find_macro(id));
                    nodes.push_back(n);
                    num_pins++;

                } else if (this->has_terminal(id)){
                    Node* n = new Node(this->find_terminal(id));
                    nodes.push_back(n);
                    pins.push_back(id);
                    
                } else {
                    notsupported_check("Only Terminals and Macros are allowed!");
                }
            }
            // Sub-State Machine ended -continue global
            i = j-1;

            assertion_check (nodes.size() >= 2);
            char from_case = 0;
            if (nodes[0]->is_terminal()){
                from_case = 't';
            } else if (nodes[0]->has_macro()){
                from_case = 'm';
            } else if (nodes[0]->has_cell()){
                from_case = 'c';
            } else {
                notimplemented_check();
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
                    notimplemented_check();
                }
                std::string from_pin;
                std::string to_pin;
                if (this->get_minimize_die_mode()){
                    from_pin  = "center";
                    to_pin = "center";
                } else {
                    from_pin  = pins[0];
                    to_pin = pins[node_index];
                }

                if (from_case == 'm' && to_case == 'm'){
                    Macro* from = nodes[0]->get_macro();
                    nullpointer_check(from);

                    Macro* to = nodes[node_index]->get_macro();
                    nullpointer_check(to);

                    m_tree->insert_edge<Macro, Macro>(from, to, from_pin, to_pin, "");
                } else if (from_case == 't' && to_case == 'm'){
                    Terminal* from = nodes[0]->get_terminal();
                    nullpointer_check(from);

                    Macro* to = nodes[node_index]->get_macro();
                    nullpointer_check(to);

                    if (!(this->get_skip_power_network() && from->is_power_terminal())){
                        m_tree->insert_edge<Terminal, Macro>(from, to, from_pin, to_pin, "");
                    }
                } else if (from_case == 'm' && to_case == 't'){
                    Macro* from = nodes[0]->get_macro();
                    nullpointer_check(from);

                    Terminal* to = nodes[node_index]->get_terminal();
                    nullpointer_check(to);

                    if (!(this->get_skip_power_network() && to->is_power_terminal())){
                        m_tree->insert_edge<Macro, Terminal>(from, to, from_pin, to_pin, "");
                    }
                } else {
                    notimplemented_check();
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
    m_logger->bookshelf_read_place(m_pl_file);

    bool force_free = true;

    if(!boost::filesystem::exists(m_pl_file)){
        throw PlacerException("Can not open place file (" + m_pl_file + ")");
    }

    std::ifstream place_stream(m_pl_file);
    std::vector<std::string> place_content;
    std::string line;

    std::vector<std::string> processed_terminals;
    std::vector<std::string> processed_macros;

    this->calc_estimated_die_area();

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
                                     macro->height);
                nullpointer_check(m);
                m_macros.push_back(m);
                
                Pin* p = new Pin("center", macro->name, eBidirectional);
                nullpointer_check(p);
                m->add_pin(p);
            } else if((x != 0) || (y != 0)){
                assert (0);
               // m_macros.push_back(new Macro(name,name, width, heigth, x, y, 0));
            // Free Macro
            } else {
               Macro* m = new Macro(macro->name,
                                     macro->name,
                                     macro->width,
                                     macro->height);
                nullpointer_check(m);
                m_macros.push_back(m);

                if (this->get_minimize_die_mode()){
                    Pin* p = new Pin("center", macro->name, eBidirectional);
                    nullpointer_check(p);
                    m->add_pin(p);
                }
            }

        } else if(terminal != m_terminal_definitions.end()){
            std::string name = token[0];
            processed_terminals.push_back(name);

            if ((token.size() == 3) && !this->get_free_terminals()){
                size_t x = std::stoi(token[1]);
                size_t y = std::stoi(token[2]);

                Terminal* tmp = new Terminal(name, x, y, e_pin_direction::eUnknown, eNorth);
                m_terminals.push_back(tmp);
            } else {
               Terminal* tmp = new Terminal(name, e_pin_direction::eUnknown);
                m_terminals.push_back(tmp);
            }
        } else {
            notsupported_check(line);
        }
    }

    if(processed_macros.size() < m_expected_macros){
        throw PlacerException("Not all Macros have been processed!");
    }
    if (processed_terminals.size() < m_excepted_terminals){
        throw PlacerException("Not all Terminals have been processed!");
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
    nullpointer_check (tree);
    
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
    assert (ret_val != nullptr);
    assert (name == ret_val->get_id());
    
    return ret_val;
}

/**
 * @brief Check if Macro Exists
 * 
 * @param name Macro Identifier
 * @return bool
 */
bool Bookshelf::has_macro(std::string const & name)
{
    Macro* ret_val = nullptr;

    for (auto itor: m_macros){
        if (itor->get_name() == name){
            ret_val = itor;
            break;
        }
    }

    return ret_val != nullptr;
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

    assertion_check(name == ret_val->get_name());

    return ret_val;
}

/**
 * @brief Check for existing Terminal
 * 
 * @param name Terminal Identifier
 * @return Placer::Terminal*
 */
bool Bookshelf::has_terminal(std::string const & name)
{
    Terminal* ret_val = nullptr;

    for (auto itor: m_terminals){
        if (itor->get_name() == name){
            ret_val = itor;
            break;
        }
    }
    return ret_val != nullptr;
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
        Pin* p = nullptr;
        e_pin_direction dir = eUnknown;

        if (direction == "B"){
            dir = eBidirectional;
        } else {
            notimplemented_check();
        }

        if (!(this->has_macro(macro) || this->has_terminal(macro))){
            std::string msg = "Macro/Terminal " + macro + " does not exist";
            throw PlacerException(msg);
        }

        if (this->has_macro(macro)){
            Macro* m = this->find_macro(macro);
            // Pin does not yet exist
            if (!m->has_pin(pin)){
                p = new Pin(pin, macro, dir);
                nullpointer_check (p);

                if (!rel_pos_x.empty()){
                    std::string x = rel_pos_x.substr(1 ,rel_pos_x.size());
                    int x_i = std::stoi(x);
                    p->set_x_offset_percentage(x_i);
                }
                if (!rel_pos_y.empty()){
                    std::string y = rel_pos_x.substr(1 ,rel_pos_y.size());
                    int y_i = std::stoi(y);
                    p->set_y_offset_percentage(y_i);
                }

                m->add_pin(p);
            }
        }

    } catch (std::exception const & exp){
        throw PlacerException(exp.what());
    }
}

/**
 * @brief Dump Placement of Macros to Placement File
 */
void Bookshelf::write_placement(size_t const solution_id)
{
    if (!boost::filesystem::exists(this->get_active_results_directory())){
        boost::filesystem::create_directories(this->get_active_results_directory());
    }
    boost::filesystem::current_path(this->get_active_results_directory());

    this->write_aux();
    this->write_blocks();
    this->write_nets();
    this->write_pl(solution_id);
}

/**
 * @brief Write Aux File
 */
void Bookshelf::write_aux()
{
    std::string filename = this->get_bookshelf_export() + ".aux";
    m_logger->bookshelf_write_aux(filename);

    std::ofstream auxFile(filename);
    auxFile << this->get_bookshelf_export() << ".pl ";
    auxFile << this->get_bookshelf_export() << ".blocks ";
    auxFile << this->get_bookshelf_export() << ".nets ";
    auxFile.close();
}

/**
 * @brief Write Blocks File
 */
void Bookshelf::write_blocks()
{
    std::string filename = this->get_bookshelf_export() + ".blocks";
    m_logger->bookshelf_write_blocks(filename);

    std::ofstream blkFile(filename);

    std::stringstream feed;
    feed << "UCSC blocks 1.0" << std::endl;
    feed << "# Created " << Utils::Utils::get_current_time();
    feed << "# User " <<  Utils::Utils::get_current_user() << std::endl;
    feed << "# Platform " << Utils::Utils::get_plattform() << std::endl;
    feed << "# Exported by SMT_MacroPlacer" << std::endl << std::endl;
    
    feed << "NumSoftRectangularBlocks : 0" << std::endl;
    feed << "NumHardRectilinearBlocks : " << m_macros.size() << std::endl;
    feed << "NumTerminals : " << m_terminals.size() << std::endl << std::endl;

    for(auto& curMacro : m_macros) {
        //feed << "# " << curMacro->get_id() << std::endl;
        if (this->check_name_dac2002(curMacro->get_id())){
            feed << curMacro->get_id() << " hardrectilinear 4 ";
        } else {
            feed << "o" << curMacro->get_key() << " hardrectilinear 4 ";
        }
       
        feed << "(0, 0) ";
        feed << "(0, " << curMacro->get_width().get_numeral_uint() << ") ";
        feed << "(" << curMacro->get_height().get_numeral_uint() << ", " << curMacro->get_width().get_numeral_uint() << ") ";
        feed << "(" << curMacro->get_height().get_numeral_uint() << ",0) " << std::endl;
    }
    feed << std::endl;

    for(auto terminal: m_terminals){
        //feed << "# " << terminal->get_name() << std::endl;
        if (this->check_name_dac2002(terminal->get_name())){
            feed << terminal->get_name()  << " terminal" << std::endl;
        } else {
            feed << "o" << terminal->get_key()  << " terminal" << std::endl;
        }
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
    std::string filename = this->get_bookshelf_export() + ".nets";
    m_logger->bookshelf_write_nets(filename);
    
    if (this->get_minimize_die_mode()){
        boost::filesystem::copy(this->get_working_directory() + "/" + m_nets_file, filename);
    } else {
           std::ofstream netsFile(filename);

        auto steiner_tree = m_tree->get_steiner_tree();

        size_t pin_cnt = 0;
        for (auto itor: steiner_tree){
            pin_cnt += itor.second.size();
            pin_cnt++;
        }

        std::stringstream feed;
        feed << "UCLA nets 1.0" << std::endl;
        feed << "# Created " << Utils::Utils::get_current_time();
        feed << "# User " << Utils::Utils::get_current_user() << std::endl;
        feed << "# Platform " << Utils::Utils::get_plattform() << std::endl;
        feed << "# Exported by SMT_MacroPlacer" << std::endl;
        feed << std::endl;
        feed << "NumNets : " << steiner_tree.size() << std::endl;
        feed << "NumPins : "  << pin_cnt << std::endl;
        feed << std::endl;

        for(auto itor: steiner_tree){
            size_t netdegree = itor.second.size() + 1;
            feed << "NetDegree : " << netdegree << std::endl;

            std::vector<std::string> token = Utils::Utils::tokenize(itor.first, ":");
            assertion_check (token.size() == 2);
            // Terminal
            std::cout << token[0] << " " << token[1] << std::endl;

            if (this->has_terminal(token[0])){
                Terminal* t = this->find_terminal(token[0]);
                nullpointer_check(t)
                //feed << "# " << t->get_name() << std::endl;
                if (this->check_name_dac2002(t->get_name())){
                    feed << t->get_name() << " B" << std::endl;
                } else {
                    feed << "o" <<  t->get_key() << " B" << std::endl;
                }
                feed << "# The above terminal is a pad" << std::endl;
            } else {
                std::vector<std::string> token2 = Utils::Utils::tokenize(token[1], "_");
                Macro* m = this->find_macro(token[0]);
                nullpointer_check(m);
                size_t width = m->get_width_numeral();
                size_t height = m->get_height_numeral();
                
                double factor_width = 0.0;
                double factor_height = 0.0;
                if (this->get_minimize_die_mode()){
                    factor_width = 0.5;
                    factor_height = 0.5;
                } else {
                    factor_width = (std::stoi(token2[0].substr(1, token2[0].size()))/100.0);
                    factor_height = (std::stoi(token2[1].substr(1, token2[1].size()))/100.0);
                }
            
                //feed << "# " << m->get_id() << std::endl;
                if (this->check_name_dac2002(m->get_id())){
                    feed << m->get_id() << " B : " << width * factor_width << " " << height * factor_height << std::endl;
                } else {
                    feed << "o" << m->get_key() << " B : " << width * factor_width << " " << height * factor_height << std::endl;
                }
            }

            for(auto itor2: itor.second){
                std::vector<std::string> token = Utils::Utils::tokenize(itor2, ":");
                assertion_check (token.size() == 2);
                // Terminal
                std::cout << token[0] << " " << token[1] << std::endl;
                if (token[0] == token[1]){
                    Terminal* t = this->find_terminal(token[0]);
                    nullpointer_check(t);
                    //feed << "# " << t->get_name() << std::endl;
                    if (this->check_name_dac2002(t->get_name())){
                        feed << t->get_name() << " B" << std::endl;
                    } else {
                        feed << "o" <<  t->get_key() << " B" << std::endl;
                    }
                } else {
                    std::vector<std::string> token2 = Utils::Utils::tokenize(token[1], "_");
                    Macro* m = this->find_macro(token[0]);
                    nullpointer_check(m);
                    size_t width = m->get_width().get_numeral_uint();
                    size_t height = m->get_height().get_numeral_uint();
                    
                    double factor_width = 0.0;
                    double factor_height = 0.0;
                    if (this->get_minimize_die_mode()){
                        factor_width = 0.5;
                        factor_height = 0.5;
                    } else {
                        factor_width = (std::stoi(token2[0].substr(1, token2[0].size()))/100.0);
                        factor_height = (std::stoi(token2[1].substr(1, token2[1].size()))/100.0);
                    }

                    //feed << "# " << m->get_id() << std::endl;
                    if (this->check_name_dac2002(m->get_id())){
                        feed << m->get_id() << " B : " << width * factor_width << " " << height * factor_height << std::endl;
                    } else {
                        feed << "o" << m->get_key() << " B : " << width * factor_width << " " << height * factor_height << std::endl;
                    }
                }
            }
        }

        netsFile << feed.str();
        netsFile.close();
    }
}

/**
 * @brief Write Placement File
 */
void Bookshelf::write_pl(size_t const solution_id)
{
    std::string filename = this->get_bookshelf_export() + ".pl";
    m_logger->bookshelf_write_place(filename);

    std::ofstream plFile(filename);

    std::stringstream feed;
    feed << "UCSC pl 1.0" << std::endl;
    feed << "# Created " << Utils::Utils::get_current_time();
    feed << "# User " << Utils::Utils::get_current_user() << std::endl;
    feed << "# Platform " << Utils::Utils::get_plattform() << std::endl;
    feed << "# Exported by SMT_MacroPlacer" << std::endl;
    feed << std::endl;

    for(auto macro: m_macros){
        //feed << "# " << macro->get_id() << std::endl;
        if (macro->is_free()){
            if (macro->has_solution(solution_id)){
                size_t lx = macro->get_solution_lx(solution_id);
                size_t ly = macro->get_solution_ly(solution_id);
                eOrientation o = macro->get_solution_orientation(solution_id);
                size_t h = macro->get_height_numeral();
                size_t w = macro->get_width_numeral();
                
                if (this->check_name_dac2002(macro->get_id())){
                    if (o == eNorth){
                        feed << macro->get_id() << " " << lx << " " << ly << std::endl;
                    } else if (o == eWest){
                        feed << macro->get_id() << " " << lx -h << " " << ly << std::endl;
                    } else if (o == eSouth){
                        feed << macro->get_id() << " " << lx - w << " " << ly - h << std::endl;
                    } else if (o == eEast){
                        feed << macro->get_id() << " " << lx << " " <<  ly - h << std::endl;
                    } else {
                        notimplemented_check();
                    }
                } else {
                    if (o == eNorth){
                        feed << "o" << macro->get_key() << " " << lx << " " << ly << std::endl;
                    } else if (o == eWest){
                        feed << "o" << macro->get_key() << " " << lx -h << " " << ly << std::endl;
                    } else if (o == eSouth){
                        feed << "o" << macro->get_key() << " " << lx - w << " " << ly - h << std::endl;
                    } else if (o == eEast){
                        feed << "o" << macro->get_key() << " " << lx << " " <<  ly - h << std::endl;
                    } else {
                        notimplemented_check();
                    }
                }
            } else {
                if (this->check_name_dac2002(macro->get_id())){
                     feed << macro->get_id() << " " << "0" << " " << "0" << std::endl;   
                } else {
                    feed << "o" <<macro->get_key() << " " << "0" << " " << "0" << std::endl;   
                }
            
            }
        } else {
            if (this->check_name_dac2002(macro->get_id())){
                feed << macro->get_id() << " " << macro->get_lx_numeral() << " " << macro->get_ly_numeral() << std::endl;
            } else {
                feed << "o" << macro->get_key() << " " << macro->get_lx_numeral() << " " << macro->get_ly_numeral() << std::endl;
            }
           
        }
    }
    feed << std::endl;

    for(auto terminal: m_terminals){
        //feed << "# " << terminal->get_name() << std::endl;
        if (terminal->is_free()){
            if (terminal->has_solution(solution_id)){
                if (this->check_name_dac2002(terminal->get_name())){
                    feed << terminal->get_name() << " " << terminal->get_solution_pos_x(solution_id) << " "<< terminal->get_solution_pos_y(solution_id) << std::endl;
                } else {
                    feed << "o" << terminal->get_key() << " " << terminal->get_solution_pos_x(solution_id) << " "<< terminal->get_solution_pos_y(solution_id) << std::endl;
                }
            } else {
                if (this->check_name_dac2002(terminal->get_name())){
                    feed << terminal->get_name() << " 0 0" << std::endl;
                } else {
                    feed << "o" << terminal->get_key() << " 0 0" << std::endl;
                }
            }
        } else {
            if (this->check_name_dac2002(terminal->get_name())){
                feed << terminal->get_name() << " " << terminal->get_pox_x_numerical() << " " << terminal->get_pos_y_numerical() << std::endl;
            } else {
                feed << "o" << terminal->get_key() << " " << terminal->get_pox_x_numerical() << " " << terminal->get_pos_y_numerical() << std::endl;
            }
          
        }
    }

    plFile << feed.str();
    plFile.close();
}

/**
 * @brief Deduce Layout from Placed Terminals
 */
void Bookshelf::deduce_layout()
{
    if (this->get_free_terminals()){
        throw PlacerException("Can not deduce Layout from Free Terminals!");
    }
    
    size_t x = 0;
    size_t y = 0;

    for (Terminal* terminal: m_terminals){
        assertion_check (terminal->get_pos_x().is_numeral());
        assertion_check (terminal->get_pos_y().is_numeral());

        if (terminal->get_pos_x().get_numeral_uint() > x){
            x = terminal->get_pos_x().get_numeral_uint();
        }
        if (terminal->get_pos_y().get_numeral_uint() > y){
            y = terminal->get_pos_y().get_numeral_uint();
        }
    }

    m_logger->deduce_layout(x,y);
    m_could_duduce_layout = (x != 0) && (y != 0);
    m_deduced_layout = std::make_pair(x,y);
}

/**
 * @brief Check if layout could be deduced from placed terminals
 * 
 * @return bool
 */
bool Bookshelf::could_deduce_layout()
{
    return m_could_duduce_layout;
}

/**
 * @brief Deduce Layout from Placed Terminals
 * 
 * @return std::pair< size_t, size_t >
 */
std::pair<size_t, size_t> Bookshelf::get_deduced_layout()
{
    return m_deduced_layout;
}

/**
 * @brief Remove all Edges with Terminals
 */
void Bookshelf::strip_terminals()
{
    m_logger->strip_terminals();

    m_tree->strip_terminals();
}

/**
 * @brief Check if DAC 2002 Naming is used 
 * 
 * @param name Name to check
 * @return bool
 */
bool Bookshelf::check_name_dac2002(std::string const & name)
{
    if ((name[0] == 'p') || (name[0] == 'o')){
        return true;
    } else {
        return false;
    }
}
