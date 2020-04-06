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


/**
 * @brief Constructor
 * 
 * @param z3_ctx Z3 Context
 */
Partitioning::Partitioning(z3::context* z3_ctx):
    m_components_in_partition(z3_ctx->int_val(0)),
    m_components_non_overlapping(z3_ctx->int_val(0)),
    m_hpwl_cost_function(z3_ctx->int_val(0))
{
    assert (z3_ctx != nullptr);
    
    m_z3_ctx = z3_ctx;
    m_z3_utils = new EncodingUtils();
    m_kmeans = new KMeans();
}

/**
 * @brief Destructor
 */
Partitioning::~Partitioning()
{
    m_z3_ctx = nullptr;
    delete m_z3_utils; m_z3_utils = nullptr;
    delete m_kmeans; m_kmeans = nullptr;
}

/**
 * @brief Run Partitioning
 */
void Partitioning::run()
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
            Partition* next_partition = new Partition(m_z3_ctx);
            m_z3_opt = new z3::optimize(*m_z3_ctx);
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
        for (size_t j = 0; j < itor.second.size(); j+= m_partition_size){
            Partition* next_partition = new Partition(m_z3_ctx);
    
            for (size_t k = j; (k < (m_partition_size+j)) && (k < itor.second.size()); ++k){
                next_partition->add_macro(itor.second[k]);
            }
            m_components.push_back(next_partition);
            m_z3_opt = new z3::optimize(*m_z3_ctx);
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
                               size_t partition_size)
{
    m_macros = macros;
    m_partition_size = partition_size;
}

/**
 * @brief Encode SMT for Partition
 * 
 * @param next_partition Partition to Encode
 */
