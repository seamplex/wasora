/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora common framework header
 *
 *  Copyright (C) 2009--2020 jeremy theler
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

#ifndef _WASORA_H_
#define _WASORA_H_

#include <stdio.h>
#include <fcntl.h>
#include <semaphore.h>

#include "thirdparty/uthash.h"
#include "thirdparty/utlist.h"

#define HAVE_INLINE
#define GSL_RANGE_CHECK_OFF

// we need all the includes here so they all follow the inline directive above
#include <gsl/gsl_blas.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_deriv.h>
#include <gsl/gsl_heapsort.h>
#include <gsl/gsl_integration.h>
#include <gsl/gsl_interp.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_min.h>
#include <gsl/gsl_multifit_nlin.h>
#include <gsl/gsl_multimin.h>
#include <gsl/gsl_multiroots.h>
#include <gsl/gsl_qrng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_roots.h>
#include <gsl/gsl_spline.h>
#include <gsl/gsl_sf.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_sort_double.h>
#include <gsl/gsl_sort_vector_double.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_version.h>




#ifdef HAVE_IDA
 #include <ida/ida.h>
 #include <nvector/nvector_serial.h>
 #include <sundials/sundials_types.h>
 #include <sundials/sundials_math.h>
 #if IDA_VERSION == 2
  #include <ida/ida_dense.h>
 #elif IDA_VERSION == 3
  #include <sunmatrix/sunmatrix_dense.h> /* access to dense SUNMatrix            */
  #include <sunlinsol/sunlinsol_dense.h> /* access to dense SUNLinearSolver      */
  #include <ida/ida_direct.h>            /* access to IDADls interface           */
 #endif
#endif

// cantidad de funciones internas
#define N_BUILTIN_FUNCTIONS         53
//#define N_BUILTIN_FUNCTIONS  (sizeof(builtin_function)/sizeof(struct builtin_function_t))

// cantidad de funcionales internos
#define N_BUILTIN_FUNCTIONALS       8
//#define N_BUILTIN_FUNCTIONALS  (sizeof(builtin_functional)/sizeof(struct builtin_functional_t))

// cantidad de funciones sobre vectores
#define N_BUILTIN_VECTOR_FUNCTIONS  8

// macro para acceder al valor de una variable
#define wasora_var(var)        (*(var->value))
#define wasora_value(var)      (*(var->value))
#define wasora_var_value(var)  (*(var->value))
#define wasora_value_ptr(var)  (var->value)

// macro para acceder a las variables internas
#define wasora_special_var(var) (wasora.special_vars.var)

// macros para facilitar las sumas
#define gsl_vector_add_to_element(vector,i,x)    gsl_vector_set((vector),(i),gsl_vector_get((vector),(i))+(x))
#define gsl_matrix_add_to_element(matrix,i,j,x)  gsl_matrix_set((matrix),(i),j,gsl_matrix_get((matrix),(i),(j))+(x))

// macro para verificar errores en llamadas a funciones
#define wasora_call(function)   if ((function) != WASORA_RUNTIME_OK) return WASORA_RUNTIME_ERROR

// macro para hacer free y apuntar a NULL
#define wasora_free(p) free(p);p=NULL;

extern const char operators[];
extern const char factorseparators[];

// tamanios maximos del buffer para un token 
#define BUFFER_SIZE        255

// delimitadores de tokens para el parser 
#define UNQUOTED_DELIM    " \t\n"
#define QUOTED_DELIM      "\""

extern const char operators[];
extern const char factorseparators[];

#define VERSION_COMPACT    0
#define VERSION_COPYRIGHT  1
#define VERSION_INFO       2

// tipo de componentes del phase space (los numeros son de la sundials), no cambiar 
#define DAE_ALGEBRAIC                      0.0
#define DAE_DIFFERENTIAL                   1.0

// tipos de ecuaciones (lo de arriba se refiere a variables) 
// esto tiene aires de ser una mascara 
#define EQN_FLAG_GENERIC                   0   
#define EQN_FLAG_DIFFERENTIAL              1
#define EQN_FLAG_ALGEBRAIC                 2
#define EQN_FLAG_SCALAR                    4
#define EQN_FLAG_VECTORIAL                 8
#define EQN_FLAG_MATRICIAL                16
#define EQN_FLAG_IMPLICIT                 32
#define EQN_FLAG_EXPLICIT                 64

#define STEP_ALL                           0
#define STEP_BEFORE_DAE                    1
#define STEP_AFTER_DAE                     3

// defaults razonables 
#define DEFAULT_PRINT_FORMAT               "%g"
#define DEFAULT_PRINT_SEPARATOR            "\t"

#define DEFAULT_M4_FORMAT                  "%g"

#define DEFAULT_INTERPOLATION              (*gsl_interp_linear)

#define DEFAULT_RANDOM_METHOD              gsl_rng_knuthran2002

#define DEFAULT_NLIN_FIT_METHOD            gsl_multifit_fdfsolver_lmsder
#define DEFAULT_NLIN_FIT_MAX_ITER          100
#define DEFAULT_NLIN_FIT_EPSREL            1e-4
#define DEFAULT_NLIN_FIT_EPSABS            1e-6
#define DEFAULT_NLIN_FIT_GRAD_H            1e-2

#define DEFAULT_SOLVE_METHOD               gsl_multiroot_fsolver_dnewton
#define DEFAULT_SOLVE_EPSREL               0   // cero quiere decir que no mire deltas en derivadas
#define DEFAULT_SOLVE_EPSABS               1e-6
#define DEFAULT_SOLVE_MAX_ITER             1024

#define DEFAULT_MINIMIZER_METHOD           gsl_multimin_fminimizer_nmsimplex2
#define DEFAULT_MINIMIZER_TOL              1e-1
#define DEFAULT_MINIMIZER_GRADTOL          1e-3
#define DEFAULT_MINIMIZER_MAX_ITER         100
#define DEFAULT_MINIMIZER_POPULATION       20
#define DEFAULT_MINIMIZER_STEPS            5
#define DEFAULT_MINIMIZER_F_STEP           1
#define DEFAULT_MINIMIZER_FDF_STEP         1e-2
#define DEFAULT_MINIMIZER_GRAD_H           1e-2


#define DEFAULT_ROOT_MAX_TER               1024
#define DEFAULT_ROOT_TOLERANCE             (9.765625e-4)         // (1/2)^-10

#define DEFAULT_INTEGRATION_INTERVALS      1024
#define DEFAULT_INTEGRATION_TOLERANCE      (9.765625e-4)         // (1/2)^-10
#define DEFAULT_INTEGRATION_KEY            GSL_INTEG_GAUSS31

#define DEFAULT_DERIVATIVE_STEP            (9.765625e-4)         // (1/2)^-10

#define DEFAULT_MULTIDIM_INTERPOLATION_THRESHOLD   9.5367431640625e-07 // (1/2)^-20
#define DEFAULT_SHEPARD_RADIUS                     1.0
#define DEFAULT_SHEPARD_EXPONENT                   2

#define MINMAX_ARGS       10

// infinito y cero 
#define INFTY         (1125899906842624.0)                    // 2^50
#define ZERO          (8.881784197001252323389053344727e-16)  // (1/2)^-50

#define M_SQRT5 2.23606797749978969640917366873127623544061835961152572427089

#define DEFAULT_MESH_FAILED_INTERPOLATION_FACTOR   -1


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 *  global structures
 * -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 *  forward type definitions
 * -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/
typedef struct var_t var_t;
typedef struct vector_t vector_t;
typedef struct matrix_t matrix_t;
typedef struct alias_t alias_t;

typedef struct vector_sort_t vector_sort_t;

typedef struct phase_object_t phase_object_t;
typedef struct dae_t dae_t;

typedef struct file_t file_t;
typedef struct loadable_routine_t loadable_routine_t;

typedef struct expr_t expr_t;
typedef struct factor_t factor_t;

typedef struct function_t function_t;

typedef struct instruction_t instruction_t;
typedef struct conditional_block_t conditional_block_t;

typedef struct history_t history_t;
typedef struct io_t io_t;
typedef struct io_thing_t io_thing_t;
typedef struct assignment_t assignment_t;
typedef struct call_t call_t;
typedef struct print_t print_t;
typedef struct print_token_t print_token_t;
typedef struct print_function_t print_function_t;
typedef struct print_vector_t print_vector_t;
typedef struct m4_t m4_t;
typedef struct m4_macro_t m4_macro_t;
typedef struct shell_t shell_t;
typedef struct solve_t solve_t;


typedef struct builtin_function_t builtin_function_t;
typedef struct builtin_vectorfunction_t builtin_vectorfunction_t;
typedef struct builtin_functional_t builtin_functional_t;

typedef struct multidim_range_t multidim_range_t;

typedef struct physical_entity_t physical_entity_t;
typedef struct geometrical_entity_t geometrical_entity_t;
typedef struct mesh_t mesh_t;


// -- expresion algebraic ------------ -----        ----           --     -
// una expresion algebraica consiste en una cantidad de tokens y 
// un arreglo de tokens que la representa 
struct expr_t {
  int n_tokens;
  factor_t *token;
  double value;
  
