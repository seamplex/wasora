/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related header
 *
 *  Copyright (C) 2015--2016 jeremy theler
 *
 *  This file is part of wasora.
 *
 *  wasora is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  wasora is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with wasora.  If not, see <http://www.gnu.org/licenses/>.
 *------------------- ------------  ----    --------  --     -       -         -
 */

#ifndef _MESH_H_
#define _MESH_H_
typedef struct mesh_t mesh_t;
typedef struct physical_property_t physical_property_t;
typedef struct property_data_t property_data_t;
typedef struct material_t material_t;
typedef struct mesh_post_t mesh_post_t;
typedef struct mesh_post_dist_t mesh_post_dist_t;
typedef struct mesh_fill_vector_t mesh_fill_vector_t;
typedef struct mesh_find_max_t mesh_find_max_t;
typedef struct mesh_integrate_t mesh_integrate_t;

typedef struct physical_entity_t physical_entity_t;
typedef struct physical_name_t physical_name_t;

typedef struct node_t node_t;
typedef struct element_t element_t;
typedef struct element_list_item_t element_list_item_t;
typedef struct element_type_t element_type_t;
typedef struct cell_t cell_t;
typedef struct neighbor_t neighbor_t;
typedef struct gauss_t gauss_t;

typedef struct elementary_entity_t elementary_entity_t;
typedef struct bc_string_based_t bc_string_based_t;
typedef struct node_data_t node_data_t;


typedef enum {
    centering_default,
    centering_nodes,
    centering_cells
  } centering_t;


// le copiamos a gmsh
#define ELEMENT_TYPE_UNDEFINED    0
#define ELEMENT_TYPE_LINE         1
#define ELEMENT_TYPE_TRIANGLE     2
#define ELEMENT_TYPE_QUADRANGLE   3
#define ELEMENT_TYPE_TETRAHEDRON  4
#define ELEMENT_TYPE_HEXAHEDRON   5
#define ELEMENT_TYPE_PRISM        6
#define ELEMENT_TYPE_PYRAMID      7
#define ELEMENT_TYPE_POINT        15

#define GAUSS_POINTS_SINGLE       0
#define GAUSS_POINTS_CANONICAL    1

struct material_t {
  char *name;
  physical_entity_t *physical_entity;
  property_data_t *property_datums;

  // este es un apuntador generico que le dejamos a alguien
  // (plugins) por si necesitan agregar informacion al material
  void *ext;

  UT_hash_handle hh;
};

struct physical_property_t {
  char *name;
  property_data_t *property_datums;

  UT_hash_handle hh;
};

struct property_data_t {
  physical_property_t *property;
  material_t *material;
  expr_t expr;

  UT_hash_handle hh;
};


struct {
    
  int initialized;
    
  mesh_t *meshes;
  mesh_t *main_mesh;

  // flag que el codigo partcular rellena (preferentemente en init_before_parser)
  // para indicar si el default es trabajar sobre celdas (FVM) o sobre nodos (FEM)
  centering_t default_centering;
  
  // estas tres variables estan reallocadas para apuntar a vec_x
  struct {
    var_t *x;
    var_t *y;
    var_t *z;
    var_t *arr_x[3];   // x, y y z en un array de tamanio 3
    vector_t *vec_x;
    var_t *eps;
    
    var_t *nodes;
    var_t *cells;
    var_t *elements;
  } vars;
  
  // esto deberia ir en cada malla porque va a tener informacion sobre los puntos de gauss
  // en verdad deberia ir sobre un espacio fem, no sobre una malla
  element_type_t *element_type;
  
  physical_entity_t *physical_entities;            // por orden de aparicion
  physical_entity_t *physical_entities_by_name;
  physical_entity_t *physical_entities_by_id;
  
  material_t *materials;
  physical_property_t *physical_properties;
  mesh_post_t *posts;
  mesh_fill_vector_t *fill_vectors;
  mesh_find_max_t *find_maxs;
  mesh_integrate_t *integrates;
  
} wasora_mesh;

// nodos
struct node_t {
  int id;
  
  double x[3];       // coordenadas espaciales del nodo
  int *index;        // indice del vector incognita para cada uno de los grados de libertad
  
  element_list_item_t *associated_elements;
};


// estructura fija con tipos de elementos, incluyendo apuntadores a las funciones de forma
// los numeros son los propuestos por gmsh (ver abajo la lista)
struct element_type_t {
  char *name;

