//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : macrocircuit.cpp
//
// Date         : 22. December 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : SoC Macro Circuit
//==================================================================
#include "macrocircuit.hpp"

using namespace Placer;
using namespace Placer::Utils;

/**
 * @brief Constructor
 */
MacroCircuit::MacroCircuit():
    Object(),
    m_encode(new EncodingUtils()),
    m_components_non_overlapping(m_encode->get_value(0)),
    m_components_inside_die(m_encode->get_value(0)),
    m_terminals_on_frontier(m_encode->get_value(0)),
    m_terminals_non_overlapping(m_encode->get_value(0))
{
    m_z3_opt = new z3::optimize(m_z3_ctx);
    m_layout = new Layout();
    m_eval = new Evaluate(this);
    m_supplement = new Supplement();
    m_logger = Utils::Logger::getInstance();
    m_timer = new Utils::Timer();
    m_partitioning = new Partitioning();
    m_parquet = new ParquetFrontend();

    m_circuit = nullptr;
    m_solutions = 0;
    m_bookshelf = nullptr;
}

/**
 * @brief Destructor
 */
MacroCircuit::~MacroCircuit()
{
    for(auto itor: m_macros){
        delete itor; itor = nullptr;
    }
    
    for(auto itor: m_partitons){
        delete itor; itor = nullptr;
    }
    
    for(auto itor: m_terminals){
        delete itor; itor = nullptr;
    }

    for(auto itor: m_cells){
        delete itor; itor = nullptr;
    }

    if(m_circuit != nullptr){
        delete m_circuit; m_circuit = nullptr;
    }

    if(m_bookshelf != nullptr){
        delete m_bookshelf; m_bookshelf = nullptr;
    }

    delete m_tree; m_tree = nullptr;
    delete m_z3_opt; m_z3_opt = nullptr;
    delete m_eval; m_eval = nullptr;
    delete m_supplement; m_supplement = nullptr;
    delete m_layout; m_layout = nullptr;
    delete m_partitioning; m_partitioning = nullptr;
    delete m_parquet; m_parquet = nullptr;
    delete m_encode; m_encode = nullptr;
    delete m_timer; m_timer = nullptr;
    delete m_db; m_db = nullptr;

    m_logger = nullptr;
}

/**
 * @brief Build Circuit from LEF/DEF or Bookshelf Files
 */
void MacroCircuit::build_circuit()
{
    try {
        m_supplement->read_supplement_file();
        std::string db_file = this->get_database_dir() + this->get_database_file();
        m_db = new Utils::Database(db_file);
        m_db->init_database();

        if(!this->get_def().empty() && !this->get_lef().empty()) {
           this->build_circuit_lefdef();

        } else if (!this->get_bookshelf_file().empty()){
            this->build_circuit_bookshelf();

        } else {
            notsupported_check("Invalid Input Files Defined!");
        }
    } catch (std::exception const & exp){
        throw PlacerException(exp.what());
    }
}

/**
 * @brief Build Macro Circuit from LEFDEF Input Files
 */
void MacroCircuit::build_circuit_lefdef()
{
     m_circuit = new Circuit::Circuit(this->get_lef(),
                                      this->get_def());
            m_tree = new Tree();
            nullpointer_check (m_tree );
            nullpointer_check (m_circuit);
            this->set_design_name(m_circuit->defDesignName);

            bool found = false;
            for(auto itor: m_circuit->lefSiteStor) {
                if(itor.siteClass() == this->get_site()){
                    m_standard_cell_height = itor.sizeY();
                    found = true;
                    break;
                }
            }

            if(!found){
                throw std::runtime_error("Site (" + this->get_site() + ") not found!");
            }
            if (!this->get_minimize_die_mode()){
                m_layout->set_lx(m_circuit->defDieArea.xl());
                m_layout->set_ux(m_circuit->defDieArea.xh());
                m_layout->set_ly(m_circuit->defDieArea.yl());
                m_layout->set_uy(m_circuit->defDieArea.yh());
            }

            this->create_macro_definitions();
            std::thread area_estimator(&MacroCircuit::area_estimator, this);
            area_estimator.join();

            m_logger->min_die_area(m_estimated_area);

            std::thread macro_worker(&MacroCircuit::add_macros, this);
            macro_worker.join();
            std::thread cell_worker(&MacroCircuit::add_cells, this);
            cell_worker.join();
            std::thread terminal_worker(&MacroCircuit::add_terminals, this);
            terminal_worker.join();

            m_layout->set_units(m_circuit->defUnit);

            this->init_tree(eLEFDEF);
}

/**
 * @brief Build Circuit from Bookshelf Input Files
 */
void MacroCircuit::build_circuit_bookshelf()
{
      m_bookshelf = new Bookshelf();
            m_bookshelf->read_files();
            this->set_design_name(m_bookshelf->get_design_name());
            m_estimated_area = m_bookshelf->get_estimated_area();
            m_logger->min_die_area(m_estimated_area);

            if (!this->get_parquet_fp()){
                if (!this->get_minimize_die_mode()){
                    if (m_bookshelf->could_deduce_layout()){
                        std::pair<size_t, size_t> upper_corner = m_bookshelf->get_deduced_layout();
                        m_layout->set_lx(0);
                        m_layout->set_ux(upper_corner.first);
                        m_layout->set_ly(0);
                        m_layout->set_uy(upper_corner.second);

                    } else {
                           if (!m_supplement->has_layout()){
                        throw std::runtime_error("No Layout Supplement Defined!");
                        } else {
                            SupplementLayout* _layout = m_supplement->get_layout();
                            m_layout->set_lx(_layout->get_lx());
                            m_layout->set_ux(_layout->get_ux());
                            m_layout->set_ly(_layout->get_ly());
                            m_layout->set_uy(_layout->get_uy());
                        }
                    }
                }
            }

            m_layout->set_min_die_predition(m_estimated_area);
            m_macros = m_bookshelf->get_macros();
            m_terminals = m_bookshelf->get_terminals();
            this->set_design_name(m_bookshelf->get_design_name());

            this->init_tree(eBookshelf);
}

/**
 * @brief Initialize Conectivity Tree for Given Input Format
 * 
 * @param type Input Format Type
 */
void MacroCircuit::init_tree(eInputFormat const type)
{
    if (type == eLEFDEF){
          this->build_tree_from_lefdef();
    } else if (type == eBookshelf){
            m_tree = m_bookshelf->get_tree();
    } else {
        notsupported_check("Only LEF/DEF and Bookshelf are supported!");
    }

    m_tree->construct_tree();
    //m_tree->dump();
    m_tree->export_hypergraph();
    //m_tree->visualize();
}