void Partitioning::encode(Partition* next_partition)
{
    assert (next_partition != nullptr);
    
    next_partition->set_lx(0);
    next_partition->set_ly(0);
    next_partition->free_ux();
    next_partition->free_uy();
    
    z3::params param(*m_z3_ctx);
    param.set(":opt.priority", "lex");
    m_z3_opt->set(param);
    
    this->encode_components_non_overlapping(next_partition, e2D);
    this->encode_components_in_partition(next_partition, e2D);
    
    m_z3_opt->add(m_components_in_partition);
    m_z3_opt->add(m_components_non_overlapping);
    
    //m_z3_opt->minimize(next_partition->get_ux());
    //m_z3_opt->minimize(next_partition->get_uy());
    m_z3_opt->minimize(next_partition->get_ux() + next_partition->get_uy());
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
        assert (next_partition != nullptr);
        
        z3::expr_vector clauses(*m_z3_ctx);

        z3::expr die_lx = next_partition->get_lx();
        z3::expr die_ux = next_partition->get_ux();
        z3::expr die_ly = next_partition->get_ly();
        z3::expr die_uy = next_partition->get_uy();
        
        for(Component* itor: next_partition->get_components()){
            z3::expr is_N = itor->get_orientation() == m_z3_ctx->int_val(eNorth);
            z3::expr is_W = itor->get_orientation() == m_z3_ctx->int_val(eWest);
            z3::expr is_S = itor->get_orientation() == m_z3_ctx->int_val(eSouth);
            z3::expr is_E = itor->get_orientation() == m_z3_ctx->int_val(eEast);
            
            z3::expr_vector case_N(*m_z3_ctx);
            case_N.push_back(m_z3_utils->ge(itor->get_lx(eNorth), die_lx)); ///< LX
            case_N.push_back(m_z3_utils->ge(itor->get_ly(eNorth), die_ly)); ///< LY
            case_N.push_back(m_z3_utils->le(itor->get_ux(eNorth), die_ux)); ///< UX
            case_N.push_back(m_z3_utils->le(itor->get_uy(eNorth), die_uy)); ///< UY

            z3::expr_vector case_W(*m_z3_ctx);
            case_W.push_back(m_z3_utils->ge(itor->get_lx(eWest), die_lx)); ///< LX
            case_W.push_back(m_z3_utils->ge(itor->get_ly(eWest), die_ly)); ///< LY
            case_W.push_back(m_z3_utils->le(itor->get_ux(eWest), die_ux)); ///< UX
            case_W.push_back(m_z3_utils->le(itor->get_uy(eWest), die_uy)); ///< UY

            z3::expr_vector case_S(*m_z3_ctx);
            case_S.push_back(m_z3_utils->ge(itor->get_lx(eSouth), die_lx)); ///< LX
            case_S.push_back(m_z3_utils->ge(itor->get_ly(eSouth), die_ly)); ///< LY
            case_S.push_back(m_z3_utils->le(itor->get_ux(eSouth), die_ux)); ///< UX
            case_S.push_back(m_z3_utils->le(itor->get_uy(eSouth), die_uy)); ///< UY

            z3::expr_vector case_E(*m_z3_ctx);
            case_E.push_back(m_z3_utils->ge(itor->get_lx(eEast), die_lx)); ///< LX
            case_E.push_back(m_z3_utils->ge(itor->get_ly(eEast), die_ly)); ///< LY
            case_E.push_back(m_z3_utils->le(itor->get_ux(eEast), die_ux)); ///< UX
            case_E.push_back(m_z3_utils->le(itor->get_uy(eEast), die_uy)); ///< UY
            
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

        m_components_in_partition = z3::mk_and(clauses);

    } catch (z3::exception const & exp){
        std::cout << exp.msg() << std::endl;
        exit(0);
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
        z3::expr_vector clauses(*m_z3_ctx);

        z3::expr N = m_z3_ctx->int_val(eNorth);
        z3::expr W = m_z3_ctx->int_val(eWest);
        z3::expr S = m_z3_ctx->int_val(eSouth);
        z3::expr E = m_z3_ctx->int_val(eEast);

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
                z3::expr_vector case_nn(*m_z3_ctx);
                case_nn.push_back(m_z3_utils->ge(free->get_lx(eNorth), fixed->get_ux(eNorth))); ///< Right
                case_nn.push_back(m_z3_utils->le(free->get_ux(eNorth), fixed->get_lx(eNorth))); ///< Left
                case_nn.push_back(m_z3_utils->ge(free->get_ly(eNorth), fixed->get_uy(eNorth))); ///< Upper
                case_nn.push_back(m_z3_utils->le(free->get_uy(eNorth), fixed->get_ly(eNorth))); ///< Below
//}}}
//{{{           Case West North
                z3::expr_vector case_wn(*m_z3_ctx);
                case_wn.push_back(m_z3_utils->ge(free->get_lx(eWest), fixed->get_ux(eNorth))); ///< Right
                case_wn.push_back(m_z3_utils->le(free->get_ux(eWest), fixed->get_lx(eNorth))); ///< Left
                case_wn.push_back(m_z3_utils->ge(free->get_ly(eWest), fixed->get_uy(eNorth))); ///< Upper
                case_wn.push_back(m_z3_utils->le(free->get_uy(eWest), fixed->get_ly(eNorth))); ///< Below
//}}}
//{{{           Case South North
                z3::expr_vector case_sn(*m_z3_ctx);
                case_sn.push_back(m_z3_utils->ge(free->get_lx(eSouth), fixed->get_ux(eNorth))); ///< Right
                case_sn.push_back(m_z3_utils->le(free->get_ux(eSouth), fixed->get_lx(eNorth))); ///< Left
                case_sn.push_back(m_z3_utils->ge(free->get_ly(eSouth), fixed->get_uy(eNorth))); ///< Upper
                case_sn.push_back(m_z3_utils->le(free->get_uy(eSouth), fixed->get_ly(eNorth))); ///< Below
//}}}
//{{{           Case East North
                z3::expr_vector case_en(*m_z3_ctx);
                case_en.push_back(m_z3_utils->ge(free->get_lx(eEast), fixed->get_ux(eNorth))); ///< Right
                case_en.push_back(m_z3_utils->le(free->get_ux(eEast), fixed->get_lx(eNorth))); ///< Left
                case_en.push_back(m_z3_utils->ge(free->get_ly(eEast), fixed->get_uy(eNorth))); ///< Upper
                case_en.push_back(m_z3_utils->le(free->get_uy(eEast), fixed->get_ly(eNorth))); ///< Below
//}}}
//{{{           Case North West
                z3::expr_vector case_nw(*m_z3_ctx);
                case_nw.push_back(m_z3_utils->ge(free->get_lx(eNorth), fixed->get_ux(eWest))); ///< Right
                case_nw.push_back(m_z3_utils->le(free->get_ux(eNorth), fixed->get_lx(eWest))); ///< Left
                case_nw.push_back(m_z3_utils->ge(free->get_ly(eNorth), fixed->get_uy(eWest))); ///< Upper
                case_nw.push_back(m_z3_utils->le(free->get_uy(eNorth), fixed->get_ly(eWest))); ///< Below
//}}}                
//{{{           Case West West
                z3::expr_vector case_ww(*m_z3_ctx);
                case_ww.push_back(m_z3_utils->ge(free->get_lx(eWest), fixed->get_ux(eWest))); ///< Right
                case_ww.push_back(m_z3_utils->le(free->get_ux(eWest), fixed->get_lx(eWest))); ///< Left
                case_ww.push_back(m_z3_utils->ge(free->get_ly(eWest), fixed->get_uy(eWest))); ///< Upper
                case_ww.push_back(m_z3_utils->le(free->get_uy(eWest), fixed->get_ly(eWest))); ///< Below
//}}}
//{{{           Case South West
                z3::expr_vector case_sw(*m_z3_ctx);
                case_sw.push_back(m_z3_utils->ge(free->get_lx(eSouth), fixed->get_ux(eWest))); ///< Right
                case_sw.push_back(m_z3_utils->le(free->get_ux(eSouth), fixed->get_lx(eWest))); ///< Left
                case_sw.push_back(m_z3_utils->ge(free->get_ly(eSouth), fixed->get_uy(eWest))); ///< Upper
                case_sw.push_back(m_z3_utils->le(free->get_uy(eSouth), fixed->get_ly(eWest))); ///< Below
//}}}
//{{{           Case East West
                z3::expr_vector case_ew(*m_z3_ctx);
                case_ew.push_back(m_z3_utils->ge(free->get_lx(eEast), fixed->get_ux(eWest))); ///< Right
                case_ew.push_back(m_z3_utils->le(free->get_ux(eEast), fixed->get_lx(eWest))); ///< Left
                case_ew.push_back(m_z3_utils->ge(free->get_ly(eWest), fixed->get_uy(eWest))); ///< Upper
                case_ew.push_back(m_z3_utils->le(free->get_uy(eEast), fixed->get_ly(eWest))); ///< Below
//}}}
//{{{           Case North South
                z3::expr_vector case_ns(*m_z3_ctx);
                case_ns.push_back(m_z3_utils->ge(free->get_lx(eNorth), fixed->get_ux(eSouth))); ///< Right
                case_ns.push_back(m_z3_utils->le(free->get_ux(eNorth), fixed->get_lx(eSouth))); ///< Left
                case_ns.push_back(m_z3_utils->ge(free->get_ly(eNorth), fixed->get_uy(eSouth))); ///< Upper
                case_ns.push_back(m_z3_utils->le(free->get_uy(eNorth), fixed->get_ly(eSouth))); ///< Below
//}}}
//{{{           Case West South
                z3::expr_vector case_ws(*m_z3_ctx);
                case_ws.push_back(m_z3_utils->ge(free->get_lx(eWest), fixed->get_ux(eSouth))); ///< Right
                case_ws.push_back(m_z3_utils->le(free->get_ux(eWest), fixed->get_lx(eSouth))); ///< Left
                case_ws.push_back(m_z3_utils->ge(free->get_ly(eWest), fixed->get_uy(eSouth))); ///< Upper
                case_ws.push_back(m_z3_utils->le(free->get_uy(eWest), fixed->get_ly(eSouth))); ///< Below
//}}}
//{{{           Case South South
                z3::expr_vector case_ss(*m_z3_ctx);
                case_ss.push_back(m_z3_utils->ge(free->get_lx(eSouth), fixed->get_ux(eSouth))); ///< Right
                case_ss.push_back(m_z3_utils->le(free->get_ux(eSouth), fixed->get_lx(eSouth))); ///< Left
                case_ss.push_back(m_z3_utils->ge(free->get_ly(eSouth), fixed->get_uy(eSouth))); ///< Upper
                case_ss.push_back(m_z3_utils->le(free->get_uy(eSouth), fixed->get_ly(eSouth))); ///< Below
//}}} 
//{{{           Case East South
                z3::expr_vector case_es(*m_z3_ctx);
                case_es.push_back(m_z3_utils->ge(free->get_lx(eEast), fixed->get_ux(eSouth))); ///< Right
                case_es.push_back(m_z3_utils->le(free->get_ux(eEast), fixed->get_lx(eSouth))); ///< Left
                case_es.push_back(m_z3_utils->ge(free->get_ly(eEast), fixed->get_uy(eSouth))); ///< Upper
                case_es.push_back(m_z3_utils->le(free->get_uy(eEast), fixed->get_ly(eSouth))); ///< Below
//}}}
//{{{           Case North East
                z3::expr_vector case_ne(*m_z3_ctx);
                case_ne.push_back(m_z3_utils->ge(free->get_lx(eNorth), fixed->get_ux(eEast))); ///< Right
                case_ne.push_back(m_z3_utils->le(free->get_ux(eNorth), fixed->get_lx(eEast))); ///< Left
                case_ne.push_back(m_z3_utils->ge(free->get_ly(eNorth), fixed->get_uy(eEast))); ///< Upper
                case_ne.push_back(m_z3_utils->le(free->get_uy(eNorth), fixed->get_ly(eEast))); ///< Below
//}}}
//{{{           Case West East
                z3::expr_vector case_we(*m_z3_ctx);
                case_we.push_back(m_z3_utils->ge(free->get_lx(eWest), fixed->get_ux(eEast))); ///< Right
                case_we.push_back(m_z3_utils->le(free->get_ux(eWest), fixed->get_lx(eEast))); ///< Left
                case_we.push_back(m_z3_utils->ge(free->get_ly(eWest), fixed->get_uy(eEast))); ///< Upper
                case_we.push_back(m_z3_utils->le(free->get_uy(eWest), fixed->get_ly(eEast))); ///< Below
//}}}
//{{{           Case South East
                z3::expr_vector case_se(*m_z3_ctx);
                case_se.push_back(m_z3_utils->ge(free->get_lx(eSouth), fixed->get_ux(eEast))); ///< Right
                case_se.push_back(m_z3_utils->le(free->get_ux(eSouth), fixed->get_lx(eEast))); ///< Left
                case_se.push_back(m_z3_utils->ge(free->get_ly(eSouth), fixed->get_uy(eEast))); ///< Upper
                case_se.push_back(m_z3_utils->le(free->get_uy(eSouth), fixed->get_ly(eEast))); ///< Below
//}}}
//{{{           Case East East
                z3::expr_vector case_ee(*m_z3_ctx);
                case_ee.push_back(m_z3_utils->ge(free->get_lx(eEast), fixed->get_ux(eEast))); ///< Right
                case_ee.push_back(m_z3_utils->le(free->get_ux(eEast), fixed->get_lx(eEast))); ///< Left
                case_ee.push_back(m_z3_utils->ge(free->get_ly(eEast), fixed->get_uy(eEast))); ///< Upper
                case_ee.push_back(m_z3_utils->le(free->get_uy(eEast), fixed->get_ly(eEast))); ///< Below
//}}}
//{{{
                assert (case_nn.size() == 4);
                assert (case_nw.size() == 4);
                assert (case_ns.size() == 4);
                assert (case_ne.size() == 4);
                assert (case_wn.size() == 4);
                assert (case_ww.size() == 4);
                assert (case_ws.size() == 4);
                assert (case_we.size() == 4);
                assert (case_sn.size() == 4);
                assert (case_sw.size() == 4);
                assert (case_ss.size() == 4);
                assert (case_se.size() == 4);
                assert (case_en.size() == 4);
                assert (case_ew.size() == 4);
                assert (case_es.size() == 4);
                assert (case_ee.size() == 4);
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
 * @brief Solve Partition Encoding
 * 
 * @param next_partition Partition to Solve
 */
void Partitioning::solve(Partition* next_partition)
{
    assert (next_partition != nullptr);

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
    /*
    try {
        z3::expr_vector clauses(*m_z3_ctx);
        std::vector<Edge*> edges = m_tree->get_edges();
        
        for(auto edge: edges){
            z3::expr from_x(*m_z3_ctx);
            z3::expr from_y(*m_z3_ctx);
            z3::expr to_x(*m_z3_ctx);
            z3::expr to_y(*m_z3_ctx);
            
        Node* from = edge->get_from();
        if(from->is_node()){
            if(from->has_macro()){
                from_x = from->get_macro()->get_lx();
                from_y = from->get_macro()->get_ly();
            } else if (from->has_cell()){
                continue;
                assert (0 && "Not Implemented");
            } else {
                assert (0);
            }
        } else if (from->is_terminal()){
            from_x = from->get_terminal()->get_pin_pos_x();
            from_y = from->get_terminal()->get_pin_pos_y();
        } else {
            assert (0);
        }

        Node* to = edge->get_to();
        if(to->is_node()){
            if(to->has_macro()){
                to_x = to->get_macro()->get_lx();
                to_y = to->get_macro()->get_ly();
            } else if (to->has_cell()){
                continue;
                    assert (0 && "Not Implemented");
            } else {
                assert (0);
            }
        } else if (to->is_terminal()){
            to_x = to->get_terminal()->get_pin_pos_x();
            to_y = to->get_terminal()->get_pin_pos_y();
        } else {
            assert (0);
        }
            
        // Calculate Manhattan Distance
        //Z3 Sqrt Function Costly?
         
        z3::expr distance_x = (to_x - from_x) * (to_x - from_x);
        z3::expr distance_y = (to_y - from_y) * (to_y - from_y);
        z3::expr sum = distance_x + distance_y;
        clauses.push_back(sum);
        
        }
        //return this->mk_sum(clauses);
    
    } catch(z3::exception const & exp){
        std::cout << exp.msg() << std::endl;
        assert (0);
    }
    */
}