  // por si acaso nos guardamos el string
  char *string;

  expr_t *next;
};


// -- variable -------- ------------- -----        ----           --     -
struct var_t {
  char *name;
  int initialized;
  
  // apuntadores para que alguien pueda tirar el holder
  // y hacer apuntar la variable a otro lado (por ejemplo variables
  // que estan en commons de fortran, si, asi de weird puede ser la vida)
  double *value;
  double *initial_static;
  double *initial_transient;

  // TODO: otros tipos

  // flag para saber si es hay que evaluarla en cada paso o queda constante
  int constant;
  
  // flag para saber si el apuntador de arriba lo alocamos nosotros o alguien mas
  int realloced;

  UT_hash_handle hh;
};


// -- vector ------------ -----        ----           --     -
struct vector_t {
  char *name;
  int initialized;
  
  expr_t *size_expr; // el apuntador es porque quiero hacerlo en define_vector en lugar del parser.c 
  int size;
  int constant;

  gsl_vector *value;
  gsl_vector *initial_transient;
  gsl_vector *initial_static;

  // flag para saber si el apuntador de arriba lo alocamos nosotros o alguien mas
  int realloced;
  
  // funcion para sacarle los datos
  function_t *function_data;
  function_t *function_arg;
  int function_n_arg;
  
  // linked list con las expresiones de datos
  expr_t *datas;

  UT_hash_handle hh;
};


// -- matriz ------------ -----        ----           --     -
struct matrix_t {
  char *name;
  int initialized;  
  expr_t *cols_expr;
  expr_t *rows_expr;
  
  int cols;
  int rows;
  int constant;
  
  gsl_matrix *value;
  gsl_matrix *initial_transient;
  gsl_matrix *initial_static;

  // flag para saber si el apuntador de arriba lo alocamos nosotros o alguien mas
  int realloced;
  
  expr_t *datas;
  
  UT_hash_handle hh;
};

// -- alias --------- -------       --------           -
struct alias_t {
  int initialized;
  
  var_t *new_variable;
  
  expr_t row;
  expr_t col;
  
  var_t *variable;
  vector_t *vector;
  matrix_t *matrix;
  
  alias_t *next;
};

struct multidim_range_t {
  int dimensions;

  expr_t *min;
  expr_t *max;
  expr_t *step;
  expr_t *nsteps;
};

struct vector_sort_t {
  int descending;
  
  vector_t *v1;
  vector_t *v2;
};
  
  
// -- function ------------ -----        ----           --     -
struct function_t {
  char *name;
  char *name_in_mesh;
  int initialized;
  
  // might be:
  //   - algebraic
  //   - pointwise-defined
  //       + given in the input
  //       + given in a file
  //       + given in wasora vectors
  //       + in a mesh
  //          * as another function defined over materials (physical groups)
  //          * data at nodes
  //          * data at cells
  //  - comptued in a user-provided routine
  //
  enum  {
    type_undefined,
    type_algebraic,
    type_pointwise_data,
    type_pointwise_file,
    type_pointwise_vector,
    type_pointwise_mesh_property,
    type_pointwise_mesh_node,
    type_pointwise_mesh_cell,
    type_routine,
    type_routine_internal,
  } type;
  
  // cantidad de argumentos que toma la funcion 
  int n_arguments;

  // arreglo de apuntadores a variables (ya existen) que son los argumentos 
  var_t **var_argument;
  int var_argument_alloced;


  // ----------- --------- --------- ----   --     -
  // expresion para funciones algebraicas
  expr_t algebraic_expression;


  // ---------------- ------------ -------- -- --- --
  // cantidad de pares de datos x-y para funciones point-wise
  size_t data_size;

  // arreglos con los datos
  double **data_argument;
  int data_argument_alloced;
  double *data_value;
  
  // esto es por si esto es una derivada y queremos interpolar con las funciones de forma
  // ojo! hay que poner en data la funcion original y no las derivadas
  function_t *spatial_derivative_of;
  int spatial_derivative_with_respect_to;

  // ayudita por si la malla es regular
  int rectangular_mesh;
  expr_t expr_x_increases_first;
  int x_increases_first;
  expr_t *expr_rectangular_mesh_size;
  int *rectangular_mesh_size;
  double **rectangular_mesh_point;

  // archivo con los datos point-wise
  char *data_file;

  // columnas donde hay que ir a buscar los datos 
  // array de tamanio n_arguments+1 (la ultima es el valor) 
  int *column;

  // vectores con los datos point-wise 
  vector_t **vector_argument;
  vector_t *vector_value;

  
  // apuntadores para interpolar con 1D gsl 
  gsl_interp *interp;
  gsl_interp_accel *interp_accel;
  gsl_interp_type interp_type;

  // flag para saber que interpolacion multidimensional se selecciona 
  enum {
    nearest,
    shepard,
    shepard_kd,
    bilinear
  } multidim_interp;

  
  expr_t expr_multidim_threshold;
  double multidim_threshold;
  expr_t expr_shepard_radius;
  double shepard_radius;
  expr_t expr_shepard_exponent;
  double shepard_exponent;

  // propiedad
  void *property;

  // malla no-estructurada sobre la que esta definida la funcion
  mesh_t *mesh;
  double mesh_time;
  
  // apuntador a un arbol k-dimensional para nearest neighbors 
  void *kd;
  
  // ----- ------- -----------   --        -       - 
  // funcion que hay que llamar para funciones tipo usercall 
  double (*routine)(const double *);
  
  // ----- ------- -----------   --        -       - 
  // funcion que hay que llamar para funciones internas
  double (*routine_internal)(const double *, function_t *);
  void *params;

  UT_hash_handle hh;
};

// archivo (puede se de entrada o de salida)
struct file_t {
  char *name;
  int initialized;
  
  char *format;
  int n_args;
  expr_t *arg;
  char *mode;
  int do_not_open;

  char *path;
  FILE *pointer;

  UT_hash_handle hh;
};

struct loadable_routine_t {
  char *name;
  int initialized;
    
  double (*routine)(const double *);
// TODO: acordarse de este para poder hacer dlclose
//  void *library;

  UT_hash_handle hh;
};

// -- token algebraic ------------ -----        ----           --     -
// un token es un parentesis, un operador, una constante, una variable o una funcion 
struct factor_t {
// constante, variable, funcion, valor inicial, etc
  int type;
// nivel de parentesis 
  int level;
// nivel temporal de parentesis, para sumas parciales 
  int tmp_level;

// numero identificatorio del operador, si es el caso 
  int oper;

// numerito de la constante numerica, si es el caso 
  double constant;

// valor actual 
  double value;
// vector con cosas auxiliares si es que se necesitan (valor anterior,
//   acumulador de la integral, etc, incluso el random generator)
  double *aux;

  // TODO: union
  builtin_function_t *builtin_function;
  builtin_vectorfunction_t *builtin_vectorfunction;
  builtin_functional_t *builtin_functional;
  var_t *variable;
  vector_t *vector;
  matrix_t *matrix;
  function_t *function;

  vector_t **vector_arg;
  
// variable sobre la que opera el funcional (segundo argumento)  
  var_t *functional_var_arg;

// expresiones algebraicas de los argumentos a una function (o vector)
  expr_t *arg;

};


// -- historia ------------ -----        ----           --     -

// historia de una variable metida en una funcion que se va actualizando tiempo a tiempo
struct history_t {
  int initialized;
  
  var_t *variable;
  function_t *function;
  int position;

  history_t *next;
};



// -- bloques condicionales ------ -       ----           --     -
struct conditional_block_t {
  conditional_block_t *father;
  conditional_block_t *else_of;
  
  expr_t *condition;
  
  instruction_t *first_true_instruction;
  instruction_t *first_false_instruction;
  
  int evaluated_to_true;
  int evaluated_to_false;
  
  conditional_block_t *next;
};

// -- instruccion ------------ -----        ----           --     -
struct instruction_t {
  int (*routine)(void *);
  void *argument;
  int argument_alloced;

  instruction_t *next;
};

// -- acoples ------------ -----        ----           --     -

// semaforo 
struct semaphore_t {
  char *name;
  int initialized;
  
  sem_t *pointer;
  enum {
    wasora_sem_wait, wasora_sem_post
  } operation;
  
  struct semaphore_t *next;
};

// I/O 
struct io_t {
  int initialized;
    
  enum {
    io_read,
    io_write
  } direction;

  enum {
    io_undefined,
    io_shm,
    io_file_ascii,
    io_file_binary,
    io_tcp,
    io_comedi
  } type;

/*  
  enum {
    format_double,
    format_float,
    format_char
  } floating_point_format;
*/
  int size;
  int n_things;
  
  int ignorenull;

  io_thing_t *things;

  char *shm_name;

  FILE *filepointer;
  file_t *file;
  
  // TODO: tcp & comedi
  double *shm_pointer_double;
  float *shm_pointer_float;
  char *shm_pointer_char;

  io_t *next;

};


struct io_thing_t {

  var_t *variable;
  vector_t *vector;
  matrix_t *matrix;
  expr_t expr;
  
