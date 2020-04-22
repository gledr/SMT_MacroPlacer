//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : placer.cpp
//
// Date         : 27. December 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Macro Placer Main Class
//==================================================================
#include "placer.hpp"

using namespace Placer;
using namespace Placer::Utils;

/**
 * @brief 
 * 
 * @param argc
 * @param argv
 */
MacroPlacer::MacroPlacer(int const argc, char ** argv):
    Object()
{
    assert (argv != nullptr);

    m_argc = argc;
    m_argv = argv;
}

/**
 * @brief 
 */
MacroPlacer::~MacroPlacer()
{
    Utils::Logger::destroy();
    
    delete m_options_functions; m_options_functions = nullptr;
    delete m_timer;  m_timer = nullptr; 
    delete m_mckt; m_mckt = nullptr;
}

/**
 * @brief 
 */
void MacroPlacer::read_configuration()
{
    try {
        m_options_functions  = new boost::program_options::options_description("Usage: " + this->get_binary_name() + " [options]");

        m_options_functions->add_options()
            ("help",                "Displays information about usage")
            ("bash_completion",     "Generate a Bash Autocompletion Script")
            ("save-best",           "Save Best Solution Generated")
            ("dump-best",           "Shows the best result using Gnuplot")
            ("dump-all",            "Shows all the results using Gnuplot")
            ("allsat",              "Search for all possible solutions")
            ("save-all",            "Save All Generated Solutions")
            ("verbose",             "Show Debug Information")
            ("store-log",           "Store Debug Information to Logfile")
            ("store-smt",           "Specifies if the generated SMT Instance should be stored to the Filesystem")
            ("pareto",              "Utilize Z3s Pareto Optimizer")
            ("lex",                 "Utilize Z3s Lex Optimizer")
            ("box",                 "Utilize Z3s Box Optimizer")
            ("parquet",             "Utilize Parquet Floorplanning")
            ("partition",           "Enable Partitioning Mode")
            ("partition-size",      boost::program_options::value<size_t>(),        "Parition Size")
            ("def",                 boost::program_options::value<std::string>(),   "Circuit as DEF File")
            ("lef",                 boost::program_options::value<std::string>(),   "Library as LEF File")
            ("bookshelf",           boost::program_options::value<std::string>(),   "Circuits as Bookshelf Format")
            ("supplement",          boost::program_options::value<std::string>(),   "JSON File holding additional information")
            ("site",                boost::program_options::value<std::string>(),   "Specifies the used site for the Standard/Macro Cells")
            ("timeout",             boost::program_options::value<size_t>(),        "Timeout for Solving a SAT Instance")
            ("solutions",           boost::program_options::value<size_t>(),        "Maximum Number of AllSAT results to generate");
        
        // Top Level Priority: Command Line:
        boost::program_options::command_line_parser parser(m_argc, m_argv);
        parser.options(*m_options_functions).allow_unregistered().style(
                boost::program_options::command_line_style::default_style | 
                boost::program_options::command_line_style::allow_slash_for_short);

        // Second Level Priority: Local Ini File
        boost::program_options::parsed_options parsed_options1 = parser.run();
        boost::program_options::store(parsed_options1, m_vm);

        std::fstream project_ini = std::fstream("config.ini", std::ios::in);
        boost::program_options::store(boost::program_options::parse_config_file(project_ini, *m_options_functions), m_vm);
        project_ini.close();

        boost::program_options::notify(m_vm);

        if(m_vm.count("help")){
            std::cout << *m_options_functions << std::endl;
            exit(0);
        }
        if(m_vm.count("bash_completion")){
            this->bash_completion_script();
            exit(0);
        }
        if(m_vm.count("lef")){
            this->add_lef(m_vm["lef"].as<std::string>());
        }
        if(m_vm.count("def")){
            this->add_def(m_vm["def"].as<std::string>());
        }
        if(m_vm.count("bookshelf")){
            this->set_bookshelf_file(m_vm["bookshelf"].as<std::string>());
        }
        if (m_vm.count("site")){
            this->set_site(m_vm["site"].as<std::string>());
        }
        if(m_vm.count("partition-size")){
            this->set_partition_size(m_vm["partition-size"].as<size_t>());
        }
        if(m_vm.count("allsat")){
            this->set_allsat(true);
        }
        if(m_vm.count("solutions")){
            this->set_max_solutions(m_vm["solutions"].as<size_t>());
        }
        if(m_vm.count("save-best")){
            this->set_save_best(true);
        }
        if(m_vm.count("save-all")){
            this->set_save_all(true);
        }
        if(m_vm.count("dump-all")){
            this->set_dump_all(true);
        }
        if (m_vm.count("dump-best")){
            this->set_dump_best(true);
        }
        if(m_vm.count("timeout")){
            this->set_timeout(m_vm["timeout"].as<size_t>());
        }
        if(m_vm.count("store-smt")){
            this->set_store_smt(true);
        }
        if(m_vm.count("supplement")){
            this->set_supplement(m_vm["supplement"].as<std::string>());
        }
        if(m_vm.count("pareto")){
            this->set_pareto_optimizer(true);
        }
        if(m_vm.count("lex")){
            this->set_lex_optimizer(true);
        }
        if(m_vm.count("box")){
            this->set_box_optimizer(true);
        }
        if(m_vm.count("partition")){
            this->set_partitioning(true);
        }
        if(m_vm.count("parquet")){
            this->set_parquet_fp(true);
        }
        if(m_vm.count("verbose")){
            this->set_verbose(true);
        }
        if(m_vm.count("store-log")){
            this->set_log_active(true);
        }
        
        if((this->get_def().empty() || this->get_lef().empty()) && this->get_bookshelf_file().empty()){
            delete m_options_functions; m_options_functions = nullptr;
            throw std::runtime_error("No LEF/DEF or Bookshelf file has been specified!");
        }

        if(this->get_site().empty() && this->get_bookshelf_file().empty()){
            delete m_options_functions; m_options_functions = nullptr;
            throw std::runtime_error("Site has not been specified!");
        }
        
        if(!this->get_def().empty() &&!boost::filesystem::exists(this->get_def())){
            delete m_options_functions; m_options_functions = nullptr;
            throw std::runtime_error("Could not find DEF File!");
        }

        if(!this->get_bookshelf_file().empty() && !boost::filesystem::exists(this->get_bookshelf_file())){
            delete m_options_functions; m_options_functions = nullptr;
            throw std::runtime_error("Could not find Bookshelf file!");
        }
        
        if(!this->get_lef().empty()){
            for(auto itor: this->get_lef()){
                if(!boost::filesystem::exists(itor)){
                    delete m_options_functions; m_options_functions = nullptr;
                    throw std::runtime_error("Could not find LEF File (" + itor + ")");
                }
            }
        }
        this->set_logic(eInt);
        this->set_working_directory(boost::filesystem::current_path().string());
        this->set_results_directory("results");
        this->set_results_id(this->existing_results() + 1);
        this->set_image_directory("images");
        this->set_smt_directory("smt");
        this->set_parquet_directory("parquet");
        this->set_log_name("placer.log");
        
        boost::filesystem::create_directories(this->get_results_directory() + "/" + std::to_string(this->get_results_id()));
        
        // Create Logger Singleton once the Commandline Information is known!
        m_logger = Utils::Logger::getInstance();
        m_timer  = new Placer::Utils::Timer();
        m_mckt = new Placer::MacroCircuit();
        
    } catch (std::exception const & exp){
        std::cerr << std::endl;
        std::cerr << "### Configuration Error! ###" << std::endl;
        std::cerr << "Description: " << exp.what() << std::endl;
        std::cerr << "See: " << this->get_binary_name() <<  " --help" << std::endl;
        exit(-1);
    }
}

