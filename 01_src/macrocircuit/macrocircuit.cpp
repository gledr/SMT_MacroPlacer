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
MacroCircuit::MacroCircuit(z3::context* z3_ctx):
    Object(),
    m_z3_ctx(z3_ctx),
    m_components_non_overlapping(z3_ctx->int_val(0)),
    m_components_inside_die(z3_ctx->int_val(0)),
    m_terminals_on_frontier(z3_ctx->int_val(0)),
    m_terminals_non_overlapping(z3_ctx->int_val(0))
{
    m_z3_opt = new z3::optimize(*z3_ctx);
    m_layout = new Layout(z3_ctx);
    m_eval = new Evaluate(this);
    m_supplement = new Supplement();
    m_logger = Utils::Logger::getInstance();
    m_timer = new Utils::Timer();
    m_partitioning = new Partitioning(z3_ctx);
    m_parquet = new ParquetFrontend();
    m_z3_utils = new EncodingUtils();

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
    //delete m_tree; m_tree = nullptr;
    delete m_z3_opt; m_z3_opt = nullptr;
    delete m_z3_ctx; m_z3_ctx = nullptr;
    delete m_eval; m_eval = nullptr;
    delete m_supplement; m_supplement = nullptr;
    delete m_layout; m_layout = nullptr;
    delete m_partitioning; m_partitioning = nullptr;
    delete m_parquet; m_parquet = nullptr;
    delete m_z3_utils; m_z3_utils = nullptr;
    delete m_timer; m_timer = nullptr;
    
    m_logger = nullptr;
}

/**
 * @brief Build Circuit from LEF/DEF or Bookshelf Files
 */
void MacroCircuit::build_circuit()
{
    try {
        if(!this->get_def().empty() && !this->get_lef().empty()) {
            m_supplement->read_supplement_file();
            m_circuit = new Circuit::Circuit(this->get_lef(),
                                             this->get_def());
            m_tree = new Tree(m_z3_ctx);
            assert (m_tree != nullptr);
            assert (m_circuit != nullptr);
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

            std::thread area_estimator(&MacroCircuit::area_estimator, this);
            area_estimator.join();

            m_logger->min_die_area(m_estimated_area);
            std::thread macro_worker(&MacroCircuit::add_macros, this);
            std::thread cell_worker(&MacroCircuit::add_cells, this);
            std::thread terminal_worker(&MacroCircuit::add_terminals, this);

            macro_worker.join();
            cell_worker.join();
            terminal_worker.join();

            m_layout->set_units(m_circuit->defUnit);

            this->build_tree();
            //m_tree->visualize();

        } else if (!this->get_bookshelf_file().empty()){
            m_bookshelf = new Bookshelf(m_z3_ctx);
            m_bookshelf->read_files();
            this->set_design_name(m_bookshelf->get_design_name());
            m_estimated_area = m_bookshelf->get_estimated_area();
            m_logger->min_die_area(m_estimated_area);
            m_layout->set_min_die_predition(m_estimated_area);
            m_macros = m_bookshelf->get_macros();
            m_terminals = m_bookshelf->get_terminals();
            this->set_design_name(m_bookshelf->get_design_name());
            m_tree = m_bookshelf->get_tree();
            m_tree->construct_tree();
            //m_tree->visualize();
        } else {
            throw("Invalid Input Files Defined!");
        }
    } catch (std::exception const & exp){
        std::cerr << exp.what() << std::endl;
        exit(0);
    }
}

void MacroCircuit::partitioning()
{
    if(this->get_partitioning()){
        m_logger->run_partitioning();
        m_partitioning->set_problem(m_macros, this->get_partition_size());
        m_partitioning->run();
        std::vector<Component*> partitions = m_partitioning->get_partitions();

        for(auto itor: partitions){
            m_partitons.push_back(dynamic_cast<Partition*>(itor));
        }
    }
}

/**
 * @brief 
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
 * @brief 
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
 * @brief 
 */
void MacroCircuit::encode_smt()
{
    m_layout->set_lx(0);
    m_layout->set_ly(0);
    m_layout->free_uy();
    m_layout->free_ux();

    this->config_z3();
    this->run_encoding();
}

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

void MacroCircuit::add_macros()
{
    m_logger->start_macro_thread();

    for(auto& itor: m_circuit->defComponentStor){
        if(this->is_macro(itor)){
            this->add_macro(itor);
        }
    }
    m_logger->end_macro_thread();
}

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

void MacroCircuit::add_terminals()
{
    m_logger->start_terminal_thread();

    for(auto & itor: m_circuit->defPinStor){
        int x = itor.placementX();
        int y = itor.placementY();
        
        e_pin_direction direction = Pin::string2enum(itor.direction());
        Terminal* tmp;
        
        if(x == 0 || y == 0){
            tmp = new Terminal(itor.pinName(), m_z3_ctx, direction);
        } else {
            tmp = new Terminal(itor.pinName(), x, y, m_z3_ctx, direction);
        }
        assert(tmp != nullptr);
        
        m_terminals.push_back(tmp);
        m_id2terminal[itor.pinName()] = tmp;
    }
    
    m_logger->end_terminal_thread();
}

void MacroCircuit::dump_all()
{
    m_logger->dump_all();
    
    for(size_t i = 0; i < m_solutions; ++i){
        this->create_image(i);
    }
}