  int id;              // id segun gmsh  
  int dim;             // dimensiones espaciales del elemento
  int nodes;           // cantidad de nodos en el elemento
  int faces;           // superficies == cantidad de vecinos
  int nodes_per_face;  // cantidad de nodos en las caras

  // apuntadores a funciones de forma y sus derivadas
  double (*h)(int, gsl_vector *);
  double (*dhdr)(int, int, gsl_vector *);
  int (*point_in_element)(element_t *, const double *);
  double (*element_volume)(element_t *);
  
  gauss_t *gauss;      // juegos de puntos puntos de gauss
};


struct gauss_t {
  int V;               // numero de puntos (v=1,2,...,V )
  double *w;           // pesos (w[v] es el epso del punto v)
  double **r;          // coordenadas (r[v][m] es la coordenada del punto v en la dimension m)
  
  double **h;          // funciones de forma evaluadas en los puntos de gauss h[v][j]
  double ***dhdr;      // derivadas evaluadas dhdr[v][j][m]
};


struct physical_entity_t {
  char *name;
  int id;
  int dimension;
  
  mesh_t *mesh;
  
  // apuntador al material
  material_t *material;
  
  char *bc_type_string; // el tipo de CC es arbitraria, despues cada codigo la interpreta
  int bc_type_int;
  
  // linked list con los argumentos que definen la cc
  // dependiendo del problema, los grados de libertad y
  // el tipo de cc se interpretan de diferentes maneras
  expr_t *bc_args;
  
  // linked list con strings arbitrarias que despues cada codigo interpreta
  bc_string_based_t *bc_strings;
  
  // entero que indica en que direccion hay que aplicar una cierta cc
  // en mallas estructuradas (1 = x<0, 2 = x>0, 3 = y<0, 4 = y>0, 5 = z<0, 6 = z>0)
  enum {
    structured_direction_undefined,
    structured_direction_left,
    structured_direction_right,
    structured_direction_front,
    structured_direction_rear,
    structured_direction_bottom,
    structured_direction_top    
  } struct_bc_direction;
  
  // expresiones que indican la posicion de la entidad en malla estructudada
  // (1 = x<0, 2 = x>0, 3 = y<0, 4 = y>0, 5 = z<0, 6 = z>0)
  expr_t pos[6];
  
  // variables que contienen las reacciones de vinculo
  var_t *R[3];

  physical_entity_t *next;
  UT_hash_handle hh_id;
  UT_hash_handle hh_name;
};


struct elementary_entity_t {
  int id;
  elementary_entity_t *next;
};

struct bc_string_based_t {
  char *string;
  
  int bc_type_int;
  int dof;
  
  expr_t expr;
  expr_t expr_a;  // para robin
  expr_t expr_b;
  
  bc_string_based_t *next;
};


struct element_t {

  int id;

  element_type_t *type;                      // apuntador a tipo de elemento
  physical_entity_t *physical_entity;        // apuntador a la entidad fisica
  node_t **node;                             // apuntadores a los nodos
  cell_t *cell;                              // apuntador a la celda asociada al elemento (solo para FVM)

  int ntags;                                 // cantidad de tags (ver documentacion de gmsh)
  int *tag;                                  // el primero es la entidad fisica
};


struct element_list_item_t {
  element_t *element;
  element_list_item_t *next;
};
        
struct cell_t {
  
  int id;
  
  element_t *element;
  
  int n_neighbors;
  int *ineighbor;                // array de ids de elementos vecinos
  int **ifaces;                  // array de arrays de ids de nodos que forman las caras
  
  neighbor_t *neighbor;   // array de vecinos

  // TODO: hacer un union con x, y, z
  double x[3];     // coordenadas espaciales del baricentro de la celda
  int *index;        // indice del vector incognita para cada uno de los grados de libertad

  double volume;

};


struct neighbor_t {
  
  cell_t *cell;
  element_t *element;
 
  double **face_coord;
  double x_ij[3];
  double n_ij[3];
  double S_ij;
  
  // distancia entre los centro de la cara y el centro de la celda
  // (cache para mallas estructuradas)
//  double di;     // celda principal
//  double dj;     // celda vecina  
  
};

struct node_data_t {
  char *name_in_mesh;
  function_t *function;
  
  node_data_t *next;
};

// estructura principal de la malla 
struct mesh_t {
  char *name;
  int initialized;
  
  file_t *file;
  
  int spatial_dimensions;        // dimensiones espaciales
  int bulk_dimensions;           // dimension del mayor elemento