/**
 * @brief Create Macro Definitions from Parser Ouput
 */
void MacroCircuit::create_macro_definitions()
{
    for(auto& itor: m_circuit->defComponentStor){
        if(this->is_macro(itor)){
            auto idx = m_circuit->lefMacroMap.find(itor.name());
            LefDefParser::lefiMacro& lef_data = m_circuit->lefMacroStor[idx->second];
            std::vector<LefDefParser::lefiPin> lef_pins = m_circuit->lefPinStor[idx->second];

            MacroDefinition macro_definition;
            macro_definition.name = itor.name();
            macro_definition.id = itor.id();
            macro_definition.width = lef_data.sizeX();
            macro_definition.height = lef_data.sizeY();

            for(auto pin: lef_pins){
                PinDefinition pin_def;
                pin_def.parent = itor.name();
                pin_def.name = pin.name();
                pin_def.direction = pin.direction();

                macro_definition.pin_definitions.push_back(pin_def);
            }
            m_macro_definitions.push_back(macro_definition);
        }
    }
}

/**
 * @brief Run Partitioning
 */
void MacroCircuit::partitioning()
{
    if(this->get_partitioning()){
        m_logger->run_partitioning();
        m_partitioning->set_problem(m_macros, m_terminals, m_tree);
        m_partitioning->run();
        std::vector<Component*> partitions = m_partitioning->get_partitions();

        for(auto itor: partitions){
            m_partitons.push_back(dynamic_cast<Partition*>(itor));
        }
    }
}

/**
 * @brief Run Encoding
 */
void MacroCircuit::encode()
{
    // TODO Mixture of Macros, Partitions and Cells
    if (this->get_partitioning()){
        std::copy(m_partitons.begin(), m_partitons.end(), std::back_inserter(m_components));
    } else {
        std::copy(m_macros.begin(), m_macros.end(), std::back_inserter(m_components));
    }

    if (this->get_parquet_fp()){
        this->encode_parquet();
    } else {
        this->encode_smt();
    }
}

/**
 * @brief Encode for Running Parquet Backend
 */
void MacroCircuit::encode_parquet()
{
    m_parquet->set_macros(m_macros);
    m_parquet->set_terminals(m_terminals);
    m_parquet->set_tree(m_tree);
    m_parquet->set_layout(m_layout);
    m_parquet->build_db();
}

/**
 * @brief Encode for Solving SMT
 */
void MacroCircuit::encode_smt()
{
    if (this->get_minimize_die_mode()){
        m_layout->set_lx(0);
        m_layout->set_ly(0);
        m_layout->free_uy();
        m_layout->free_ux();
    }

    this->config_z3();
    this->run_encoding();
}

/**
 * @brief Run Placement
 */
void MacroCircuit::place()
{
    m_timer->start_timer("total");
    
    if (this->get_parquet_fp()){
        m_parquet->run_parquet();
        m_parquet->data_from_parquet();
        m_parquet->store_bookshelf_results();
        m_solutions = 1;
    } else {
        this->solve();
    }

    m_timer->stop_timer("total");
}

/**
 * @brief Add Macros using worker thread
 */
void MacroCircuit::add_macros()
{
    m_logger->start_macro_thread();

    for (MacroDefinition macro_definition: m_macro_definitions){
        Macro* m = new Macro(macro_definition.name,
                             macro_definition.id,
                             macro_definition.width,
                             macro_definition.height);

        for (PinDefinition pin_definition: macro_definition.pin_definitions){
            Pin* p = new Pin(pin_definition.name,
                             pin_definition.parent,
                             Pin::string2enum(pin_definition.direction));
            nullpointer_check (p);
            m->add_pin(p);
        }

        if(m_supplement->has_supplement()){
            m->set_supplement(m_supplement);
        }

        m_macros.push_back(m);
        m_id2macro[macro_definition.id] = m;
    }
    m_logger->end_macro_thread();
}

/**
 * @brief Add Standard-Cells using worker thread
 */
void MacroCircuit::add_cells()
{
    m_logger->start_cell_thread();
    
    for(auto& itor: m_circuit->defComponentStor){
        if(this->is_standard_cell(itor)){
            this->add_cell(itor);
        }
    }
    m_logger->end_cell_thread();
}

/**
 * @brief Add Terminals using worker thread
 */
void MacroCircuit::add_terminals()
{
    m_logger->start_terminal_thread();

    for(auto & itor: m_circuit->defPinStor){
        e_pin_direction direction = Pin::string2enum(itor.direction());
        Terminal* tmp;
        
        if (itor.hasPlacement() && !this->get_free_terminals()){
            int x = itor.placementX();
            int y = itor.placementY();
            tmp = new Terminal(itor.pinName(), x, y, direction);

        } else {
            tmp = new Terminal(itor.pinName(), direction);
        }

        nullpointer_check(tmp);

        m_terminals.push_back(tmp);
        m_id2terminal[itor.pinName()] = tmp;
    }
    
    m_logger->end_terminal_thread();
}

/**
 * @brief Dump solutions to filesystem as image
 */
void MacroCircuit::dump_all()
{
    m_logger->dump_all();

    for(size_t i = 0; i < m_solutions; ++i){
        this->create_image(i);
    }
}

/**
 * @brief Dump best solution to filesystem as image
 */
void MacroCircuit::dump_best()
{
    std::cout << "Dump best" << std::endl;
    std::pair<size_t, size_t> best_hpwl = m_eval->best_hpwl();
    this->create_image(best_hpwl.first);
    std::cout << "Best Solution: HPWL = " << best_hpwl.second << std::endl;
}

/**
 * @brief Create image for particular found solution
 * 
 * @param solution Solution to dump as image
 */