void MacroCircuit::dump_best()
{
    std::pair<size_t, size_t> best_hpwl = m_eval->best_hpwl();
    this->create_image(best_hpwl.first);
    std::cout << "Best Solution: HPWL = " << best_hpwl.second << std::endl;
}

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

    if(this->get_partitioning()){
        for(size_t j = 0; j < m_partitons.size(); ++j){
            gnu_plot_file << "# " << m_macros[j]->get_id() << ":" << m_macros[j]->get_name() 
                        << " Orientation: " << m_macros[j]->get_solution_orientation(solution) << std::endl;

            size_t width = m_partitons[j]->get_width().get_numeral_uint();
            size_t height = m_partitons[j]->get_height().get_numeral_uint();
        
            // North
            if(m_partitons[j]->get_orientation().get_numeral_uint() == eNorth){
                _lx = m_partitons[j]->get_lx().get_numeral_uint();
                _ly = m_partitons[j]->get_ly().get_numeral_uint();
                _ux = m_partitons[j]->get_lx().get_numeral_uint() + width;
                _uy = m_partitons[j]->get_ly().get_numeral_uint() + height;

            // West
            } else if (m_partitons[j]->get_orientation().get_numeral_uint() == eWest){
                _lx = m_partitons[j]->get_lx().get_numeral_uint() - height;
                _ly = m_partitons[j]->get_ly().get_numeral_uint();
                _ux = m_partitons[j]->get_lx().get_numeral_uint();
                _uy = m_partitons[j]->get_ly().get_numeral_uint() + width;

            // South
            } else if (m_partitons[j]->get_orientation().get_numeral_uint() == eSouth){
                _lx = m_partitons[j]->get_lx().get_numeral_uint() - width;
                _ly = m_partitons[j]->get_ly().get_numeral_uint() - height;
                _ux = m_partitons[j]->get_lx().get_numeral_uint();
                _uy = m_partitons[j]->get_ly().get_numeral_uint();

            // East
            } else if (m_partitons[j]->get_orientation().get_numeral_uint() == eEast){
                _lx = m_partitons[j]->get_lx().get_numeral_uint();
                _ly = m_partitons[j]->get_ly().get_numeral_uint() - width;
                _ux = m_partitons[j]->get_lx().get_numeral_uint() + height;
                _uy = m_partitons[j]->get_ly().get_numeral_uint();
            // Error
            } else {
                    assert (0);
            }
             gnu_plot_file  << "set object " << j+1 << " rect from " << std::to_string(_lx) << "," << std::to_string(_ly) 
                   << " to "  << std::to_string(_ux) << ","<< std::to_string(_uy) <<" lw 5;"<< std::endl;
        }
    } else {
        for(size_t j = 0; j < m_macros.size(); ++j){
            gnu_plot_file << "# " << m_macros[j]->get_id() << ":" << m_macros[j]->get_name() 
                        << " Orientation: " << m_macros[j]->get_solution_orientation(solution) << std::endl;

            size_t width = m_macros[j]->get_width().get_numeral_uint();
            size_t height = m_macros[j]->get_height().get_numeral_uint();
        
            // North
            if(m_macros[j]->get_solution_orientation(solution) == eNorth){
                _lx = m_macros[j]->get_solution_lx(solution);
                _ly = m_macros[j]->get_solution_ly(solution);
                _ux = m_macros[j]->get_solution_lx(solution) + width;
                _uy = m_macros[j]->get_solution_ly(solution) + height;

            // West
            } else if (m_macros[j]->get_solution_orientation(solution) == eWest){
                _lx = m_macros[j]->get_solution_lx(solution) - height;
                _ly = m_macros[j]->get_solution_ly(solution);
                _ux = m_macros[j]->get_solution_lx(solution);
                _uy = m_macros[j]->get_solution_ly(solution) + width;

            // South
            } else if (m_macros[j]->get_solution_orientation(solution) == eSouth){
                _lx = m_macros[j]->get_solution_lx(solution) - width;
                _ly = m_macros[j]->get_solution_ly(solution) - height;
                _ux = m_macros[j]->get_solution_lx(solution);
                _uy = m_macros[j]->get_solution_ly(solution);

            // East
            } else if (m_macros[j]->get_solution_orientation(solution) == eEast){
                _lx = m_macros[j]->get_solution_lx(solution);
                _ly = m_macros[j]->get_solution_ly(solution) - width;
                _ux = m_macros[j]->get_solution_lx(solution) + height;
                _uy = m_macros[j]->get_solution_ly(solution);
            // Error
            } else {
                    assert (0);
            }
            gnu_plot_file  << "set object " << j+1 << " rect from " << std::to_string(_lx) << "," << std::to_string(_ly) 
                   << " to "  << std::to_string(_ux) << ","<< std::to_string(_uy) <<" lw 5;"<< std::endl;
        }
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

void MacroCircuit::save_all()
{
    for(size_t i = 0; i < m_solutions; ++i){
        std::string name = "placed_" + std::to_string(i) + "_" + this->get_def();
        this->write_def(name, i);
    }
}

void MacroCircuit::save_best()
{
    std::cout << "in save best" << std::endl;
    std::pair<size_t, size_t> best_hpwl = m_eval->best_hpwl();
    std::string name = "best_" + this->get_def();
    this->write_def(name, best_hpwl.first);
}

void MacroCircuit::build_tree()
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
                assert (0);
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
                assert (0);
            }

            if(_case.str() == "mm"){
                assert (from_m != nullptr);
                assert (to_m != nullptr);
                m_tree->insert_edge<Macro, Macro>(from_m, to_m, itor.pin(0), itor.pin(i), itor.name());

            } else if (_case.str() == "mc"){
                assert (from_m != nullptr);
                assert (to_c != nullptr);

                m_tree->insert_edge<Macro, Cell>(from_m, to_c, itor.pin(0), itor.pin(i), itor.name());
            } else if (_case.str() == "mt"){
                  assert (from_m != nullptr);
                  assert (to_t != nullptr);
                m_tree->insert_edge<Macro, Terminal>(from_m, to_t, itor.pin(0), itor.pin(i), itor.name());

            } else if (_case.str() == "cm"){
                assert (from_c != nullptr);
                assert (to_m != nullptr);
                m_tree->insert_edge<Cell, Macro>(from_c, to_m, itor.pin(0), itor.pin(i), itor.name());

            } else if (_case.str() == "cc"){
                assert (from_c != nullptr);
                assert (to_c != nullptr);
                m_tree->insert_edge<Cell, Cell>(from_c, to_c, itor.pin(0), itor.pin(i), itor.name());

            } else if (_case.str() == "ct"){
                assert (from_c != nullptr);
                assert (to_t != nullptr);
                m_tree->insert_edge<Cell, Terminal>(from_c, to_t, itor.pin(0), itor.pin(i), itor.name());

            } else if (_case.str() == "tm"){
                assert (from_t != nullptr);
                assert (to_m != nullptr);
                m_tree->insert_edge<Terminal, Macro>(from_t, to_m, itor.pin(0), itor.pin(i), itor.name());

            } else if (_case.str() == "tc"){
                assert (from_t != nullptr);
                assert (to_c != nullptr);
                m_tree->insert_edge<Terminal, Cell>(from_t, to_c, itor.pin(0), itor.pin(i), itor.name());

            } else if (_case.str() == "tt"){
                assert (from_t != nullptr);
                assert (to_t != nullptr);
                m_tree->insert_edge<Terminal, Terminal>(from_t, to_t, itor.pin(0), itor.pin(i), itor.name());

            } else {
                assert (0);
            }
        }
    }
    
    m_tree->construct_tree();
}

