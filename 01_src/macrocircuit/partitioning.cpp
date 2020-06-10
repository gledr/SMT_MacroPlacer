//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : partitioning.cpp
//
// Date         : 24. February 2020
// Compiler     : gcc version 9.2.1 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Floorplan Partitioning
//==================================================================
#include "partitioning.hpp"

using namespace Placer;
using namespace Placer::Utils;


/**
 * @brief Constructor
 */
Partitioning::Partitioning():
    m_encode(new EncodingUtils()),
    m_components_in_partition(m_encode->get_value(0)),
    m_components_non_overlapping(m_encode->get_value(0)),
    m_hpwl_cost_function(m_encode->get_value(0))
{
    m_kmeans = new KMeans();
    m_logger = Logger::getInstance();
}

/**
 * @brief Destructor
 */
Partitioning::~Partitioning()
{
    delete m_encode; m_encode = nullptr;
    delete m_kmeans; m_kmeans = nullptr;
    m_tree = nullptr;
    m_logger = nullptr;
}

/*
 * @brief Run Partitioning
 */
void Partitioning::run()
{
    //this->kmeans_clustering();
    this->hypergraph_partitioning();
}

/**
 * @brief Create Initial Partition
 * 
 * Cluster Macros with Same Dimensions
 */
void Partitioning::create_initial_partitions()
{
    std::map<std::pair<size_t, size_t>, std::vector<Macro*>> sorted;
    for (size_t i = 0; i < m_macros.size(); ++i){
        Macro* m = m_macros[i];
        std::pair<size_t, size_t> current(m_macros[i]->get_width().get_numeral_uint(),
                                          m_macros[i]->get_height().get_numeral_uint());

        auto find_current = std::find_if(sorted.begin(), sorted.end(), [&current] (std::pair<std::pair<size_t, size_t>, std::vector<Macro*>> key){
            bool case_a = (current.first == key.first.first) && (current.second = key.first.second);
            bool case_b = (current.first == key.first.second) && (current.second = key.first.first);
            
            if (case_a || case_b){
                return true;
            } else {
                return false;
            }
        });
        
        if (find_current == sorted.end()){
            std::vector <Macro*> v;
            v.push_back(m);
            sorted[current] = v;
        } else {
            sorted[current].push_back(m);
        }
    }
    std::cout << "Sorted: " << sorted.size() << std::endl;
    
    for (auto itor: sorted){
        for (size_t j = 0; j < itor.second.size(); j+= this->get_partition_size()){
            Partition* next_partition = new Partition();
    
            for (size_t k = j; (k < (this->get_partition_size()+j)) && (k < itor.second.size()); ++k){
                next_partition->add_macro(itor.second[k]);
            }
            m_components.push_back(next_partition);
            m_z3_opt = new z3::optimize(m_z3_ctx);
            this->encode(next_partition);
            this->solve(next_partition);
            size_t w = next_partition->get_ux().get_numeral_uint();
            size_t h = next_partition->get_uy().get_numeral_uint();
            delete m_z3_opt; m_z3_opt = nullptr;

            next_partition->set_height(h);
            next_partition->set_witdh(w);
            next_partition->free_lx();
            next_partition->free_ly();
        }
    }
    std::cout << "Partitions: " << m_components.size() << std::endl;
}

/**
 * @brief Get Access to the created Partitions
 * 
 * @return std::vector< Placer::Partition* >
 */
std::vector<Component*> Partitioning::get_partitions()
{
    return m_components;
}

/**
 * @brief Set Initial Problem for Partitioning
 * 
 * @param macros Initial Macros
 * @param partition_size Number of Partitions
 */
void Partitioning::set_problem(std::vector<Macro *> & macros,
                                std::vector<Terminal*> & terminals,
                               Tree* tree)
{
    nullpointer_check(tree);
    
    m_macros = macros;
    m_tree = tree;
}

/**
 * @brief Encode SMT for Partition
 * 
 * @param next_partition Partition to Encode
 */