void MacroCircuit::create_image(size_t const solution)
{
    if(!boost::filesystem::exists(this->get_image_directory())){
        boost::filesystem::create_directories(this->get_image_directory());
    }

    boost::filesystem::current_path(this->get_image_directory());

    std::string gnu_plot_script = "script_" + std::to_string(solution) + ".plt";

    size_t die_lx = m_layout->get_lx().get_numeral_uint();
    size_t die_ly = m_layout->get_ly().get_numeral_uint();

    std::stringstream img_name;
    img_name << "placement_" << this->get_design_name() << "_" << solution << ".png";
    std::ofstream gnu_plot_file(gnu_plot_script);
    gnu_plot_file << "set terminal png size 400,300;"  << std::endl;
    gnu_plot_file << "set output '" << img_name.str() << "';" << std::endl;

    if(m_layout->is_free_ux()){
        gnu_plot_file << "set xrange[" << die_lx << ":" << m_layout->get_solution_ux(solution) << "];" << std::endl;
        gnu_plot_file << "set yrange[" << die_ly << ":" << m_layout->get_solution_uy(solution) << "];" << std::endl;
    } else {
        gnu_plot_file << "set xrange[" << die_lx << ":" << m_layout->get_ux().get_numeral_uint()<< "];" << std::endl;
        gnu_plot_file << "set yrange[" << die_ly << ":" << m_layout->get_uy().get_numeral_uint() << "];" << std::endl;
    }

    size_t _lx = 0;
    size_t _ly = 0;
    size_t _ux = 0;
    size_t _uy = 0;

    for(size_t j = 0; j < m_components.size(); ++j){
        gnu_plot_file << "# " << m_components[j]->get_id() << ":" << m_components[j]->get_name() 
                      << " Orientation: " << m_components[j]->get_solution_orientation(solution) << std::endl;

        size_t width = m_components[j]->get_width().get_numeral_uint();
        size_t height = m_components[j]->get_height().get_numeral_uint();

        size_t o  = m_components[j]->get_solution_orientation(solution);
        size_t lx = m_components[j]->get_solution_lx(solution);
        size_t ly = m_components[j]->get_solution_ly(solution);

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

        gnu_plot_file  << "set object " << j+1 << 
                          " rect from " << _lx << "," << _ly <<
                          " to "  << _ux << ","<<_uy << 
                          " lw 5;"<< std::endl;
        }
#ifdef PLOT_TERMINAL
        for(size_t i = 0 ; i < m_terminals.size(); ++ i){
            size_t x = m_terminals[i]->get_solution_pin_pos_x(solution);
            size_t y = m_terminals[i]->get_solution_pin_pos_y(solution);
            
           gnu_plot_file  << "set object " << i+100 << " rect from " << std::to_string(x-0.25) << "," << std::to_string(y-0.25) 
                           << " to "  << std::to_string(x+0.25) << ","<< std::to_string(y+0.25) <<" lw 5;"<< std::endl;
        }
#endif
        gnu_plot_file << "plot x " << std::endl,
        gnu_plot_file.close();
        
        std::string cmd = "gnuplot " + gnu_plot_script;
        system(cmd.c_str());
}

/**
 * @brief Save all found solutions as DEF file
 */
void MacroCircuit::save_all()
{
    for(size_t i = 0; i < m_solutions; ++i){
        std::string name = "placed_" + std::to_string(i) + "_" + this->get_def();
        this->write_def(name, i);
    }
}

/**
 * @brief Save best solution as DEF file
 */
void MacroCircuit::save_best()
{
    std::pair<size_t, size_t> best_hpwl = m_eval->best_hpwl();
    std::string name = "best_" + this->get_def();
    this->write_def(name, best_hpwl.first);
}

/**
 * @brief Build Connectivity Tree from DEF file
 */
void MacroCircuit::build_tree_from_lefdef()
{
    for(auto itor: m_circuit->defNetStor){
        if(itor.numConnections() < 2){
            continue;
        }

        for(size_t i = 1; i < itor.numConnections(); ++i){
            Macro* from_m = nullptr;
            Cell* from_c = nullptr;
            Terminal* from_t = nullptr;

            Macro* to_m = nullptr;
            Cell* to_c = nullptr;
            Terminal* to_t = nullptr;

            std::stringstream _case;
            if(m_id2macro.find(itor.instance(0)) != m_id2macro.end()){
                from_m =  m_id2macro[itor.instance(0)];
                _case << "m";
            } else if (m_id2cell.find(itor.instance(0)) != m_id2cell.end()){
                from_c = m_id2cell[itor.instance(0)];
                 _case << "c";
            } else if (std::string(itor.instance(0)) == "PIN" && m_id2terminal.find(itor.pin(0)) != m_id2terminal.end()){
                from_t = m_id2terminal[itor.pin(0)];
                 _case << "t";
            } else {
                std::cout << itor.instance(0) << std::endl;
                notimplemented_check();
            }

            if(m_id2macro.find(itor.instance(i)) != m_id2macro.end()){
                  _case << "m";
                  to_m = m_id2macro[itor.instance(i)];
            } else if (m_id2cell.find(itor.instance(i)) != m_id2cell.end()){
                to_c = m_id2cell[itor.instance(i)];
                _case << "c";
            } else if (std::string(itor.instance(i)) == "PIN" && m_id2terminal.find(itor.pin(i)) != m_id2terminal.end()){
                to_t = m_id2terminal[itor.pin(i)];
                  _case << "t";
            } else {
                std::cout << itor.instance(i) << std::endl;
                notimplemented_check();
            }

            if(_case.str() == "mm"){
                nullpointer_check (from_m);
                nullpointer_check (to_m);
                m_tree->insert_edge<Macro, Macro>(from_m, to_m, itor.pin(0), itor.pin(i), itor.name());

            } else if (_case.str() == "mc"){
                nullpointer_check (from_m);
                nullpointer_check (to_c);

                m_tree->insert_edge<Macro, Cell>(from_m, to_c, itor.pin(0), itor.pin(i), itor.name());
            } else if (_case.str() == "mt"){
                  nullpointer_check (from_m);
                  nullpointer_check (to_t );
                m_tree->insert_edge<Macro, Terminal>(from_m, to_t, itor.pin(0), itor.pin(i), itor.name());

            } else if (_case.str() == "cm"){
                nullpointer_check (from_m);
                nullpointer_check (to_m);
                m_tree->insert_edge<Cell, Macro>(from_c, to_m, itor.pin(0), itor.pin(i), itor.name());

            } else if (_case.str() == "cc"){
                nullpointer_check (from_c);
                nullpointer_check (to_c);
                m_tree->insert_edge<Cell, Cell>(from_c, to_c, itor.pin(0), itor.pin(i), itor.name());

            } else if (_case.str() == "ct"){
                nullpointer_check (from_c);
                nullpointer_check (to_t);
                m_tree->insert_edge<Cell, Terminal>(from_c, to_t, itor.pin(0), itor.pin(i), itor.name());

            } else if (_case.str() == "tm"){
                nullpointer_check (from_t);
                nullpointer_check (to_m);
                m_tree->insert_edge<Terminal, Macro>(from_t, to_m, itor.pin(0), itor.pin(i), itor.name());

            } else if (_case.str() == "tc"){
                nullpointer_check (from_t);
                nullpointer_check (to_c);
                m_tree->insert_edge<Terminal, Cell>(from_t, to_c, itor.pin(0), itor.pin(i), itor.name());

            } else if (_case.str() == "tt"){
                nullpointer_check (from_t);
                nullpointer_check (to_t);
                m_tree->insert_edge<Terminal, Terminal>(from_t, to_t, itor.pin(0), itor.pin(i), itor.name());

            } else {
                notimplemented_check();
            }
        }
    }

    m_tree->construct_tree();
}

