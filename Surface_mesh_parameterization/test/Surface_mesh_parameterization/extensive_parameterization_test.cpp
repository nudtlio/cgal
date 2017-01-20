#define CGAL_CHECK_EXPENSIVE

#include <CGAL/Simple_cartesian.h>

#include <CGAL/Surface_mesh.h>
#include <CGAL/boost/graph/graph_traits_Surface_mesh.h>

#include <CGAL/Polyhedron_3.h>
#include <CGAL/boost/graph/graph_traits_Polyhedron_3.h>

#include <CGAL/boost/graph/Seam_mesh.h>
#include <CGAL/boost/graph/graph_traits_Seam_mesh.h>

#include <CGAL/Surface_mesh_parameterization/internal/shortest_path.h>
#include <CGAL/Surface_mesh_parameterization/Error_code.h>
#include <CGAL/surface_mesh_parameterization.h>

#include <CGAL/Polygon_mesh_processing/measure.h>

#include <boost/foreach.hpp>
#include <boost/functional/hash.hpp>

#include <iostream>
#include <fstream>

namespace SMP = CGAL::Surface_mesh_parameterization;

typedef CGAL::Simple_cartesian<double>            Kernel;
typedef Kernel::Point_2                           Point_2;
typedef Kernel::Point_3                           Point_3;

#define MVC_POLYHEDRON_MESH
#define ARAP_POLYHEDRON_MESH
#define BARY_SURF_MESH
#define ARAP_SURF_MESH
#define DCM_PM_SEAM_MESH
#define DAC_SM_SEAM_MESH
#define ORBIFOLD_SM_MESH

// POLYHEDRON_MESH
typedef CGAL::Polyhedron_3<Kernel>                                PMesh;

typedef boost::graph_traits<PMesh>::vertex_descriptor             PM_vertex_descriptor;
typedef boost::graph_traits<PMesh>::halfedge_descriptor           PM_halfedge_descriptor;

typedef CGAL::Unique_hash_map<PM_halfedge_descriptor, Point_2>    PM_UV_hmap;
typedef boost::associative_property_map<PM_UV_hmap>               PM_UV_pmap;

// SURF_MESH
typedef CGAL::Surface_mesh<Point_3>                               SMesh;

typedef boost::graph_traits<SMesh>::vertex_descriptor             SM_vertex_descriptor;
typedef boost::graph_traits<SMesh>::halfedge_descriptor           SM_halfedge_descriptor;

typedef SMesh::Property_map<SM_halfedge_descriptor, Point_2>      SM_UV_pmap;

// PM_SEAM_MESH
typedef boost::graph_traits<PMesh>::edge_descriptor               PM_edge_descriptor;

typedef CGAL::Unique_hash_map<PM_edge_descriptor, bool>           PM_seam_edge_hmap;
typedef boost::associative_property_map<PM_seam_edge_hmap>        PM_seam_edge_pmap;
typedef CGAL::Unique_hash_map<PM_vertex_descriptor, bool>         PM_seam_vertex_hmap;
typedef boost::associative_property_map<PM_seam_vertex_hmap>      PM_seam_vertex_pmap;

typedef CGAL::Seam_mesh<PMesh, PM_seam_edge_pmap, PM_seam_vertex_pmap>
                                                                  PM_Seam_mesh;

typedef boost::graph_traits<PM_Seam_mesh>::vertex_descriptor      PM_SE_vertex_descriptor;
typedef boost::graph_traits<PM_Seam_mesh>::halfedge_descriptor    PM_SE_halfedge_descriptor;

// SM_SEAM_MESH
typedef boost::graph_traits<SMesh>::edge_descriptor               SM_edge_descriptor;

typedef SMesh::Property_map<SM_edge_descriptor, bool>             SM_seam_edge_pmap;
typedef SMesh::Property_map<SM_vertex_descriptor, bool>           SM_seam_vertex_pmap;

typedef CGAL::Seam_mesh<SMesh, SM_seam_edge_pmap, SM_seam_vertex_pmap>
                                                                  SM_Seam_mesh;