  int degrees_of_freedom;        // grados de libertad por incognita
  enum  {
    ordering_node_based,
    ordering_unknown_based,
  } ordering;
  
  enum {
      data_type_element,
      data_type_node,
  } data_type;
  
  int structured;                // flag que indica si la tenemos que fabricar nosotros
  
  expr_t *scale_factor;           // factor de escala al leer la posicion de los nodos
  expr_t *offset_x;               // offset en nodos
  expr_t *offset_y;               // offset en nodos
  expr_t *offset_z;               // offset en nodos
  
  int n_nodes;                   // cantidad de nodos
  int n_elements;                // cantidad de elementos (fem)
  int n_cells;                   // cantidad de celdas    (fvm)
  
  double **nodes_argument;
  double **cells_argument;

  node_data_t *node_datas;
  
  // para mallas estructuradas
  int rectangular_mesh_size[3];
  double *rectangular_mesh_point[3];
  
  expr_t *expr_ncells_x;
  int ncells_x;
  expr_t *expr_ncells_y;
  int ncells_y;
  expr_t *expr_ncells_z;
  int ncells_z;
  
  expr_t *expr_length_x;
  double length_x;
  expr_t *expr_length_y;
  double length_y;
  expr_t *expr_length_z;
  double length_z;
  
  expr_t *expr_uniform_delta_x;
  double uniform_delta_x;
  expr_t *expr_uniform_delta_y;
  double uniform_delta_y;
  expr_t *expr_uniform_delta_z;
  double uniform_delta_z;
  
  double *delta_x;
  double *delta_y;
  double *delta_z;
  
  double *nodes_x;
  double *nodes_y;
  double *nodes_z;

  double *cells_x;
  double *cells_y;
  double *cells_z;
  

  int n_physical_names;          // cantidad de nombres de entidades fisicas
  
  node_t *node;
  element_t *element;
  cell_t *cell;
  
  node_t bounding_box_max;
  node_t bounding_box_min;
  
  int max_nodes_per_element;     // maxima cantidad de nodos por elemento
  int max_faces_per_element;     // maxima cantidad de caras por elemento
  int max_first_neighbor_nodes;  // maxima cantidad de nodos vecinos (para estimar el ancho de banda)

  // kd-trees para hacer busquedas eficientes
  void *kd_nodes;
  void *kd_cells;

  struct {
    // nomenclatura como en la documentacion
    int M;            // dimensiones
    int J;            // nodos locales
    int D;            // grados de libertad
    int N;            // tamanio de la matriz elemental N = JG
    int MD;           // producto MD
    int V;            // puntos de integracion (gauss incluyendo dimensiones)

    gsl_vector *r;    // coordenadas naturales
    gsl_vector *x;    // coordenadas globales
    gsl_vector *h;    // funciones de forma
    gsl_matrix *dhdr; // jacobiano de las funciones de forma con respecto a las coordenadas naturales
    gsl_matrix *dhdx; // jacobiano de las funciones de forma con respecto a las coordenadas reales
    gsl_matrix *drdx; // jacobiano de las coordenadas naturales con respecto a las reales
    gsl_matrix *dxdr; // jacobiano de las coordenadas reales con respecto a las naturales
    
    gsl_matrix *H;
    gsl_matrix *B;
    
    int *l;           // vector con mapeo de indices locales a globales
  } fem;

  physical_entity_t *left;
  physical_entity_t *right;
  physical_entity_t *front;
  physical_entity_t *rear;
  physical_entity_t *bottom;
  physical_entity_t *top;    
  
  UT_hash_handle hh;

};


struct mesh_post_dist_t {
  centering_t centering;
  
  function_t *scalar;
  function_t **vector;
  function_t ***tensor;
  
  mesh_post_dist_t *next;
};

struct mesh_post_t {
  mesh_t *mesh;
  file_t *file;
  int no_mesh;
  
  enum  {
    post_format_fromextension,
    post_format_gmsh,
    post_format_vtk,
  } format;

  centering_t centering;
  
  int (*write_header)(FILE *);
  int (*write_mesh)(mesh_t *, FILE *);
  int (*write_scalar)(mesh_post_t *, function_t *, centering_t);
  int (*write_vector)(mesh_post_t *, function_t **, centering_t);
  
  // estos dos son para saber si tenemos que cambiar de tipo en VTK
  int point_init;
  int cell_init;
  
  mesh_post_dist_t *mesh_post_dists;
  