/**
 * @brief Add single cell to macrocircuit
 * 
 * @param cmp Cell to add
 */
void MacroCircuit::add_cell(LefDefParser::defiComponent const & cmp)
{
    std::string name = cmp.name();
    std::string id   = cmp.id();

    Cell* c = new Cell(name, id);
    m_cells.push_back(c);
    m_id2cell[id] = c;
}

/**
 * @brief Export Solution as DEF file
 * 
 * @param name Filename to be used
 * @param solution Solution to be exported
 */
void MacroCircuit::write_def(std::string const & name, size_t const solution)
{
    for(auto itor: m_macros){
        if(!itor->is_free()){
            continue;
        }
        auto idx = m_circuit->defComponentMap.find(itor->get_id());
        LefDefParser::defiComponent& macro = m_circuit->defComponentStor[idx->second];

        macro.setPlacementStatus(DEFI_COMPONENT_PLACED);
        macro.setPlacementLocation(itor->get_solution_lx(solution),
                                   itor->get_solution_ly(solution),
                                   itor->get_solution_orientation(solution));
    }

   /*
    
    for(auto itor: m_terminals){
        if(!itor->is_free()){
            continue;
        }

        size_t pos_x = itor->get_solution_pin_pos_x(solution);
        size_t pos_y = itor->get_solution_pin_pos_y(solution);

        auto idx = m_circuit->defPinMap.find(itor->get_id());
        LefDefParser::defiPin * pin = &m_circuit->defPinStor[idx->second];

        pin->addLayer("METAL1");
        pin->setPlacement(1,1,1,0);
        
    }
    */
    FILE* fp = fopen(name.c_str(), "w");
    m_circuit->WriteDef(fp);
    fclose(fp);
}

/**
 * @brief Check if component is macro
 * 
 * @param macro Component to check
 * @return bool
 */
bool MacroCircuit::is_macro(LefDefParser::defiComponent const & macro)
{
    auto idx = m_circuit->lefMacroMap.find(macro.name());
    LefDefParser::lefiMacro& lef_data = m_circuit->lefMacroStor[idx->second];

    return lef_data.sizeY() != m_standard_cell_height;
}

/**
 * @brief Check if component is standard cell
 * 
 * @param cell Cell to check
 * @return bool
 */
bool MacroCircuit::is_standard_cell(LefDefParser::defiComponent const & cell)
{
    auto idx = m_circuit->lefMacroMap.find(cell.name());
    LefDefParser::lefiMacro& lef_data = m_circuit->lefMacroStor[idx->second];

    return lef_data.sizeY() == m_standard_cell_height;
}

/**
 * @brief Dump macro circuit information to given stream
 * 
 * @param stream Stream to dump to
 */
void MacroCircuit::dump(std::ostream& stream)
{
    stream << std::string(30, '+') << std::endl;
    stream << "Design (" << this->get_design_name() << ")" << std::endl;
    stream << "DieArea (" << m_layout->get_ux().get_numeral_uint() -
                             m_layout->get_lx().get_numeral_uint() 
                          << " x " << m_layout->get_uy().get_numeral_uint()-
                            m_layout->get_lx().get_numeral_uint() << ")" << std::endl;
    stream << "Units: " << m_layout->get_units().get_numeral_uint() << std::endl;
    for(auto itor: m_macros){
        itor->dump(stream);
    }
    for(auto itor: m_terminals){
        itor->dump();
    }
    stream << std::string(30, '+') << std::endl;
}

/**
 * @brief Calculate the minimum possible die are for given problem
 */
void MacroCircuit::area_estimator()
{
    m_estimated_area = 0;

    for (MacroDefinition macro_definition: m_macro_definitions){
        m_estimated_area += (macro_definition.width * macro_definition.height);
    }

    m_layout->set_min_die_predition(m_estimated_area);
}


/**
 * @brief Get access to the connectivity tree
 * 
 * @return Placer::Tree*
 */
Tree* MacroCircuit::get_tree()
{
    return m_tree;
}

/**
 * @brief Get access to the layout
 * 
 * @return Placer::Layout*
 */
Layout* MacroCircuit::get_layout()
{
    return m_layout;
}

/**
 * @brief Get number of found solutions
 * 
 * @return size_t
 */
size_t MacroCircuit::get_solutions()
{
    return m_solutions;
}

/**
 * @brief Configure Z3 backend
 */
void MacroCircuit::config_z3()
{
    z3::params param(m_z3_ctx);
    
    if (this->get_logic() == eInt){
        //m_z3_ctx.set("logic", "LIA");
        m_logger->encode_int();
    } else if (this->get_logic() == eBitVector) {
        // http://smtlib.cs.uiowa.edu/logics-all.shtml
        m_z3_ctx.set("logic", "QF_BV");
        m_logger->encode_bv();
    }

    //param.set(":opt.solution_prefix", "intermediate_result");
    //param.set(":opt.dump_models", true);
    param.set(":opt.pb.compile_equality", true);
    if(this->get_pareto_optimizer()){
        m_logger->use_pareto_optimizer();
        param.set(":opt.priority", "pareto");
    } else if (this->get_box_optimizer()){
        m_logger->use_box_optimizer();
        param.set(":opt.priority", "box");
    } else if (this->get_lex_optimizer()){
        m_logger->use_lex_optimizer();
        param.set(":opt.priority", "lex");
    } else {
        m_logger->use_lex_optimizer();
        param.set(":opt.priority", "lex");
    }
    
    if(this->get_timeout() != 0){
      m_logger->use_timeout(this->get_timeout());
      param.set(":timeout", (unsigned)this->get_timeout() * 1000);
    }

    m_z3_opt->set(param);
}

/**
 * @brief Run SMT encoding
 */
void MacroCircuit::run_encoding()
{
    this->encode_components_inside_die(e2D);
    this->encode_components_non_overlapping(e2D);
    
    if (this->get_free_terminals()){
        this->encode_terminals_non_overlapping();
        this->encode_terminals_on_frontier();
        
        m_z3_opt->add(m_terminals_non_overlapping);
        m_z3_opt->add(m_terminals_on_frontier);
    }

    m_z3_opt->add(m_components_inside_die);
    m_z3_opt->add(m_components_non_overlapping);
    m_z3_opt->add(z3::mk_and(this->get_stored_constraints()));

    if (this->get_minimize_die_mode()){
        m_z3_opt->minimize(m_layout->get_ux());
        m_z3_opt->minimize(m_layout->get_uy());

        if(this->get_box_optimizer()){
            m_z3_opt->minimize(m_layout->get_uy() + m_layout->get_ux());
            m_z3_opt->check();
        }
    }
}

