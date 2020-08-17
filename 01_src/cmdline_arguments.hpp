//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : cmdline_arguments.hpp
//
// Date         : 04. June 2020
// Compiler     : gcc version 10.1.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Commandline Input Definitions
//==================================================================
#ifndef CMDLINE_ARGUMENTS
#define CMDLINE_ARGUMENTS

namespace Placer {

    constexpr const char CMD_INI_FILE[]
        = "ini";
    constexpr const char CMD_INI_FILE_TEXT[]
        = "Choose Configuration File";

    constexpr const char CMD_HELP[] 
        = "help";
    constexpr const char CMD_HELP_TEXT[] 
        = "Displays information about the usage";

    constexpr const char CMD_VERBOSE[]
        = "verbose";
    constexpr const char CMD_VERBOSE_TEXT[] 
        = "Show Debug Information";

    constexpr const char CMD_BASH_COMPLETION[]
        = "bash-completion";
    constexpr const char CMD_BASH_COMPLETION_TEXT[] 
        = "Generate a Bash Autocompletion Script";

    constexpr const char CMD_TIMEOUT[] 
        = "timeout";
    constexpr const char CMD_TIMEOUT_TEXT[] 
        = "Timeout for Solving a SAT Instance";

    constexpr const char CMD_LEF[] 
        = "lef";
    constexpr const char CMD_LEF_TEXT[]
        = "Library as LEF File";

    constexpr const char CMD_DEF[]
        = "def";
    constexpr const char CMD_DEF_TEXT[] 
        = "Circuit as DEF File";

    constexpr const char CMD_SITE[] 
        = "site";
    constexpr const char CMD_SITE_TEXT[] 
        = "Specifies the used site for the Standard/Macro Cells";

    constexpr const char CMD_BOOKSHELF[] 
        = "bookshelf";
    constexpr const char CMD_BOOKSHELF_TEXT[] 
        = "Circuits as Bookshelf Format";

    constexpr const char CMD_SUPPLEMENT[] 
        = "supplement";
    constexpr const char CMD_SUPPLEMENT_TEXT[] 
        = "JSON File holding additional information";

    constexpr const char CMD_PARETO[] 
        = "pareto";
    constexpr const char CMD_PARETO_TEXT[] 
        = "Utilize Z3s Pareto Optimizer";

    constexpr const char CMD_LEX[] 
        = "lex";
    constexpr const char CMD_LEX_TEXT[] 
        = "Utilize Z3s Lex Optimizer";

    constexpr const char CMD_PARQUET[] 
        = "parquet";
    constexpr const char CMD_PARQUET_TEXT[] 
        = "Utilize Parquet Floorplanning";

    constexpr const char CMD_SOLUTIONS[] 
        = "solutions";
    constexpr const char CMD_SOLUTIONS_TEXT[] 
        = "Maximum Number of results to generate";

    constexpr const char CMD_MIN_AREA[] 
        = "minimize-area";
    constexpr const char CMD_MIN_AREA_TEXT[] 
        = "Add Die Area as Minimization Target";

    constexpr const char CMD_MIN_HPWL[] 
        = "minimize-hpwl";
    constexpr const char CMD_MIN_HPWL_TEXT[] 
        = "Add HPWL as Minimization Target";

    constexpr const char CMD_FREE_TERMINALS[] 
        = "free-terminals";
    constexpr const char CMD_FREE_TERMINALS_TEXT[] 
        = "Ignore Any Existing Terminal Placement";
    constexpr const char CMD_FREE_COMPONENTS[]
        = "free-components";
    constexpr const char CMD_FREE_COMPONENTS_TEXT[]
        = "Ignore Any Existing FIXED Component Placement";

    constexpr const char CMD_SKIP_PWR_SUPPLY[] 
        = "skip-power-supply";
    constexpr const char CMD_SKIP_PWR_SUPPLY_TEXT[] 
        = "Ignore Power Network for HPWL";

    constexpr const char CMD_SAVE_ALL[] 
        = "save-all";
    constexpr const char CMD_SAVE_ALL_TEXT[] 
        = "Save All Generated Solutions";

    constexpr const char CMD_SAVE_BEST[] 
        = "save-best";
    constexpr const char CMD_SAVE_BEST_TEXT[] 
        = "Save Best Solution Generated";

    constexpr const char CMD_DUMP_ALL[] 
        = "dump-all";
    constexpr const char CMD_DUMP_ALL_TEXT[] 
        = "Shows all the results using Gnuplot";

    constexpr const char CMD_DUMP_BEST[] 
        = "dump-best";
    constexpr const char CMD_DUMP_BEST_TEXT[] 
        = "Shows the best result using Gnuplot";

    constexpr const char CMD_STORE_LOG[] 
        = "store-log";
    constexpr const char CMD_STORE_LOG_TEXT[] 
        = "Store Debug Information to Logfile";

    constexpr const char CMD_STORE_SMT[] 
        = "store-smt";
    constexpr const char CMD_STORE_SMT_TEXT[] = 
        "Store the generated SMT2 problems to the filesystem";

    constexpr const char CMD_PARTITION[] 
        = "partition";
    constexpr const char CMD_PARTITION_TEXT[] 
        = "Enable Partitioning Mode";

    constexpr const char CMD_PARTITION_SIZE[] 
        = "partition-size";
    constexpr const char CMD_PARTITION_SIZE_TEXT[] 
        = "Parition Size";

    constexpr const char CMD_PARTITION_COUNT[]
        = "partition-count";
    constexpr const char CMD_PARTITION_COUNT_TEXT[] 
        ="Number of Partitions";

} /* namespace Placer */

#endif /* CMDLINE_ARGUMENTS */