  expr_t expr_row_min;
  expr_t expr_row_max;
  expr_t expr_col_min;
  expr_t expr_col_max;
  
  int row_min;
  int row_max;
  int col_min;
  int col_max;
  
  io_thing_t *next;
};

// -- funcion interna ------------ -----        ----           --     -
// funcion interna, se inicializa en el data space desde builinfunctions.h 
struct builtin_function_t {
  char name[BUFFER_SIZE];
  int min_arguments;
  int max_arguments;
  double (*routine)(struct factor_t *);
};


// -- funcion interna de vectores ------------ -----        ----           --     -
// funcion sobre vectores interna, se inicializa en el data space desde builinfunctions.h 
struct builtin_vectorfunction_t {
  char name[BUFFER_SIZE];
  int min_arguments;
  int max_arguments;
  double (*routine)(struct vector_t **);
};

// -- funcional interno ------------ -----        ----           --     -

// funcional interno, se inicializa en el data space
// un funcional es como una funcion pero su segundo argumento es una
// variable (no una expresion), que se usa para calcular la expresion
// del primer argumento en varios lugares y hacer algo con esto, como
// por ejemplo derivar o integrar. Ademas se agregan mas argumentos.
// i.e. integral(1+x,x,0,1)
// ie.e derivative(1+x,x,0.5)
struct builtin_functional_t {
  char name[BUFFER_SIZE];
  int min_arguments; // contando la variable del segundo argumento
  int max_arguments;
  double (*routine)(struct factor_t *, struct var_t *);
};




// -- asignacion de variables ------------ -----        ----           --     -

// igualacion algebraica 
struct  assignment_t {
  int initialized;
  
  var_t *variable;
  vector_t *vector;
  matrix_t *matrix;
  
  // expresiones que indican el rango de tiempo
  expr_t t_min;
  expr_t t_max;

  // expresiones que indican el rango de indices para asignaciones vectoriales/matriciales
  expr_t i_min;
  expr_t i_max;
  expr_t j_min;
  expr_t j_max;

  // expresiones que indican la fila y la columna en funcion de i y de j
  expr_t row;
  expr_t col;
  
  // to sum up:
  // a(row, col)<i_min,i_max;j_min,j_max>[t_min,tmax]
  
  // la expresion del miembro derecho
  expr_t rhs;
  
  // flag que indica si el assignment pide _init o _0
  int initial_static;
  int initial_transient;
  
  // si este es true entonces la asignacion es una sola (aun cuando el miembro
  // izquierdo sea un vector o matriz), i.e.
  // A(127,43)  es escalar
  // A(2*i+1,14) no es escalar
  int scalar;

  // si esto es true, entonces las asignaciones vector o matriz son plain, i.e.
  // A(i,j)
  int plain;
  
  // diferenciamos, en caso de las matrices, aquellas dependencias solo en i o en j
  // A(i,4) = 5 es un assignment que depende solo de i
  int expression_only_of_i;
  
  // A(8,j) = 1 es un assignment que depende solo de j
  int expression_only_of_j;
  
  
  assignment_t *next;

};


// -- user call ------------ -----        ----           --     -

// llamada a funcion del usuario como instruccion
struct call_t {
  char *name;
  double (*function)(const double *);

  int n_arguments;
  expr_t *arguments;

  double *x;

  call_t *next;
};


// -- print ------------ -----        ----           --     -

// instruccion imprimir linea 
struct print_t {
  // apuntador al archivo de salida 
  file_t *file;

  // una linked list con las cosas a imprimir
  print_token_t *tokens;
 
  // separador de cosas "\t" 
  // por ahora uno solo para todo el print 
  char *separator;

  // flag para indicar si hay que mandar el \n al final (es no newline asi el
  // default es cero y le manda el enter como loco de una) 
  int nonewline;

  // cosas para hacer skips 
  int last_static_step;
  int last_step;
  double last_time;

  expr_t skip_static_step;
  expr_t skip_step;
  expr_t skip_time;

  int header;
  expr_t skip_header_step;
  int last_header_step;
  int header_already_printed;
  
  print_t *next;
};


struct print_token_t {
  char *format;
  char *text;
  expr_t expression;
  
  vector_t *vector;
  matrix_t *matrix;
  function_t *function;
  
  print_token_t *next;
};


// instruccion imprimir una o mas funciones 
struct print_function_t {
  // apuntador al archivo de salida 
  file_t *file;

  // linked list con las cosas a imprimir
  print_token_t *tokens;

  // apuntador a la primera funcion que aparezca (la que tiene la cantidad posta
  // de argumentos), no es igual a token->function porque esa puede ser NULL
  function_t *first_function;

  // rango explicito donde hay que imprimir la funcion
  multidim_range_t range;
  
  // entidad fisica donde hay que evaluar la funcion
  physical_entity_t *physical_entity;

  // flag para saber que estamos imprimiendo
  int header;
  // formato de los numeritos "%e"
  char *format;
  // separador de cosas "\t"
  char *separator;

  print_function_t *next;

};

// instruccion imprimir uno o mas vectores 
struct print_vector_t {
  // apuntador al archivo de salida 
  file_t *file;

  // una linked list con las cosas a imprimir
  print_token_t *tokens;

  // apuntador a la primera funcion que aparezca (la que tiene la cantidad posta
  // de argumentos), no es igual a first_token->function porque esa puede ser NULL
  vector_t *first_vector;
  
  // formato de los numeritos "%e" 
  char *format;
  // separador de cosas "\t" 
  char *separator;
  
  int horizontal;
  expr_t elems_per_line;

  print_vector_t *next;
};

// -- m4 ------------ -----        ----           --     -
// instruccion para llamar a m4
struct m4_t {
  
  file_t *input_file;
  file_t *output_file;

  m4_macro_t *macros;
 
  m4_t *next;
};

struct m4_macro_t {
  char *name;
  print_token_t print_token;
    
  m4_macro_t *next;
};

// -- shell ------------ -----        ----           --     -
// ejecutamos un comando 
struct shell_t {
  char *format;
  int n_args;
  expr_t *arg;  

  shell_t *next;
};

// -- solve ------------ -----        ----           --     -
// resolvemos sistemas de ecuaciones no lineales
struct solve_t {
  int n;
  var_t **unknown;
  expr_t *residual;
  
  expr_t *guess;
  
  expr_t epsabs;
  expr_t epsrel;
  int max_iter;
  int verbose;

  const gsl_multiroot_fsolver_type *type;
  
  solve_t *next;  
};

// -- parametric ------------ -----        ----           --     -
// estudio parametrico 
typedef struct {
  int dimensions;
  
  enum {
    parametric_linear,
    parametric_logarithmic,
    parametric_random,
    parametric_gaussianrandom,
    parametric_sobol,
    parametric_niederreiter,
    parametric_halton,
    parametric_reversehalton
  } type;
  
  var_t **variable;
  
  // rango donde debe evaluarse la funcion a ajustar
  multidim_range_t range;
  // rangos evaluados
  double *min;
  double *max;
  double *step;
  int *nsteps;
  
  // numero total de pasos
  int outer_steps;
  
  // offset para reusar calculos ya hechos
  int offset;
  
  // lo tenemos que hacer en paralelo?
  int max_daughters;
  
  // tenemos que hacer el parametrico adiabatico?
  int adiabatic;
  
} parametric_t;


// informacion para el fit 
typedef struct {
  
  const gsl_multifit_fdfsolver_type *algorithm;
  int max_iter;
  int norerun;
  int verbose;
  
  // cantidad de parametros (i.e. a y b -> 2)
  int p;
  // cantidad de datos experimentales a ajustar (i.e. del orden de 1000)
  int n;
  
  // apuntador a la funcion cuyos parametros hay que ajustar
  function_t *function;
  // apuntador a la funcion point-wise que tiene los datos experimentales
  function_t *data;
  
  // arreglo de apuntadores a las variables que funcionan como parametros
  // y a sus incertezas estadisticas resultantes
  var_t **param;
  var_t **sigma;

  // arreglo de tamanio n_params con las expresiones de la derivada de 
  // la funcion function con respecto a los parametros
  // si es NULL hay que calcularlo a manopla numericamente
  expr_t *gradient;  
  
  // lo mismo pero para el guess inicial
  expr_t *guess;
  
  // rango donde debe evaluarse la funcion a ajustar
  multidim_range_t range;
  
  expr_t deltaepsabs;
  expr_t deltaepsrel;
  
} fit_t;