/**
 * @brief Ensure all Components are placed within the die
 * 
 * @param type Rotation degree of freedom
 */
void MacroCircuit::encode_components_inside_die(eRotation const type)
{
    try {
        z3::expr_vector clauses(m_z3_ctx);

        z3::expr die_lx = m_layout->get_lx();
        z3::expr die_ux = m_layout->get_ux();
        z3::expr die_ly = m_layout->get_ly();
        z3::expr die_uy = m_layout->get_uy();

        for (Component* itor: m_components){
            z3::expr is_N = itor->get_orientation() == m_encode->get_value(eNorth);
            z3::expr is_W = itor->get_orientation() == m_encode->get_value(eWest);
            z3::expr is_S = itor->get_orientation() == m_encode->get_value(eSouth);
            z3::expr is_E = itor->get_orientation() == m_encode->get_value(eEast);

            z3::expr_vector case_N(m_z3_ctx);
            case_N.push_back(m_encode->ge(itor->get_lx(eNorth), die_lx)); ///< LX
            case_N.push_back(m_encode->ge(itor->get_ly(eNorth), die_ly)); ///< LY
            case_N.push_back(m_encode->le(itor->get_ux(eNorth), die_ux)); ///< UX
            case_N.push_back(m_encode->le(itor->get_uy(eNorth), die_uy)); ///< UY

            z3::expr_vector case_W(m_z3_ctx);
            case_W.push_back(m_encode->ge(itor->get_lx(eWest), die_lx)); ///< LX
            case_W.push_back(m_encode->ge(itor->get_ly(eWest), die_ly)); ///< LY
            case_W.push_back(m_encode->le(itor->get_ux(eWest), die_ux)); ///< UX
            case_W.push_back(m_encode->le(itor->get_uy(eWest), die_uy)); ///< UY

            z3::expr_vector case_S(m_z3_ctx);
            case_S.push_back(m_encode->ge(itor->get_lx(eSouth), die_lx)); ///< LX
            case_S.push_back(m_encode->ge(itor->get_ly(eSouth), die_ly)); ///< LY
            case_S.push_back(m_encode->le(itor->get_ux(eSouth), die_ux)); ///< UX
            case_S.push_back(m_encode->le(itor->get_uy(eSouth), die_uy)); ///< UY

            z3::expr_vector case_E(m_z3_ctx);
            case_E.push_back(m_encode->ge(itor->get_lx(eEast), die_lx)); ///< LX
            case_E.push_back(m_encode->ge(itor->get_ly(eEast), die_ly)); ///< LY
            case_E.push_back(m_encode->le(itor->get_ux(eEast), die_ux)); ///< UX
            case_E.push_back(m_encode->le(itor->get_uy(eEast), die_uy)); ///< UY

            if(type == eRotation::e2D){
                z3::expr ite = z3::ite(is_N, z3::mk_and(case_N),
                               z3::ite(is_W, z3::mk_and(case_W), m_z3_ctx.bool_val(false)));
                clauses.push_back(ite);
                
            } else if (type == eRotation::e4D){
                z3::expr ite = z3::ite(is_N, z3::mk_and(case_N),
                               z3::ite(is_E, z3::mk_and(case_E),
                               z3::ite(is_S, z3::mk_and(case_S),
                               z3::ite(is_W, z3::mk_and(case_W), m_z3_ctx.bool_val(false)))));
                clauses.push_back(ite);
            } else {
                notsupported_check("Only 2D and 4D Rotation are Supported!");
            }

            if(type == eRotation::e2D){
                clauses.push_back(m_encode->ge(itor->get_orientation(), m_encode->get_value(eNorth)));
                clauses.push_back(m_encode->le(itor->get_orientation(), m_encode->get_value(eWest)));
            } else if (type == eRotation::e4D){
                clauses.push_back(itor->get_orientation() >= m_encode->get_value(eNorth));
                clauses.push_back(itor->get_orientation() <= m_encode->get_value(eEast));
            } else {
                notsupported_check("Only 2D and 4D Rotation are Supported!");
            }
        }

        m_components_inside_die = z3::mk_and(clauses);

    } catch (z3::exception const & exp){
        throw PlacerException(exp.msg());
    }
}

/**
 * @brief Ensure all Components are placed non overlapping
 * 
 * @param type Rotation degree of freedom
 */