void MacroCircuit::add_macro(LefDefParser::defiComponent const & cmp)
{
    std::string name = cmp.name();
    std::string id   = cmp.id();

    auto idx = m_circuit->lefMacroMap.find(name);
    LefDefParser::lefiMacro& lef_data = m_circuit->lefMacroStor[idx->second];

    size_t width = lef_data.sizeX();
    size_t height = lef_data.sizeY();

    std::vector<LefDefParser::lefiPin> lef_pins = m_circuit->lefPinStor[idx->second];
    Macro*m = new Macro(m_z3_ctx, name, id, width, height);
    for(auto itor: lef_pins){
        Pin* p = new Pin(itor.name(), id, Pin::string2enum(itor.direction()), m_z3_ctx);
        assert (p != nullptr);
        m->add_pin(p);
    }
    
    if(m_supplement->has_supplement()){
        m->set_supplement(m_supplement);
    }

    m_macros.push_back(m);
    m_id2macro[id] = m;
}

void MacroCircuit::add_cell(LefDefParser::defiComponent const & cmp)
{
    std::string name = cmp.name();
    std::string id   = cmp.id();

    Cell* c = new Cell(m_z3_ctx, name, id);
    m_cells.push_back(c);
    m_id2cell[id] = c;
}

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

bool MacroCircuit::is_macro(LefDefParser::defiComponent const & macro)
{
    auto idx = m_circuit->lefMacroMap.find(macro.name());
    LefDefParser::lefiMacro& lef_data = m_circuit->lefMacroStor[idx->second];

    return lef_data.sizeY() != m_standard_cell_height;
}

bool MacroCircuit::is_standard_cell(LefDefParser::defiComponent const & cell)
{
    auto idx = m_circuit->lefMacroMap.find(cell.name());
    LefDefParser::lefiMacro& lef_data = m_circuit->lefMacroStor[idx->second];

    return lef_data.sizeY() == m_standard_cell_height;
}

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

size_t MacroCircuit::get_minimal_die_size_prediction()
{
    return m_estimated_area;
}

void MacroCircuit::area_estimator() 
{
    m_estimated_area = 0;

    for(auto& itor: m_circuit->defComponentStor){
        std::string name = itor.name();
        if(this->is_macro(itor)){
            auto idx = m_circuit->lefMacroMap.find(name);
            LefDefParser::lefiMacro& lef_data = m_circuit->lefMacroStor[idx->second];

            size_t width = lef_data.sizeX();
            size_t height = lef_data.sizeY();
            m_estimated_area += (width * height);
        }
    }
    m_layout->set_min_die_predition(m_estimated_area);
}

void MacroCircuit::store_results()
{
    if(!m_layout->get_ux().is_const()){
        std::ofstream results(this->get_results_directory() + "/" + std::to_string(this->get_results_id()) + "/results.txt");
        for(size_t i = 0; i < m_solutions; i++){
            results << "Solution: " << i << std::endl;
            results << "die_ux:" << m_layout->get_solution_ux(i) << std::endl;
            results << "die_uy:" << m_layout->get_solution_uy(i) << std::endl;
            results << "die_area:" << ((((double)m_layout->get_solution_ux(i)) * ((double)m_layout->get_solution_uy(i)))/1000000);
            results << std::endl;
        }

        size_t best_area = m_layout->get_idx_best_solution();
        results << "Best Result [Geometric]: " << best_area << std::endl;
        results << "die_ux:" << m_layout->get_solution_ux(best_area) << std::endl;
        results << "die_uy:" << m_layout->get_solution_uy(best_area) << std::endl;
        results << "die_area:" << std::fixed << ((((double)m_layout->get_solution_ux(best_area)) * ((double)m_layout->get_solution_uy(best_area)))/1000000);

        results.close();
    }
}