// informacion para la optimizacion 
typedef struct {
  
  int norerun;
  int verbose;
  
  // dimension de la optimizacion (ej. n = 2)
  int n;

  const gsl_multimin_fminimizer_type *f_type;
  const gsl_multimin_fdfminimizer_type *fdf_type;
  const gsl_rng_type *siman_type;
  int genetic;

  // apuntador a la funcion objetivo
  function_t *function;
  
  // o las rutinas de siman
  double (*siman_Efunc) (void *);
//  double (*siman_metric) (void *, void *);
  void *(*siman_init) (void);
  void (*siman_step) (const gsl_rng *, void *, double);
  void (*siman_copy) (void *, void *);
  void *(*siman_copy_construct) (void *);
  void (*siman_print) (void *);
  void (*siman_best) (void *);
  void (*siman_destroy) (void *);
  

  
  // arreglo de apuntadores a las variables que son los argumentos de la funcion objetivo
  var_t **x;
  // apuntador al vector que tiene el estado del solver (depende del metodo)
  gsl_vector *solver_status;
  
  // arreglo de tamanio n_params con las expresiones de la derivada de 
  // la funcion function con respecto a los parametros
  // si es NULL y el algoritmo las necesita, las tenemos que calcular
  // numericamente
  expr_t *gradient;  
  
  // lo mismo pero para el el gues inicial y los steps
  expr_t *guess;
  multidim_range_t range;

  int n_steps;

  expr_t max_iter;
  expr_t tol;
  expr_t gradtol;
} min_t;

// plugin dinamico
typedef struct {
  void *handle;
  
  char *library_file;
  char *wasorahmd5_string;
  char *name_string;
  char *version_string;
  char *longversion_string;
  char *description_string;
  char *usage_string;
  char *copyright_string;

  char *(*name)(void);
  char *(*version)(void);
  char *(*longversion)(void);
  char *(*description)(void);
  char *(*usage)(void);
  char *(*wasorahmd5)(void);
  char *(*copyright)(void);
  int (*init_before_parser)(void);
  int (*parse_line)(char *);
  int (*init_after_parser)(void);
  int (*init_before_run)(void);
  int (*finalize)(void);
  
} plugin_t;

 struct {
  int argc;            // estos son los apuntadores a los del sistema
  char **argv;         // que getopt cambia de orden y demas
  int optind;

  int argc_orig;       // estos son copias de los originales
  char **argv_orig;    // antes de llamar a getopt

  int argc_unknown;    // estos son los que get_opt no conoce
  char **argv_unknown; // y se los pasamos a los plugins
  
  
  char *main_input_dirname_aux;
  char *main_input_dirname;
  char *main_input_basename;

  int rank;
  int nprocs;
  parametric_t parametric;
  fit_t fit;
  min_t min;
  
  plugin_t *plugin;
  
  var_t *vars;
  vector_t *vectors;
  matrix_t *matrices;
  alias_t *aliases;
  function_t *functions;
  file_t *files;
  loadable_routine_t *loadable_routines;

  print_t *prints;
  print_function_t *print_functions;
  print_vector_t *print_vectors;

  struct semaphore_t *semaphores;
  io_t *ios;
  assignment_t *assignments;
  call_t *calls;
  history_t *histories;
  m4_t *m4s;
  shell_t *shells;
  solve_t *solves;

  conditional_block_t *conditional_blocks;
  instruction_t *instructions;
  instruction_t *last_defined_instruction;

  struct {
    var_t *done_static;
    var_t *done_transient;
    var_t *done_outer;
    var_t *done;
  
    var_t *step_static;
    var_t *step_transient;
    var_t *step_outer;
    var_t *step_inner;

    var_t *in_outer_initial;
    var_t *in_static;
    var_t *in_static_first;
    var_t *in_static_last;
    var_t *in_transient;
    var_t *in_transient_first;
    var_t *in_transient_last;

    var_t *static_steps;
    var_t *end_time;

    var_t *time;        // estos dos son alias
    var_t *t;
    var_t *dt;

    var_t *rel_error;
    var_t *min_dt;
    var_t *max_dt;
  
    var_t *i;
    var_t *j;
    var_t *pi;
    var_t *zero;
    var_t *infinite;

    var_t *ncores;
    var_t *pid;
    
    var_t *on_nan;
    var_t *on_gsl_error;
    var_t *on_ida_error;
    var_t *realtime_scale;
  } special_vars;
	
  struct {
    vector_t *abs_error;
  } special_vectors;

  struct {
    file_t *stdin_;
    file_t *stdout_;
    file_t *stderr_;
  } special_files;


  enum {
    mode_normal,
    mode_ignore_debug,
    mode_debug,
    mode_single_step,
    mode_list_vars
  } mode;
  expr_t cond_breakpoint;

  // instruction pointer
  instruction_t *ip;
  instruction_t *next_flow_instruction;

  // este es una arreglo a expresiones
  expr_t *time_path;
  // este es un apuntador a la proxima expresion
  expr_t *current_time_path;
  double next_time;

  struct timeval t0;
  struct timeval t;

  char **error;
  int error_level;
  
  int implicit_none;
  int do_not_evaluate_assigns_at_parse_time;
  
  int parametric_mode;
  int fit_mode;
  int min_mode;
  
  char *lock_dir;

  
  conditional_block_t *active_conditional_block;

  int i_plugin;
  
// este es un apuntador a un buffer interno para tener memoria sobre los
// strtoken para poder parsear cosas con comillas
  char *strtok_internal;

  size_t page_size;
  size_t actual_buffer_size; 
  char *line;
  
} wasora;

struct phase_object_t {
  int offset;
  int size;
  int differential;
  char *name;

  var_t *variable;
  var_t *variable_dot;
  vector_t *vector;
  vector_t *vector_dot;
  matrix_t *matrix;
  matrix_t *matrix_dot;

  phase_object_t *next;
};

struct dae_t {
  expr_t residual;
  
  vector_t *vector;
  matrix_t *matrix;
  
  expr_t expr_i_min;
  expr_t expr_i_max;
  expr_t expr_j_min;
  expr_t expr_j_max;
  
  int i_min;
  int i_max;
  int j_min;
  int j_max;
  
  int equation_type;
  
  dae_t *next;
};

struct {
  int dimension;
  // arreglo de apuntadores a los current y a las derivadas de los
  // objectos que pertenecen al phase space
  double **phase_value;
  double **phase_derivative;
  phase_object_t *phase_objects;
  
  // linked list con las ecuaciones de los residuos
  dae_t *daes;

  void *system;

  enum {
    as_provided,
    from_variables,
    from_derivatives,
  } initial_conditions_mode;

  int reading_daes;
  instruction_t *instruction;

// ojo que el tamanio de esta estructura depende de si esta
// definido HAVE_IDA o no (ojo plugins!)
#if HAVE_IDA
  N_Vector x;
  N_Vector dxdt;
  N_Vector id;
  N_Vector abs_error;
 #if IDA_VERSION == 3
  SUNMatrix A;
  SUNLinearSolver LS;
 #endif 
#endif

} wasora_dae;


extern builtin_function_t builtin_function[N_BUILTIN_FUNCTIONS];
extern builtin_vectorfunction_t builtin_vectorfunction[N_BUILTIN_VECTOR_FUNCTIONS];
extern builtin_functional_t builtin_functional[N_BUILTIN_FUNCTIONALS];



#define WASORA_PARSER_OK              0
#define WASORA_PARSER_UNHANDLED       1
#define WASORA_PARSER_ERROR          -1
#define WASORA_PARSER_OTHER          -2

#define WASORA_RUNTIME_OK             0
#define WASORA_RUNTIME_ERROR         -1
#define WASORA_RUNTIME_NAN           -2
#define WASORA_RUNTIME_GSL           -3


// no son enums porque hacemos operaciones con las mascaras de abajo
#define EXPR_OPERATOR                             0
#define EXPR_CONSTANT                             1
#define EXPR_VARIABLE                             2
#define EXPR_VECTOR                               3
#define EXPR_MATRIX                               4
// old-school numbers
//#define EXPR_NUM                                  5
#define EXPR_BUILTIN_FUNCTION                     6
#define EXPR_BUILTIN_VECTORFUNCTION               7
#define EXPR_BUILTIN_FUNCTIONAL                   8
#define EXPR_FUNCTION                             9

#define EXPR_BASICTYPE_MASK                    1023
#define EXPR_CURRENT                              0
#define EXPR_INITIAL_TRANSIENT                 2048
#define EXPR_INITIAL_STATIC                    4096


#define ON_ERROR_NO_QUIT            1
#define ON_ERROR_NO_REPORT          2


// algebra.c 
extern int wasora_parse_expression(const char *, struct expr_t *);
extern int wasora_parse_madeup_expression(char *, struct factor_t *);
extern int wasora_parse_factor(char *, struct factor_t *);

extern double wasora_evaluate_expression(struct expr_t *);
extern double wasora_evaluate_expression_in_string(const char *);

extern int wasora_count_divisions(expr_t *);

// assignment.c 
extern void wasora_check_initial_variable(struct var_t *);
extern void wasora_check_initial_vector(struct vector_t *);
extern void wasora_check_initial_matrix(struct matrix_t *);
extern int wasora_assign_scalar(struct assignment_t *, int, int);
extern struct var_t *wasora_get_assignment_variable(struct assignment_t *, int, int);
extern int wasora_get_assignment_array_boundaries(assignment_t *, int *, int *, int *, int *);
extern int wasora_get_assignment_rowcol(assignment_t *, int, int, int *, int *);


// builtinfunctionals.c 
extern double wasora_gsl_function(double, void *);

// builtinvectorfunctions 

// call.c 