void MacroCircuit::encode_components_non_overlapping(eRotation const type)
{
     try {
        z3::expr_vector clauses(m_z3_ctx);

        z3::expr N = m_encode->get_value(eNorth);
        z3::expr W = m_encode->get_value(eWest);
        z3::expr S = m_encode->get_value(eSouth);
        z3::expr E = m_encode->get_value(eEast);

        for(size_t i = 0; i < m_components.size(); i++){
            for(size_t j = 0; j < m_components.size(); j++){
                if(i == j){
                    continue;
                }
                /*
                 * Macro i is fixed and Marco j is placed around it
                 */
                Component* fixed = m_components[i];
                Component* free  = m_components[j];

//{{{           Case North North
                z3::expr_vector case_nn(m_z3_ctx);
                case_nn.push_back(m_encode->ge(free->get_lx(eNorth), fixed->get_ux(eNorth))); ///< Right
                case_nn.push_back(m_encode->le(free->get_ux(eNorth), fixed->get_lx(eNorth))); ///< Left
                case_nn.push_back(m_encode->ge(free->get_ly(eNorth), fixed->get_uy(eNorth))); ///< Upper
                case_nn.push_back(m_encode->le(free->get_uy(eNorth), fixed->get_ly(eNorth))); ///< Below
//}}}
//{{{           Case West North
                z3::expr_vector case_wn(m_z3_ctx);
                case_wn.push_back(m_encode->ge(free->get_lx(eWest), fixed->get_ux(eNorth))); ///< Right
                case_wn.push_back(m_encode->le(free->get_ux(eWest), fixed->get_lx(eNorth))); ///< Left
                case_wn.push_back(m_encode->ge(free->get_ly(eWest), fixed->get_uy(eNorth))); ///< Upper
                case_wn.push_back(m_encode->le(free->get_uy(eWest), fixed->get_ly(eNorth))); ///< Below
//}}}
//{{{           Case South North
                z3::expr_vector case_sn(m_z3_ctx);
                case_sn.push_back(m_encode->ge(free->get_lx(eSouth), fixed->get_ux(eNorth))); ///< Right
                case_sn.push_back(m_encode->le(free->get_ux(eSouth), fixed->get_lx(eNorth))); ///< Left
                case_sn.push_back(m_encode->ge(free->get_ly(eSouth), fixed->get_uy(eNorth))); ///< Upper
                case_sn.push_back(m_encode->le(free->get_uy(eSouth), fixed->get_ly(eNorth))); ///< Below
//}}}
//{{{           Case East North
                z3::expr_vector case_en(m_z3_ctx);
                case_en.push_back(m_encode->ge(free->get_lx(eEast), fixed->get_ux(eNorth))); ///< Right
                case_en.push_back(m_encode->le(free->get_ux(eEast), fixed->get_lx(eNorth))); ///< Left
                case_en.push_back(m_encode->ge(free->get_ly(eEast), fixed->get_uy(eNorth))); ///< Upper
                case_en.push_back(m_encode->le(free->get_uy(eEast), fixed->get_ly(eNorth))); ///< Below
//}}}
//{{{           Case North West
                z3::expr_vector case_nw(m_z3_ctx);
                case_nw.push_back(m_encode->ge(free->get_lx(eNorth), fixed->get_ux(eWest))); ///< Right
                case_nw.push_back(m_encode->le(free->get_ux(eNorth), fixed->get_lx(eWest))); ///< Left
                case_nw.push_back(m_encode->ge(free->get_ly(eNorth), fixed->get_uy(eWest))); ///< Upper
                case_nw.push_back(m_encode->le(free->get_uy(eNorth), fixed->get_ly(eWest))); ///< Below
//}}}                
//{{{           Case West West
                z3::expr_vector case_ww(m_z3_ctx);
                case_ww.push_back(m_encode->ge(free->get_lx(eWest), fixed->get_ux(eWest))); ///< Right
                case_ww.push_back(m_encode->le(free->get_ux(eWest), fixed->get_lx(eWest))); ///< Left
                case_ww.push_back(m_encode->ge(free->get_ly(eWest), fixed->get_uy(eWest))); ///< Upper
                case_ww.push_back(m_encode->le(free->get_uy(eWest), fixed->get_ly(eWest))); ///< Below
//}}}
//{{{           Case South West
                z3::expr_vector case_sw(m_z3_ctx);
                case_sw.push_back(m_encode->ge(free->get_lx(eSouth), fixed->get_ux(eWest))); ///< Right
                case_sw.push_back(m_encode->le(free->get_ux(eSouth), fixed->get_lx(eWest))); ///< Left
                case_sw.push_back(m_encode->ge(free->get_ly(eSouth), fixed->get_uy(eWest))); ///< Upper
                case_sw.push_back(m_encode->le(free->get_uy(eSouth), fixed->get_ly(eWest))); ///< Below
//}}}
//{{{           Case East West
                z3::expr_vector case_ew(m_z3_ctx);
                case_ew.push_back(m_encode->ge(free->get_lx(eEast), fixed->get_ux(eWest))); ///< Right
                case_ew.push_back(m_encode->le(free->get_ux(eEast), fixed->get_lx(eWest))); ///< Left
                case_ew.push_back(m_encode->ge(free->get_ly(eWest), fixed->get_uy(eWest))); ///< Upper
                case_ew.push_back(m_encode->le(free->get_uy(eEast), fixed->get_ly(eWest))); ///< Below
//}}}
//{{{           Case North South
                z3::expr_vector case_ns(m_z3_ctx);
                case_ns.push_back(m_encode->ge(free->get_lx(eNorth), fixed->get_ux(eSouth))); ///< Right
                case_ns.push_back(m_encode->le(free->get_ux(eNorth), fixed->get_lx(eSouth))); ///< Left
                case_ns.push_back(m_encode->ge(free->get_ly(eNorth), fixed->get_uy(eSouth))); ///< Upper
                case_ns.push_back(m_encode->le(free->get_uy(eNorth), fixed->get_ly(eSouth))); ///< Below
//}}}
//{{{           Case West South
                z3::expr_vector case_ws(m_z3_ctx);
                case_ws.push_back(m_encode->ge(free->get_lx(eWest), fixed->get_ux(eSouth))); ///< Right
                case_ws.push_back(m_encode->le(free->get_ux(eWest), fixed->get_lx(eSouth))); ///< Left
                case_ws.push_back(m_encode->ge(free->get_ly(eWest), fixed->get_uy(eSouth))); ///< Upper
                case_ws.push_back(m_encode->le(free->get_uy(eWest), fixed->get_ly(eSouth))); ///< Below
//}}}
//{{{           Case South South
                z3::expr_vector case_ss(m_z3_ctx);
                case_ss.push_back(m_encode->ge(free->get_lx(eSouth), fixed->get_ux(eSouth))); ///< Right
                case_ss.push_back(m_encode->le(free->get_ux(eSouth), fixed->get_lx(eSouth))); ///< Left
                case_ss.push_back(m_encode->ge(free->get_ly(eSouth), fixed->get_uy(eSouth))); ///< Upper
                case_ss.push_back(m_encode->le(free->get_uy(eSouth), fixed->get_ly(eSouth))); ///< Below
//}}} 
//{{{           Case East South
                z3::expr_vector case_es(m_z3_ctx);
                case_es.push_back(m_encode->ge(free->get_lx(eEast), fixed->get_ux(eSouth))); ///< Right
                case_es.push_back(m_encode->le(free->get_ux(eEast), fixed->get_lx(eSouth))); ///< Left
                case_es.push_back(m_encode->ge(free->get_ly(eEast), fixed->get_uy(eSouth))); ///< Upper
                case_es.push_back(m_encode->le(free->get_uy(eEast), fixed->get_ly(eSouth))); ///< Below
//}}}
//{{{           Case North East
                z3::expr_vector case_ne(m_z3_ctx);
                case_ne.push_back(m_encode->ge(free->get_lx(eNorth), fixed->get_ux(eEast))); ///< Right
                case_ne.push_back(m_encode->le(free->get_ux(eNorth), fixed->get_lx(eEast))); ///< Left
                case_ne.push_back(m_encode->ge(free->get_ly(eNorth), fixed->get_uy(eEast))); ///< Upper
                case_ne.push_back(m_encode->le(free->get_uy(eNorth), fixed->get_ly(eEast))); ///< Below
//}}}
//{{{           Case West East
                z3::expr_vector case_we(m_z3_ctx);
                case_we.push_back(m_encode->ge(free->get_lx(eWest), fixed->get_ux(eEast))); ///< Right
                case_we.push_back(m_encode->le(free->get_ux(eWest), fixed->get_lx(eEast))); ///< Left
                case_we.push_back(m_encode->ge(free->get_ly(eWest), fixed->get_uy(eEast))); ///< Upper
                case_we.push_back(m_encode->le(free->get_uy(eWest), fixed->get_ly(eEast))); ///< Below
//}}}
//{{{           Case South East
                z3::expr_vector case_se(m_z3_ctx);
                case_se.push_back(m_encode->ge(free->get_lx(eSouth), fixed->get_ux(eEast))); ///< Right
                case_se.push_back(m_encode->le(free->get_ux(eSouth), fixed->get_lx(eEast))); ///< Left
                case_se.push_back(m_encode->ge(free->get_ly(eSouth), fixed->get_uy(eEast))); ///< Upper
                case_se.push_back(m_encode->le(free->get_uy(eSouth), fixed->get_ly(eEast))); ///< Below
//}}}
//{{{           Case East East
                z3::expr_vector case_ee(m_z3_ctx);
                case_ee.push_back(m_encode->ge(free->get_lx(eEast), fixed->get_ux(eEast))); ///< Right
                case_ee.push_back(m_encode->le(free->get_ux(eEast), fixed->get_lx(eEast))); ///< Left
                case_ee.push_back(m_encode->ge(free->get_ly(eEast), fixed->get_uy(eEast))); ///< Upper
                case_ee.push_back(m_encode->le(free->get_uy(eEast), fixed->get_ly(eEast))); ///< Below
//}}}
//{{{
                assertion_check (case_nn.size() == 4);
                assertion_check (case_nw.size() == 4);
                assertion_check (case_ns.size() == 4);
                assertion_check (case_ne.size() == 4);
                assertion_check (case_wn.size() == 4);
                assertion_check (case_ww.size() == 4);
                assertion_check (case_ws.size() == 4);
                assertion_check (case_we.size() == 4);
                assertion_check (case_sn.size() == 4);
                assertion_check (case_sw.size() == 4);
                assertion_check (case_ss.size() == 4);
                assertion_check (case_se.size() == 4);
                assertion_check (case_en.size() == 4);
                assertion_check (case_ew.size() == 4);
                assertion_check (case_es.size() == 4);
                assertion_check (case_ee.size() == 4);
//}}}
//{{{           Orientation
                z3::expr is_NN((free->get_orientation() == N) && (fixed->get_orientation() == N));
                z3::expr is_NW((free->get_orientation() == N) && (fixed->get_orientation() == W));
                z3::expr is_NS((free->get_orientation() == N) && (fixed->get_orientation() == S));
                z3::expr is_NE((free->get_orientation() == N) && (fixed->get_orientation() == E));
                
                z3::expr is_WN((free->get_orientation() == W) && (fixed->get_orientation() == N));
                z3::expr is_WW((free->get_orientation() == W) && (fixed->get_orientation() == W));
                z3::expr is_WS((free->get_orientation() == W) && (fixed->get_orientation() == S));
                z3::expr is_WE((free->get_orientation() == W) && (fixed->get_orientation() == E));
                
                z3::expr is_SN((free->get_orientation() == S) && (fixed->get_orientation() == N));
                z3::expr is_SW((free->get_orientation() == S) && (fixed->get_orientation() == W));
                z3::expr is_SS((free->get_orientation() == S) && (fixed->get_orientation() == S));
                z3::expr is_SE((free->get_orientation() == S) && (fixed->get_orientation() == E));
                
                z3::expr is_EN((free->get_orientation() == E) && (fixed->get_orientation() == N));
                z3::expr is_EW((free->get_orientation() == E) && (fixed->get_orientation() == W));
                z3::expr is_ES((free->get_orientation() == E) && (fixed->get_orientation() == S));
                z3::expr is_EE((free->get_orientation() == E) && (fixed->get_orientation() == E));
//}}}
//{{{           Encoding
                z3::expr clause = m_z3_ctx.int_val(0);
                
                if(type == eRotation::e2D){
                    clause = z3::ite(is_NN, z3::mk_or(case_nn),
                             z3::ite(is_NW, z3::mk_or(case_nw),
                             z3::ite(is_WN, z3::mk_or(case_wn),
                             z3::ite(is_WW, z3::mk_or(case_ww), m_z3_ctx.bool_val(false)))));
                } else if (type == eRotation::e4D){
                    clause = z3::ite(is_NN, z3::mk_or(case_nn),
                             z3::ite(is_NW, z3::mk_or(case_nw),
                             z3::ite(is_NS, z3::mk_or(case_ns),
                             z3::ite(is_NE, z3::mk_or(case_ne),
                             z3::ite(is_WN, z3::mk_or(case_wn),
                             z3::ite(is_WW, z3::mk_or(case_ww),
                             z3::ite(is_WS, z3::mk_or(case_ws),
                             z3::ite(is_WE, z3::mk_or(case_we),
                             z3::ite(is_SN, z3::mk_or(case_sn),
                             z3::ite(is_SW, z3::mk_or(case_sw),
                             z3::ite(is_SS, z3::mk_or(case_ss),
                             z3::ite(is_SE, z3::mk_or(case_se),
                             z3::ite(is_EN, z3::mk_or(case_en),
                             z3::ite(is_EW, z3::mk_or(case_ew),
                             z3::ite(is_ES, z3::mk_or(case_es),
                             z3::ite(is_EE, z3::mk_or(case_ee), m_z3_ctx.bool_val(false)
                             ))))))))))))))));
                } else {
                    notsupported_check("Only 2D and 4D Rotation are supported!");
                }
                clauses.push_back(clause);
//}}}
            }
        }

        m_components_non_overlapping = z3::mk_and(clauses);
    } catch (z3::exception const & exp){
        throw PlacerException(exp.msg());
    }
}

