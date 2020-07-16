//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : macrocircuit.cpp
//
// Date         : 22. December 2019
// Compiler     : gcc version 10.0.1 (GCC) 
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
    m_terminals_non_overlapping(m_encode->get_value(0)),
    m_terminals_center_edge(m_encode->get_value(0)),
    m_hpwl_cost_function(m_encode->get_value(0)),
    m_hpwl_edges(m_z3_ctx)
{
    m_z3_opt = new z3::optimize(m_z3_ctx);
    m_layout = new Layout();
    m_eval = new Evaluate(this);
    m_supplement = new Supplement();
    m_logger = Utils::Logger::getInstance();
    m_timer = new Utils::Timer();
    m_partitioning = new Partitioning();
    m_parquet = new ParquetFrontend();
    m_plotter = new Plotter();

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

    m_tree->destroy();
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
    delete m_plotter; m_plotter = nullptr;

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
                        std::cout << "Could not deduce" << std::endl;
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
            m_tree->merge_edges();
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
    using namespace LefDefParser;
    for(auto& itor: m_circuit->defComponentStor){
        if(this->is_macro(itor)){
            auto idx = m_circuit->lefMacroMap.find(itor.name());
            assert (idx != m_circuit->lefMacroMap.end());
            lefiMacro& lef_data = m_circuit->lefMacroStor[idx->second];
            std::vector<lefiPin> lef_pins = m_circuit->lefPinStor[idx->second];

            MacroDefinition macro_definition;
            macro_definition.name = itor.name();
            macro_definition.id = itor.id();
            macro_definition.width = lef_data.sizeX();
            macro_definition.height = lef_data.sizeY();
            macro_definition.lx = itor.placementX();
            macro_definition.ly = itor.placementY();
            macro_definition.orientation = 
                static_cast<eOrientation>(itor.placementOrient());
            macro_definition.is_placed = itor.isPlaced();

            for(auto pin: lef_pins){
                PinDefinition pin_def;
                pin_def.parent = itor.id();
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
    if (this->get_minimize_die_mode()){
        std::cout << Utils::Utils::get_bash_string_blink_red("Minimize Die Mode") << std::endl;
    } 
    if (this->get_minimize_hpwl_mode()){
        std::cout << Utils::Utils::get_bash_string_blink_red("Minimize HPWL Mode") << std::endl;
    }
    
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
    this->dump_minizinc();
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
        this->solve_no_api();
        //this->solve();
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
        Macro* m = nullptr;

        if (macro_definition.is_placed){
            m = new Macro(macro_definition.name,
                          macro_definition.id,
                          macro_definition.width,
                          macro_definition.height,
                          macro_definition.lx,
                          macro_definition.ly,
                          macro_definition.orientation);
        } else {
            m = new Macro(macro_definition.name,
                          macro_definition.id,
                          macro_definition.width,
                          macro_definition.height);
        }
        nullpointer_check(m);

        if (this->get_minimize_die_mode()){
            if (macro_definition.pin_definitions.size() > 0){
                Pin* p = new Pin ("center",
                                  macro_definition.id,
                                  e_pin_direction::eBidirectional);
                nullpointer_check(p);
                m->add_pin(p);
            }
        } else {
            for (PinDefinition pin_definition: macro_definition.pin_definitions){
                Pin* p = new Pin(pin_definition.name,
                    pin_definition.parent,
                    Pin::string2enum(pin_definition.direction));
                nullpointer_check (p);
                m->add_pin(p);
            }
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
        Terminal* tmp = nullptr;

        assert (itor.numPorts() == 1);

        if (this->get_free_terminals()){
            tmp = new Terminal(itor.pinName(), direction);
        } else if (itor.pinPort(0)->isPlaced()){
            int x = itor.pinPort(0)->placementX();
            int y = itor.pinPort(0)->placementY();
            eOrientation o = static_cast<eOrientation>(itor.pinPort(0)->orient());
            tmp = new Terminal(itor.pinName(), x, y, direction, o);
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
        m_plotter->set_data(m_terminals, m_components, i, m_layout);
        m_plotter->run();
    }
}

/**
 * @brief Dump best solution to filesystem as image
 */
void MacroCircuit::dump_best()
{
    m_logger->dump_best();
    m_plotter->set_filename("best_" + this->get_def());
    if (this->get_minimize_die_mode()){
        std::pair<size_t, size_t> best_area = m_eval->best_area();
        m_plotter->set_data(m_terminals, m_components, best_area.first, m_layout);
    } else if (this->get_minimize_hpwl_mode()){
        std::pair<size_t, size_t> best_hpwl = m_eval->best_hpwl();
        m_plotter->set_data(m_terminals, m_components, best_hpwl.first, m_layout);
    } else {
        assert (0);
    };
    m_plotter->run();
}

/**
 * @brief Save all Placed Solutions as LEF/DEF files
 */
void MacroCircuit::save_all()
{
    boost::filesystem::current_path(this->get_active_results_directory());

     if(!this->get_def().empty() && !this->get_lef().empty()) {
        m_logger->save_all(eLEFDEF);
        for(size_t i = 0; i < m_solutions; ++i){
            std::string def_name = "placed_" + std::to_string(i) + "_" + this->get_def();
            std::string lef_name = "placed_" + std::to_string(i) + "_" + this->get_lef()[0];
            this->write_def(def_name, i);
            this->write_lef(lef_name);
        }
    } else if (!this->get_bookshelf_file().empty()){
        m_logger->save_all(eBookshelf);
        for(size_t i = 0; i < m_solutions; ++i){
            this->set_bookshelf_export("placed_" + std::to_string(i) + "_" + this->get_design_name()) ;
            m_bookshelf->write_placement(i);
            SupplementLayout* layout = new SupplementLayout(0, // TODO
                                                            0, // TODO
                                                            m_layout->get_solution_ux(i),
                                                            m_layout->get_solution_uy(i));
            m_supplement->set_layout(layout);
            m_supplement->write_supplement_file();
        }
    } else {
        assert (0);
    }
}

/**
 * @brief Save best solution as LEF/DEF files
 */
void MacroCircuit::save_best()
{
    boost::filesystem::current_path(this->get_active_results_directory());

    if(!this->get_def().empty() && !this->get_lef().empty()) {
        m_logger->save_best(eLEFDEF);
        if (this->get_minimize_die_mode()){
            std::pair<size_t, size_t> best_area = m_eval->best_area();
            std::string def_name = "best_" + this->get_def();
            std::string lef_name = "best_" + this->get_lef()[0];
            this->write_def(def_name, best_area.first);
            this->write_lef(lef_name);
        } else if (this->get_minimize_hpwl_mode()){
            std::pair<size_t, size_t> best_hpwl = m_eval->best_hpwl();
            std::string def_name = "best_" + this->get_def();
            std::string lef_name = "best_" + this->get_lef()[0];
            this->write_def(def_name, best_hpwl.first);
            this->write_lef(lef_name);
        }
    } else if (!this->get_bookshelf_file().empty()){
        m_logger->save_best(eBookshelf);
        if (this->get_minimize_die_mode()){
            std::pair<size_t, size_t> best_area = m_eval->best_area();
            std::string name = "best_" + this->get_design_name();
            this->set_bookshelf_export(name);
            SupplementLayout* layout = new SupplementLayout(0, // TODO
                                                            0, // TODO
                                                            m_layout->get_solution_ux(best_area.first),
                                                            m_layout->get_solution_uy(best_area.first));
            m_supplement->set_layout(layout);
            m_supplement->write_supplement_file();
            m_bookshelf->write_placement(best_area.first);
        } else if (this->get_minimize_hpwl_mode()){
            std::pair<size_t, size_t> best_hpwl = m_eval->best_hpwl();
            std::string name = "best_" + this->get_design_name();
            this->set_bookshelf_export(name);
            m_bookshelf->write_placement(best_hpwl.first);
        }
    } else {
        assert (0);
    }
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
                std::cout << itor.instance(0) << " " << itor.pin(0) << std::endl;
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
    m_logger->write_def(name);

    // Write Shape of Die
///{{{
    size_t lx = 0, ly = 0, ux = 0, uy = 0;
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
        ux = m_layout->get_solution_ux(solution);
    } else {
        ux = m_layout->get_ux_numercial();
    }
     if (m_layout->is_free_uy()){
        uy = m_layout->get_solution_uy(solution);
    } else {
        uy = m_layout->get_uy_numerical();
    }

    defiGeometries data(nullptr);
    data.Init();
    data.startList(lx,ly);
    data.addToList(ux,uy);
    
    m_circuit->defDieArea.Destroy();
    m_circuit->defDieArea.Init();
    m_circuit->defDieArea.addPoint(&data);
///}}}
    // Write Position and Orientation of Macros
///{{{
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
///}}}
    // Write Position of Terminals
///{{{
    for(auto itor: m_terminals){
        if(!itor->is_free()){
            continue;
        }

        if (itor->has_solution(solution)){
            size_t pos_x = itor->get_solution_pos_x(solution);
            size_t pos_y = itor->get_solution_pos_y(solution);

            auto idx = m_circuit->defPinMap.find(itor->get_id());
            LefDefParser::defiPin& pin = m_circuit->defPinStor[idx->second];
            m_circuit->defPinStor[idx->second].setPlacement(DEFI_COMPONENT_PLACED, pos_x, pos_y, 1);
        }
    }
///}}}
    FILE* fp = fopen(name.c_str(), "w");
    m_circuit->WriteDef(fp);
    fclose(fp);
}

/**
 * @brief Export LEF File
 * 
 * @param name Filename
 */
void MacroCircuit::write_lef(std::string const & name)
{
    m_logger->write_lef(name);
    
    FILE* fp = fopen(name.c_str(), "w");
    m_circuit->WriteLef(fp);
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
    try {
        auto idx = m_circuit->lefMacroMap.find(macro.name());
        assert (idx != m_circuit->lefMacroMap.end());

        LefDefParser::lefiMacro& lef_data = m_circuit->lefMacroStor[idx->second];

        return lef_data.sizeY() != m_standard_cell_height;

    } catch(std::exception const & exp){
        throw PlacerException(exp.what());
    }
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
    assert (idx != m_circuit->lefMacroMap.end());
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
    std::string solver_version = m_encode->get_version();
    m_logger->solver_version(solver_version);

    z3::set_param("verbose", 0);
    z3::set_param("smt.auto_config", false);
    z3::set_param("smt.arith.solver", 5);
    z3::set_param("smt.threads", 3);
    z3::set_param("parallel.enable", true);
    
    z3::params param(m_z3_ctx);
    //param.set(":opt.solution_prefix", "intermediate_result");
    param.set("opt.dump_models", true);
    param.set("opt.pb.compile_equality", true);
    //param.set(":opt.optsmt_engine", "symba");
  
    if(this->get_pareto_optimizer()){
        m_logger->use_pareto_optimizer();
        param.set("opt.priority", "pareto");
    } else if (this->get_lex_optimizer()){
        m_logger->use_lex_optimizer();
        param.set("opt.priority", "lex");
    } else {
        m_logger->use_lex_optimizer();
        param.set("opt.priority", "lex");
    }

    if(this->get_timeout() != 0){
      m_logger->use_timeout(this->get_timeout());
      param.set("timeout", (unsigned)this->get_timeout() * 1000);
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
        //this->encode_terminals_non_overlapping();
        //this->encode_terminals_on_frontier();
        this->encode_terminals_center_edge();

        //m_z3_opt->add(m_terminals_non_overlapping.simplify());
        //m_z3_opt->add(m_terminals_on_frontier.simplify());
        m_z3_opt->add(m_terminals_center_edge.simplify());
    }
    
    if (this->get_minimize_die_mode()){
        m_z3_opt->add(m_components_inside_die.simplify());
        m_z3_opt->add(m_components_non_overlapping.simplify());
    }
    if (this->get_stored_constraints().size() > 0){
        m_z3_opt->add(z3::mk_and(this->get_stored_constraints()));
    }

    if (this->get_partitioning()){
        z3::expr_vector clauses(m_z3_ctx);
        for (Partition* p: m_partitons){
            p->encode_partition();
            clauses.push_back(p->get_partition_constraints());
        }
        m_z3_opt->add(z3::mk_and(clauses));
    } else {
        z3::expr_vector clauses(m_z3_ctx);
        for (Macro* m: m_macros){
            m->encode_pins();
            clauses.push_back(m->get_pin_constraints());
        }
        m_z3_opt->add(z3::mk_and(clauses));
    }

    if (this->get_minimize_die_mode()){
        //m_z3_opt->minimize(m_layout->get_ux() * m_layout->get_uy());
        m_z3_opt->minimize(m_layout->get_ux());
        m_z3_opt->minimize(m_layout->get_uy());
    }

    this->encode_hpwl_length();
    for (size_t i = 0; i < m_hpwl_edges.size(); ++i){
    //    m_z3_opt->minimize(m_hpwl_edges[i].simplify());
    }
    
    m_z3_opt->minimize(m_hpwl_cost_function);
    
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
        std::vector<std::string> _clauses;

        z3::expr die_lx = m_layout->get_lx();
        z3::expr die_ux = m_layout->get_ux();
        z3::expr die_ly = m_layout->get_ly();
        z3::expr die_uy = m_layout->get_uy();
        
        std::stringstream _die_lx; _die_lx << "0";
        std::stringstream _die_ly; _die_ly << "0";
        std::stringstream _die_ux; _die_ux << m_layout->get_ux();
        std::stringstream _die_uy; _die_uy << m_layout->get_uy();
    
        for (Component* itor: m_components){
            z3::expr is_N = itor->get_orientation() == m_encode->get_value(eNorth);
            z3::expr is_W = itor->get_orientation() == m_encode->get_value(eWest);
            z3::expr is_S = itor->get_orientation() == m_encode->get_value(eSouth);
            z3::expr is_E = itor->get_orientation() == m_encode->get_value(eEast);
            
            std::string _is_N = itor->get_orientation().to_string() + " == " + std::to_string(eNorth);
            std::string _is_W = itor->get_orientation().to_string() + " == " + std::to_string(eWest);
            std::string _is_S = itor->get_orientation().to_string() + " == " + std::to_string(eSouth);
            std::string _is_E = itor->get_orientation().to_string() + " == " + std::to_string(eEast);

            z3::expr_vector case_N(m_z3_ctx);
            case_N.push_back(m_encode->ge(itor->get_lx(eNorth), die_lx)); ///< LX
            case_N.push_back(m_encode->ge(itor->get_ly(eNorth), die_ly)); ///< LY
            case_N.push_back(m_encode->le(itor->get_ux(eNorth), die_ux)); ///< UX
            case_N.push_back(m_encode->le(itor->get_uy(eNorth), die_uy)); ///< UY
            
            std::vector<std::string> _case_N;
            _case_N.push_back(itor->_get_lx(eNorth).str() + " >= " + _die_lx.str());
            _case_N.push_back(itor->_get_ly(eNorth).str() + " >= " + _die_ly.str()); ///< LY
            _case_N.push_back(itor->_get_ux(eNorth).str() + " <= " + _die_ux.str()); ///< UX
            _case_N.push_back(itor->_get_uy(eNorth).str() + " <= " + _die_uy.str()); ///< UY

            z3::expr_vector case_W(m_z3_ctx);
            case_W.push_back(m_encode->ge(itor->get_lx(eWest), die_lx)); ///< LX
            case_W.push_back(m_encode->ge(itor->get_ly(eWest), die_ly)); ///< LY
            case_W.push_back(m_encode->le(itor->get_ux(eWest), die_ux)); ///< UX
            case_W.push_back(m_encode->le(itor->get_uy(eWest), die_uy)); ///< UY

            std::vector<std::string> _case_W;
            _case_W.push_back(itor->_get_lx(eWest).str() + " >= " + _die_lx.str()); ///< LX
            _case_W.push_back(itor->_get_ly(eWest).str() + " >= " + _die_ly.str()); ///< LY
            _case_W.push_back(itor->_get_ux(eWest).str() + " <= " + _die_ux.str()); ///< UX
            _case_W.push_back(itor->_get_uy(eWest).str() + " <= " + _die_uy.str()); ///< UY

            z3::expr_vector case_S(m_z3_ctx);
            case_S.push_back(m_encode->ge(itor->get_lx(eSouth), die_lx)); ///< LX
            case_S.push_back(m_encode->ge(itor->get_ly(eSouth), die_ly)); ///< LY
            case_S.push_back(m_encode->le(itor->get_ux(eSouth), die_ux)); ///< UX
            case_S.push_back(m_encode->le(itor->get_uy(eSouth), die_uy)); ///< UY
            
            std::vector<std::string> _case_S;
            _case_S.push_back(itor->_get_lx(eSouth).str() + " >= " + _die_lx.str()); ///< LX
            _case_S.push_back(itor->_get_ly(eSouth).str() + " >= " + _die_ly.str()); ///< LY
            _case_S.push_back(itor->_get_ux(eSouth).str() + " <= " + _die_ux.str()); ///< UX
            _case_S.push_back(itor->_get_uy(eSouth).str() + " <= " + _die_uy.str()); ///< UY

            z3::expr_vector case_E(m_z3_ctx);
            case_E.push_back(m_encode->ge(itor->get_lx(eEast), die_lx)); ///< LX
            case_E.push_back(m_encode->ge(itor->get_ly(eEast), die_ly)); ///< LY
            case_E.push_back(m_encode->le(itor->get_ux(eEast), die_ux)); ///< UX
            case_E.push_back(m_encode->le(itor->get_uy(eEast), die_uy)); ///< UY
            
            std::vector<std::string> _case_E;
            _case_E.push_back(itor->_get_lx(eEast).str() + " >= " + _die_lx.str()); ///< LX
            _case_E.push_back(itor->_get_ly(eEast).str() + " >= " + _die_ly.str()); ///< LY
            _case_E.push_back(itor->_get_ux(eEast).str() + " <= " + _die_ux.str()); ///< UX
            _case_E.push_back(itor->_get_uy(eEast).str() + " <= " + _die_uy.str()); ///< UY

            if(type == eRotation::e2D){
                std::stringstream _case_N_and;
                std::stringstream _case_W_and;
                _case_N_and << "(" << _case_N[0] << " /\\ "
                            << _case_N[1] << " /\\ "
                            << _case_N[2] << " /\\ " << _case_N[3] << ")";
                            
                _case_W_and << "(" << _case_W[0] << " /\\ "
                            << _case_W[1] << " /\\ "
                            << _case_W[2] << " /\\ " << _case_W[3] << ")";
                            
                std::stringstream _ite;
                _ite << "if " << _is_N << " then " << _case_N_and.str() << " else " << _case_W_and.str() << " endif";
                _clauses.push_back(_ite.str());
                
                z3::expr ite = z3::ite(is_N, z3::mk_and(case_N),
                               z3::ite(is_W, z3::mk_and(case_W), m_z3_ctx.bool_val(false)));
                clauses.push_back(ite);

            } else if (type == eRotation::e4D){
                assert (0);
                z3::expr ite = z3::ite(is_N, z3::mk_and(case_N),
                               z3::ite(is_E, z3::mk_and(case_E),
                               z3::ite(is_S, z3::mk_and(case_S),
                               z3::ite(is_W, z3::mk_and(case_W), m_z3_ctx.bool_val(false)))));
                clauses.push_back(ite);
            } else {
                notsupported_check("Only 2D and 4D Rotation are Supported!");
            }

            if(type == eRotation::e2D){
                _clauses.push_back(itor->get_orientation().to_string() + " >= " + std::to_string(eNorth));
                _clauses.push_back(itor->get_orientation().to_string() + " <= " + std::to_string(eWest));
                
                clauses.push_back(m_encode->ge(itor->get_orientation(), m_encode->get_value(eNorth)));
                clauses.push_back(m_encode->le(itor->get_orientation(), m_encode->get_value(eWest)));
            } else if (type == eRotation::e4D){
                assert (0);
                clauses.push_back(itor->get_orientation() >= m_encode->get_value(eNorth));
                clauses.push_back(itor->get_orientation() <= m_encode->get_value(eEast));
            } else {
                notsupported_check("Only 2D and 4D Rotation are Supported!");
            }
        }

        for (auto itor: _clauses){
            m_components_inside_die_constraints.push_back("constraint " + itor + ";");
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
        std::vector<std::string> _clauses;

        z3::expr N = m_encode->get_value(eNorth);
        z3::expr W = m_encode->get_value(eWest);
        z3::expr S = m_encode->get_value(eSouth);
        z3::expr E = m_encode->get_value(eEast);
        
        std::string _N = std::to_string(eNorth);
        std::string _W = std::to_string(eWest);
        std::string _S = std::to_string(eSouth);
        std::string _E = std::to_string(eEast);

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
                
                std::vector<std::string> _case_nn;
                _case_nn.push_back(free->_get_lx(eNorth).str() + " >= " + fixed->_get_ux(eNorth).str()); ///< Right
                _case_nn.push_back(free->_get_ux(eNorth).str() + " <= " + fixed->_get_lx(eNorth).str()); ///< Left
                _case_nn.push_back(free->_get_ly(eNorth).str() + " >= " + fixed->_get_uy(eNorth).str()); ///< Upper
                _case_nn.push_back(free->_get_uy(eNorth).str() + " <= " + fixed->_get_ly(eNorth).str()); ///< Below
//}}}
//{{{           Case West North
                z3::expr_vector case_wn(m_z3_ctx);
                case_wn.push_back(m_encode->ge(free->get_lx(eWest), fixed->get_ux(eNorth))); ///< Right
                case_wn.push_back(m_encode->le(free->get_ux(eWest), fixed->get_lx(eNorth))); ///< Left
                case_wn.push_back(m_encode->ge(free->get_ly(eWest), fixed->get_uy(eNorth))); ///< Upper
                case_wn.push_back(m_encode->le(free->get_uy(eWest), fixed->get_ly(eNorth))); ///< Below
                
                std::vector<std::string> _case_wn;
                _case_wn.push_back(free->_get_lx(eWest).str() + " >= " + fixed->_get_ux(eNorth).str()); ///< Right
                _case_wn.push_back(free->_get_ux(eWest).str() + " <= " + fixed->_get_lx(eNorth).str()); ///< Left
                _case_wn.push_back(free->_get_ly(eWest).str() + " >= " + fixed->_get_uy(eNorth).str()); ///< Upper
                _case_wn.push_back(free->_get_uy(eWest).str() + " <= " + fixed->_get_ly(eNorth).str()); ///< Below
//}}}
//{{{           Case South North
                z3::expr_vector case_sn(m_z3_ctx);
                case_sn.push_back(m_encode->ge(free->get_lx(eSouth), fixed->get_ux(eNorth))); ///< Right
                case_sn.push_back(m_encode->le(free->get_ux(eSouth), fixed->get_lx(eNorth))); ///< Left
                case_sn.push_back(m_encode->ge(free->get_ly(eSouth), fixed->get_uy(eNorth))); ///< Upper
                case_sn.push_back(m_encode->le(free->get_uy(eSouth), fixed->get_ly(eNorth))); ///< Below
                
                std::vector<std::string> _case_sn;
                _case_sn.push_back(free->_get_lx(eSouth).str() + " >= " + fixed->_get_ux(eNorth).str()); ///< Right
                _case_sn.push_back(free->_get_ux(eSouth).str() + " <= " + fixed->_get_lx(eNorth).str()); ///< Left
                _case_sn.push_back(free->_get_ly(eSouth).str() + " >= " + fixed->_get_uy(eNorth).str()); ///< Upper
                _case_sn.push_back(free->_get_uy(eSouth).str() + " <= " + fixed->_get_ly(eNorth).str()); ///< Below
//}}}
//{{{           Case East North
                z3::expr_vector case_en(m_z3_ctx);
                case_en.push_back(m_encode->ge(free->get_lx(eEast), fixed->get_ux(eNorth))); ///< Right
                case_en.push_back(m_encode->le(free->get_ux(eEast), fixed->get_lx(eNorth))); ///< Left
                case_en.push_back(m_encode->ge(free->get_ly(eEast), fixed->get_uy(eNorth))); ///< Upper
                case_en.push_back(m_encode->le(free->get_uy(eEast), fixed->get_ly(eNorth))); ///< Below
                
                std::vector<std::string> _case_en;
                _case_en.push_back(free->_get_lx(eEast).str() + " >= " + fixed->_get_ux(eNorth).str()); ///< Right
                _case_en.push_back(free->_get_ux(eEast).str() + " <= " + fixed->_get_lx(eNorth).str()); ///< Left
                _case_en.push_back(free->_get_ly(eEast).str() + " >= " + fixed->_get_uy(eNorth).str()); ///< Upper
                _case_en.push_back(free->_get_uy(eEast).str() + " <= " + fixed->_get_ly(eNorth).str()); ///< Below
//}}}
//{{{           Case North West
                z3::expr_vector case_nw(m_z3_ctx);
                case_nw.push_back(m_encode->ge(free->get_lx(eNorth), fixed->get_ux(eWest))); ///< Right
                case_nw.push_back(m_encode->le(free->get_ux(eNorth), fixed->get_lx(eWest))); ///< Left
                case_nw.push_back(m_encode->ge(free->get_ly(eNorth), fixed->get_uy(eWest))); ///< Upper
                case_nw.push_back(m_encode->le(free->get_uy(eNorth), fixed->get_ly(eWest))); ///< Below
                
                std::vector<std::string> _case_nw;
                _case_nw.push_back(free->_get_lx(eNorth).str() + " >= " + fixed->_get_ux(eWest).str()); ///< Right
                _case_nw.push_back(free->_get_ux(eNorth).str() + " <= " + fixed->_get_lx(eWest).str()); ///< Left
                _case_nw.push_back(free->_get_ly(eNorth).str() + " >= " + fixed->_get_uy(eWest).str()); ///< Upper
                _case_nw.push_back(free->_get_uy(eNorth).str() + " <= " + fixed->_get_ly(eWest).str()); ///< Below
//}}}                
//{{{           Case West West
                z3::expr_vector case_ww(m_z3_ctx);
                case_ww.push_back(m_encode->ge(free->get_lx(eWest), fixed->get_ux(eWest))); ///< Right
                case_ww.push_back(m_encode->le(free->get_ux(eWest), fixed->get_lx(eWest))); ///< Left
                case_ww.push_back(m_encode->ge(free->get_ly(eWest), fixed->get_uy(eWest))); ///< Upper
                case_ww.push_back(m_encode->le(free->get_uy(eWest), fixed->get_ly(eWest))); ///< Below
                
                std::vector<std::string> _case_ww;
                _case_ww.push_back(free->_get_lx(eWest).str() + " >= " + fixed->_get_ux(eWest).str()); ///< Right
                _case_ww.push_back(free->_get_ux(eWest).str() + " <= " + fixed->_get_lx(eWest).str()); ///< Left
                _case_ww.push_back(free->_get_ly(eWest).str() + " >= " + fixed->_get_uy(eWest).str()); ///< Upper
                _case_ww.push_back(free->_get_uy(eWest).str() + " <= " + fixed->_get_ly(eWest).str()); ///< Below
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
                
                std::string _is_NN = "(" + free->get_orientation().to_string() + " == " + _N + ") /\\ (" + fixed->get_orientation().to_string() + " == " + _N + ")";
                std::string _is_NW = "(" + free->get_orientation().to_string() + " == " + _N + ") /\\ (" + fixed->get_orientation().to_string() + " == " + _W + ")";
                
                z3::expr is_WN((free->get_orientation() == W) && (fixed->get_orientation() == N));
                z3::expr is_WW((free->get_orientation() == W) && (fixed->get_orientation() == W));
                z3::expr is_WS((free->get_orientation() == W) && (fixed->get_orientation() == S));
                z3::expr is_WE((free->get_orientation() == W) && (fixed->get_orientation() == E));
                
                std::string _is_WN = "(" + free->get_orientation().to_string() + " == " + _W + ") /\\ (" + fixed->get_orientation().to_string() + " == " + _N + ")";
                std::string _is_WW = "(" + free->get_orientation().to_string() + " == " + _W + ") /\\ (" + fixed->get_orientation().to_string() + " == " + _W + ")";
                 
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
                std::stringstream _clause;
                
                if(type == eRotation::e2D){
                    std::stringstream case_nn_or;
                    std::stringstream case_nw_or;
                    std::stringstream case_wn_or;
                    std::stringstream case_ww_or;
                    
                    case_nn_or << _case_nn[0] << " \\/ " << _case_nn[1] << " \\/ " << _case_nn[2] << " \\/ " << _case_nn[3];
                    case_nw_or << _case_nw[0] << " \\/ " << _case_nw[1] << " \\/ " << _case_nw[2] << " \\/ " << _case_nw[3];
                    case_wn_or << _case_wn[0] << " \\/ " << _case_wn[1] << " \\/ " << _case_wn[2] << " \\/ " << _case_wn[3];
                    case_ww_or << _case_ww[0] << " \\/ " << _case_ww[1] << " \\/ " << _case_ww[2] << " \\/ " << _case_ww[3];
                    
                    _clause << "if (" << _is_NN << ") then " << case_nn_or.str()
                            << " elseif (" << _is_NW << ") then " << case_nw_or.str()
                            << " elseif (" << _is_WN << ") then " << case_wn_or.str()
                            << " else " << case_ww_or.str() << " endif";
                            
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
                _clauses.push_back(_clause.str());
//}}}
            }
        }

        m_components_non_overlapping = z3::mk_and(clauses);
        
        for (auto itor: _clauses){
            m_components_non_overlapping_constraints.push_back("constraint " + itor + ";");
        }
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
 * @brief Make Total a Number of 4 Terminals to be placed for Connectivity
 */
void MacroCircuit::encode_terminals_center_edge()
{
    z3::expr_vector clauses(m_z3_ctx);

    z3::expr n_x = m_layout->get_ux() / 2;
    z3::expr n_y = m_layout->get_uy();

    z3::expr w_x = m_layout->get_lx();
    z3::expr w_y = m_layout->get_uy() / 2;

    z3::expr s_x = m_layout->get_lx();
    z3::expr s_y = m_layout->get_uy() / 2;

    z3::expr e_x = m_layout->get_ux();
    z3::expr e_y = m_layout->get_uy() / 2;

    for (Terminal* t: m_terminals){
        z3::expr_vector clause(m_z3_ctx);
        z3::expr x = t->get_pos_x();
        z3::expr y = t->get_pos_y();

        z3::expr_vector n(m_z3_ctx);
        z3::expr_vector w(m_z3_ctx);
        z3::expr_vector s(m_z3_ctx);
        z3::expr_vector e(m_z3_ctx);

        n.push_back(x == n_x);
        n.push_back(y == n_y);

        w.push_back(x == w_x);
        w.push_back(y == w_y);

        s.push_back(x == s_x);
        s.push_back(y == s_y);

        e.push_back(x == e_x);
        e.push_back(y == e_y);

        clause.push_back(z3::mk_and(n));
        clause.push_back(z3::mk_and(w));
        clause.push_back(z3::mk_and(s));
        clause.push_back(z3::mk_and(e));

        clauses.push_back(z3::mk_or(clause));
    }
    m_terminals_center_edge = z3::mk_and(clauses);
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

        if(sat == z3::check_result::unsat){
            m_logger->unsat_solution();
            exit(0);

        } else if (sat == z3::check_result::unknown){
            m_logger->unknown_solution();
            //std::cout << m_z3_opt->get_model() << std::endl;
            exit(0);
            
        } else if (sat == z3::check_result::sat){
            m_solutions = 0;

            do {
                z3::model m = m_z3_opt->get_model();
               this->process_results(m);

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
        throw PlacerException(exp.msg());
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
    out_file << "(set-logic UFNIA)" << std::endl;
    out_file << *m_z3_opt;
    size_t sol = 1;

    do {
    
        if (m_layout->is_free_ux()){
            out_file << "(get-value (" << m_layout->get_ux() << "))" << std::endl;
        }
        if (m_layout->is_free_uy()){
            out_file << "(get-value (" << m_layout->get_uy() << "))" << std::endl;
        }
        
        for (Terminal* t: m_terminals){
            if (t->is_free()){
                out_file << "(get-value (" << t->get_pos_x() << "))" << std::endl;
                out_file << "(get-value (" << t->get_pos_y() << "))" << std::endl;
            }
        }
        for(Component* c: m_components){
            if (c->is_free()){
                out_file << "(get-value (" << c->get_lx() << "))" << std::endl;
                out_file << "(get-value (" << c->get_ly() << "))" << std::endl;
                out_file << "(get-value (" << c->get_orientation() << "))" << std::endl;
                
                for (Pin* p: c->get_pins()){
                    out_file << "(get-value (" << p->get_pin_pos_x() << "))" << std::endl;
                    out_file << "(get-value (" << p->get_pin_pos_y() << "))" << std::endl;
                }
            }
        }
        sol++;
        if (sol <= this->get_max_solutions()){
            out_file << "(check-sat)" << std::endl;
        }
    } while(sol <= this->get_max_solutions());
    
    out_file.close();
}

/**
 * @brief Fill Database with the Obtained Results
 */
void MacroCircuit::results_to_db()
{
    for (size_t i = 0; i < m_solutions; ++i){
        for (Component* component: m_components){
            m_db->place_component(i, component);

            for (Pin* pin: component->get_pins()){
                m_db->place_pin(i, component, pin);
            }
        }
        for (Terminal* terminal: m_terminals){
            m_db->place_terminal(i, terminal);
        }

        size_t lx = 0, ly = 0, ux = 0, uy = 0;
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
            ux = m_layout->get_solution_ux(i);
        } else {
            ux = m_layout->get_ux_numercial();
        }
        if (m_layout->is_free_uy()){
            uy = m_layout->get_solution_uy(i);
        } else {
            uy = m_layout->get_uy_numerical();
        }
        m_db->insert_layout(i, lx, ly, ux, uy);
    }
    m_db->export_as_csv("results.csv");
}

/**
 * @brief Wirelenght Cost Function
 */
void MacroCircuit::encode_hpwl_length()
{
    z3::expr_vector clauses(m_z3_ctx);

    for (Edge* edge: m_tree->get_edges()){
        
        if (edge->get_from() == edge->get_to()){
            /* Edge Points to Itself */
            continue;
        }

        nullpointer_check(edge);

        Node* from = edge->get_from();
        Node* to   = edge->get_to();
        nullpointer_check(from);
        nullpointer_check(to);

        z3::expr from_x = m_encode->get_value(0);
        z3::expr from_y = m_encode->get_value(0);

        z3::expr to_x = m_encode->get_value(0);
        z3::expr to_y = m_encode->get_value(0);

        if (from->is_terminal()){
            Terminal* t = from->get_terminal();
            nullpointer_check(t);
            from_x = t->get_pos_x();
            from_y = t->get_pos_y();
        } else if (from->has_macro()){
            Macro* m = from->get_macro();
            nullpointer_check(m);
            Pin* p = m->get_pin(edge->get_from_pin());
            nullpointer_check(p);

            from_x = p->get_pin_pos_x();
            from_y = p->get_pin_pos_y();
        } else {
            notimplemented_check();
        }

        if (to->is_terminal()){
            Terminal* t = to->get_terminal();
            nullpointer_check(t);

            to_x = t->get_pos_x();
            to_y = t->get_pos_y();
        } else if (to->has_macro()){
            Macro* m = to->get_macro();
            nullpointer_check(m);

            Pin* p = m->get_pin(edge->get_to_pin());
            nullpointer_check(p);

            to_x = p->get_pin_pos_x();
            to_y = p->get_pin_pos_y();
        } else {
            notimplemented_check();
        }
        
        z3::expr hpwl = this->manhattan_distance(from_x, from_y, to_x, to_y);
        if (edge->get_weight() > 1){
            z3::expr cost = m_encode->get_value(edge->get_weight());
            z3::expr smt = hpwl * cost;
            clauses.push_back(smt);
        } else {
            clauses.push_back(hpwl);
        }
    }

    for (size_t i = 0; i < clauses.size(); ++i){
        m_hpwl_edges.push_back(clauses[i]);
    }

    m_hpwl_cost_function = m_encode->mk_sum(clauses);
}

/**
 * @brief Calculate the Manhattan Distance between 2 Points in 2D System
 * 
 * @param from_x X Coordinate Point 1
 * @param from_y Y Coordinate Point 1
 * @param to_x X Coordinate Point 2
 * @param to_y Y Coordinate Point 2
 * @return z3::expr
 */
z3::expr MacroCircuit::manhattan_distance(z3::expr const & from_x,
                                          z3::expr const & from_y,
                                          z3::expr const & to_x,
                                          z3::expr const & to_y)
{
    z3::expr a = z3::abs(to_x - from_x);
    z3::expr b = z3::abs(to_y - from_y);

    return a+b;
}

/**
 * @brief Create Area and HPWL Statistics
 */
void MacroCircuit::create_statistics()
{
    return;
    if (this->get_minimize_die_mode()){
        auto all_area = m_eval->all_area();
        
        for (auto itor: all_area){
            std::cout << "Statistics Area: " << std::endl;
            std::cout << itor.first << ": " << itor.second << std::endl;
        }
    }
    if (this->get_minimize_hpwl_mode()){
        auto all_hpwl = m_eval->all_hpwl();
        
        for (auto itor: all_hpwl){
            std::cout << "Statistics HPWL: " << std::endl;
            std::cout << itor.first << ": " << itor.second << std::endl;
        }
    }
}

/**
 * @brief Process the results from a found model
 * 
 * @param m The model to process
 */
void MacroCircuit::process_results(z3::model const & m)
{
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

        //if (this->get_minimize_hpwl_mode()){
            std::vector<Pin*> pins = component->get_pins();

            for (Pin* p: pins){
                //if (p->is_free()){
                    z3::expr x = p->get_pin_pos_x();
                    z3::expr y = p->get_pin_pos_y();

                    z3::expr x_val = m.eval(x);
                    z3::expr y_val = m.eval(y);

                    size_t x_pos = x_val.get_numeral_uint();
                    size_t y_pos = y_val.get_numeral_uint();

                    p->add_solution_pin_pos_x(x_pos);
                    p->add_solution_pin_pos_y(y_pos);
                //}
            }
        //}
    }

    for (Terminal* terminal: m_terminals){
        z3::expr clause_x = terminal->get_pos_x();
        z3::expr clause_y = terminal->get_pos_y();

        if (this->get_free_terminals()){
            size_t val_x = m.eval(clause_x).get_numeral_uint();
            size_t val_y = m.eval(clause_y).get_numeral_uint();

            terminal->add_solution_pos_x(val_x);
            terminal->add_solution_pos_y(val_y);

            m_logger->place_terminal(terminal->get_name(),
                                     val_x,
                                     val_y);
        }
    }
}

/**
 * @brief Invoke Z3 Frontend for Problem Solving
 * 
 * Z3's Frontend shows much fast solving times than directly
 * invoking the API for solving. 
 * 
 * TODO Figure out why the API seems to run slower
 */
void MacroCircuit::solve_no_api()
{
    this->dump_smt_instance();
    boost::filesystem::current_path(this->get_smt_directory());
    std::string smt_file = "top_" + this->get_design_name() + ".smt2";
    std::string results_file = this->get_smt_directory() +  "/top_" 
                                 + this->get_design_name() + "_results.txt";

    std::vector<std::string> args;
    args.push_back(smt_file);
    
    std::cout << this->get_third_party_bin() << std::endl;
    Utils::Utils::system_execute("z3", args, results_file, true);

    std::map<std::string, std::vector<size_t>> key_value_results;
    std::vector<std::string> z3_results;
    std::string line;
    std::ifstream results(results_file);
    while(std::getline(results, line)){
        z3_results.push_back(line);
    }
    results.close();

    if (z3_results[0] == "sat"){
        
    } else if (z3_results[0] == "unsat"){
         m_logger->unsat_solution();
        exit(0);
    } else if (z3_results[0] == "unknown"){
        m_logger->unknown_solution();
        exit(0);
    } else {
        assert (0);
    }
    m_solutions = 1;

    for (size_t i = 1; i < z3_results.size(); ++i){
        std::string line = z3_results[i];
        
        if (line == "sat"){
            m_solutions++;
        } else if (line == "unsat"){
            m_logger->unsat_solution();
            break;
        } else if (line == "unknown"){
            m_logger->unknown_solution();
            break;
        }

        std::string opening = line.substr(0,2);
        std::string closing = line.substr(line.size() -2, 2);

        if (opening == "((" && closing == "))"){
            std::string content = line.substr(2, line.size() - 2);
            content = content.substr(0, content.size() - 2);
            std::vector<std::string> token = Utils::Utils::tokenize(content, " ");
            std::string key = token[0];
            size_t val = std::stoi(token[1]);
            
            key_value_results[key].push_back(val);
        }
    }

    for (size_t i = 0; i < m_solutions; ++i){
        this->process_key_value_results(key_value_results, i);
    }
}

/**
 * @brief Process a generated result
 * 
 * @param solution Results
 * @param id ID to Process
 */
void MacroCircuit::process_key_value_results(std::map<std::string, std::vector<size_t>> & solution, size_t const id)
{
    size_t ux = solution[m_layout->get_ux().to_string()][id];
    size_t uy = solution[m_layout->get_uy().to_string()][id];

    double area_estimation = ux * uy;
    double white_space = 100 - ((m_estimated_area/area_estimation)*100.0);
    m_logger->result_die_area(area_estimation);
    m_logger->white_space(white_space);

    m_layout->set_solution_ux(ux);
    m_layout->set_solution_uy(uy);
    m_logger->add_solution_layout(ux, uy);
    
    for (Terminal* terminal: m_terminals){
        z3::expr clause_x = terminal->get_pos_x();
        z3::expr clause_y = terminal->get_pos_y();

        if (this->get_free_terminals()){
            size_t val_x = solution[clause_x.to_string()][id];
            size_t val_y = solution[clause_y.to_string()][id];

            terminal->add_solution_pos_x(val_x);
            terminal->add_solution_pos_y(val_y);

            m_logger->place_terminal(terminal->get_name(),
                                     val_x,
                                     val_y);
        }
    }

    for(Component* component: m_components){

        size_t x = solution[component->get_lx().to_string()][id];
        size_t y = solution[component->get_ly().to_string()][id];
        eOrientation o = static_cast<eOrientation>(solution[component->get_orientation().to_string()][id]);

        component->add_solution_lx(x);
        component->add_solution_ly(y);
        component->add_solution_orientation(o);

        m_logger->place_macro(component->get_id(), x ,y, o);

        //if (this->get_minimize_hpwl_mode()){
            std::vector<Pin*> pins = component->get_pins();

            for (Pin* p: pins){
                if (p->is_free()){
                    z3::expr x = p->get_pin_pos_x();
                    z3::expr y = p->get_pin_pos_y();

                    size_t x_pos = solution[x.to_string()][id];
                    size_t y_pos = solution[y.to_string()][id];

                    p->add_solution_pin_pos_x(x_pos);
                    p->add_solution_pin_pos_y(y_pos);
                }
            }
        //}
    }
}

void MacroCircuit::dump_minizinc()
{
    size_t area = m_estimated_area;
    double root = std::sqrt(area);
    size_t up = static_cast<size_t>(std::ceil(root));
    std::string range = "var 0.." + std::to_string(up) + ": ";
    
    std::ofstream file("file.mzn");
    
    file << range << m_layout->get_ux() << ";" << std::endl;
    file << range << m_layout->get_uy() << ";" << std::endl;
    
    for(Macro* m: m_macros){
        file << range << m->get_lx() << ";" << std::endl;
        file << range << m->get_ly() << ";" << std::endl;
        file << "var 0..1: " << m->get_orientation() << ";" << std::endl;
    }
    
    for (auto itor: m_components_inside_die_constraints){
        file << itor << std::endl;
    }
    for (auto itor: m_components_non_overlapping_constraints){
        file << itor << std::endl;
    }
    file << std::endl;
    file << "var int: area = die_uy*die_ux;" << std::endl;
    file << "solve minimize area;" << std::endl;
    file.close();
}