  // flags genericos para codigos particulares
  int flags;
  
  mesh_post_t *next;
};


struct mesh_fill_vector_t {
  mesh_t *mesh;
  vector_t *vector;
  
  function_t *function;
  expr_t expr;
  centering_t centering;
   
  mesh_fill_vector_t *next;
};

struct mesh_find_max_t {
  mesh_t *mesh;
  function_t *function;
  expr_t expr;
  centering_t centering;
  
  var_t *max;
  var_t *i_max;
  var_t *x_max;
  var_t *y_max;
  var_t *z_max;
      
  mesh_find_max_t *next;
};


struct mesh_integrate_t {
  mesh_t *mesh;
  function_t *function;
  expr_t expr;
  physical_entity_t *physical_entity;
  centering_t centering;
  int gauss_points;
  
  var_t *result;
      
  mesh_integrate_t *next;
};


// mesh.c
extern element_t *mesh_find_element(mesh_t *, const double *);
extern int mesh_free(mesh_t *);
extern mesh_t *wasora_get_mesh_ptr(const char *);


// cell.c
extern int mesh_element2cell(mesh_t *);
extern int mesh_compute_coords(mesh_t *);
extern int mesh_cell_indexes(mesh_t *, int);

// element.c
extern int wasora_mesh_element_types_init(void);
extern int mesh_alloc_gauss(gauss_t *, element_type_t *, int);
extern int mesh_init_shape_at_gauss(gauss_t *, element_type_t *);
extern int mesh_create_element(element_t *, int, int, physical_entity_t *);
extern int mesh_add_element_to_list(element_list_item_t **, element_t *);
extern int mesh_compute_element_barycenter(element_t *, double []);
extern int mesh_node_indexes(mesh_t *, int);

// gauss.c
extern int mesh_init_gauss_points(mesh_t *, int);
extern int mesh_init_gauss_weights(int, int, double **, double **);
extern double mesh_integral_over_element(function_t *, element_t *, expr_t *);
extern double mesh_integration_weight(mesh_t *, element_t *, int);
extern void mesh_init_fem_objects(mesh_t *mesh);

// gmsh.c
extern int mesh_gmsh_readmesh(mesh_t *);
extern int mesh_gmsh_write_header(FILE *);
extern int mesh_gmsh_write_mesh(mesh_t *, FILE *);
extern int mesh_gmsh_write_scalar(mesh_post_t *, function_t *, centering_t);
extern int mesh_gmsh_write_vector(mesh_post_t *, function_t **, centering_t);

// vtk.c
extern int mesh_vtk_write_header(FILE *);
extern int mesh_vtk_write_mesh(mesh_t *, FILE *);
extern int mesh_vtk_write_structured_mesh(mesh_t *, FILE *);
extern int mesh_vtk_write_unstructured_mesh(mesh_t *, FILE *);
extern int mesh_vtk_write_scalar(mesh_post_t *, function_t *, centering_t);
extern int mesh_vtk_write_vector(mesh_post_t *, function_t **, centering_t);

// init.c
extern int wasora_mesh_init_before_parser(void);

// interpolate.c
extern double mesh_interpolate_function_node(function_t *, const double *);
extern double mesh_interpolate_function_cell(function_t *, const double *);
extern double mesh_interpolate_function_property(function_t *, const double *);
extern int mesh_interp_residual(const gsl_vector *, void *, gsl_vector *);
extern int mesh_interp_jacob(const gsl_vector *, void *, gsl_matrix *);
extern int mesh_interp_residual_jacob(const gsl_vector *, void *, gsl_vector *, gsl_matrix *);
extern int mesh_interp_solve_for_r(element_t *, const double *, gsl_vector *);



// fem.c
extern void mesh_compute_dxdr(element_t *, gsl_vector *, gsl_matrix *);
extern void mesh_inverse(int, gsl_matrix *, gsl_matrix *);
extern double mesh_determinant(int, gsl_matrix *);
extern void mesh_compute_dhdx(element_t *, gsl_vector *, gsl_matrix *, gsl_matrix *);
extern void mesh_compute_h(element_t *, gsl_vector *, gsl_vector *);
extern void mesh_compute_x(element_t *, gsl_vector *, gsl_vector *);
extern int mesh_compute_l(mesh_t *, element_t *);
extern double mesh_compute_fem_objects_at_gauss(mesh_t *, element_t *, int);


extern int mesh_compute_B(mesh_t *, element_t *);
int mesh_compute_H(mesh_t *, element_t *);