/**
 * @brief Encode Terminals to be on the Dies frontier
 * 
 * @return z3::expr
 */
void MacroCircuit::encode_terminals_on_frontier()
{
    z3::expr_vector clauses(m_z3_ctx);

    z3::expr die_lx = m_layout->get_lx();
    z3::expr die_ux = m_layout->get_ux();
    z3::expr die_ly = m_layout->get_ly();
    z3::expr die_uy = m_layout->get_uy();

    for(auto itor: m_terminals){
        z3::expr_vector subclause(m_z3_ctx);
        z3::expr x = itor->get_pos_x();
        z3::expr y = itor->get_pos_y();

        // Case 1 x = moveable, y = ly
        z3::expr_vector case_1(m_z3_ctx);
        case_1.push_back(y == die_ly);
        case_1.push_back(x > die_lx);
        case_1.push_back(x < die_ux);

        // Case 2 x = moveable, y = uy
        z3::expr_vector case_2(m_z3_ctx);
        case_2.push_back(y == die_uy);
        case_2.push_back(x > die_lx);
        case_2.push_back(x < die_ux);

        // Case 3 x = lx, y = moveable
        z3::expr_vector case_3(m_z3_ctx);
        case_3.push_back(x == die_lx);
        case_3.push_back(y > die_ly);
        case_3.push_back(y < die_uy);

        // Case 4 x = ux, y = moveable
        z3::expr_vector case_4(m_z3_ctx);
        case_4.push_back(x == die_ux);
        case_4.push_back(y > die_ly);
        case_4.push_back(y < die_uy);

        subclause.push_back(z3::mk_and(case_1));
        subclause.push_back(z3::mk_and(case_2));
        subclause.push_back(z3::mk_and(case_3));
        subclause.push_back(z3::mk_and(case_4));

        clauses.push_back(z3::mk_or(subclause));
    }

    m_terminals_on_frontier = z3::mk_and(clauses);
}

