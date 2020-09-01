//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : parquet.cpp
//
// Date         : 13. March 2020
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Parquet Frontend
//==================================================================
#include "parquet.hpp"

using namespace Placer;
using namespace Placer::Utils;

/**
 * @brief Constructor
 */
ParquetFrontend::ParquetFrontend():
    Object()
{
#ifdef ENABLE_PARQUET
    m_db = new parquetfp::DB();
#endif
}

/**
 * @brief Destructor
 */
ParquetFrontend::~ParquetFrontend()
{
#ifdef ENABLE_PARQUET
    delete m_db; m_db = nullptr;
#endif
    m_tree = nullptr;
    m_layout = nullptr;
}

/**
 * @brief Set Macros for Parquet
 * 
 * @param macros Macros to feed Parquet
 */
void ParquetFrontend::set_macros(std::vector<Macro*> & macros)
{
    m_macros = macros;
}

/**
 * @brief Set Terminals for Parquet
 * 
 * @param terminals Terminals to feed Parquet
 */
void ParquetFrontend::set_terminals(std::vector<Terminal*> & terminals)
{
    m_terminals = terminals;
}

/**
 * @brief Set Connection tree for Parquet
 *
 * @param tree Connection Tree
 */
void ParquetFrontend::set_tree(Tree* tree)
{
    nullpointer_check (tree);

    m_tree = tree;
}

/**
 * @brief Set Layout for Parquet
 * 
 * @param layout Layout Instance
 */
void ParquetFrontend::set_layout(Layout* layout)
{
    nullpointer_check (layout);

    m_layout = layout;
}

/**
 * @brief Fill Parquets internal Database
 */
void ParquetFrontend::build_db()
{
#ifdef ENABLE_PARQUET
    m_layout->set_lx(0);
    m_layout->set_ly(0);

    m_nodes = m_db->getNodes();
    m_nets = m_db->getNets();

    std::vector<std::pair<int, int> > netStor;
    std::vector<int> costStor;

    for (size_t i = 0; i < m_macros.size(); ++i){
        Macro* macro = m_macros[i];
        float area = static_cast<float>(macro->get_area());

        size_t w = macro->get_width().get_numeral_uint();
        size_t h= macro->get_height().get_numeral_uint();

        float max = std::max(w,h);
        float min = std::min(w,h);
        float ar = max/min;


        parquetfp::Node next_node(macro->get_id().c_str(),
                                  area,
                                  ar,
                                  ar,
                                  i,
                                  true);
        
        m_nodes->putNewNode(next_node);
    }

    for (size_t i = 0; i < m_terminals.size(); ++i){
        std::string terminal_name = m_terminals[i]->get_id();

        parquetfp::Node next_term(terminal_name.c_str(),
                                0.0,
                                0,
                                0,
                                i,
                                false);

        m_nodes->putNewTerm(next_term);
    }
    
    std::vector<Edge*> edges = m_tree->get_edges();
    
    for(size_t i = 0; i < edges.size(); ++i){
        Edge* edge = edges[i];
        
        Node* from = edge->get_from();
        Node* to   = edge->get_to();

        parquetfp::Net tmpEdge;
        parquetfp::pin tempPin1(from->get_id().c_str(), true, 0, 0, i);
        parquetfp::pin tempPin2(to->get_id().c_str(), true, 0, 0, i );

        tmpEdge.addNode(tempPin1);
        tmpEdge.addNode(tempPin2);
        tmpEdge.putIndex(i);
        tmpEdge.putName(std::string("n"+std::to_string(i)).c_str());
        tmpEdge.putWeight(1);

        m_nets->putNewNet(tmpEdge);
    }

    m_nets->updateNodeInfo(*m_nodes);
    m_nodes->updatePinsInfo(*m_nets);
#else 
    throw PlacerException("Parquet has not been activated!");
#endif 
}

/**
 * @brief Run Parquet Floorplan
 */