void Partitioning::encode(Partition* next_partition)
{
    nullpointer_check (next_partition);
    
    std::cout << "Encode Partition..." << std::endl;
    
    next_partition->set_lx(0);
    next_partition->set_ly(0);
    next_partition->free_ux();
    next_partition->free_uy();
    
    z3::params param(m_z3_ctx);
    param.set(":opt.priority", "lex");
    m_z3_opt->set(param);
    
    this->encode_components_non_overlapping(next_partition, e2D);
    this->encode_components_in_partition(next_partition, e2D);
    
    m_z3_opt->add(m_components_in_partition);
    m_z3_opt->add(m_components_non_overlapping);
    
    m_z3_opt->minimize(next_partition->get_ux());
    m_z3_opt->minimize(next_partition->get_uy());
    //m_z3_opt->minimize(next_partition->get_ux() + next_partition->get_uy());
}

/**
 * @brief Ensure Components to be within the Partition boundry
 * 
 * @param next_partition Partition to Check
 * @param type Rotation Freedom 2D/4D
 */
void Partitioning::encode_components_in_partition(Partition* next_partition,
                                                  eRotation const type)
{
    try {
        nullpointer_check (next_partition);
        
        z3::expr_vector clauses(m_z3_ctx);

        z3::expr die_lx = next_partition->get_lx();
        z3::expr die_ux = next_partition->get_ux();
        z3::expr die_ly = next_partition->get_ly();
        z3::expr die_uy = next_partition->get_uy();
        
        for(Component* itor: next_partition->get_components()){
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
                               z3::ite(is_W, z3::mk_and(case_W), m_encode->get_flag(false)));
                clauses.push_back(ite);
                
            } else if (type == eRotation::e4D){
                z3::expr ite = z3::ite(is_N, z3::mk_and(case_N),
                               z3::ite(is_E, z3::mk_and(case_E),
                               z3::ite(is_S, z3::mk_and(case_S),
                               z3::ite(is_W, z3::mk_and(case_W), m_encode->get_flag(false)))));
                clauses.push_back(ite);
            } else {
                notsupported_check("Only 2D and 4D Rotation is supported!")
            }

            if(type == eRotation::e2D){
                clauses.push_back(m_encode->ge(itor->get_orientation(), m_encode->get_value(eNorth)));
                clauses.push_back(m_encode->le(itor->get_orientation(), m_encode->get_value(eWest)));
            } else if (type == eRotation::e4D){
                clauses.push_back(itor->get_orientation() >= m_encode->get_value(eNorth));
                clauses.push_back(itor->get_orientation() <= m_encode->get_value(eEast));
            } else {
                notsupported_check("Only 2D and 4D Rotation is supported!")
            }
        }

        m_components_in_partition = z3::mk_and(clauses);

    } catch (z3::exception const & exp){
        throw PlacerException(exp.msg());
    }
}

/**
 * @brief Ensure Non Overlappning Components in Partition
 * 
 * @param next_partition Partition to Check
 * @param type Rotation Freedom 2D/4D
 */