// cleanup.c 
extern void wasora_polite_exit(int);
extern void wasora_free_shm(void);
extern void wasora_destroy_expression(struct expr_t *);
extern void wasora_free_print_vectors(void);
extern void wasora_free_print_functions(void);
extern void wasora_free_prints(void);
extern void wasora_free_solves(void);
extern void wasora_free_m4(void);
extern void wasora_free_dae(void);
extern void wasora_free_assignments(void);

extern void wasora_finalize(void);

extern void wasora_free_function(function_t *);
extern void wasora_free_functions(void);
extern void wasora_free_var(var_t *);
extern void wasora_free_vars(void);
extern void wasora_free_vector(vector_t *);
extern void wasora_free_vectors(void);
extern void wasora_free_matrix(matrix_t *);
extern void wasora_free_matrices(void);



// wasora.c 
extern int wasora_standard_run(void);
extern int wasora_step(int);

// parser.c 
extern int wasora_allocate_objects(void);
extern int wasora_parse_line(char *);
extern int wasora_parse_main_input_file(char *);
extern int wasora_parse_input_file(char *, int, int);
extern char *wasora_get_nth_token(char *, int);

// dae.c 
extern int wasora_dae_init(void);
extern int wasora_dae_ic(void);
#if HAVE_IDA
extern int wasora_ida_dae(realtype, N_Vector, N_Vector, N_Vector, void *);
#else
extern int wasora_ida_dae(void);
#endif

// debug.c 
extern void wasora_debug_printerror(void);
extern void wasora_debug(void);
extern char **wasora_rl_completion(const char *, int, int);
extern char *wasora_rl_symbol_generator(const char *, int);
extern void wasora_list_symbols(void);

// dyncall.c 
typedef double (*user_func_t)(const double*);
extern user_func_t set_dyn_call_so(const char *, const char *);
extern user_func_t get_dyn_call_so(const char *);

// error.c 
extern void wasora_push_error_message(const char *, ...);
extern void wasora_pop_error_message(void);
extern void wasora_pop_errors(void);
extern void wasora_runtime_error(void);
extern void wasora_nan_error(void);
extern void wasora_gsl_handler (const char *, const char *, int, int);

// file.c 
extern char *wasora_evaluate_string(char *, int, expr_t *);
extern FILE *wasora_fopen(const char *, const char *);


// fit.c 
extern int wasora_fit_run();
extern void wasora_fit_read_params_from_solver(const gsl_vector *);
extern int wasora_fit_compute_f(gsl_vector *);
extern void wasora_fit_compute_analytical_df(gsl_matrix *);
extern int wasora_fit_compute_numerical_df(gsl_matrix *);
extern int wasora_gsl_fit_f(const gsl_vector *m, void *, gsl_vector *);
extern int wasora_gsl_fit_df(const gsl_vector *, void *, gsl_matrix *);
extern int wasora_gsl_fit_fdf(const gsl_vector *, void *, gsl_vector *, gsl_matrix *);
extern void wasora_fit_print_state(int iter, int status, gsl_multifit_fdfsolver *s);

// function.c 
extern void wasora_set_function_args(function_t *, double *);
extern double wasora_evaluate_factor_function(struct factor_t *);
extern int wasora_function_init(function_t *);
extern double wasora_evaluate_function(function_t *, const double *);
extern int wasora_structured_scalar_index(int, int *, int *, int);

extern double mesh_interpolate_function_node_dummy(function_t *, const double *);
extern double mesh_interpolate_function_cell_dummy(function_t *, const double *);
extern double mesh_interpolate_function_property_dummy(function_t *, const double *);


extern double mesh_interpolate_function_node(function_t *, const double *);
extern double mesh_interpolate_function_cell(function_t *, const double *);

// getptr.c 
extern var_t *wasora_get_variable_ptr(const char *);
extern vector_t *wasora_get_vector_ptr(const char *);
extern matrix_t *wasora_get_matrix_ptr(const char *);
extern function_t *wasora_get_function_ptr(const char *);
extern builtin_function_t *wasora_get_builtin_function_ptr(const char *);
extern builtin_vectorfunction_t *wasora_get_builtin_vectorfunction_ptr(const char *);
extern builtin_functional_t *wasora_get_builtin_functional_ptr(const char *);
extern file_t  *wasora_get_file_ptr(const char *);
extern loadable_routine_t *wasora_get_loadable_routine(const char *);
extern double (*wasora_get_routine_ptr(const char *))(const double *);
extern struct semaphore_t *wasora_get_sem_ptr(char *);
extern vector_t *wasora_get_first_vector(const char *);
extern matrix_t *wasora_get_first_matrix(const char *);
extern char *wasora_get_first_dot(const char *s);

// handler.c 
extern void wasora_signal_handler(int);

// history.c 
extern void wasora_init_history(history_t *);

// init.c 
extern int wasora_init_before_parser(void);
extern int wasora_init_after_parser(void);
extern int wasora_init_before_run(void);
extern int wasora_is_structured_grid_2d(double *, double *, int, int *, int *);
extern int wasora_is_structured_grid_3d(double *, double *, double *, int, int *, int *, int *);

// io.c 
extern int wasora_io_init(io_t *);
extern int wasora_io_read_shm(io_t *, double *, int);
extern int wasora_io_write_shm(io_t *, double *, int);
extern int wasora_io_read_ascii_file(io_t *, double *, int);
extern int wasora_io_write_ascii_file(io_t *, double *, int);
extern int wasora_io_read_binary_file(io_t *, double *, int);
extern int wasora_io_write_binary_file(io_t *, double *, int);


// matrix.c
extern double wasora_matrix_get(matrix_t *, const size_t, const size_t);
extern double wasora_matrix_get_initial_transient(matrix_t *, const size_t,  const size_t);
extern double wasora_matrix_get_initial_static(matrix_t *, const size_t,  const size_t);
extern int wasora_matrix_init(matrix_t *);

// minimize.c 
extern int wasora_min_run();
extern void wasora_min_read_params_from_solver(const gsl_vector *) ;
extern double wasora_min_compute_f(const double *);
extern double wasora_gsl_min_f(const gsl_vector *, void *);
extern void wasora_gsl_min_df(const gsl_vector *, void *, gsl_vector *);
extern void wasora_gsl_min_fdf(const gsl_vector *, void *, double *, gsl_vector *);

// multiminf.c 
void wasora_min_multiminf(gsl_vector *);
extern void wasora_multiminf_print_state(int, gsl_multimin_fminimizer *);

// multiminfdf.c 
void wasora_min_multiminfdf(gsl_vector *);
extern void wasora_multiminfdf_print_state(int, gsl_multimin_fdfminimizer *);

// multirootc
extern int wasora_gsl_solve_f(const gsl_vector *x, void *params, gsl_vector *f);

// parametric.c 
extern int wasora_parametric_run();
extern void wasora_parametric_run_parallel();


// parser.c
extern int wasora_parse_first_pass(const char *, int, int);

// parseaux.c 
extern int wasora_parser_expression(expr_t *);
extern int wasora_parser_expressions(expr_t *[], size_t);
extern int wasora_parser_expression_in_string(double *);
extern int wasora_parser_match_keyword_expression(char *, char *[], expr_t *[], size_t);
extern int wasora_parser_string(char **);
extern int wasora_parser_string_format(char **, int *);
extern int wasora_parser_file(file_t **);
extern int wasora_parser_file_path(file_t **, char *);
extern int wasora_parser_function(function_t **);
extern int wasora_parser_vector(vector_t **);
extern int wasora_parser_variable(var_t **);
extern int wasora_parser_keywords_ints(char *[], int *, int *);
extern int wasora_parse_assignment(char *, assignment_t *);

extern void wasora_realloc_variable_ptr(var_t *, double *, int);
extern void wasora_realloc_vector_ptr(vector_t *, double *, int);
extern void wasora_realloc_matrix_ptr(matrix_t *, double *, int);
extern var_t *wasora_define_variable(char *);
extern vector_t *wasora_define_vector(char *, int, expr_t *, expr_t *);
extern matrix_t *wasora_define_matrix(char *, int, expr_t *, int, expr_t *, expr_t *);
extern function_t *wasora_define_function(const char *, int);
extern file_t *wasora_define_file(char *, char *, int, expr_t *, char *, int);

extern loadable_routine_t *wasora_define_loadable_routine(char *, void *);
extern instruction_t *wasora_define_instruction(int (*)(void *), void *);
extern var_t *wasora_get_or_define_variable_ptr(char *);
extern char *wasora_get_next_token(char *);
extern int wasora_check_name(const char *);
extern int wasora_parse_range(char *, const char, const char, const char, struct expr_t *, struct expr_t *);

extern int wasora_parse_factor(char *, struct factor_t *);
extern int wasora_read_line(FILE *);
extern int wasora_read_data_line(FILE *, char *);
extern void wasora_strip_blanks(char *);
extern void wasora_strip_brackets(char *);
extern void wasora_add_leading_zeros(char **);
extern int wasora_replace_arguments(char **);
extern int wasora_count_arguments(char *);
extern void wasora_syntax_error(char *, int);