std::pair<size_t, size_t> MacroCircuit::biggest_macro()
{
    size_t w = 0;
    size_t h = 0;

    for(auto itor: m_macros){
        if(itor->get_width().get_numeral_uint() > w){
            w = itor->get_width().get_numeral_uint();
        }
        if(itor->get_height().get_numeral_uint() > h){
            h = itor->get_height().get_numeral_uint();
        }
    }
    return std::make_pair(w,h);
}

std::vector<Macro*>& MacroCircuit::get_macros()
{
    return m_macros;
}

std::vector<Partition *> & MacroCircuit::get_partitions()
{
    return m_partitons;
}

std::vector<Terminal*> MacroCircuit::get_terminals()
{
    return m_terminals;
}

std::vector<Component*> MacroCircuit::get_components()
{
    return m_components;
}

Tree* MacroCircuit::get_tree()
{
    return m_tree;
}

Layout* MacroCircuit::get_layout()
{
    return m_layout;
}

Circuit::Circuit* MacroCircuit::get_circuit()
{
    return m_circuit;
}

void MacroCircuit::set_solutions(size_t const solutions)
{
    m_solutions = solutions;
}

size_t MacroCircuit::get_solutions()
{
    return m_solutions;
}

/**
 * @brief 
 */
void MacroCircuit::config_z3()
{
    z3::params param(*m_z3_ctx);
    
    m_z3_ctx->set("logic", "LIA");
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
        std::cout << "Default Optimiter: Lex" << std::endl;
        param.set(":opt.priority", "lex");
    }
    
    if(this->get_timeout() != 0){
      m_logger->use_timeout(this->get_timeout());
      param.set(":timeout", (unsigned)this->get_timeout() * 1000);
    }

    m_z3_opt->set(param);
}

/**
 * @brief 
 */
void MacroCircuit::run_encoding()
{
    this->encode_components_inside_die(e2D);
    this->encode_components_non_overlapping(e2D);
    
    m_z3_opt->add(m_components_inside_die);
    m_z3_opt->add(m_components_non_overlapping);
    
    m_z3_opt->minimize(m_layout->get_ux());
    m_z3_opt->minimize(m_layout->get_uy());
   
    if(this->get_box_optimizer()){
        m_z3_opt->minimize(m_layout->get_uy() + m_layout->get_ux());
        m_z3_opt->check();
    }
}

void MacroCircuit::encode_components_inside_die(eRotation const type)
{
    try {
        z3::expr_vector clauses(*m_z3_ctx);

        z3::expr die_lx = m_layout->get_lx();
        z3::expr die_ux = m_layout->get_ux();
        z3::expr die_ly = m_layout->get_ly();
        z3::expr die_uy = m_layout->get_uy();
        
        for (Component* itor: m_components){
            z3::expr is_N = itor->get_orientation() == m_z3_ctx->int_val(eNorth);
            z3::expr is_W = itor->get_orientation() == m_z3_ctx->int_val(eWest);
            z3::expr is_S = itor->get_orientation() == m_z3_ctx->int_val(eSouth);
            z3::expr is_E = itor->get_orientation() == m_z3_ctx->int_val(eEast);

            z3::expr_vector case_N(*m_z3_ctx);
            case_N.push_back(m_z3_utils->ge(itor->get_lx(), die_lx));                                           ///< LX
            case_N.push_back(m_z3_utils->ge(itor->get_ly(), die_ly));                                           ///< LY
            case_N.push_back(m_z3_utils->le(m_z3_utils->add(itor->get_lx(), itor->get_width()), die_ux));       ///< UX
            case_N.push_back(m_z3_utils->le(m_z3_utils->add(itor->get_ly(), itor->get_height()), die_uy));      ///< UY

            z3::expr_vector case_W(*m_z3_ctx);
            case_W.push_back(m_z3_utils->ge(m_z3_utils->sub(itor->get_lx(), itor->get_height()),die_lx));       ///< LX
            case_W.push_back(m_z3_utils->ge(itor->get_ly(), die_ly));                                           ///< LY
            case_W.push_back(m_z3_utils->le(itor->get_lx(), die_ux));                                           ///< UX
            case_W.push_back(m_z3_utils->le(m_z3_utils->add(itor->get_ly(), itor->get_width()),die_uy));        ///< UY

            z3::expr_vector case_S(*m_z3_ctx);
            z3::expr s_x = m_z3_utils->sub(itor->get_lx(), itor->get_width());
            z3::expr s_y = m_z3_utils->sub(itor->get_ly(), itor->get_height());
            case_S.push_back(m_z3_utils->ge(s_x, die_lx));                ///< LX
            case_S.push_back(m_z3_utils->ge(s_y, die_ly));                ///< LY
            case_S.push_back(m_z3_utils->le(itor->get_lx(), die_ux));     ///< UX
            case_S.push_back(m_z3_utils->le(itor->get_ly(), die_uy));     ///< UY

            z3::expr_vector case_E(*m_z3_ctx);
            case_E.push_back(m_z3_utils->ge(itor->get_lx(), die_lx));
            case_E.push_back(m_z3_utils->ge(m_z3_utils->sub(itor->get_ly(), itor->get_width()), die_ly));
            case_E.push_back(m_z3_utils->le(m_z3_utils->add(itor->get_lx(), itor->get_height()), die_ux));
            case_E.push_back(m_z3_utils->le(itor->get_ly(), die_uy));
            
            if(type == eRotation::e2D){
                z3::expr ite = z3::ite(is_N, z3::mk_and(case_N),
                               z3::ite(is_W, z3::mk_and(case_W), m_z3_ctx->bool_val(false)));
                clauses.push_back(ite);
                
            } else if (type == eRotation::e4D){
                z3::expr ite = z3::ite(is_N, z3::mk_and(case_N),
                               z3::ite(is_E, z3::mk_and(case_E),
                               z3::ite(is_S, z3::mk_and(case_S),
                               z3::ite(is_W, z3::mk_and(case_W), m_z3_ctx->bool_val(false)))));
                clauses.push_back(ite);
            } else {
                assert (0);
            }

            if(type == eRotation::e2D){
                clauses.push_back(m_z3_utils->ge(itor->get_orientation(), m_z3_ctx->int_val(eNorth)));
                clauses.push_back(m_z3_utils->le(itor->get_orientation(), m_z3_ctx->int_val(eWest)));
            } else if (type == eRotation::e4D){
                clauses.push_back(itor->get_orientation() >= m_z3_ctx->int_val(eNorth));
                clauses.push_back(itor->get_orientation() <= m_z3_ctx->int_val(eEast));
            } else {
                assert (0);
            }
        }

        m_components_inside_die = z3::mk_and(clauses);

    } catch (z3::exception const & exp){
        std::cout << exp.msg() << std::endl;
        exit(0);
    }
}