/**
 * @brief Encode Terminals not to overlap
 * 
 * @return z3::expr
 */
void MacroCircuit::encode_terminals_non_overlapping()
{
    try {
        z3::expr_vector clauses(m_z3_ctx);
        
        for(size_t i = 0; i < m_terminals.size(); ++i){
            for(size_t j = 0; j < m_terminals.size(); ++j){
                z3::expr_vector subclause(m_z3_ctx);
                if(i == j){
                    continue;
                }

                Terminal* a = m_terminals[i];
                Terminal* b = m_terminals[j];

                nullpointer_check (a);
                nullpointer_check (b);

                // Case 1: x moveable
                z3::expr_vector case_1(m_z3_ctx);
                case_1.push_back(a->get_pos_x() > b->get_pos_x());
                case_1.push_back(a->get_pos_x() < b->get_pos_x());

                // Case 2: y moveable
                z3::expr_vector case_2(m_z3_ctx);
                case_2.push_back(a->get_pos_y() > b->get_pos_y());
                case_2.push_back(a->get_pos_y() < b->get_pos_y());

                subclause.push_back(z3::mk_or(case_1));
                subclause.push_back(z3::mk_or(case_2));
                clauses.push_back(z3::mk_or(subclause));
            }
        }
        m_terminals_non_overlapping = z3::mk_and(clauses);

    } catch (z3::exception const & exp){
        throw PlacerException(exp.msg());
    }
}

/**
 * @brief Solve encoded SMT problem
 */
void MacroCircuit::solve()
{
    try {
        m_logger->solve_optimize();
        
        if(this->get_store_smt()){
            this->dump_smt_instance();
        }

        if(this->get_pareto_optimizer()){
            m_logger->pareto_solutions(this->get_max_solutions());
        }

        z3::check_result sat = m_z3_opt->check();

        if(sat == z3::check_result::unsat || sat == z3::check_result::unknown){
            m_logger->unsat_solution();
            exit(0);

        } else if (sat == z3::check_result::sat){
            m_solutions = 0;

            do {
                z3::model m = m_z3_opt->get_model();

                if (this->get_minimize_die_mode()){
                    size_t ux =  m.eval(m_layout->get_ux()).get_numeral_uint();
                    size_t uy =  m.eval(m_layout->get_uy()).get_numeral_uint();

                    double area_estimation = ux * uy;
                    double white_space = 100 - ((m_estimated_area/area_estimation)*100.0);
                    m_logger->result_die_area(area_estimation);
                    m_logger->white_space(white_space);

                    m_layout->set_solution_ux(ux);
                    m_layout->set_solution_uy(uy);
                    m_logger->add_solution_layout(ux, uy);
                }
                m_solutions++;

                for(Component* component: m_components){
                        std::string name = component->get_name();

                        size_t x = m.eval(component->get_lx()).get_numeral_int();
                        size_t y = m.eval(component->get_ly()).get_numeral_int();
                        eOrientation o = static_cast<eOrientation>(m.eval(component->get_orientation()).get_numeral_int());

                        component->add_solution_lx(x);
                        component->add_solution_ly(y);
                        component->add_solution_orientation(o);

                        m_logger->place_macro(component->get_id(), x ,y, o);
                }

                for (Terminal* terminal: m_terminals){
                    z3::expr clause_x = m.eval(terminal->get_pos_x());
                    z3::expr clause_y = m.eval(terminal->get_pos_y());

                    if (clause_x.is_numeral() && clause_y.is_numeral()){
                        size_t pos_x = clause_x.get_numeral_uint();
                        size_t pos_y = clause_y.get_numeral_uint();

                        terminal->add_solution_pos_x(pos_x);
                        terminal->add_solution_pos_y(pos_y);

                        m_logger->place_terminal(terminal->get_name(),
                                                 pos_x,
                                                 pos_y);
                    }
                }

                if(/*this->get_pareto_optimizer() &&*/  (m_solutions < this->get_max_solutions())){
                    m_logger->pareto_step();
                    sat = m_z3_opt->check();

                } else {
                    break;
                }
            } while (sat == z3::check_result::sat);

        } else {
            throw std::runtime_error("Must not happen!");
        }
    } catch (z3::exception const & exp){
        throw std::runtime_error(exp.msg());
    }
}

/**
 * @brief Dump encoded SMT problem as *.smt2 to the filesystem
 */
void MacroCircuit::dump_smt_instance()
{
    std::string smt_file = "top_" + this->get_design_name() + ".smt2";
    if(!boost::filesystem::exists(this->get_smt_directory())){
        boost::filesystem::create_directories(this->get_smt_directory());
    }
    boost::filesystem::current_path(this->get_smt_directory());

    std::ofstream out_file(smt_file);
    out_file << "(set-option :produce-models true)" << std::endl;
    out_file << *m_z3_opt;
    out_file << "(get-value(die_ux))" << std::endl;
    out_file << "(get-value(die_uy))" << std::endl;
    out_file << "(get-model)" << std::endl;
    out_file.close();
}

/**
 * @brief Fill Database with the Obtained Results
 */
void MacroCircuit::results_to_db()
{
    for (size_t i = 0; i < m_solutions; ++i){
        for (Macro* macro: m_macros){
            m_db->place_macro(i, macro);
        }
        for (Terminal* terminal: m_terminals){
            m_db->place_terminal(i, terminal);
        }
    }
    m_db->export_as_csv("results.csv");
}