extern char *wasora_ends_in_zero(char *);
extern char *wasora_ends_in_init(char *);
extern char *wasora_ends_in_i(char *);
extern char *wasora_is_vector_underscore_something(char *);
extern char *wasora_is_matrix_underscore_something(char *, char **, char **);



// plugin.c
extern int wasora_load_hardcoded_plugin(void);
extern int wasora_load_plugin(const char *);
extern void *wasora_dlopen(const char *);
extern int wasora_dlopen_try(const char *, const char *, const char *, void **);


// print.c

// randomline.c
extern void wasora_print_random_line(FILE *, int);

// realtime.c
extern void wasora_init_realtime(void);
extern void wasora_wait_realtime(void);

// shell.c


// shmem.c
extern void *wasora_get_shared_pointer(char *, size_t);
extern void wasora_free_shared_pointer(void *, char *, size_t);
extern sem_t *wasora_get_semaphore(char *);
extern void wasora_free_semaphore(sem_t *, char *);
extern int wasora_create_lock(char *, int);
extern void wasora_remove_lock(char *, int);

// siman.c
void wasora_min_siman(gsl_vector *);
extern double wasora_siman_Efunc_real(void *);
extern double wasora_siman_metric_real(void *, void *);
extern void wasora_siman_step_real(const gsl_rng *, void *, double);
extern void *wasora_siman_copy_construct_real(void *);
extern void wasora_siman_copy_real(void *, void *);
extern void wasora_siman_destroy_real(void *);
extern void wasora_siman_print_real(void *);

// vector.c
extern double wasora_vector_get(vector_t *, const size_t);
extern double wasora_vector_get_initial_transient(vector_t *, const size_t);
extern double wasora_vector_get_initial_static(vector_t *, const size_t);
extern int wasora_vector_set(vector_t *, const size_t, double);
extern int wasora_vector_init(vector_t *);
extern int wasora_instruction_vector_sort(void *);

// version.c
extern void wasora_show_help();
extern void wasora_show_version(int);
extern void wasora_shortversion(void);
extern void wasora_copyright(void);
extern void wasora_longversion(void);

// str_replace.c
char *str_replace (const char *, const char *, const char *);

// instructions
extern int wasora_instruction_if(void *);
extern int wasora_instruction_else(void *);
extern int wasora_instruction_endif(void *);
extern int wasora_instruction_parametric(void *);


extern int wasora_instruction_alias(void *);
extern int wasora_instruction_sem(void *);
extern int wasora_instruction_io(void *);
extern int wasora_instruction_history(void *);
extern int wasora_instruction_print(void *);
extern int wasora_instruction_print_function(void *);
extern int wasora_instruction_print_vector(void *);
extern int wasora_instruction_print_matrix(void *);
extern int wasora_instruction_m4(void *);
extern int wasora_instruction_shell(void *);
extern int wasora_instruction_solve(void *);
extern int wasora_instruction_call(void *);
extern int wasora_instruction_assignment(void *);
extern int wasora_instruction_dae(void *);
extern int wasora_instruction_file(void *);
extern int wasora_instruction_open_file(void *);
extern int wasora_instruction_close_file(void *);
extern int wasora_instruction_abort(void *);
extern int wasora_instruction_mesh(void *);
extern int wasora_instruction_mesh_post(void *);
extern int wasora_instruction_mesh_fill_vector(void *);
extern int wasora_instruction_mesh_find_minmax(void *);
extern int wasora_instruction_mesh_integrate(void *arg);


// interface.h
// variable-related functions
extern int wasora_exists_var(const char *);
extern double wasora_get_var_value(const char *);
extern double *wasora_get_var_value_ptr(const char *);
extern void wasora_set_var_value(const char *, double);

// vector-related functions
extern int wasora_exists_vector(const char *);
extern double wasora_get_vector_value(const char *, int);
extern gsl_vector *wasora_get_vector_gsl_ptr(const char *);
extern void wasora_set_vector_value(const char *, int, double);
extern double *wasora_get_crisp_pointer(const char *);
extern int wasora_get_vector_size(const char *);

// function-related functions
extern int wasora_exists_function(const char *);
extern double wasora_get_function_value(const char *name, double *arg);

extern int wasora_exists_var(const char *);
extern double wasoraexistsvar_(const char *, int);
extern double wasora_get_var_value(const char *);
extern double wasoragetvarvalue_(const char *, int);
extern void wasora_set_var_value(const char *, double);
extern int wasorasetvarvalue_(const char *, double *, int);

extern int wasora_exists_vector(const char *);
extern double wasoraexistsvector_(const char *, int);
extern double wasora_get_vector_value(const char *, int);
extern double wasoragetvectorvalue_(const char *, int *, int);
extern void wasora_set_vector_value(const char *, int, double);
extern int wasorasetvectorvalue_(const char *, int *, double *, int);
extern double *wasora_get_crisp_pointer(const char *);
extern int wasora_get_vector_size(const char *);
extern int wasoragetvectorsize_(const char *, int);

extern int wasora_exists_matrix(const char *);
extern double wasoraexistsmatrix_(const char *, int);
extern double wasora_get_matrix_value(const char *, int , int);
extern gsl_matrix *wasora_get_matrix_gsl_ptr(const char *);
extern double wasoragetmatrixvalue_(const char *, int *, int *, int);
extern void wasora_set_matrix_value(const char *, int , int , double);
extern int wasorasetmatrixvalue_(const char *, int *, int *, double *, int);
extern double *wasora_get_matrix_crisp_pointer(const char *);
extern int wasora_get_matrix_rows(const char *);
extern int wasora_get_matrix_cols(const char *);
extern int wasoragetmatrixcols_(const char *, int);
extern int wasoragetmatrixrows_(const char *, int);


extern int wasora_exists_function(const char *);
extern double wasoraexistsfunction_(const char *, int);
extern double wasoragetfunctionvalue(const char *, double *, int);
extern double wasora_get_function_value(const char *name, double *arg);



// ------------------------------------------------------------------------
//  mesh
// ------------------------------------------------------------------------

typedef struct physical_property_t physical_property_t;
typedef struct property_data_t property_data_t;
typedef struct material_t material_t;
typedef struct material_list_item_t material_list_item_t;
typedef struct mesh_post_t mesh_post_t;
typedef struct mesh_post_dist_t mesh_post_dist_t;
typedef struct mesh_fill_vector_t mesh_fill_vector_t;
typedef struct mesh_find_minmax_t mesh_find_minmax_t;
typedef struct mesh_integrate_t mesh_integrate_t;

// es esta mas arriba porque se necesita en print_function
//typedef struct physical_entity_t physical_entity_t;
typedef struct physical_name_t physical_name_t;

typedef struct node_t node_t;
typedef struct node_relative_t node_relative_t;
typedef struct element_t element_t;
typedef struct element_list_item_t element_list_item_t;
typedef struct element_type_t element_type_t;
typedef struct cell_t cell_t;
typedef struct neighbor_t neighbor_t;
typedef struct gauss_t gauss_t;

typedef struct elementary_entity_t elementary_entity_t;
typedef struct bc_t bc_t;
typedef struct node_data_t node_data_t;


typedef enum {
    centering_default,
    centering_nodes,
    centering_cells
  } centering_t;


// usamos los de gmsh, convertimos a vtk y frd con tablas
#define ELEMENT_TYPE_UNDEFINED      0
#define ELEMENT_TYPE_LINE2          1
#define ELEMENT_TYPE_TRIANGLE3      2
#define ELEMENT_TYPE_QUADRANGLE4    3
#define ELEMENT_TYPE_TETRAHEDRON4   4
#define ELEMENT_TYPE_HEXAHEDRON8    5
#define ELEMENT_TYPE_PRISM6         6
#define ELEMENT_TYPE_PYRAMID5       7
#define ELEMENT_TYPE_LINE3          8
#define ELEMENT_TYPE_TRIANGLE6      9
#define ELEMENT_TYPE_QUADRANGLE9    10
#define ELEMENT_TYPE_TETRAHEDRON10  11
#define ELEMENT_TYPE_HEXAHEDRON27   12 
#define ELEMENT_TYPE_POINT1         15
#define ELEMENT_TYPE_QUADRANGLE8    16
#define ELEMENT_TYPE_HEXAHEDRON20   17
#define ELEMENT_TYPE_PRISM15        18
#define NUMBER_ELEMENT_TYPE         19

//#define GAUSS_POINTS_FULL      0
//#define GAUSS_POINTS_REDUCED   1

struct material_t {
  char *name;
  mesh_t *mesh;
  property_data_t *property_datums;

  // este es un apuntador generico que le dejamos a alguien
  // (plugins) por si necesitan agregar informacion al material
  void *ext;

  UT_hash_handle hh;
};

struct material_list_item_t {
  material_t *material;
  material_list_item_t *next;
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
  int need_cells;
    
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
    
    var_t *nx;
    var_t *ny;
    var_t *nz;
    var_t *arr_n[3];
    vector_t *vec_n;
    
    var_t *eps;

    var_t *nodes;
    var_t *cells;
    var_t *elements;
    
    vector_t *bbox_min;
    vector_t *bbox_max;
    
