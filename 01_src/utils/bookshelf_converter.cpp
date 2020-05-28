//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : bookshelf_converter.cpp
//
// Date         : 27.May 2019
// Compiler     : gcc version 9.3.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Convert Benchmarks into Extended DAC2002 Bookshelf Format
//==================================================================
#include <iostream>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <object.hpp>
#include <bookshelf.hpp>
#include <exception.hpp>

namespace po = boost::program_options;

namespace Placer {

class Converter: public virtual Object {
public:
    Converter (int _argc, char** _argv):
        Object()
    {
        m_argv = _argv;
        m_argc = _argc;
    }
    
    virtual ~Converter()
    {
    }
    
    void init(){
        try {
            std::cout << "SMT_MacroPlacer: Bookshelf Converter" << std::endl
                      << "Johannes Kepler University 2020 " << std::endl
                      << std::endl
                      << "This tool is used in order to convert existing " << std::endl
                      << "Benchmark Circuits into the extended Bookshelf DAC2020 format" 
                      << std::endl;

            po::variables_map vm;
            po::options_description* options_functions =
                new po::options_description("Usage: " + std::string(m_argv[0]) + " [options]");

            options_functions->add_options()
                ("help", "Displays information about usage")
                ("in",   po::value<std::string>(), "Input Bookshelf Aux file")
                ("out",  po::value<std::string>(), "Output Basefilename");

            po::command_line_parser parser(m_argc, m_argv);
            parser.options(*options_functions).allow_unregistered().style(
                        po::command_line_style::default_style | 
                        po::command_line_style::allow_slash_for_short);
            po::parsed_options parsed_options = parser.run();
            po::store(parsed_options, vm);
            po::notify(vm);

            if(vm.count("help")){
                std::cout << *options_functions << std::endl;
                exit(0);
            }
            if (vm.count("in")){
                this->set_bookshelf_file(vm["in"].as<std::string>());
            }
            if (vm.count("out")){
                this->set_bookshelf_export(vm["out"].as<std::string>());
            }
            if (this->get_bookshelf_file() == ""){
                throw Utils::PlacerException("No Bookshelf Input Files Defined!");
            }
            if (this->get_bookshelf_export() == ""){
                throw Utils::PlacerException("No Output Basefilename has been Defined!");
            }
        } catch (Utils::PlacerException const & exp){
            std::cerr << exp.what() << std::endl;
        }
    }

    void run()
    {
        Bookshelf* bookshelf = new Bookshelf();
        bookshelf->read_files();
        bookshelf->write_placement();

        delete bookshelf; bookshelf = nullptr;
    }

private:
    int m_argc;
    char** m_argv;
};

} /* namespace Placer */


int main(int const argc, char** argv)
{
    Placer::Converter* converter = new Placer::Converter(argc, argv);
    converter->init();
    converter->run();

    delete converter; converter = nullptr;

    return 0;
}