void Partitioning::encode_components_non_overlapping(Partition* next_partition,
                                                    eRotation const type)
{
     try {
        z3::expr_vector clauses(m_z3_ctx);

        z3::expr N = m_encode->get_value(eNorth);
        z3::expr W = m_encode->get_value(eWest);
        z3::expr S = m_encode->get_value(eSouth);
        z3::expr E = m_encode->get_value(eEast);

        std::vector<Component*> components = next_partition->get_components();
        
        for(size_t i = 0; i < components.size(); i++){
            for(size_t j = 0; j < components.size(); j++){
                if(i == j){
                    continue;
                }
                /*
                 * Macro i is fixed and Marco j is placed around it
                 */
                Component* fixed = components[i];
                Component* free  = components[j];

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
                // Ensure No Nasty Copy Paste Error 
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
                z3::expr clause = m_encode->get_value(0);
                
                if(type == eRotation::e2D){
                    clause = z3::ite(is_NN, z3::mk_or(case_nn),
                             z3::ite(is_NW, z3::mk_or(case_nw),
                             z3::ite(is_WN, z3::mk_or(case_wn),
                             z3::ite(is_WW, z3::mk_or(case_ww), m_encode->get_flag(false)))));
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
                             z3::ite(is_EE, z3::mk_or(case_ee), m_encode->get_flag(false)
                             ))))))))))))))));
                } else {
                    notsupported_check("Only 2D and 4D Rotation is supported!");
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
 * @brief Solve Partition Encoding
 * 
 * @param next_partition Partition to Solve
 */
void Partitioning::solve(Partition* next_partition)
{
    nullpointer_check (next_partition);

    std::cout << "Solve Partition" << std::endl;

    z3::check_result sat = m_z3_opt->check();

    if(sat == z3::check_result::sat){
        z3::model m = m_z3_opt->get_model();

        size_t ux = m.eval(next_partition->get_ux()).get_numeral_uint();
        size_t uy = m.eval(next_partition->get_uy()).get_numeral_uint();
        next_partition->set_ux(ux);
        next_partition->set_uy(uy);
           
        for(auto itor: next_partition->get_components()){
            size_t x = m.eval(itor->get_lx()).get_numeral_uint();
            size_t y = m.eval(itor->get_ly()).get_numeral_uint();
            eOrientation o = static_cast<eOrientation>(m.eval(itor->get_orientation()).get_numeral_uint());

            itor->add_solution_lx(x);
            itor->add_solution_ly(y);
            itor->add_solution_orientation(o);
        }
        //next_partition->get_white_space_percentage();
    }
}

/**
 * @brief Encode HPWL as Cost Function for Optimizer
 * 
 * @return z3::expr
 */
void Partitioning::encode_hpwl_cost_function(Partition* next_partition)
{
    notimplemented_check();
}

void Partitioning::kmeans_clustering()
{
    //this->create_initial_partitions();

    m_kmeans->set_problem(m_components, 11);
    m_kmeans->initialize();
    m_kmeans->run(50);

    std::vector<Cluster*> clusters = m_kmeans->get_cluster();
    m_components.clear();
    for (auto cluster: clusters){
        std::vector<Partition*> partitions = cluster->get_partitions();
        std::cout << "Cluster: " << cluster->get_id() << " in progress..." << std::endl;
        
        if (partitions.size() > 0){
            Partition* next_partition = new Partition();
            m_z3_opt = new z3::optimize(m_z3_ctx);
            next_partition->add_subparititions(partitions);
            this->encode(next_partition);
            this->solve(next_partition);
            size_t w = next_partition->get_ux().get_numeral_uint();
            size_t h = next_partition->get_uy().get_numeral_uint();
            delete m_z3_opt; m_z3_opt = nullptr;
            next_partition->set_height(h);
            next_partition->set_witdh(w);
            next_partition->free_lx();
            next_partition->free_ly();
            m_components.push_back(next_partition);
        }
    }
}

/**
 * @brief Find a macro based on its identifier
 * 
 * @param id Macro identifier
 * @return Placer::Macro*
 */
Macro* Partitioning::find_macro(std::string const & id)
{
    Macro* m = nullptr;

    for (Macro* itor: m_macros){
        if (id == itor->get_id()){
            m = itor;
            break;
        }
    }
    std::cout << id << std::endl;
    assert (m != nullptr);
    nullpointer_check(m);
    assertion_check (id == m->get_id());

    return m;
}

/**
 * @brief Invoke Kahypar for Partitioning
 */
void Partitioning::hypergraph_partitioning()
{
    //this->file_based_partitioning();
    this->api_based_partitioning();
}

/**
 * @brief Perform Kahypar Partitioning using the Library API
 */
void Partitioning::api_based_partitioning()
{
    m_logger->start_kahypar();

    // Redirect Kahypar Output to File
    std::ofstream out(this->get_active_results_directory() + "/kahypar.txt");
    std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
    std::cout.rdbuf(out.rdbuf()); //redirect std::cout to kahypar.txt!

    // Create Kahypar Instance 
    kahypar_context_t* context = kahypar_context_new();
    std::string config_file = this->get_base_path() + "/04_configuration/kahypar.ini";
    kahypar_configure_context_from_file(context, config_file.c_str());

    // Translation Map Key <-> Macro
    std::map<size_t, Macro*> key_to_macro;
    for(Macro* m: m_macros){
        key_to_macro[m->get_key()] = m;
    }

    size_t hyperedge_idx = 0;
    size_t hyperedge_idx_idx = 0;

    Tree tmp_tree(*m_tree);
    tmp_tree.strip_terminals();
    
    
    std::map<std::string, std::set<std::string>> steiner_tree = m_tree->get_steiner_tree();

    // Resolve Number of Total Needed Array Spaces for Hyperedge Container
    std::vector<std::set<std::string>> key_tree;
    for (auto itor: steiner_tree){
        std::set<std::string> tmp;
        std::vector<std::string> token = Utils::Utils::tokenize(itor.first, ":");
        tmp.insert(token[0]);
        for (auto itor2: itor.second){
            std::vector<std::string> token2 = Utils::Utils::tokenize(itor2, ":");
            tmp.insert(token2[0]);
        }
        key_tree.push_back(tmp);
    }
    size_t hyperedge_cnt =0;
    for (auto itor: key_tree){
        hyperedge_cnt += itor.size();
    }

    kahypar_hypernode_id_t num_vertices =  m_macros.size() + m_terminals.size();
    kahypar_hyperedge_id_t num_hyperedges = steiner_tree.size();

    std::unique_ptr<kahypar_hyperedge_id_t[]> hyperedges = std::make_unique<kahypar_hyperedge_id_t[]>(hyperedge_cnt);
    std::unique_ptr<size_t[]> hyperedge_indices = std::make_unique<size_t[]>(num_hyperedges+1);

     for (auto edge: steiner_tree){
        std::vector<std::string> root_token = Utils::Utils::tokenize(edge.first, ":");
        Macro* root = this->find_macro(root_token[0]);
        nullpointer_check(root);
        size_t root_key = root->get_key();
        hyperedges[hyperedge_idx] = root_key;
        hyperedge_indices[hyperedge_idx_idx] = hyperedge_idx;

        hyperedge_idx++;
        hyperedge_idx_idx++;

        std::set<size_t> sub_keys;
        for (auto itor2: edge.second){
            std::vector<std::string> token = Utils::Utils::tokenize(itor2, ":");
            Macro* m = this->find_macro(token[0]);
            nullpointer_check(m);
            size_t sub_key = m->get_key();

            // No Edge can point at itself
            // Use a set to get unique subkeys
            sub_keys.insert(sub_key);
        }
        for (auto itor2: sub_keys){
            if (itor2 != root_key){
                hyperedges[hyperedge_idx] = itor2;
                hyperedge_idx++;
            }
        }
    }
    hyperedge_indices[hyperedge_idx_idx] = hyperedge_idx;

    double imbalance = 0.03;
    kahypar_partition_id_t k = this->get_num_partitions();

    kahypar_hyperedge_weight_t objective = 1;

    kahypar_hyperedge_weight_t* w1 = nullptr;
    kahypar_hypernode_weight_t* w2 = nullptr;

    std::vector<kahypar_partition_id_t> partition(num_vertices);

    kahypar_partition(num_vertices,
                      num_hyperedges,
                      imbalance,
                      k,
                      w1,
                      w2,
                      hyperedge_indices.get(),
                      hyperedges.get(),
                      &objective,
                      context,
                      partition.data());

    kahypar_context_free(context);
    std::cout.rdbuf(coutbuf); //reset to standard output again

    m_logger->kahypar_finished();

    // Partition ID as key, Values as Set of Macro IDs
    std::map<size_t, std::set<size_t>> partition_map;

    for(int i = 0; i != num_vertices; ++i) {
        partition_map[partition[i]].insert(i);
    }

    for (auto itor: partition_map){
        Partition* next_partition = new Partition();
        for(auto itor2: itor.second){
            Macro* m = key_to_macro[itor2];
            nullpointer_check (m);
            next_partition->add_macro(m);
            m->set_parent_partition(next_partition);
        }
        size_t area = 0;
        for (Component* c: next_partition->get_components()){
            area += c->get_area();
        }
        std::pair<size_t, size_t> wh = this->find_shape(area);

        next_partition->set_height(wh.first);
        next_partition->set_witdh(wh.second);
        next_partition->free_lx();
        next_partition->free_ly();
        next_partition->encode_partition();
        next_partition->push_up_pins();
        m_components.push_back(next_partition);
    }
}

/**
 * @brief Perform Kahypar Partitioning based on an external Hypergraph File
 */
void Partitioning::file_based_partitioning()
{
    m_logger->start_kahypar();
    m_tree->strip_terminals();

    std::cout << "Partition Size: " << this->get_partition_size() << std::endl;
    // Redirect Kahypar Output to File
    std::ofstream out(this->get_active_results_directory() + "/kahypar.txt");
    std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
    std::cout.rdbuf(out.rdbuf()); //redirect std::cout to kahypar.txt!

    kahypar_context_t* context = kahypar_context_new();
    std::string config_file = this->get_base_path() + "/04_configuration/kahypar.ini";
    kahypar_configure_context_from_file(context, config_file.c_str());

    std::map<size_t, Macro*> key_to_macro;
    for(Macro* m: m_macros){
        key_to_macro[m->get_key()] = m;
    }

    kahypar_hypernode_id_t num_vertices = 0;
    kahypar_hyperedge_id_t num_hyperedges = 0;

    kahypar_hyperedge_id_t* hyperedges;
    size_t* hyperedge_indices;

    double imbalance = 0.03;
    kahypar_partition_id_t k = this->get_num_partitions();

    kahypar_hyperedge_weight_t objective = 1;

    kahypar_hyperedge_weight_t* w1;
    kahypar_hypernode_weight_t* w2;

    kahypar_read_hypergraph_from_file(
        std::string(this->get_working_directory() + "/hp.hgr").c_str(),
        &num_vertices,
        &num_hyperedges,
        &hyperedge_indices,
        &hyperedges,
        &w1,
        &w2);

    std::vector<kahypar_partition_id_t> partition(num_vertices, -1);

    kahypar_partition(num_vertices,
                      num_hyperedges,
                      imbalance,
                      k,
                      w1,
                      w2,
                      hyperedge_indices,
                      hyperedges,
                      &objective,
                      context,
                      partition.data());

    // Partition ID as key, Values as Set of Macro IDs
    std::map<size_t, std::set<size_t>> partition_map;

    for(int i = 0; i != num_vertices; ++i) {
        partition_map[partition[i]].insert(i);
    }

    for (auto itor: partition_map){
        Partition* next_partition = new Partition();
        m_z3_opt = new z3::optimize(m_z3_ctx);
        for(auto itor2: itor.second){
            Macro* m = key_to_macro[itor2];
            next_partition->add_macro(m);
            m->set_parent_partition(next_partition);
        }
        this->encode(next_partition);
        this->solve(next_partition);
        size_t w = next_partition->get_ux().get_numeral_uint();
        size_t h = next_partition->get_uy().get_numeral_uint();
        delete m_z3_opt; m_z3_opt = nullptr;
        next_partition->set_height(h);
        next_partition->set_witdh(w);
        next_partition->free_lx();
        next_partition->free_ly();
        m_components.push_back(next_partition);
    }

    kahypar_context_free(context);
    std::cout.rdbuf(coutbuf); //reset to standard output again

    m_logger->kahypar_finished();
}

std::pair<size_t, size_t> Partitioning::find_shape(size_t const _area)
{
    try {
        std::pair<size_t, size_t> ret_val;

        z3::optimize opt(m_z3_ctx);
        z3::expr a = m_z3_ctx.int_const("a");
        z3::expr b = m_z3_ctx.int_const("b");
        z3::check_result sat;
        int area_corration = -1;
        z3::model m(m_z3_ctx);

        do {
            area_corration++;
            opt.push();
            opt.add(a > 1);
            opt.add(b > 1);
            opt.add (a*b == m_z3_ctx.int_val(_area + area_corration));

            opt.minimize(z3::abs(a-b));
            opt.minimize(z3::abs(b-a));
            sat = opt.check();
            
            if (sat == z3::check_result::sat){
                m = opt.get_model();
            }
            opt.pop();

        } while (sat != z3::check_result::sat);

        size_t ret_a = m.eval(a).get_numeral_uint();
        size_t ret_b = m.eval(b).get_numeral_uint();
        ret_val = std::make_pair(ret_a, ret_b);
        std::cout << _area << " = " << ret_a << " * " << ret_b << std::endl;

        return ret_val;
    } catch (z3::exception const & exp){
        std::cout << exp.msg() << std::endl;
        assert (0);
    }
}