    var_t *mesh_failed_interpolation_factor;
  } vars;

  // esto deberia ir en cada malla porque va a tener informacion sobre los puntos de gauss
  // en verdad deberia ir sobre un espacio fem, no sobre una malla
  element_type_t *element_type;

  material_t *materials;
  physical_property_t *physical_properties;
  mesh_post_t *posts;
  mesh_fill_vector_t *fill_vectors;
  mesh_find_minmax_t *find_minmaxs;
  mesh_integrate_t *integrates;

} wasora_mesh;

// nodes
struct node_t {
  int tag;                  // number assigned by Gmsh
  int index_mesh;           // index within the node array

  double x[3];              // spatial coordinates of the node
  int *index_dof;           // index within the solution vector for each DOF
  
  double *phi;              // values of the solution functions at the node
  gsl_matrix *dphidx;       // derivative of the m-th DOF with respect to coordinate g
                            // (this is a gsl_matrix to avoid having to do double mallocs and forgetting about row/col-major
  gsl_matrix *delta_dphidx; // same as above but for the standard deviations of the derivatives
  double *f;                // holder of arbitrary functions evaluated at the node (sigmas and taus)
  
  element_list_item_t *associated_elements;
};


struct node_relative_t {
  int index;
  node_relative_t *next;
};


struct gauss_t {
  int V;               // numero de puntos (v=1,2,...,V )
  double *w;           // pesos (w[v] es el epso del punto v)
  double **r;          // coordenadas (r[v][m] es la coordenada del punto v en la dimension m)
  
  double **h;          // funciones de forma evaluadas en los puntos de gauss h[v][j]
  gsl_matrix **dhdr;   // derivadas evaluadas dhdr[v](j,m)
  
  // uso gsl_matrix asi no tengo que hacer muchos allocs ni hacerme cargo de row/col-major
  gsl_matrix *extrap;  // matrix de VxV para extrapolar valores desde los puntos de gauss a los nodos de primer orden
  
};


// estructura fija con tipos de elementos, incluyendo apuntadores a las funciones de forma
// los numeros son los propuestos por gmsh (ver abajo la lista)
struct element_type_t {
  char *name;

  int id;              // id segun gmsh  
  int dim;             // dimensiones espaciales del elemento
  int order;           // eso
  int nodes;           // cantidad de nodos en el elemento
  int first_order_nodes;
  int faces;           // superficies == cantidad de vecinos
  int nodes_per_face;  // cantidad de nodos en las caras

  double *barycenter_coords;
  double **node_coords;
  node_relative_t **node_parents;
  
  // apuntadores a funciones de forma y sus derivadas
  double (*h)(int, double *);
  double (*dhdr)(int, int, double *);
  int (*point_in_element)(element_t *, const double *);
  double (*element_volume)(element_t *);
  
  gauss_t gauss[2];    // juegos de puntos puntos de gauss
                       // 0 - full integration
                       // 1 - reduced integration
};


struct physical_entity_t {
  char *name;
  int tag;
  int dimension;
  
  material_t *material;    // apuntador
  bc_t *bcs;                // linked list 
  
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
  
  // o area o longitud segun dimension, a veces se llama masa
  double volume;
  // centro de gravedad
  double cog[3];
  
  var_t *var_vol;
  vector_t *vector_cog;
  
  // una linked list es muy cara
  int n_elements;
  int i_element;
  int *element;
  
  UT_hash_handle hh;
  UT_hash_handle hh_tag[4];
};

struct geometrical_entity_t {
  int tag;
  double boxMinX, boxMinY, boxMinZ, boxMaxX, boxMaxY, boxMaxZ;
  int num_physicals;
  int *physical;
  int num_bounding;
  int *bounding;

  UT_hash_handle hh[4];
};

struct elementary_entity_t {
  int id;
  elementary_entity_t *next;
};

struct bc_t {
  char *string;
  
  // these are ints and not enums because from wasora we cannot know what they are going to be
  int type_math;
  int type_phys;  
  int dof;   // this can have high values with meanings (i.e. dof=213 can be Mx)

  expr_t condition;  // if it is not null the BC only applies if it is != 0
  expr_t *expr;
  physical_entity_t *slave;  // TODO: change to non-racist name
  
  bc_t *next;
};


struct element_t {
  int index;
  int tag;
  
  double quality;
  double volume;
  double weight;              // this weight is used to average the contribution of this element to nodal gradients
  double *w;                  // weights of the gauss points
  double **x;                 // coordinates fo the gauss points 
  
  // these matrices are evalauted at the gauss points
  gsl_matrix **dhdx;
  gsl_matrix **dxdr;
  gsl_matrix **drdx;
  gsl_matrix **H;
  gsl_matrix **B;
  
  // these are the number of gauss points currently being used
  // if this number changes, everything needs to be re-computed
  // for instance sub-integration might be used for building matrices
  // but the canonical set of gauss points might be needed to recover stresses  
  // we need one size for each of the seven objects above because we need
  // to change them individually otherwise the first wins and the others loose
  int V_w, V_x, V_H, V_B, V_dxdr, V_drdx, V_dhdx;    

  
  int *l;  // node-major-orderer vector with the global indexes of the DOFs in the element

  
  gsl_matrix **dphidx_gauss;  // spatial derivatives of the DOFs at the gauss points
  gsl_matrix **dphidx_node;   // spatial derivatives of the DOFs at the nodes (extrapoladed or evaluated)
  double **property_node;

  element_type_t *type;                      // pointer to the element type
  physical_entity_t *physical_entity;        // pointer to the physical entity
  node_t **node;                             // pointer to the nodes
  cell_t *cell;                              // pointet to the associated cell (only for FVM)
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
  enum  {
    mesh_format_fromextension,
    mesh_format_gmsh,
    mesh_format_vtk,
    mesh_format_frd,
  } format;
  
  int spatial_dimensions;        // dimensiones espaciales
  int bulk_dimensions;           // dimension del mayor elemento

  int n_nodes;                   // cantidad de nodos
  int n_elements;                // cantidad de elementos (fem)
  int n_cells;                   // cantidad de celdas    (fvm)

  int degrees_of_freedom;        // grados de libertad por incognita
  int order;                     // mayor orden de algun elemento

  physical_entity_t *physical_entities;              // hash table
  physical_entity_t *physical_entities_by_tag[4];    // 4 hash tables por tag
  int physical_tag_max;          // el mayor tag de las entities
  
  // number of geometric entities of each dimension
  int points, curves, surfaces, volumes;
  geometrical_entity_t *geometrical_entities[4];     // 4 hash tables, one for each dimension

  int sparse;         // flag that indicates if the nodes are sparse
  int *tag2index;     // array to map tags to indexes
  
  
  enum  {
    ordering_node_major,
    ordering_dof_major,
  } ordering;
  
  enum {
      data_type_element,
      data_type_node,
  } data_type;
  
  enum {
    integration_full,
    integration_reduced
  } integration;
  
  int re_read;
  
  int structured;                 // flag que indica si la tenemos que fabricar nosotros
  
  expr_t *scale_factor;           // factor de escala al leer la posicion de los nodos
  expr_t *offset_x;               // offset en nodos
  expr_t *offset_y;               // offset en nodos
  expr_t *offset_z;               // offset en nodos
    
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

  // cache for interpolation
  element_t *last_chosen_element;
  
  physical_entity_t *origin;
  physical_entity_t *left;
  physical_entity_t *right;
  physical_entity_t *front;
  physical_entity_t *rear;
  physical_entity_t *bottom;
  physical_entity_t *top;    
//  physical_entity_t *bulk;
  
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

  int no_physical_names;
  centering_t centering;
  
  int (*write_header)(FILE *);
  int (*write_mesh)(mesh_t *, int, FILE *);
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

struct mesh_find_minmax_t {
  mesh_t *mesh;
  physical_entity_t *physical_entity;
  function_t *function;
  expr_t expr;
  centering_t centering;
  
  var_t *min;
  var_t *i_min;
  var_t *x_min;
  var_t *y_min;
  var_t *z_min;

  var_t *max;
  var_t *i_max;
  var_t *x_max;
  var_t *y_max;
  var_t *z_max;
      