void MacroCircuit::encode_components_non_overlapping(eRotation const type)
{
     try {
        z3::expr_vector clauses(*m_z3_ctx);

        z3::expr N = m_z3_ctx->int_val(eNorth);
        z3::expr W = m_z3_ctx->int_val(eWest);
        z3::expr S = m_z3_ctx->int_val(eSouth);
        z3::expr E = m_z3_ctx->int_val(eEast);

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
                z3::expr_vector case_nn(*m_z3_ctx);
                /* Right */
                case_nn.push_back(m_z3_utils->ge(free->get_lx(), 
                                  m_z3_utils->add(fixed->get_lx(), fixed->get_width())));
                /* Left */
                case_nn.push_back(m_z3_utils->le(m_z3_utils->add(free->get_lx(), free->get_width()),
                                           fixed->get_lx()));
                /* Upper */
                case_nn.push_back(m_z3_utils->ge(free->get_ly(),
                                           m_z3_utils->add(fixed->get_ly(), fixed->get_height())));
                /* Below */
                case_nn.push_back(m_z3_utils->le(m_z3_utils->add(free->get_ly(), free->get_height()),
                                           fixed->get_ly()));
//}}}
//{{{           Case North West
                z3::expr_vector case_nw(*m_z3_ctx);
                /* Right */
                case_nw.push_back(m_z3_utils->ge(m_z3_utils->sub(free->get_lx(), free->get_height()),
                                           m_z3_utils->add(fixed->get_lx(), fixed->get_width())));
                 /* Left */
                case_nw.push_back(m_z3_utils->le(free->get_lx(), fixed->get_lx()));
                /* Upper */
                case_nw.push_back(m_z3_utils->ge(free->get_ly(), 
                                           m_z3_utils->add(fixed->get_ly(), fixed->get_height())));
                /* Below */
                case_nw.push_back(m_z3_utils->le(m_z3_utils->add(free->get_ly(), free->get_width()),
                                           fixed->get_ly()));
//}}}
//{{{           Case North South
                z3::expr_vector case_ns(*m_z3_ctx);
                 /* Right */
                case_ns.push_back(m_z3_utils->ge(m_z3_utils->sub(free->get_lx(), free->get_width()),
                                           m_z3_utils->add(fixed->get_lx(), fixed->get_width())));
                 /* Left */
                case_ns.push_back(m_z3_utils->le(free->get_lx(),fixed->get_lx()));
                /* Upper */
                case_ns.push_back(m_z3_utils->ge(m_z3_utils->sub(free->get_ly(), free->get_height()),
                                           m_z3_utils->add(fixed->get_ly(), fixed->get_width())));
                /* Below */
                case_ns.push_back(m_z3_utils->le(free->get_ly(),fixed->get_ly()));
//}}}
//{{{           Case North East
                z3::expr_vector case_ne(*m_z3_ctx);
                /* Right */
                case_ne.push_back(m_z3_utils->ge(free->get_lx(),
                                  m_z3_utils->add(fixed->get_lx(), fixed->get_width())));
                 /* Left */
                case_ne.push_back(m_z3_utils->le(m_z3_utils->add(free->get_lx(), free->get_height()),
                                           fixed->get_lx()));
                /* Upper */
                case_ne.push_back(m_z3_utils->ge(m_z3_utils->sub(free->get_ly(), free->get_width()),
                                  m_z3_utils->add(fixed->get_ly(), fixed->get_height())));
                /* Below */
                case_ne.push_back(m_z3_utils->le(free->get_ly(), fixed->get_ly()));
//}}}
//{{{           Case West North
                z3::expr_vector case_wn(*m_z3_ctx);
                /* Right */
                case_wn.push_back(m_z3_utils->ge(free->get_lx(), fixed->get_lx()));
                 /* Left */
                case_wn.push_back(m_z3_utils->le(m_z3_utils->add(free->get_lx(), free->get_width()),
                                           m_z3_utils->sub(fixed->get_lx(), fixed->get_height())));
                /* Upper */
                case_wn.push_back(m_z3_utils->ge(free->get_ly(),
                                           m_z3_utils->add(fixed->get_ly(), fixed->get_width())));
                /* Below */
                case_wn.push_back(m_z3_utils->le(m_z3_utils->add(free->get_ly(), free->get_height()),
                                           fixed->get_ly()));
//}}}                
//{{{           Case West West
                z3::expr_vector case_ww(*m_z3_ctx);
                /* Right */
                case_ww.push_back(m_z3_utils->ge(m_z3_utils->sub(free->get_lx(), free->get_height()),
                                           fixed->get_lx()));
                /* Left */
                case_ww.push_back(m_z3_utils->le(free->get_lx(),
                                           m_z3_utils->sub(fixed->get_lx(), fixed->get_height())));
                /* Upper */
                case_ww.push_back(m_z3_utils->ge(free->get_ly(),
                                           m_z3_utils->add(fixed->get_ly(),  fixed->get_width())));
                /* Below */
                case_ww.push_back(m_z3_utils->le(m_z3_utils->add(free->get_ly(), free->get_width()),
                                           fixed->get_ly()));
//}}}
//{{{           Case West South
                z3::expr_vector case_ws(*m_z3_ctx);
                /* Right */
                case_ws.push_back(m_z3_utils->ge(m_z3_utils->sub(free->get_lx(), free->get_width()),
                                           fixed->get_lx()));
                 /* Left */
                case_ws.push_back(m_z3_utils->le(free->get_lx(), 
                                           m_z3_utils->sub(fixed->get_lx(), fixed->get_height())));
                /* Upper */
                case_ws.push_back(m_z3_utils->ge(m_z3_utils->sub(free->get_ly(), free->get_height()),
                                           m_z3_utils->add(fixed->get_ly(), fixed->get_width())));
                /* Below */
                case_ws.push_back(m_z3_utils->le(free->get_ly(), fixed->get_ly()));
//}}}
//{{{           Case West East
                z3::expr_vector case_we(*m_z3_ctx);
                /* Right */
                case_we.push_back(m_z3_utils->ge(free->get_lx(), fixed->get_lx()));
                 /* Left */
                case_we.push_back(m_z3_utils->le(m_z3_utils->add(free->get_lx(), free->get_height()), 
                                           m_z3_utils->sub(fixed->get_lx(), fixed->get_height())));
                /* Upper */
                case_we.push_back(m_z3_utils->ge(m_z3_utils->sub(free->get_ly(), free->get_width()),
                                           m_z3_utils->add(fixed->get_ly(), fixed->get_width())));
                /* Below */
                case_we.push_back(m_z3_utils->le(free->get_ly(), fixed->get_ly()));
//}}}
//{{{           Case South North
                z3::expr_vector case_sn(*m_z3_ctx);
                /* Right */
                case_sn.push_back(m_z3_utils->ge(free->get_lx(), fixed->get_lx()));
                 /* Left */
                case_sn.push_back(m_z3_utils->le(m_z3_utils->add(free->get_lx(), free->get_width()),
                                           m_z3_utils->sub(fixed->get_lx(), fixed->get_width())));
                /* Upper */
                case_sn.push_back(m_z3_utils->ge(free->get_ly(), fixed->get_ly()));
                /* Below */
                case_sn.push_back(m_z3_utils->le(m_z3_utils->add(free->get_ly(), free->get_height()),
                                           m_z3_utils->sub(fixed->get_ly(), fixed->get_height())));
//}}}
//{{{           Case South West
                z3::expr_vector case_sw(*m_z3_ctx);
                 /* Right */
                case_sw.push_back(m_z3_utils->ge(m_z3_utils->sub(free->get_lx(), free->get_height()),
                                           fixed->get_lx()));
                 /* Left */
                case_sw.push_back(m_z3_utils->le(free->get_lx(), 
                                           m_z3_utils->sub(fixed->get_lx(), fixed->get_width())));
                /* Upper */
                case_sw.push_back(m_z3_utils->ge(free->get_ly(),fixed->get_ly()));
                /* Below */
                case_sw.push_back(m_z3_utils->le(m_z3_utils->add(free->get_ly(), free->get_width()),
                                           m_z3_utils->sub(fixed->get_ly(), fixed->get_height())));
//}}}
//{{{           Case South South
                z3::expr_vector case_ss(*m_z3_ctx);
                /* Right */
                case_ss.push_back(m_z3_utils->ge(m_z3_utils->sub(free->get_lx(), free->get_width()),
                                           fixed->get_lx()));
                /* Left */
                case_ss.push_back(m_z3_utils->le(free->get_lx(), 
                                           m_z3_utils->sub(fixed->get_lx(),fixed->get_width())));
                /* Upper */
                case_ss.push_back(m_z3_utils->ge(m_z3_utils->sub(free->get_ly(), free->get_height()),
                                           fixed->get_ly()));
                /* Below */
                case_ss.push_back(m_z3_utils->le(free->get_ly(), 
                                           m_z3_utils->sub(fixed->get_ly(), fixed->get_height())));
//}}}
//{{{           Case South East
                z3::expr_vector case_se(*m_z3_ctx);
                /* Right */
                case_se.push_back(m_z3_utils->ge(free->get_lx(), fixed->get_lx()));
                 /* Left */
                case_se.push_back(m_z3_utils->le(m_z3_utils->add(free->get_lx(), free->get_height()),
                                           m_z3_utils->sub(fixed->get_lx(), fixed->get_width())));
                /* Upper */
                case_se.push_back(m_z3_utils->ge(m_z3_utils->sub(free->get_ly(), free->get_width()),
                                           fixed->get_ly()));
                /* Below */
                case_se.push_back(m_z3_utils->le(free->get_ly(), 
                                           m_z3_utils->sub(fixed->get_ly(), fixed->get_height())));
//}}}
//{{{           Case East North
                z3::expr_vector case_en(*m_z3_ctx);
                 /* Right */
                case_en.push_back(m_z3_utils->ge(free->get_lx(),
                                           m_z3_utils->add(fixed->get_lx(), fixed->get_width())));
                 /* Left */
                case_en.push_back(m_z3_utils->le(m_z3_utils->add(free->get_lx(), fixed->get_height()),
                                           fixed->get_lx()));
                /* Upper */
                case_en.push_back(m_z3_utils->ge(free->get_ly(), fixed->get_ly()));
                /* Below */
                case_en.push_back(m_z3_utils->le(m_z3_utils->add(free->get_ly(), free->get_width()),
                                           m_z3_utils->sub(fixed->get_ly(), fixed->get_height())));
//}}}
//{{{           Case East West
                z3::expr_vector case_ew(*m_z3_ctx);
                /* Right */
                case_ew.push_back(m_z3_utils->ge(m_z3_utils->sub(free->get_lx(), free->get_height()),
                                           m_z3_utils->add(fixed->get_lx(), fixed->get_height())));
                 /* Left */
                case_ew.push_back(m_z3_utils->le(free->get_lx(), fixed->get_lx()));
                /* Upper */
                case_ew.push_back(m_z3_utils->ge(free->get_ly(), fixed->get_ly()));
                /* Below */
                case_ew.push_back(m_z3_utils->le(m_z3_utils->add(free->get_ly(), free->get_width()),
                                           m_z3_utils->sub(fixed->get_ly(), fixed->get_width())));
//}}}
//{{{           Case East South
                z3::expr_vector case_es(*m_z3_ctx);
                /* Right */
                case_es.push_back(m_z3_utils->ge(m_z3_utils->sub(free->get_lx(), free->get_height()),
                                           m_z3_utils->add(fixed->get_lx(), fixed->get_width())));
                 /* Left */
                case_es.push_back(m_z3_utils->le(free->get_lx(), fixed->get_lx()));
                /* Upper */
                case_es.push_back(m_z3_utils->ge(m_z3_utils->sub(free->get_ly(), free->get_width()),
                                           fixed->get_ly()));
                /* Below */
                case_es.push_back(m_z3_utils->le(free->get_ly(), 
                                           m_z3_utils->sub(fixed->get_ly(), fixed->get_height())));
//}}}
//{{{           Case East East
                z3::expr_vector case_ee(*m_z3_ctx);
                /* Right */
                case_ee.push_back(m_z3_utils->ge(free->get_lx(),
                                           m_z3_utils->add(fixed->get_lx(), fixed->get_height())));
                 /* Left */
                case_ee.push_back(m_z3_utils->le(m_z3_utils->add(free->get_lx(), free->get_height()),
                                           fixed->get_lx()));
                /* Upper */
                case_ee.push_back(m_z3_utils->ge(m_z3_utils->sub(free->get_ly(), free->get_width()),
                                           fixed->get_ly()));
                /* Below */
                case_ee.push_back(m_z3_utils->le(free->get_ly(), 
                                           m_z3_utils->sub(fixed->get_ly(), fixed->get_width())));
//}}}
//{{{           Orientation
                z3::expr is_NN((fixed->get_orientation() == N) && (free->get_orientation() == N));
                z3::expr is_NW((fixed->get_orientation() == N) && (free->get_orientation() == W));
                z3::expr is_NS((fixed->get_orientation() == N) && (free->get_orientation() == S));
                z3::expr is_NE((fixed->get_orientation() == N) && (free->get_orientation() == E));
                
                z3::expr is_WN((fixed->get_orientation() == W) && (free->get_orientation() == N));
                z3::expr is_WW((fixed->get_orientation() == W) && (free->get_orientation() == W));
                z3::expr is_WS((fixed->get_orientation() == W) && (free->get_orientation() == S));
                z3::expr is_WE((fixed->get_orientation() == W) && (free->get_orientation() == E));
                
                z3::expr is_SN((fixed->get_orientation() == S) && (free->get_orientation() == N));
                z3::expr is_SW((fixed->get_orientation() == S) && (free->get_orientation() == W));
                z3::expr is_SS((fixed->get_orientation() == S) && (free->get_orientation() == S));
                z3::expr is_SE((fixed->get_orientation() == S) && (free->get_orientation() == E));
                
                z3::expr is_EN((fixed->get_orientation() == E) && (free->get_orientation() == N));
                z3::expr is_EW((fixed->get_orientation() == E) && (free->get_orientation() == W));
                z3::expr is_ES((fixed->get_orientation() == E) && (free->get_orientation() == S));
                z3::expr is_EE((fixed->get_orientation() == E) && (free->get_orientation() == E));
//}}}
//{{{           Encoding
                z3::expr clause = m_z3_ctx->int_val(0);
                
                if(type == eRotation::e2D){
                    clause = z3::ite(is_NN, z3::mk_or(case_nn),
                             z3::ite(is_NW, z3::mk_or(case_nw),
                             z3::ite(is_WN, z3::mk_or(case_wn),
                             z3::ite(is_WW, z3::mk_or(case_ww), m_z3_ctx->bool_val(false)))));
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
                             z3::ite(is_EE, z3::mk_or(case_ee), m_z3_ctx->bool_val(false)
                             ))))))))))))))));
                } else {
                    assert (0);
                }
                clauses.push_back(clause);