typedef boost::graph_traits<SM_Seam_mesh>::vertex_descriptor      SM_SE_vertex_descriptor;
typedef boost::graph_traits<SM_Seam_mesh>::halfedge_descriptor    SM_SE_halfedge_descriptor;

int main(int, char**)
{
  std::cout.precision(17);
  CGAL::set_pretty_mode(std::cout);

  // ***************************************************************************
  // Default case
  // ***************************************************************************

#ifdef MVC_POLYHEDRON_MESH
  {
    std::cout << " ----------- MVC POLYHEDRON -----------" << std::endl;

    std::ifstream in("data/mushroom.off");
    PMesh pm;
    in >> pm;
    if(!in || num_vertices(pm) == 0) {
      std::cerr << "Problem loading the input data" << std::endl;
      return 1;
    }

    PM_halfedge_descriptor hd = CGAL::Polygon_mesh_processing::longest_border(pm).first;

    CGAL::Unique_hash_map<PM_vertex_descriptor, Point_2,
                          boost::hash<PM_vertex_descriptor> > uvhm;
    boost::associative_property_map<
      CGAL::Unique_hash_map<PM_vertex_descriptor, Point_2,
                            boost::hash<PM_vertex_descriptor> > > uvpm(uvhm);

    // Go to default (MVC)
    SMP::Error_code status = SMP::parameterize(pm, hd, uvpm);

    if(status != SMP::OK) {
      std::cout << "Encountered a problem: " << status << std::endl;
      return 1;
    }
    else {
      std::cout << "Parameterized with MVC (POLY)!" << std::endl;
    }
  }
#endif

  // ***************************************************************************
  // ARAP WITH POLYHEDRON_MESH
  // ***************************************************************************

#ifdef ARAP_POLYHEDRON_MESH
  {
    std::cout << " ----------- ARAP POLYHEDRON -----------" << std::endl;

    std::ifstream in("data/three_peaks.off");
    PMesh pm;
    in >> pm;
    if(!in || num_vertices(pm) == 0) {
      std::cerr << "Problem loading the input data" << std::endl;
      return 1;
    }

    PM_halfedge_descriptor hd = CGAL::Polygon_mesh_processing::longest_border(pm).first;

    // UV map
    CGAL::Unique_hash_map<PM_vertex_descriptor, Point_2,
                          boost::hash<PM_vertex_descriptor> > uvhm;
    boost::associative_property_map<
      CGAL::Unique_hash_map<PM_vertex_descriptor, Point_2,
                            boost::hash<PM_vertex_descriptor> > > uvpm(uvhm);

    // Indices map
    typedef boost::unordered_map<PM_vertex_descriptor, int> Indices;
    Indices indices;
    CGAL::Polygon_mesh_processing::connected_component(
      face(opposite(hd, pm), pm),
      pm,
      boost::make_function_output_iterator(
                 SMP::internal::Index_map_filler<PMesh, Indices>(pm, indices)));

    boost::associative_property_map<Indices> vipm(indices);

    // Vertex parameterized map
    boost::unordered_set<PM_vertex_descriptor> vs;
    SMP::internal::Bool_property_map<boost::unordered_set<PM_vertex_descriptor> > vpm(vs);

    // Parameterizer
    typename SMP::ARAP_parameterizer_3<PMesh> parameterizer;
    SMP::Error_code status = parameterizer.parameterize(pm, hd, uvpm, vipm, vpm);

    if(status != SMP::OK) {
      std::cout << "Encountered a problem: " << status << std::endl;
      return 1;
    }
    else {
      std::cout << "Parameterized with ARAP (POLY)!" << std::endl;
    }
  }
#endif

  // ***************************************************************************
  // Barycentric mapping
  // ***************************************************************************

#ifdef BARY_SURF_MESH
  {
    std::cout << " ----------- BARY SURFACE MESH ----------- " << std::endl;

    std::ifstream in("data/oni.off");
    SMesh sm;
    in >> sm;
    if(!in || num_vertices(sm) == 0) {
      std::cerr << "Problem loading the input data" << std::endl;
      return 1;
    }

    SM_halfedge_descriptor hd = CGAL::Polygon_mesh_processing::longest_border(sm).first;
    assert(hd != SM_halfedge_descriptor());

    // UV map
    typedef SMesh::Property_map<SM_vertex_descriptor, Point_2>  UV_pmap;
    UV_pmap uvpm = sm.add_property_map<SM_vertex_descriptor, Point_2>("h:uv").first;

    // Indices map
    typedef boost::unordered_map<SM_vertex_descriptor, int> Indices;
    Indices indices;
    CGAL::Polygon_mesh_processing::connected_component(
      face(opposite(hd, sm), sm),
      sm,
      boost::make_function_output_iterator(
                SMP::internal::Index_map_filler<SMesh, Indices>(sm, indices)));
    boost::associative_property_map<Indices> vipm(indices);

    // Vertex parameterized map
    boost::unordered_set<SM_vertex_descriptor> vs;
    SMP::internal::Bool_property_map<boost::unordered_set<SM_vertex_descriptor> > vpm(vs);

    // Parameterizer
    typename SMP::Barycentric_mapping_parameterizer_3<SMesh> parameterizer;

    SMP::Error_code status = parameterizer.parameterize(sm, hd, uvpm, vipm, vpm);

    if(status != SMP::OK) {
      std::cout << "Encountered a problem: " << status << std::endl;
      return 1;
    }
    else {
      std::cout << "Parameterized with Barycentric (SM)!" << std::endl;
    }
  }
#endif

  // ***************************************************************************
  // ARAP WITH SURF_MESH
  // ***************************************************************************

#ifdef ARAP_SURF_MESH
  {
    std::cout << " ----------- ARAP SURFACE MESH -----------" << std::endl;

    std::ifstream in("data/nefertiti.off");
    SMesh sm;
    in >> sm;
    if(!in || num_vertices(sm) == 0) {
      std::cerr << "Problem loading the input data" << std::endl;
      return 1;
    }

    // halfedge on the longest border
    SM_halfedge_descriptor hd =
                        CGAL::Polygon_mesh_processing::longest_border(sm).first;

    CGAL::Unique_hash_map<SM_vertex_descriptor, Point_2,
                          boost::hash<SM_vertex_descriptor> > uvhm;
    boost::associative_property_map<
      CGAL::Unique_hash_map<SM_vertex_descriptor,
                            Point_2,
                            boost::hash<SM_vertex_descriptor> > > uv_pm(uvhm);

    // Indices map
    typedef boost::unordered_map<SM_vertex_descriptor, int> Indices;
    Indices indices;
    CGAL::Polygon_mesh_processing::connected_component(
      face(opposite(hd, sm), sm),
      sm,
      boost::make_function_output_iterator(
                SMP::internal::Index_map_filler<SMesh, Indices>(sm, indices)));
    boost::associative_property_map<Indices> vipm(indices);

    // Parameterized bool pmap
    boost::unordered_set<SM_vertex_descriptor> vs;
    SMP::internal::Bool_property_map< boost::unordered_set<SM_vertex_descriptor> > vpm(vs);

    // Parameterizer
    typename SMP::ARAP_parameterizer_3<SMesh> parameterizer;

    SMP::Error_code status = parameterizer.parameterize(sm, hd, uv_pm, vipm, vpm);
    if(status != SMP::OK) {
      std::cout << "Encountered a problem: " << status << std::endl;
      return 1;
    }
    else {
      std::cout << "Parameterized with ARAP (SM)!" << std::endl;
    }
  }
#endif

#ifdef DCM_PM_SEAM_MESH
  {
    std::cout << " ----------- DCM POLYHEDRON SEAM MESH -----------" << std::endl;

    std::ifstream in("data/fandisk.off");
    PMesh pm;
    in >> pm;
    if(!in || num_vertices(pm) == 0) {
      std::cerr << "Problem loading the input data" << std::endl;
      return 1;
    }
    const char* selection = "data/fandisk.dcm.selection.txt";

    PM_seam_edge_hmap seam_edge_hm(false);
    PM_seam_edge_pmap seam_edge_pm(seam_edge_hm);
    PM_seam_vertex_hmap seam_vertex_hm(false);
    PM_seam_vertex_pmap seam_vertex_pm(seam_vertex_hm);

    PM_Seam_mesh mesh(pm, seam_edge_pm, seam_vertex_pm);
    PM_halfedge_descriptor pmhd = mesh.add_seams(selection);
    if(pmhd == PM_halfedge_descriptor() ) {
      std::cerr << "Warning: No seams in input" << std::endl;
    }

    // The 2D points of the uv parametrisation will be written into this map
    // Note that this is a halfedge property map, and that the uv
    // is only stored for the canonical halfedges representing a vertex
    PM_UV_hmap uv_hm;
    PM_UV_pmap uv_pm(uv_hm);

    // a halfedge on the (possibly virtual) border
    PM_SE_halfedge_descriptor hd = CGAL::Polygon_mesh_processing::longest_border(mesh).first;

    // Indices
    typedef boost::unordered_map<PM_SE_vertex_descriptor, int> Indices;
    Indices indices;
    CGAL::Polygon_mesh_processing::connected_component(
      face(opposite(hd, mesh), mesh),
      mesh,
      boost::make_function_output_iterator(
        SMP::internal::Index_map_filler<PM_Seam_mesh, Indices>(mesh, indices)));
    boost::associative_property_map<Indices> vipm(indices);

    // Parameterized
    boost::unordered_set<PM_SE_vertex_descriptor> vs;
    SMP::internal::Bool_property_map<boost::unordered_set<PM_SE_vertex_descriptor> > vpm(vs);

    typename SMP::Discrete_conformal_map_parameterizer_3<PM_Seam_mesh> parameterizer;

    SMP::Error_code status = parameterizer.parameterize(mesh, hd, uv_pm, vipm, vpm);

    if(status != SMP::OK) {
      std::cout << "Encountered a problem: " << status << std::endl;
      return 1;
    }
    else {
      std::cout << "Parameterized with DCM (SEAM POLY)!" << std::endl;
    }
  }
#endif

  // ***************************************************************************
  // DAC WITH SEAM_MESH (SM)
  // ***************************************************************************

#ifdef DAC_SM_SEAM_MESH
  {
    std::cout << " ----------- DAC SURFACE MESH SEAM MESH -----------" << std::endl;

    std::ifstream in("data/bear.off");
    SMesh sm;
    in >> sm;
    if(!in || num_vertices(sm) == 0) {
      std::cerr << "Problem loading the input data" << std::endl;
      return 1;
    }

    const char* selection = "data/bear.dac.selection.txt";

    SM_seam_edge_pmap seam_edge_pm =
        sm.add_property_map<SM_edge_descriptor,bool>("e:on_seam", false).first;
    SM_seam_vertex_pmap seam_vertex_pm =
        sm.add_property_map<SM_vertex_descriptor,bool>("v:on_seam", false).first;

    SM_Seam_mesh mesh(sm, seam_edge_pm, seam_vertex_pm);
    SM_halfedge_descriptor smhd = mesh.add_seams(selection);
    if(smhd == SM_halfedge_descriptor() ) {
      std::cerr << "Warning: No seams in input" << std::endl;
    }

    // The 2D points of the uv parametrisation will be written into this map
    // Note that this is a halfedge property map, and that the uv
    // is only stored for the canonical halfedges representing a vertex
    SM_UV_pmap uv_pm = sm.add_property_map<SM_halfedge_descriptor,
                                           Point_2>("h:uv").first;

    // a halfedge on the (possibly virtual) border
    SM_SE_halfedge_descriptor hd = CGAL::Polygon_mesh_processing::longest_border(mesh).first;

    // Indices
    typedef boost::unordered_map<SM_SE_vertex_descriptor, int> Indices;
    Indices indices;
    CGAL::Polygon_mesh_processing::connected_component(
        face(opposite(hd, mesh), mesh),
        mesh,
        boost::make_function_output_iterator(
        SMP::internal::Index_map_filler<SM_Seam_mesh, Indices>(mesh, indices)));
    boost::associative_property_map<Indices> vipm(indices);

    // Parameterized
    boost::unordered_set<SM_SE_vertex_descriptor> vs;
    SMP::internal::Bool_property_map<boost::unordered_set<SM_SE_vertex_descriptor> > vpm(vs);

    typename SMP::Discrete_authalic_parameterizer_3<SM_Seam_mesh> parameterizer;

    SMP::Error_code status = parameterizer.parameterize(mesh, hd, uv_pm, vipm, vpm);
    if(status != SMP::OK) {
      std::cout << "Encountered a problem: " << status << std::endl;
      return 1;
    }
    else {
      std::cout << "Parameterized with DAC (SEAM SM)!" << std::endl;
    }
  }
#endif

#ifdef ORBIFOLD_SM_MESH
  {
    std::cout << " ----------- ORBIFOLD SURFACE MESH -----------" << std::endl;

    SMesh sm; // underlying mesh of the seam mesh

    std::ifstream in("data/horse.off");
    in >> sm;
    if(!in || num_vertices(sm) == 0) {
      std::cerr << "Problem loading the input data" << std::endl;
      return 1;
    }

    const char* cone_filename = "data/horse.orbifold.selection.txt";

    // Read the cones and find the corresponding vertex_descriptor in the underlying mesh 'sm'
    typedef std::vector<SM_vertex_descriptor>       Cones_in_smesh_container;
    Cones_in_smesh_container cone_sm_vds;
    SMP::internal::read_cones<SMesh>(sm, cone_filename, cone_sm_vds);

    // Two property maps to store the seam edges and vertices
    SM_seam_edge_pmap seam_edge_pm = sm.add_property_map<SM_edge_descriptor, bool>("e:on_seam", false).first;
    SM_seam_vertex_pmap seam_vertex_pm = sm.add_property_map<SM_vertex_descriptor, bool>("v:on_seam",false).first;

    // The seam mesh
    SM_Seam_mesh mesh(sm, seam_edge_pm, seam_vertex_pm);

    // Use the path provided between cones to create a seam mesh
    SM_halfedge_descriptor smhd = mesh.add_seams(cone_filename);
    if(smhd == SM_halfedge_descriptor() ) {
      std::list<SM_edge_descriptor> seam_edges;
      SMP::internal::compute_shortest_paths_between_cones(sm, cone_sm_vds, seam_edges);

      // Add the seams to the seam mesh
      BOOST_FOREACH(SM_edge_descriptor e, seam_edges) {
        mesh.add_seam(source(e, sm), target(e, sm));
      }
    }

    // Index map of the seam mesh (assuming a single connected component so far)
    typedef boost::unordered_map<SM_SE_vertex_descriptor, int> Indices;
    Indices indices;
    boost::associative_property_map<Indices> vimap(indices);
    int counter = 0;
    BOOST_FOREACH(SM_SE_vertex_descriptor vd, vertices(mesh)) {
      put(vimap, vd, counter++);
    }

    // Mark the cones in the seam mesh
    typedef boost::unordered_map<SM_SE_vertex_descriptor, SMP::Cone_type>  Cones;
    Cones cmap;
    SMP::internal::locate_cones<SM_Seam_mesh,
                                Cones_in_smesh_container,
                                Cones>(mesh, cone_sm_vds, cmap);

    // The 2D points of the uv parametrisation will be written into this map
    // Note that this is a halfedge property map, and that uv values
    // are only stored for the canonical halfedges representing a vertex
    SM_UV_pmap uvmap = sm.add_property_map<SM_halfedge_descriptor, Point_2>("h:uv").first;

    // Parameterizer
    typedef SMP::Orbifold_Tutte_parameterizer_3<SM_Seam_mesh>         Parameterizer;
    Parameterizer parameterizer(SMP::Triangle, SMP::Cotangent);

    // a halfedge on the (possibly virtual) border
    // only used in output (will also be used to handle multiple connected components in the future)
    SM_SE_halfedge_descriptor hd = CGAL::Polygon_mesh_processing::longest_border(mesh,
                  CGAL::Polygon_mesh_processing::parameters::all_default()).first;

    SMP::Error_code status = parameterizer.parameterize(mesh, hd, cmap, uvmap, vimap);
    if(status != SMP::OK) {
      std::cout << "Encountered a problem: " << status << std::endl;
      return 1;
    }
    else {
      std::cout << "Parameterized with Orbifold (SEAM SM)!" << std::endl;
    }
  }
#endif

  std::cout << "Done!" << std::endl;

  return 0;
}