  mesh_find_minmax_t *next;
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
extern element_t *mesh_find_element(mesh_t *, node_t *, const double *);
extern node_t *mesh_find_nearest_node(mesh_t *, const double *);
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
extern int mesh_create_element(element_t *, int, int, int, physical_entity_t *);
extern int mesh_add_element_to_list(element_list_item_t **, element_t *);
extern int mesh_add_material_to_list(material_list_item_t **, material_t *);
extern int mesh_compute_element_barycenter(element_t *, double []);
extern int mesh_node_indexes(mesh_t *, int);
extern int mesh_compute_local_node_index(element_t *, int);

// gauss.c
extern int mesh_init_gauss_points(mesh_t *, int);
extern int mesh_init_gauss_weights(int, int, double **, double **);
extern double mesh_integral_over_element(mesh_t *, function_t *, element_t *, expr_t *);
extern double mesh_integration_weight(mesh_t *, element_t *, int);
extern void mesh_init_fem_objects(mesh_t *mesh);

// gmsh.c
extern int mesh_gmsh_readmesh(mesh_t *);
extern int mesh_gmsh_write_header(FILE *);
extern int mesh_gmsh_write_mesh(mesh_t *, int, FILE *);
extern int mesh_gmsh_write_scalar(mesh_post_t *, function_t *, centering_t);
extern int mesh_gmsh_write_vector(mesh_post_t *, function_t **, centering_t);
extern int mesh_gmsh_update_function(function_t *, double, double);

// frd.c
extern int mesh_frd_readmesh(mesh_t *);

// vtk.c
extern int mesh_vtk_write_header(FILE *);
extern int mesh_vtk_write_mesh(mesh_t *, int, FILE *);
extern int mesh_vtk_write_structured_mesh(mesh_t *, FILE *);
extern int mesh_vtk_write_unstructured_mesh(mesh_t *, FILE *);
extern int mesh_vtk_write_scalar(mesh_post_t *, function_t *, centering_t);
extern int mesh_vtk_write_vector(mesh_post_t *, function_t **, centering_t);
extern int mesh_vtk_readmesh(mesh_t *);

// init.c
extern int wasora_mesh_init_before_parser(void);
extern void wasora_mesh_add_node_parent(node_relative_t **, int);
extern void wasora_mesh_compute_coords_from_parent(element_type_t *, int);

// interpolate.c
extern double mesh_interpolate_function_node(function_t *, const double *);
extern double mesh_interpolate_function_cell(function_t *, const double *);
extern double mesh_interpolate_function_property(function_t *, const double *);
extern int mesh_interp_residual(const gsl_vector *, void *, gsl_vector *);
extern int mesh_interp_jacob(const gsl_vector *, void *, gsl_matrix *);
extern int mesh_interp_residual_jacob(const gsl_vector *, void *, gsl_vector *, gsl_matrix *);
extern int mesh_interp_solve_for_r(element_t *, const double *, double *);
extern int mesh_compute_r_tetrahedron(element_t *, const double *, double *);



// fem.c
extern int mesh_compute_r_at_node(element_t *, int, gsl_vector *);
extern void mesh_compute_l(mesh_t *, element_t *);
extern int mesh_compute_normal(element_t *);
extern int mesh_update_coord_vars(double *);

extern int mesh_inverse(gsl_matrix *, gsl_matrix *);
extern double mesh_determinant(gsl_matrix *);

extern void mesh_compute_dxdr(element_t *, double *, gsl_matrix *);
extern void mesh_compute_dhdx(element_t *, double *, gsl_matrix *, gsl_matrix *);

extern void mesh_compute_dhdx_at_gauss(element_t *, int, int);
extern void mesh_compute_drdx_at_gauss(element_t *, int, int);
extern void mesh_compute_dxdr_at_gauss(element_t *, int, int);
extern void mesh_compute_integration_weight_at_gauss(element_t *, int, int);
extern void mesh_compute_H_at_gauss(element_t *, int, int, int);
extern void mesh_compute_B_at_gauss(element_t *, int, int, int);
extern void mesh_compute_x_at_gauss(element_t *, int, int);


extern int mesh_compute_B(mesh_t *, element_t *);
extern int mesh_compute_H(mesh_t *, element_t *);

// neighbors.c
extern int mesh_count_common_nodes(element_t *, element_t *, int *);
extern int mesh_find_neighbors(mesh_t *);
extern int mesh_fill_neighbors(mesh_t *);
extern element_t *mesh_find_element_volumetric_neighbor(element_t *);
extern element_t *mesh_find_node_neighbor_of_dim(node_t *, int);


// parser.c
extern int wasora_mesh_parse_line(char *);

// quality.c
extern int mesh_compute_quality(mesh_t *, element_t *);

// struct.c
extern int mesh_create_structured(mesh_t *);
extern void wasora_mesh_struct_init_rectangular_for_cells(mesh_t *);
extern void wasora_mesh_struct_init_rectangular_for_nodes(mesh_t *);
extern int wasora_mesh_struct_find_cell(int, double *, double *, double);

// point.c
extern int mesh_one_node_point_init(void);
extern double mesh_one_node_point_h(int, double *);
extern double mesh_one_node_point_dhdr(int, int, double *);
extern double mesh_point_vol(element_t *);


// line.c
extern int mesh_line2_init(void);
extern double mesh_line2_h(int, double *);
extern double mesh_line2_dhdr(int, int, double *);

extern int mesh_point_in_line(element_t *, const double *);
extern double mesh_line_vol(element_t *);

extern void mesh_gauss_init_line1(element_type_t *, gauss_t *);
extern void mesh_gauss_init_line2(element_type_t *, gauss_t *);
extern void mesh_gauss_init_line3(element_type_t *, gauss_t *);

// line3.c
extern int mesh_line3_init(void);
extern double mesh_line3_h(int, double *);
extern double mesh_line3_dhdr(int, int, double *);

// triang3.c
extern int mesh_triang3_init(void);
extern double mesh_triang3_h(int, double *);
extern double mesh_triang3_dhdr(int, int, double *);
extern int mesh_point_in_triangle(element_t *, const double *);
extern double mesh_triang_vol(element_t *);

extern void mesh_gauss_init_triang1(element_type_t *, gauss_t *);
extern void mesh_gauss_init_triang3(element_type_t *, gauss_t *);


// triang6.c
extern int mesh_triang6_init(void);
extern double mesh_triang6_h(int, double *);
extern double mesh_triang6_dhdr(int, int, double *);

// quad4.c
extern int mesh_quad4_init(void);
extern double mesh_quad4_h(int, double *);
extern double mesh_quad4_dhdr(int, int, double *);

extern void mesh_gauss_init_quad1(element_type_t *, gauss_t *);
extern void mesh_gauss_init_quad4(element_type_t *, gauss_t *);
extern void mesh_gauss_init_quad9(element_type_t *, gauss_t *);

extern int mesh_point_in_quadrangle(element_t *, const double *);
extern double mesh_quad_vol(element_t *);

// quad8.c
extern int mesh_quad8_init(void);
extern double mesh_quad8_h(int , double *);
extern double mesh_quad8_dhdr(int , int , double *);

// quad9.c
extern int mesh_quad9_init(void);
extern double mesh_quad9_h(int , double *);
extern double mesh_quad9_dhdr(int , int , double *);

// hexahedron8.c
extern int mesh_hexa8_init(void);
extern double mesh_hexa8_h(int, double *);
extern double mesh_hexa8_dhdr(int, int, double *);

extern void mesh_gauss_init_hexa1(element_type_t *, gauss_t *);
extern void mesh_gauss_init_hexa8(element_type_t *, gauss_t *);
extern void mesh_gauss_init_hexa27(element_type_t *, gauss_t *);

extern int mesh_point_in_hexahedron(element_t *, const double *);
extern double mesh_hexahedron_vol(element_t *);

// hexahedron20.c
extern int mesh_hexa20_init(void);
extern double mesh_hexa20_h(int, double *);
extern double mesh_hexa20_dhdr(int, int, double *);

// hexahedron27.c
extern int mesh_hexa27_init(void);
extern double mesh_hexa27_h(int, double *);
extern double mesh_hexa27_dhdr(int, int, double *);

// tet4.c
extern int mesh_tet4_init(void);
extern double mesh_tet4_h(int, double *);
extern double mesh_tet4_dhdr(int, int, double *);
extern int mesh_point_in_tetrahedron(element_t *, const double *);
extern double mesh_tetrahedron_vol(element_t *);

extern void mesh_gauss_init_tet1(element_type_t *, gauss_t *);
extern void mesh_gauss_init_tet4(element_type_t *, gauss_t *);

// tet10.c
extern int mesh_tet10_init(void);
extern double mesh_tet10_h(int, double *);
extern double mesh_tet10_dhdr(int, int, double *);


// prism6.c
extern int mesh_prism6_init(void);
extern void mesh_prism_gauss6_init(element_type_t *);
extern double mesh_prism6_h(int, double *);
extern double mesh_prism6_dhdr(int, int, double *);
extern int mesh_point_in_prism(element_t *, const double *);
extern double mesh_prism_vol(element_t *);

// prism6.c
extern int mesh_prism15_init(void);
extern double mesh_prism15_h(int, double *);
extern double mesh_prism15_dhdr(int, int, double *);

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
extern int mesh_compute_outward_normal(element_t *, double *);

extern void mesh_set_xyz(double *, struct var_t *, struct var_t *, struct var_t *);


extern mesh_t *wasora_define_mesh(char *, file_t *, int, int, int, int, int, expr_t *, expr_t *, expr_t *, expr_t *, expr_t *);
extern material_t *wasora_define_material(const char *);
extern physical_entity_t *wasora_define_physical_entity(char *, mesh_t *, int);
extern physical_property_t *wasora_define_physical_property(const char *, mesh_t *);
extern property_data_t *wasora_define_property_data(const char *, const char *, const char *);

extern material_t  *wasora_get_material_ptr(const char *);
extern physical_entity_t *wasora_get_physical_entity_ptr(const char *, mesh_t *);





#endif  /* _WASORA_H_ */