//}}}
            }
        }

        m_components_non_overlapping = z3::mk_and(clauses);
        
    } catch (z3::exception const & exp){
        std::cout << exp.msg() << std::endl;
        assert (0);
    }
}

/**
 * @brief Encode Terminals to be on the Dies frontier
 * 
 * @return z3::expr
 */
void MacroCircuit::encode_terminals_on_frontier()
{
    z3::expr_vector clauses(*m_z3_ctx);

    z3::expr die_lx = m_layout->get_lx();
    z3::expr die_ux = m_layout->get_ux();
    z3::expr die_ly = m_layout->get_ly();
    z3::expr die_uy = m_layout->get_uy();

    for(auto itor: m_terminals){
        z3::expr_vector subclause(*m_z3_ctx);
        z3::expr x = itor->get_pin_pos_x();
        z3::expr y = itor->get_pin_pos_y();

        // Case 1 x = moveable, y = ly
        z3::expr_vector case_1(*m_z3_ctx);
        case_1.push_back(y == die_ly);
        case_1.push_back(x > die_lx);
        case_1.push_back(x < die_ux);

        // Case 2 x = moveable, y = uy
        z3::expr_vector case_2(*m_z3_ctx);
        case_2.push_back(y == die_uy);
        case_2.push_back(x > die_lx);
        case_2.push_back(x < die_ux);

        // Case 3 x = lx, y = moveable
        z3::expr_vector case_3(*m_z3_ctx);
        case_3.push_back(x == die_lx);
        case_3.push_back(y > die_ly);
        case_3.push_back(y < die_uy);

        // Case 4 x = ux, y = moveable
        z3::expr_vector case_4(*m_z3_ctx);
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
        z3::expr_vector clauses(*m_z3_ctx);
        
        for(size_t i = 0; i < m_terminals.size(); ++i){
            for(size_t j = 0; j < m_terminals.size(); ++j){
                z3::expr_vector subclause(*m_z3_ctx);
                if(i == j){
                    continue;
                }
                
                Terminal* a = m_terminals[i];
                Terminal* b = m_terminals[j];
                
                assert (a != nullptr);
                assert (b != nullptr);
                
                // Case 1: x moveable
                z3::expr_vector case_1(*m_z3_ctx);
                case_1.push_back(a->get_pin_pos_x() > b->get_pin_pos_x());
                case_1.push_back(a->get_pin_pos_x() < b->get_pin_pos_x());
                
                // Case 2: y moveable
                z3::expr_vector case_2(*m_z3_ctx);
                case_2.push_back(a->get_pin_pos_y() > b->get_pin_pos_y());
                case_2.push_back(a->get_pin_pos_y() < b->get_pin_pos_y());
                
                subclause.push_back(z3::mk_or(case_1));
                subclause.push_back(z3::mk_or(case_2));
                clauses.push_back(z3::mk_or(subclause));
            }
        }
        m_terminals_non_overlapping = z3::mk_and(clauses);
        
    } catch (z3::exception const & exp){
        std::cout << exp.msg() << std::endl;
        assert (0);
    }
}

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
            m_solutions = 1;

            do {
                z3::model m = m_z3_opt->get_model();

                size_t ux =  m.eval(m_layout->get_ux()).get_numeral_uint();
                size_t uy =  m.eval(m_layout->get_uy()).get_numeral_uint();
                    
                double area_estimation = (((ux) * (uy)));
                double min_area = this->get_minimal_die_size_prediction();
                double white_space = 100-0 - ((min_area/area_estimation)*100.0);
                m_logger->result_die_area(area_estimation);
                m_logger->white_space(white_space);

                m_layout->set_solution_ux(ux);
                m_layout->set_solution_uy(uy);
                m_logger->add_solution_layout(ux, uy);
                    
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

                    if(/*this->get_pareto_optimizer() &&*/  (m_solutions < this->get_max_solutions())){
                        m_logger->pareto_step();
                        sat = m_z3_opt->check();
                        m_solutions++;
                    } else {
                        break;
                    }
                } while (sat == z3::check_result::sat);
            } else {
            assert (0);
            }
    } catch (z3::exception const & exp){
        std::cout << exp.msg() << std::endl;
        exit(0);
    }
}

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
    out_file.close();
}

void MacroCircuit::best_result()
{
    std::pair<size_t, size_t> results;
    results.first = UINT_MAX;
    results.second = UINT_MAX;
    
   for (size_t i = 0; i < m_solutions;++i){
        size_t x = m_layout->get_solution_ux(i);
        size_t y = m_layout->get_solution_uy(i);
        size_t a = x*y;
        
        if (a < results.second){
            results = std::make_pair(i, a);
        }
   }
   std::cout << "Min Die Area: " << results.second << std::endl;
}