// neighbors.c
extern int mesh_count_common_nodes(element_t *, element_t *, int *);
extern int mesh_find_neighbors(mesh_t *);
extern int mesh_fill_neighbors(mesh_t *);
extern element_t *mesh_find_element_volumetric_neighbor(element_t *);
extern element_t *mesh_find_node_neighbor_of_dim(node_t *, int);


// parser.c
extern int wasora_mesh_parse_line(char *);

// struct.c
extern int mesh_create_structured(mesh_t *);
extern void wasora_mesh_struct_init_rectangular_for_cells(mesh_t *);
extern void wasora_mesh_struct_init_rectangular_for_nodes(mesh_t *);
extern int wasora_mesh_struct_find_cell(int, double *, double *, double);

// point.c
extern int mesh_one_node_point_init(void);
extern double mesh_one_node_point_h(int, gsl_vector *);
extern double mesh_one_node_point_dhdr(int, int, gsl_vector *);
extern double mesh_point_vol(element_t *);


// line.c
extern int mesh_two_node_line_init(void);
extern double mesh_two_node_line_h(int, gsl_vector *);
extern double mesh_two_node_line_dhdr(int, int, gsl_vector *);
extern int mesh_point_in_line(element_t *, const double *);
extern double mesh_line_vol(element_t *);

// triang.c
extern int mesh_three_node_triangle_init(void);
extern double mesh_three_node_triang_h(int, gsl_vector *);
extern double mesh_three_node_triang_dhdr(int, int, gsl_vector *);
extern int mesh_point_in_triangle(element_t *, const double *);
extern double mesh_triang_vol(element_t *);


// quad.c
extern int mesh_four_node_quadrangle_init(void);
extern double mesh_four_node_quad_h(int, gsl_vector *);
extern double mesh_four_node_quad_dhdr(int, int, gsl_vector *);
extern int mesh_point_in_quadrangle(element_t *, const double *);
extern double mesh_quad_vol(element_t *);

// hexahedron.c
extern int mesh_six_node_hexahedron_init(void);
extern double mesh_eight_node_hexahedron_h(int, gsl_vector *);
extern double mesh_eight_node_hexahedron_dhdr(int, int, gsl_vector *);
extern int mesh_point_in_hexahedron(element_t *, const double *);
extern double mesh_hexahedron_vol(element_t *);

// tetrahedron.c
extern int mesh_four_node_tetrahedron_init(void);
extern double mesh_four_node_tetrahedron_h(int, gsl_vector *);
extern double mesh_four_node_tetrahedron_dhdr(int, int, gsl_vector *);
extern int mesh_point_in_tetrahedron(element_t *, const double *);
extern double mesh_tetrahedron_vol(element_t *);


// prism.c
extern int mesh_six_node_prism_init(void);
extern double mesh_six_node_prism_h(int, gsl_vector *);
extern double mesh_six_node_prism_dhdr(int, int, gsl_vector *);
extern int mesh_point_in_prism(element_t *, const double *);
extern double mesh_prism_vol(element_t *);


// geom.c
extern void mesh_subtract(const double *, const double *, double *);
extern void mesh_cross(const double *, const double *, double *);
extern void mesh_normalized_cross(const double *, const double *, double *);
extern double mesh_cross_dot(const  double *, const double *, const double *);
extern double mesh_subtract_cross_2d(const  double *, const double *, const double *);
extern double mesh_subtract_module(const double *, const double *);
extern double mesh_subtract_squared_module(const double *, const double *);
extern double mesh_subtract_squared_module2d(const double *, const double *);
extern double mesh_subtract_dot(const double *, const double *, const double *);
extern double mesh_dot(const double *, const double *);

extern void mesh_set_xyz(double *, struct var_t *, struct var_t *, struct var_t *);


extern mesh_t *wasora_define_mesh(char *, file_t *, int, int, int, int, int, expr_t *, expr_t *, expr_t *, expr_t *, expr_t *);
extern material_t *wasora_define_material(const char *);
extern physical_entity_t *wasora_define_physical_entity(char *, int, mesh_t *, int, material_t *, bc_string_based_t *, int);
extern physical_property_t *wasora_define_physical_property(const char *, mesh_t *);
extern property_data_t *wasora_define_property_data(const char *, const char *, const char *);

extern material_t  *wasora_get_material_ptr(const char *);
extern physical_entity_t *wasora_get_physical_entity_ptr(const char *);

#endif  /* _MESH_H_ */