/**
 * @brief Generate a Bash Completion Script based on the configured Boost Program Options
 */
void MacroPlacer::bash_completion_script()
{
    std::stringstream script;

    script << "#! /bin/bash"                        << std::endl;
    script                                          << std::endl;
    script << "#"                                   << std::endl;
    script << "# Howto Z-Shell:"                    << std::endl;
    script << "# $ autoload bashcompinit"           << std::endl;
    script << "# $ bashcompinit"                    << std::endl;
    script << "# $ source *.sh"                     << std::endl;
    script << "# $ complete -F _smt_placer() -o filename smt_placer" << std::endl;
    script << "#"                                   << std::endl;
    script                                          << std::endl;

    script << "_smt_placer()"                       << std::endl;
    script << "{"                                   << std::endl;
    script << "local cur"                           << std::endl;
    script                                          << std::endl;
    script << "\tCOMPREPLY=()"                      << std::endl;
    script << "\tcur=${COMP_WORDS[COMP_CWORD]}"     << std::endl;
    script                                          << std::endl;
    script << "\tcase \"$cur\" in"                  << std::endl;
    script << "\t\t-*)"                             << std::endl;
    script << "\t\tCOMPREPLY=( $( compgen -W ' \\"  << std::endl;

   for(auto itor : m_options_functions->options()){
        script << "\t\t\t--" << itor->long_name() << " \\" << std::endl;
   }

    script << "\t\t\t' -- $cur ) );;" << std::endl;
    script << "\tesac"                << std::endl;
    script                            << std::endl;
    script << "\treturn 0"            << std::endl;
    script << "}"                     << std::endl;
    
    script << std::endl;
    script << "complete -F _smt_placer  smt_placer" << std::endl;
    
   std::string current_path = boost::filesystem::current_path().string();
   std::fstream out_file(current_path + "/bash_completion_script.sh", std::ios::out);
   out_file << script.str();
   out_file.close();
}

