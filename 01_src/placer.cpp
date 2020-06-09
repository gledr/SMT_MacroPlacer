//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : placer.cpp
//
// Date         : 27. December 2019
// Compiler     : gcc version 10.1.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Macro Placer Main Class
//==================================================================
#include "placer.hpp"

using namespace Placer;
using namespace Placer::Utils;

/**
 * @brief Constructor
 *
 * @param argc Command Line Arguments Counter
 * @param argv Command Line Arguments Value
 */
MacroPlacer::MacroPlacer(int const argc, char ** argv):
    Object()
{
    nullpointer_check (argv);

    m_argc = argc;
    m_argv = argv;
}

/**
 * @brief Destructor
 */
MacroPlacer::~MacroPlacer()
{
    Utils::Logger::destroy();
    
    delete m_options_functions; m_options_functions = nullptr;
    delete m_timer; m_timer = nullptr;
    delete m_mckt; m_mckt = nullptr;
}

/**
 * @brief Read Configuration by Commandline and Inifile
 */
void MacroPlacer::read_configuration()
{
    try {
        namespace po = boost::program_options;
        namespace fs = boost::filesystem;
        
        std::string desc = "Usage: " + this->get_binary_name() + " [options]";
        m_options_functions  = new po::options_description(desc);

        m_options_functions->add_options()
            (CMD_HELP,                                      CMD_HELP_TEXT)
            (CMD_BASH_COMPLETION,                           CMD_BASH_COMPLETION)
            (CMD_SAVE_BEST,                                 CMD_SAVE_BEST_TEXT)
            (CMD_DUMP_BEST,                                 CMD_DUMP_BEST_TEXT)
            (CMD_DUMP_ALL,                                  CMD_DUMP_ALL_TEXT)
            (CMD_SAVE_ALL,                                  CMD_SAVE_ALL_TEXT)
            (CMD_VERBOSE,                                   CMD_VERBOSE_TEXT)
            (CMD_STORE_LOG,                                 CMD_STORE_LOG_TEXT)
            (CMD_STORE_SMT,                                 CMD_STORE_SMT_TEXT)
            (CMD_PARETO,                                    CMD_PARETO_TEXT)
            (CMD_LEX,                                       CMD_LEX_TEXT)
            (CMD_BOX,                                       CMD_BOX_TEXT)
            (CMD_PARQUET,                                   CMD_PARQUET_TEXT)
            (CMD_PARTITION,                                 CMD_PARTITION_TEXT)
            (CMD_MIN_AREA,                                  CMD_MIN_AREA_TEXT)
            (CMD_MIN_HPWL,                                  CMD_MIN_HPWL_TEXT)
            (CMD_FREE_TERMINALS,                            CMD_FREE_TERMINALS_TEXT)
            (CMD_SKIP_PWR_SUPPLY,                           CMD_SKIP_PWR_SUPPLY_TEXT)
            (CMD_PARTITION_SIZE,  po::value<size_t>(),      CMD_PARTITION_SIZE_TEXT)
            (CMD_PARTITION_COUNT, po::value<size_t>(),      CMD_PARTITION_COUNT_TEXT)
            (CMD_DEF,             po::value<std::string>(), CMD_DEF_TEXT)
            (CMD_LEF,             po::value<std::string>(), CMD_LEF_TEXT)
            (CMD_BOOKSHELF,       po::value<std::string>(), CMD_BOOKSHELF_TEXT)
            (CMD_SUPPLEMENT,      po::value<std::string>(), CMD_SUPPLEMENT_TEXT)
            (CMD_SITE,            po::value<std::string>(), CMD_SITE_TEXT)
            (CMD_TIMEOUT,         po::value<size_t>(),      CMD_TIMEOUT_TEXT)
            (CMD_SOLUTIONS,       po::value<size_t>(),      CMD_SOLUTIONS_TEXT);

        // Top Level Priority: Command Line:
        po::command_line_parser parser(m_argc, m_argv);
        parser.options(*m_options_functions).allow_unregistered().style(
                po::command_line_style::default_style | 
                po::command_line_style::allow_slash_for_short);

        // Second Level Priority: Local Ini File
        po::parsed_options parsed_options1 = parser.run();
        po::store(parsed_options1, m_vm);

        std::fstream project_ini = std::fstream("config.ini", std::ios::in);
        po::store(po::parse_config_file(project_ini, *m_options_functions), m_vm);
        project_ini.close();

        po::notify(m_vm);

        if(m_vm.count(CMD_HELP)){
            std::cout << *m_options_functions << std::endl;
            exit(0);
        }
        if(m_vm.count(CMD_BASH_COMPLETION)){
            this->bash_completion_script();
            exit(0);
        }
        if(m_vm.count(CMD_LEF)){
            this->add_lef(m_vm[CMD_LEF].as<std::string>());
        }
        if(m_vm.count(CMD_DEF)){
            this->add_def(m_vm[CMD_DEF].as<std::string>());
        }
        if(m_vm.count(CMD_BOOKSHELF)){
            this->set_bookshelf_file(m_vm[CMD_BOOKSHELF].as<std::string>());
        }
        if (m_vm.count(CMD_SITE)){
            this->set_site(m_vm[CMD_SITE].as<std::string>());
        }
        if(m_vm.count(CMD_PARTITION_SIZE)){
            this->set_partition_size(m_vm[CMD_PARTITION_SIZE].as<size_t>());
        }
        if(m_vm.count(CMD_PARTITION_COUNT)){
            this->set_num_partition(m_vm[CMD_PARTITION_COUNT].as<size_t>());
        }
        if(m_vm.count(CMD_SOLUTIONS)){
            this->set_max_solutions(m_vm[CMD_SOLUTIONS].as<size_t>());
        }
        if(m_vm.count(CMD_SAVE_BEST)){
            this->set_save_best(true);
        }
        if(m_vm.count(CMD_SAVE_ALL)){
            this->set_save_all(true);
        }
        if(m_vm.count(CMD_DUMP_ALL)){
            this->set_dump_all(true);
        }
        if (m_vm.count(CMD_DUMP_BEST)){
            this->set_dump_best(true);
        }
        if(m_vm.count(CMD_TIMEOUT)){
            this->set_timeout(m_vm[CMD_TIMEOUT].as<size_t>());
        }
        if(m_vm.count(CMD_STORE_SMT)){
            this->set_store_smt(true);
        }
        if(m_vm.count(CMD_SUPPLEMENT)){
            this->set_supplement(m_vm[CMD_SUPPLEMENT].as<std::string>());
        }
        if(m_vm.count(CMD_PARETO)){
            this->set_pareto_optimizer(true);
        }
        if(m_vm.count(CMD_LEX)){
            this->set_lex_optimizer(true);
        }
        if(m_vm.count(CMD_BOX)){
            this->set_box_optimizer(true);
        }
        if(m_vm.count(CMD_PARTITION)){
            this->set_partitioning(true);
        }
        if(m_vm.count(CMD_PARQUET)){
            this->set_parquet_fp(true);
        }
        if(m_vm.count(CMD_VERBOSE)){
            this->set_verbose(true);
        }
        if(m_vm.count(CMD_STORE_LOG)){
            this->set_log_active(true);
        }
        if(m_vm.count(CMD_MIN_AREA)){
            this->set_minimize_die_mode(true);
        }
        if (m_vm.count(CMD_MIN_HPWL)){
            this->set_minimize_hpwl_mode(true);
        }
        if(m_vm.count(CMD_FREE_TERMINALS)){
            this->set_free_terminals(true);
        }
        if (m_vm.count(CMD_SKIP_PWR_SUPPLY)){
            this->set_skip_power_network(true);
        }

        if((this->get_def().empty() || this->get_lef().empty()) &&
           this->get_bookshelf_file().empty()){
            delete m_options_functions; m_options_functions = nullptr;
            throw std::runtime_error("No LEF/DEF or Bookshelf file has been specified!");
        }
        if(this->get_site().empty() &&
           this->get_bookshelf_file().empty()){
            delete m_options_functions; m_options_functions = nullptr;
            throw std::runtime_error("Site has not been specified!");
        }
        if(!this->get_def().empty() &&
           !fs::exists(this->get_def())){
            delete m_options_functions; m_options_functions = nullptr;
            throw std::runtime_error("Could not find DEF File!");
        }
        if(!this->get_bookshelf_file().empty() &&
            !fs::exists(this->get_bookshelf_file())){
                delete m_options_functions; m_options_functions = nullptr;
                throw std::runtime_error("Could not find Bookshelf file!");
        }

        if(!this->get_lef().empty()){
            for(auto itor: this->get_lef()){
                if(!fs::exists(itor)){
                    delete m_options_functions; m_options_functions = nullptr;
                    throw std::runtime_error("Could not find LEF File (" + itor + ")");
                }
            }
        }
        this->set_logic(eInt);
        this->set_base_path(Utils::Utils::get_base_path());
        this->set_working_directory(fs::current_path().string());
        this->set_results_directory("results");
        this->set_results_id(this->existing_results() + 1);
        this->set_image_directory("images");
        this->set_smt_directory("smt");
        this->set_parquet_directory("parquet");
        this->set_log_name("placer.log");
        this->set_database_file("results.db");
        this->set_db_to_csv_script(this->get_base_path() + "/04_configuration/db_to_csv.sh");

        fs::create_directories(this->get_results_directory() + "/" + std::to_string(this->get_results_id()));

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
    script << "#! /bin/bash"                                         << std::endl;
    script << "#"                                                    << std::endl;
    script << "# Generted File! Do not modify!"                      << std::endl;
    script << "# " << Utils::Utils::get_current_time()                           ;
    script << "#"                                                    << std::endl;
    script                                                           << std::endl;
    script << "#"                                                    << std::endl;
    script << "# Howto Z-Shell:"                                     << std::endl;
    script << "# $ autoload bashcompinit"                            << std::endl;
    script << "# $ bashcompinit"                                     << std::endl;
    script << "# $ source *.sh"                                      << std::endl;
    script << "# $ complete -F _smt_placer() -o filename smt_placer" << std::endl;
    script << "#"                                                    << std::endl;
    script                                                           << std::endl;

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
 * @brief Print Tool Header
 */
void MacroPlacer::print_header()
{
    size_t max_len = 0;
    std::stringstream headerstream;

    std::ifstream header (this->get_base_path() + "/01_src/utils/header.ascii");
    std::string line;
    while(std::getline(header, line)){
        if (line.size() > max_len){
            max_len = line.size();
        }
        headerstream << line << std::endl;
    }
    header.close();
    m_logger->print_header(headerstream);
    m_logger->print_version(GIT_DATE, GIT_HASH, GIT_NAME, max_len);
}

/**
 * @brief Init PLacer
 */
void MacroPlacer::init ()
{
    this->read_configuration();
    this->store_configuration();
    this->set_binary_name(m_argv[0]);
    this->print_header();
}

/**
 * @brief Run Placer
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
 * @brief Post Process Results
 */
void MacroPlacer::post_process()
{
    if(this->get_save_all()){
        m_mckt->save_all();
    }
    if (this->get_save_best()){
        m_mckt->save_best();
    }

    if (this->get_dump_all()){
        m_mckt->dump_all();
    }
    if (this->get_dump_best()){
        m_mckt->dump_best();
    }
    m_mckt->create_statistics();
    m_mckt->results_to_db();
}

/**
 * @brief Check Results Directory and Returns Id for Next Result
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
 * @brief Store Used Configuration to Filesystem
 */
void MacroPlacer::store_configuration ()
{
    std::string filename = this->get_results_directory() + "/" +
                           std::to_string(this->get_results_id()) +
                           "/config.ini";

    std::ofstream config(filename);
    config << "def:" << this->get_def() << std::endl;
    config << "lef:";
    for(auto& itor: this->get_lef()){
        config << itor << ",";
    }
    config << std::endl;
    config << "bookshelf:" << this->get_bookshelf_file() << std::endl;
    config << "pareto:" << this->get_pareto_optimizer() << std::endl;
    config << "lex:" << this->get_lex_optimizer() << std::endl;
    config << "box:" << this->get_box_optimizer() << std::endl;
    config << "solutions:" << this->get_max_solutions() << std::endl;
    config << "min_die_mode:" << this->get_minimize_die_mode() << std::endl;
    config << "min_hpwl_mode:" << this->get_minimize_hpwl_mode() << std::endl;
    config.close();
}