void ParquetFrontend::run_parquet()
{
#ifdef ENABLE_PARQUET
    MixedBlockInfoTypeFromDB dbBlockInfo(*m_db);
    MixedBlockInfoType* blockInfo = reinterpret_cast<MixedBlockInfoType*> (&dbBlockInfo);

    // Command_Line object populate
    parquetfp::Command_Line param;
    param.minWL = false;
    param.noRotation = false;
    param.FPrep = "BTree";
    param.seed = 100;
    param.scaleTerms = false;
    param.softBlocks = false;
    param.reqdAR = 1.0;

    std::cout << std::endl << std::endl;
    std::cout << "BTreeAreaWireAnnealer Constructor..." << std::endl;
    
    BTreeAreaWireAnnealer* annealer = 
        new BTreeAreaWireAnnealer(*blockInfo, 
                                    const_cast<parquetfp::Command_Line*>(&param),
                                    m_db);

        std::cout << std::endl << std::endl;
        std::cout << "ParquetFP Starting..." << std::endl;

        annealer->go();

        std::cout << std::endl << std::endl;
        std::cout << "ParquetFP Terminated..." << std::endl;
        delete annealer;
#else 
    throw PlacerException("Parquet has not been activated!");
#endif
}

/**
 * @brief Merge Data back from Parquet
 */
void ParquetFrontend::data_from_parquet()
{
#ifdef ENABLE_PARQUET
    auto itor_begin = m_nodes->nodesBegin();
    auto itor_end   = m_nodes->nodesEnd();

    for (; itor_begin != itor_end; itor_begin++){
       float x = itor_begin->getX();
       float y = itor_begin->getY();

       uofm::string name  = itor_begin->getName();
       char const * const _name = name.c_str();
       Macro* m = this->find_macro(_name);
       parquetfp::ORIENT orient = itor_begin->getOrient();
       eOrientation orient_solution = eNorth;
       if (orient == parquetfp::ORIENT::N){
           orient_solution = eOrientation::eNorth;
       } else if (orient == parquetfp::ORIENT::W){
            orient_solution = eOrientation::eWest;
       } else if (orient == parquetfp::ORIENT::S){
            orient_solution = eOrientation::eSouth;
       } else if (orient == parquetfp::ORIENT::E){
            orient_solution = eOrientation::eEast;
       } else if (orient == parquetfp::ORIENT::FN){
            orient_solution = eOrientation::eFlipNorth;
       } else if (orient == parquetfp::ORIENT::FW){
            orient_solution = eOrientation::eFlipWest;
       } else if (orient == parquetfp::ORIENT::FS){
            orient_solution = eOrientation::eFlipSouth;
       } else if (orient == parquetfp::ORIENT::FE){
            orient_solution = eOrientation::eFlipEast;
       } else {
            notimplemented_check();
       }

       m->add_solution_orientation(orient_solution);
       m->add_solution_lx(x);
       m->add_solution_ly(y);
    }

    m_layout->set_solution_ux(m_db->getXMax());
    m_layout->set_solution_uy(m_db->getYMax());
    //m_db->plot("parquet", m_layout->get_solution_ux(0)* m_layout->get_solution_uy(0), 0, 1, 0, 0,false, false, true);
#else 
    throw PlacerException("Parquet has not been activated!");
#endif
}

/**
 * @brief Get Macro Pointer by Name
 * 
 * @param name Macro Name
 * @return Placer::Macro*
 */
Macro* ParquetFrontend::find_macro(std::string const & name)
{
    auto itor = std::find_if(m_macros.begin(), m_macros.end(), [&name](Macro* m){
        return m->get_id() == name;});

    return *itor;
}

/**
 * @brief Export Parquet Database
 */
void ParquetFrontend::store_bookshelf_results()
{
#ifdef ENABLE_PARQUET
    if (!boost::filesystem::exists(this->get_parquet_directory())){
        boost::filesystem::create_directories(this->get_parquet_directory());
    }

    boost::filesystem::current_path(this->get_parquet_directory());

    m_db->save("parquet_result");
#else
    throw PlacerException("Parquet has not been activated!");
#endif
}