/**
 * @brief 
 */
void MacroPlacer::print_header()
{
    size_t max_len = 0;
    
    std::ifstream header (Utils::Utils::get_base_path() + "/01_src/header.ascii");
    std::string line;
    while(std::getline(header, line)){
        if (line.size() > max_len){
            max_len = line.size();
        }
        std::cout << line << std::endl;
    }
    header.close();
    
    size_t const outline = 14;
    std::cout << std::left << std::setw(outline) << std::setfill(' ') << "Git Hash: "  << GIT_HASH << std::endl;
    std::cout << std::left << std::setw(outline) << std::setfill(' ') << "Last Author: "  << GIT_NAME << std::endl;
    std::cout << std::left << std::setw(outline) << std::setfill(' ') << "Last Changes: " << GIT_DATE << std::endl;
    
    std::cout << std::string (max_len, '-') << std::endl;
    std::cout << std::endl;
}

/**
 * @brief 
 */
void MacroPlacer::init ()
{
    this->read_configuration();
    this->store_configuration();
    this->set_binary_name(m_argv[0]);
    this->print_header();
}

/**
 * @brief 
 */
void MacroPlacer::run ()
{
    m_timer->start_timer("total");
    m_logger->execution_start();
    
    m_mckt->build_circuit();
    m_mckt->partitioning();
    m_mckt->encode();
    m_mckt->place();
   
    m_timer->stop_timer("total");
    m_logger->execution_end();
}

/**
 * @brief 
 */
void MacroPlacer::post_process()
{
    m_mckt->store_results();
    if(this->get_save_all()){
        m_mckt->save_all();
    } else if (this->get_save_best()){
        m_mckt->save_best();
    }

    if (this->get_dump_all()){
        m_mckt->dump_all();
    } else if (this->get_dump_best()){
        m_mckt->dump_best();
    }
    
    m_mckt->best_result();
}

/**
 * @brief 
 * 
 * @return size_t
 */
size_t MacroPlacer::existing_results ()
{
    size_t retval = 0;
    
    if(boost::filesystem::is_directory(this->get_results_directory())){
        for(auto& file: boost::filesystem::directory_iterator(this->get_results_directory())){
            boost::filesystem::path p(file);
            std::string name = p.filename().string();
            if(std::stoi(name) > static_cast<int>(retval)){
            retval = std::stoi(name);
            }
        }
    }
    return retval;
}

/**
 * @brief 
 */
void MacroPlacer::store_configuration ()
{
    std::ofstream config(this->get_results_directory() + "/" + std::to_string(this->get_results_id()) + "/config.ini");
    config << "def:" << this->get_def() << std::endl;
    config << "lef:";
    for(auto& itor: this->get_lef()){
        config << itor << ",";
    }
    config << std::endl;
    config << "bookshelf:" << this->get_bookshelf_file() << std::endl;
    config << "pareto:" << this->get_pareto_optimizer() << std::endl;
    config << "solutions:" << this->get_max_solutions() << std::endl;
    config.close();
}
