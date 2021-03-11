/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora parser
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
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <unistd.h>
#include <dlfcn.h>

#ifndef _WASORA_H_
#include "wasora.h"
#endif
#include "builtin.h"

typedef struct varlist_t varlist_t;
struct varlist_t {
  var_t *var;
  varlist_t *next;
};


// parsea el wasora_input file principal, haciendo llamadas a parse_wasora_input_file()
// que a su vez se puede llamar recursivamente por los INCLUDEs
int wasora_parse_main_input_file(char *filepath) {

  char *tmpfile = NULL;
  
  // miramos el tamanio de la pagina para usar un buffer de tamanio razonable
  // si despues necesitamos mas vamos pidiendo a medida que nos quedamos cortos,
  // pero no gastamos un fangote de memoria al pedo
  wasora.page_size = (size_t)sysconf(_SC_PAGESIZE);
  wasora.actual_buffer_size = wasora.page_size-64;
  wasora.line = malloc(wasora.actual_buffer_size);

#ifndef __MINGW32__
  // si nos pasaron un guion, leemos stdin (como hacemos doble pasada mandamos todo a un tmp)
  if (strcmp(filepath, "-") == 0) {
    void *buffer;
    int c;
    int tmpfd;
    
    // template must not be a string constant, but should be declared as a character array
    tmpfile = malloc(32);
    strcpy(tmpfile, "wasoraXXXXXX");
    buffer = malloc(wasora.page_size);
    
    if ((tmpfd = mkstemp(tmpfile)) == -1) {
      wasora_push_error_message("cannot create temporary file '%s' to process stdin", tmpfile);
      return WASORA_PARSER_ERROR;
    }
    while ((c = read(0, buffer, wasora.page_size)) > 0) {
      if (write(tmpfd, buffer, c) == -1) {
        wasora_push_error_message("error '%s' when writing temporary file '%s'", strerror(errno), tmpfile);
        return WASORA_PARSER_ERROR;
      }
    }
    if (c == -1) {
      wasora_push_error_message("error '%s' when writing temporary file '%s'", strerror(errno), tmpfile);
      return WASORA_PARSER_ERROR;
    }
    close(tmpfd);
    free(buffer);
    filepath = tmpfile;
  }
#endif
  
  // parseamos el wasora_input buscando plugins
  wasora_call(wasora_parse_first_pass(filepath, 0, 0));

  // inicializamos
  wasora_call(wasora_init_before_parser());

  // parseamos el wasora_input principal (que a su vez puede volver a
  // llamarse recursivamente usando INCLUDEs)
  wasora_call(wasora_parse_input_file(filepath, 0, 0));

  // inicializamos cosas que se tienen que inicializar despues de haber leido el wasora_input
  wasora_call(wasora_init_after_parser());

  free(wasora.line);
  wasora.line = NULL;
  
  // limpiamos nuestro propio pastis
  if (tmpfile != NULL) {
    unlink(tmpfile);
    free(tmpfile);
  }

  return WASORA_PARSER_OK;

}


int wasora_parse_first_pass(const char *filepath, int from, int to) {
  
  char *line;
  char *token;

  int line_num, delta_line_num;
  
  FILE *wasora_input_file;

  if (filepath == NULL) {
    return WASORA_PARSER_OK;
  }
  
  if ((wasora_input_file = wasora_fopen(filepath, "r")) == NULL) {
    wasora_push_error_message("%s file '%s' could not be opened", (strcmp(filepath, wasora.argv[wasora.optind]) == 0)?"main input":"included", filepath);
    return WASORA_PARSER_ERROR;
  }

  line_num = 0;
  while ((delta_line_num = wasora_read_line(wasora_input_file)) != 0) {
    // delta_line_num negativo indica que la linea pide un argumento $n no provisto
    // eso puede pasar en una linea ignorada por un include from/to asi que no es
    // un error catastrofico
    line_num += abs(delta_line_num);

    if (wasora.line[0] != '\0' && ((from == 0 || line_num >= from) && ((to == 0) || line_num <= to))) {
      // a menos que nos metamos aca, que quiere decir que esa linea no se ignora asi
      // que ahora si a comerla
      if (delta_line_num < 0) {
        wasora_push_error_message("input file needs at least one extra argument in commandline");
        wasora_push_error_message("%s: %d:", filepath, line_num);
        return WASORA_PARSER_ERROR;
      }
      
      line = strdup(wasora.line);
      
      if ((token = wasora_get_next_token(line)) != NULL) {
        if (strcasecmp(token, "LOAD_PLUGIN") == 0) {
      
          if ((token = wasora_get_next_token(NULL)) == NULL) {
            wasora_push_error_message("expected shared library name or path");
            wasora_push_error_message("%s: %d:", filepath, line_num);
            return WASORA_PARSER_ERROR;
          }
          
          if (wasora_load_plugin(token) != WASORA_RUNTIME_OK) {
            return WASORA_PARSER_ERROR;
          }
          
        // ----- DEFAULT_ARGUMENT_VALUE -----------------------------------------------------------
        } else if (strcasecmp(token, "DEFAULT_ARGUMENT_VALUE") == 0) {

          int n;
      
          if ((token = wasora_get_next_token(NULL)) == NULL) {
            wasora_push_error_message("expected argument number");
            wasora_push_error_message("%s: %d:", filepath, line_num);
            return WASORA_PARSER_ERROR;
          }
      
          if ((n = (int)wasora_evaluate_expression_in_string(token)) <= 0) {
            wasora_push_error_message("expected a positive value instead of '%d'", n);
            wasora_push_error_message("%s: %d:", filepath, line_num);
            return WASORA_PARSER_ERROR;
          }

          if ((token = wasora_get_next_token(NULL)) == NULL) {
            wasora_push_error_message("expected value");
            wasora_push_error_message("%s: %d:", filepath, line_num);
            return WASORA_PARSER_ERROR;
          }
          
          if (wasora.optind+n >= wasora.argc) {
            // solo si no nos lo dieron!
            wasora.argv = realloc(wasora.argv, sizeof(char *)*(wasora.optind+n + 1));
            wasora.argc = wasora.optind+n + 1;
            wasora.argv[wasora.optind+n] = strdup(token);
          }
          
        // ----- INCLUDE -----------------------------------------------------------
        } else if (strcasecmp(token, "INCLUDE") == 0) {

          double xi;
          char *filepath;
          int from = 0;
          int to = 0;

          if ((filepath = wasora_get_next_token(NULL)) == NULL) {
            wasora_push_error_message("expected file path");
            wasora_push_error_message("%s: %d:", filepath, line_num);
            return WASORA_PARSER_ERROR;
          }

          while ((token = wasora_get_next_token(NULL)) != NULL) {
            if (strcasecmp(token, "FROM") == 0) {
              wasora_call(wasora_parser_expression_in_string(&xi));
              if ((from = (int)(xi)) <= 0) {
                wasora_push_error_message("expected a positive line number");
                wasora_push_error_message("%s: %d:", filepath, line_num);
                return WASORA_PARSER_ERROR;
              }
            } else if (strcasecmp(token, "TO") == 0) {
              wasora_call(wasora_parser_expression_in_string(&xi));
              if ((to = (int)(xi)) <= 0) {
                wasora_push_error_message("expected a positive line number");
                wasora_push_error_message("%s: %d:", filepath, line_num);
                return WASORA_PARSER_ERROR;
              }
            }
          }

          if (wasora_parse_first_pass(filepath, from, to) == WASORA_PARSER_ERROR) {
            return WASORA_PARSER_ERROR;
          }
        }
      }
      free(line);
    }
  }
  
  fclose(wasora_input_file);
  
  return WASORA_PARSER_OK;
          
}

// parsea un archivo de texto (se puede llamar recursivamente por los includes)
int wasora_parse_input_file(char *filepath, int from, int to) {

  FILE *wasora_input_file;
  char *line;

  int line_num, delta_line_num;
  int i, n;
  int understood;

  if (filepath == NULL) {
    return WASORA_PARSER_OK;
  }

  if ((wasora_input_file = wasora_fopen(filepath, "r")) == NULL) {
    if (strcmp(filepath, wasora.argv[wasora.optind]) == 0) {
      wasora_push_error_message("input file '%s' could not be opened: %s", filepath, strerror(errno));
    } else {
      wasora_push_error_message("included file '%s' could not be opened: % ", filepath, strerror(errno));
    }
    return WASORA_PARSER_ERROR;
  }

  // parseamos linea por linea
  line_num = 0;
  while ((delta_line_num = wasora_read_line(wasora_input_file)) != 0) {
    // delta_line_num negativo indica que la linea pide un argumento $n no provisto
    // eso puede pasar en una linea ignorada por un include from/to asi que no es
    // un error catastrofico
    line_num += abs(delta_line_num);
    
    if (wasora.line[0] != '\0' && ((from == 0 || line_num >= from) && ((to == 0) || line_num <= to))) {
      // a menos que nos metamos aca, que quiere decir que esa linea no se ignora asi
      // que ahora si a comerla
      if (delta_line_num < 0) {
        wasora_push_error_message("input file needs at least an argument in commandline");
        return WASORA_PARSER_ERROR;
      }
      
      line = strdup(wasora.line);
      if ((n = wasora_parse_line(line)) == WASORA_PARSER_ERROR) {
        wasora_push_error_message("%s: %d:", filepath, line_num);
        free(line);
        return WASORA_PARSER_ERROR;
      } else if (n == WASORA_PARSER_UNHANDLED) {
        understood = 0;
        for (i = 0; i < wasora.i_plugin; i++) {
          // recuperamos la linea completa porque los plugins pueden romper line
          strcpy(line, wasora.line);
          if ((n = wasora.plugin[i].parse_line(line)) == WASORA_PARSER_ERROR) {
            wasora_push_error_message("%s: %d:", filepath, line_num);
            return WASORA_PARSER_ERROR;
          }
          if (n == WASORA_PARSER_OK) {
            understood = 1;
          }
        }
        if (understood == 0) {
          if (strlen(wasora.line) > 32) {
            wasora.line[30] = '.';
            wasora.line[31] = '.';
            wasora.line[32] = '.';
            wasora.line[33] = '\0';
          }
          
          wasora_push_error_message("unknown syntax '%s'", wasora.line);
          wasora_push_error_message("%s: %d:", filepath, line_num);
          return WASORA_PARSER_ERROR;
        }
      }
      wasora_pop_error_message();
      free(line);
    }
  }

  fclose(wasora_input_file);
  
  // everything apple
  return WASORA_PARSER_OK;

}




// wasora_parser
int wasora_parse_line(char *line) {

  int i, j;
  char *token;
  char *dummy;

  if ((token = wasora_get_next_token(line)) != NULL) {

// ---------------------------------------------------------------------
///kw+INCLUDE+usage INCLUDE
///kw+INCLUDE+desc Include another wasora input file.
///kw+INCLUDE+detail Includes the input file located in the string `file_path` at the current location.
///kw+INCLUDE+detail The effect is the same as copying and pasting the contents of the included file
///kw+INCLUDE+detail at the location of the `INCLUDE` keyword. The path can be relative or absolute.
///kw+INCLUDE+detail Note, however, that when including files inside `IF` blocks that instructions are
///kw+INCLUDE+detail conditionally-executed but all definitions (such as function definitions) are processed at
///kw+INCLUDE+detail parse-time independently from the evaluation of the conditional.
///kw+INCLUDE+example include1.was included2.was include2.was include3.was
    if ((strcasecmp(token, "INCLUDE") == 0)) {
      double xi;
      char *filepath;
      int from = 0;
      int to = 0;
    
///kw+INCLUDE+usage <file_path>
      if ((filepath = wasora_get_next_token(NULL)) == NULL) {
        wasora_push_error_message("expected file path");
        return WASORA_PARSER_ERROR;
      }

      while ((token = wasora_get_next_token(NULL)) != NULL) {
///kw+INCLUDE+usage [ FROM <num_expr> ]
///kw+INCLUDE+detail The optional `FROM` and `TO` keywords can be used to include only portions of a file.          
if (strcasecmp(token, "FROM") == 0) {
          wasora_call(wasora_parser_expression_in_string(&xi));
          if ((from = (int)(xi)) <= 0) {
            wasora_push_error_message("expected a positive line number");
            return WASORA_PARSER_ERROR;
          }
///kw+INCLUDE+usage [ TO <num_expr> ]
        } else if (strcasecmp(token, "TO") == 0) {
          wasora_call(wasora_parser_expression_in_string(&xi));
          if ((to = (int)(xi)) <= 0) {
            wasora_push_error_message("expected a positive line number");
            return WASORA_PARSER_ERROR;
          }
        }
      }

      if (wasora_parse_input_file(filepath, from, to) != 0) {
        return WASORA_PARSER_ERROR;
      }

      return WASORA_PARSER_OK;

// ---------------------------------------------------------------------
///kw+ABORT+usage ABORT
///kw+ABORT+desc Catastrophically abort the execution and quit wasora.
///kw+ABORT+detail Whenever the instruction `ABORT` is executed, wasora quits without closing files
///kw+ABORT+detail or unlocking shared memory objects. The objective of this instruction is, as
///kw+ABORT+detail illustrated in the examples, either to debug complex input files and check the
///kw+ABORT+detail values of certain variables or to conditionally abort the execution using `IF`
///kw+ABORT+detail clauses.
///kw+ABORT+example abort1.was abort2.was
    } else if (strcasecmp(token, "ABORT") == 0) {

      if (wasora_define_instruction(wasora_instruction_abort, NULL) == NULL) {
        return WASORA_PARSER_ERROR;
      }
      return WASORA_PARSER_OK;


// ---------------------------------------------------------------------
///kw+DEFAULT_ARGUMENT_VALUE+usage DEFAULT_ARGUMENT_VALUE
///kw+DEFAULT_ARGUMENT_VALUE+desc Give a default value for an optional commandline argument.
///kw+DEFAULT_ARGUMENT_VALUE+detail If a `$n` construction is found in the input file but the
///kw+DEFAULT_ARGUMENT_VALUE+detail commandline argument was not given, the default behavior is to
///kw+DEFAULT_ARGUMENT_VALUE+detail fail complaining that an extra argument has to be given in the
///kw+DEFAULT_ARGUMENT_VALUE+detail commandline. With this keyword, a default value can be assigned if
///kw+DEFAULT_ARGUMENT_VALUE+detail no argument is given, thus avoiding the failure and making the argument
///kw+DEFAULT_ARGUMENT_VALUE+detail optional.
    } else if (strcasecmp(token, "DEFAULT_ARGUMENT_VALUE") == 0) {

///kw+DEFAULT_ARGUMENT_VALUE+usage <constant> <string>
      
      return WASORA_PARSER_OK;
// ---------------------------------------------------------------------
///kw+IMPLICIT+usage IMPLICIT
///kw+IMPLICIT+desc Define whether implicit declaration of variables is allowed or not.
///kw+IMPLICIT+detail By default, wasora allows variables (but not vectors nor matrices) to be
///kw+IMPLICIT+detail implicitly declared. To avoid introducing errors due to typos, explicit
///kw+IMPLICIT+detail declaration of variables can be forced by giving `IMPLICIT NONE`.
///kw+IMPLICIT+detail Whether implicit declaration is allowed or explicit declaration is required
///kw+IMPLICIT+detail depends on the last `IMPLICIT` keyword given, which by default is `ALLOWED`.
///kw+IMPLICIT+example implicit.was
    } else if (strcasecmp(token, "IMPLICIT") == 0) {


///kw+IMPLICIT+usage { NONE | ALLOWED }
      char *keywords[] = {"NONE", "ALLOWED", ""};
      int values[] = {1, 0, 0};
      wasora_call(wasora_parser_keywords_ints(keywords, values, &wasora.implicit_none));

      return WASORA_PARSER_OK;
      
// ---------------------------------------------------------------------
///kw+DO_NOT_EVALUATE_AT_PARSE_TIME+usage DO_NOT_EVALUATE_AT_PARSE_TIME
///kw+DO_NOT_EVALUATE_AT_PARSE_TIME+desc Ask wasora not to evaluate assignments at parse time.
    } else if (strcasecmp(token, "DO_NOT_EVALUATE_AT_PARSE_TIME") == 0) {

      wasora.do_not_evaluate_assigns_at_parse_time = 1;
      return WASORA_PARSER_OK;
      
      

 // ---------------------------------------------------------------------
///kw+TIME_PATH+usage TIME_PATH
///kw+TIME_PATH+desc Force transient problems to pass through specific instants of time.
///kw+TIME_PATH+detail The time step `dt` will be reduced whenever the distance between
///kw+TIME_PATH+detail the current time `t` and the next expression in the list is greater
///kw+TIME_PATH+detail than `dt` so as to force `t` to coincide with the expressions given.
///kw+TIME_PATH+detail The list of expresssions should evaluate to a sorted list of values.          
    } else if ((strcasecmp(token, "TIME_PATH") == 0)) {

///kw+TIME_PATH+usage <expr_1> [ <expr_2>  [ ... <expr_n> ] ]
      i = 0;
      while ((token = wasora_get_next_token(NULL)) != NULL) {
        // el +2 es porque necesito +1 de por si y otro +1 para guardar el ultimo vacio
        wasora.time_path = realloc(wasora.time_path, (i+2)*sizeof(expr_t));
        wasora_call(wasora_parse_expression(token, &wasora.time_path[i]));
        i++;
      }

      // ponemos en vacio el ultimo
      wasora.time_path[i].n_tokens = 0;

      // y apuntamos el current al primero
      wasora.current_time_path = &wasora.time_path[0];

      return WASORA_PARSER_OK;

// ---------------------------------------------------------------------      
///kw+INITIAL_CONDITIONS_MODE+usage INITIAL_CONDITIONS_MODE
///kw+INITIAL_CONDITIONS_MODE+desc Define how initial conditions of DAE problems are computed.
    } else if ((strcasecmp(token, "INITIAL_CONDITIONS_MODE") == 0)) {

///kw+INITIAL_CONDITIONS_MODE+usage { AS_PROVIDED | FROM_VARIABLES | FROM_DERIVATIVES }
///kw+INITIAL_CONDITIONS_MODE+detail In DAE problems, initial conditions may be either:
///kw+INITIAL_CONDITIONS_MODE+detail @
///kw+INITIAL_CONDITIONS_MODE+detail  * equal to the provided expressions (`AS_PROVIDED`)@
///kw+INITIAL_CONDITIONS_MODE+detail  * the derivatives computed from the provided phase-space variables (`FROM_VARIABLES`)@
///kw+INITIAL_CONDITIONS_MODE+detail  * the phase-space variables computed from the provided derivatives (`FROM_DERIVATIVES`)@
///kw+INITIAL_CONDITIONS_MODE+detail @
///kw+INITIAL_CONDITIONS_MODE+detail In the first case, it is up to the user to fulfill the DAE system at\ $t = 0$.
///kw+INITIAL_CONDITIONS_MODE+detail If the residuals are not small enough, a convergence error will occur.
///kw+INITIAL_CONDITIONS_MODE+detail The `FROM_VARIABLES` option means calling IDA’s `IDACalcIC` routine with the parameter `IDA_YA_YDP_INIT`. 
///kw+INITIAL_CONDITIONS_MODE+detail The `FROM_DERIVATIVES` option means calling IDA’s `IDACalcIC` routine with the parameter IDA_Y_INIT.
///kw+INITIAL_CONDITIONS_MODE+detail Wasora should be able to automatically detect which variables in phase-space are differential and
///kw+INITIAL_CONDITIONS_MODE+detail which are purely algebraic. However, the `DIFFERENTIAL` keyword may be used to explicitly define them.
///kw+INITIAL_CONDITIONS_MODE+detail See the (SUNDIALS documentation)[https:/\/computation.llnl.gov/casc/sundials/documentation/ida_guide.pdf] for further information.
      char *keywords[] = {"AS_PROVIDED", "FROM_VARIABLES", "FROM_DERIVATIVES", ""};
      int values[] = {as_provided, from_variables, from_derivatives, 0};
      wasora_call(wasora_parser_keywords_ints(keywords, values, (int *)&wasora_dae.initial_conditions_mode));

      return WASORA_PARSER_OK;

// ---------------------------------------------------------------------      
///kw+LOAD_PLUGIN+usage LOAD_PLUGIN
///kw+LOAD_PLUGIN+desc Load a wasora plug-in from a dynamic shared object.
    } else if (strcasecmp(token, "LOAD_PLUGIN") == 0) {

///kw+LOAD_PLUGIN+usage { <file_path> | <plugin_name> }
///kw+LOAD_PLUGIN+detail A wasora plugin in the form of a dynamic shared object (i.e. `.so`) can be loaded
///kw+LOAD_PLUGIN+detail either with the `LOAD_PLUGIN` keyword or from the command line with the `-p` option.
///kw+LOAD_PLUGIN+detail Either a file path or a plugin name can be given. The following locations are tried:
///kw+LOAD_PLUGIN+detail @
///kw+LOAD_PLUGIN+detail  * the current directory `./`@
///kw+LOAD_PLUGIN+detail  * the parent directory `../`@
///kw+LOAD_PLUGIN+detail  * the user’s `LD_LIBRARY_PATH`@
///kw+LOAD_PLUGIN+detail  * the cache file `/etc/ld.so.cache`@
///kw+LOAD_PLUGIN+detail  * the directories `/lib`, `/usr/lib`, `/usr/local/lib`@
///kw+LOAD_PLUGIN+detail @
///kw+LOAD_PLUGIN+detail If a wasora plugin was compiled and installed following the `make install` procedure,
///kw+LOAD_PLUGIN+detail the plugin should be loaded by just passing the name to `LOAD_PLUGIN`.
        
      // ya hicimos lo que habia que hacer en otro lado
      return WASORA_PARSER_OK;

// ---------------------------------------------------------------------      
///kw+LOAD_ROUTINE+usage LOAD_ROUTINE
///kw+LOAD_ROUTINE+desc Load one or more routines from a dynamic shared object.
    } else if (strcasecmp(token, "LOAD_ROUTINE") == 0) {

      void *library;
      char *path;

///kw+LOAD_ROUTINE+usage <file_path>
      if (wasora_parser_string(&path) != WASORA_PARSER_OK) {
        return WASORA_PARSER_ERROR;
      }
      if ((library = wasora_dlopen(path)) == NULL) {
        wasora_push_error_message("'%s' when opening library '%s'", dlerror(), path);
        return WASORA_PARSER_ERROR;
      }

///kw+LOAD_ROUTINE+usage <routine_1> [ <routine_2> ... <routine_n> ]
      while ((token = wasora_get_next_token(NULL)) != NULL) {
        if (wasora_define_loadable_routine(token, library) == NULL) {
          wasora_push_error_message("from library '%s'", path);
          return WASORA_PARSER_ERROR;
        }
      }

      free(path);

      return WASORA_PARSER_OK;

// ---------------------------------------------------------------------      
///kw+VAR+usage VAR
///kw+VAR+desc Define one or more scalar variables.
    } else if ((strcasecmp(token, "VAR") == 0)) {

///kw+VAR+usage <name_1> [ <name_2> ] ... [ <name_n> ]
      // los nombres de las variables uno atras de otro como punalada de manco
      while ((token = wasora_get_next_token(NULL)) != NULL) {
        if (wasora_define_variable(token) == NULL) {
          return WASORA_PARSER_ERROR;
        }
      }

      return WASORA_PARSER_OK;

// ---------------------------------------------------------------------      
///kw+CONST+usage CONST
///kw+CONST+desc Mark a scalar variable, vector or matrix as a constant.
    } else if (strcasecmp(token, "CONST") == 0) {

      var_t *var;
      vector_t *vector;
      matrix_t *matrix;

///kw+CONST+usage name_1 [ <name_2> ] ... [ <name_n> ]
      // los nombres de las variables uno atras de otro como punalada de manco
      while ((token = wasora_get_next_token(NULL)) != NULL) {

        // puede ser que sea una variable que ya este definida
        // antes y quieren que sea constante, asi que miramos si ya existe
        if ((var = wasora_get_variable_ptr(token)) != NULL) {
          var->constant = 1;
        } else if ((vector = wasora_get_vector_ptr(token)) != NULL) {
          vector->constant = 1;
        } else if ((matrix = wasora_get_matrix_ptr(token)) != NULL) {
          matrix->constant = 1;
        } else {
          // definimos una nueva variable
          if ((var = wasora_define_variable(token)) == NULL) {
            return WASORA_PARSER_ERROR;
          }
          var->constant = 1;
        }
      }

      return WASORA_PARSER_OK;

// ---------------------------------------------------------------------      
///kw+ALIAS+usage ALIAS
///kw+ALIAS+desc Define a scalar alias of an already-defined indentifier.
///kw+ALIAS+detail The existing object can be a variable, a vector element or a matrix element. 
///kw+ALIAS+detail In the first case, the name of the variable should be given as the existing object.
///kw+ALIAS+detail In the second case, to alias the second element of vector `v` to the new name `new`, `v(2)` should be given as the existing object.
///kw+ALIAS+detail In the third case, to alias second element (2,3) of matrix `M` to the new name `new`, `M(2,3)` should be given as the existing object.
      
    } else if ((strcasecmp(token, "ALIAS") == 0)) {

      char *dummy_par;
      char *left;
      char *keyword;
      char *right;
      char *existing_object;
      char *new_name;
      
      alias_t *alias = calloc(1, sizeof(alias_t));

///kw+ALIAS+usage { <new_var_name> IS <existing_object> | <existing_object> AS <new_name> }
      wasora_call(wasora_parser_string(&left));
      wasora_call(wasora_parser_string(&keyword));
      wasora_call(wasora_parser_string(&right));

      if (strcasecmp(keyword, "IS") == 0) {
        new_name = left;
        existing_object = right;
      } else if (strcasecmp(keyword, "AS") == 0) {
        new_name = right;
        existing_object = left;
      } else {
        wasora_push_error_message("either IS or AS expected instead of '%s'", keyword);
        return WASORA_PARSER_ERROR;
      }

      if ((dummy_par = strchr(existing_object, '(')) != NULL) {
        *dummy_par = '\0';
      }

      if ((alias->matrix = wasora_get_matrix_ptr(existing_object)) == NULL) {
        if ((alias->vector = wasora_get_vector_ptr(existing_object)) == NULL) {
          if ((alias->variable = wasora_get_variable_ptr(existing_object)) == NULL) {
           wasora_push_error_message("unknown allegedly existing object '%s'", existing_object);
           return WASORA_PARSER_ERROR;
          }
        }
      }

      // si hay parentesis son subindices
      if (dummy_par != NULL) {
        if (alias->matrix == NULL && alias->vector == NULL) {
          wasora_push_error_message("symbol '%s' cannot be subscripted", existing_object);
          return WASORA_PARSER_ERROR;
        }
        *dummy_par = '(';

        if (alias->matrix != NULL) {
          if (wasora_parse_range(dummy_par, '(', ',', ')', &alias->row, &alias->col) != 0) {
            return WASORA_PARSER_ERROR;
          }
        } else if (alias->vector != NULL) {
          if ((dummy = strrchr(dummy_par, ')')) == NULL) {
            wasora_push_error_message("unmatched parenthesis for '%s'", token);
            return WASORA_PARSER_ERROR;
          }
          *dummy = '\0';
          wasora_call(wasora_parse_expression(dummy_par+1, &alias->row));
          *dummy = ')';
        }
      }
      
      if ((alias->new_variable = wasora_define_variable(new_name)) == NULL) {
        return WASORA_PARSER_ERROR;
      }
      LL_APPEND(wasora.aliases, alias);

      if (wasora_define_instruction(wasora_instruction_alias, alias) == NULL) {
        return WASORA_PARSER_ERROR;
      }
      
      return WASORA_PARSER_OK;

// ---------------------------------------------------------------------
///kw+VECTOR+usage VECTOR
///kw+VECTOR+desc Define a vector.
    } else if ((strcasecmp(token, "VECTOR") == 0)) {

      vector_t *vector = NULL;
      char *vectorname = NULL;
      function_t *function_data = NULL;
      function_t *function_arg = NULL;
      expr_t *size_expr = calloc(1, sizeof(expr_t));
      expr_t *datas = NULL;
      int vectorsize = 0;
      int function_n_arg;

///kw+VECTOR+usage <name>
      if (wasora_parser_string(&vectorname) != WASORA_PARSER_OK) {
        return WASORA_PARSER_ERROR;
      }

      while ((token = wasora_get_next_token(NULL)) != NULL) {

///kw+VECTOR+usage SIZE <expr>
        if (strcasecmp(token, "SIZE") == 0) {
          wasora_call(wasora_parser_expression(size_expr));

///kw+VECTOR+usage [ DATA <expr_1> <expr_2> ... <expr_n> |
        } else if (strcasecmp(token, "DATA") == 0) {
                      
          while ((token = wasora_get_next_token(NULL)) != NULL) {
            expr_t *data = calloc(1, sizeof(expr_t));
            wasora_call(wasora_parse_expression(token, data));
            LL_APPEND(datas, data);
          }

///kw+VECTOR+usage FUNCTION_DATA <function> |
        } else if (strcasecmp(token, "FUNCTION_DATA") == 0) {
          
          if ((token = wasora_get_next_token(NULL)) == NULL) {
            wasora_push_error_message("expected function name");
            return WASORA_PARSER_ERROR;
          }

          if ((function_data = wasora_get_function_ptr(token)) == NULL) {
            wasora_push_error_message("unknown function '%s'", token);
            return WASORA_PARSER_ERROR;
          }
          
///kw+VECTOR+usage FUNCTION_ARG <num_arg> <function> ]
        } else if (strcasecmp(token, "FUNCTION_ARG") == 0 || strcasecmp(token, "FUNCTION_ARGUMENT") == 0) {

          if ((token = wasora_get_next_token(NULL)) == NULL) {
            wasora_push_error_message("expected argument number");
            return WASORA_PARSER_ERROR;
          }
          function_n_arg = (int)wasora_evaluate_expression_in_string(token);
          if (function_n_arg <= 0) {
            wasora_push_error_message("argument number should be positive");
            return WASORA_PARSER_ERROR;
          }
          
          if ((token = wasora_get_next_token(NULL)) == NULL) {
            wasora_push_error_message("expected function name");
            return WASORA_PARSER_ERROR;
          }

          if ((function_arg = wasora_get_function_ptr(token)) == NULL) {
            wasora_push_error_message("unknown function '%s'", token);
            return WASORA_PARSER_ERROR;
          }
          
        }
      }

      // listoooo
      if ((vector = wasora_define_vector(vectorname, vectorsize, size_expr, datas)) == NULL) {
        return WASORA_PARSER_ERROR;
      }

      // TODO: ya demasiado raro es el API para wasora_define_vector, hacemos esto por afuer
      if (function_data != NULL) {
        vector->function_data= function_data;
      }
      if (function_arg != NULL) {
        vector->function_arg= function_arg;
      }
      if (function_n_arg != 0) {
        vector->function_n_arg = function_n_arg;
      }
      
      free(vectorname);
      return WASORA_PARSER_OK;

// ---------------------------------------------------------------------
///kw+VECTOR_SORT+usage VECTOR_SORT
///kw+VECTOR_SORT+desc Sort the elements of a vector using a specific numerical order,
///kw+VECTOR_SORT+desc potentially making the same rearrangement of another vector.
    } else if ((strcasecmp(token, "VECTOR_SORT") == 0)) {
      
      vector_sort_t *vector_sort = calloc(1, sizeof(vector_sort_t));

///kw+VECTOR_SORT+usage <vector>
      wasora_call(wasora_parser_vector(&vector_sort->v1));
      
      while ((token = wasora_get_next_token(NULL)) != NULL) {
        
///kw+VECTOR_SORT+usage [ ASCENDING_ORDER | DESCENDING_ORDER ]
        if (strcasecmp(token, "ASCENDING_ORDER") == 0)
          vector_sort->descending = 0;
        else if (strcasecmp(token, "DESCENDING_ORDER") == 0)
          vector_sort->descending = 1;
        
///kw+VECTOR_SORT+usage [ <vector> ]
        else if ((vector_sort->v2 = wasora_get_vector_ptr(token)) != NULL) {
          continue;
        } else {
          wasora_push_error_message("unknown keyword or vector identifier '%s'", token);
          return WASORA_PARSER_ERROR;
        }
      }
      
      wasora_define_instruction(wasora_instruction_vector_sort, vector_sort);
      
      return WASORA_PARSER_OK;
      
// ---------------------------------------------------------------------
///kw+MATRIX+usage MATRIX
///kw+MATRIX+desc Define a matrix.
    } else if ((strcasecmp(token, "MATRIX") == 0)) {

      char *matrixname = NULL;
      expr_t *rows_expr = calloc(1, sizeof(expr_t));
      expr_t *cols_expr = calloc(1, sizeof(expr_t));
      expr_t *datas = NULL;
      int rows = 0;
      int cols = 0;

///kw+MATRIX+usage <name>
      if (wasora_parser_string(&matrixname) != WASORA_PARSER_OK) {
        return WASORA_PARSER_ERROR;
      }

      while ((token = wasora_get_next_token(NULL)) != NULL) {

///kw+MATRIX+usage ROWS <expr>
        if (strcasecmp(token, "ROWS") == 0) {
          wasora_call(wasora_parser_expression(rows_expr));
          
///kw+MATRIX+usage COLS <expr>
        } else if (strcasecmp(token, "COLS") == 0) {
          wasora_call(wasora_parser_expression(cols_expr));
        
///kw+MATRIX+usage [ DATA num_expr_1 num_expr_2 ... num_expr_n ]
        } else if (strcasecmp(token, "DATA") == 0) {

          while ((token = wasora_get_next_token(NULL)) != NULL) {
            expr_t *data = calloc(1, sizeof(expr_t));
            wasora_call(wasora_parse_expression(token, data));
            LL_APPEND(datas, data);
          }

        } else {
          wasora_push_error_message("unknown keyword '%s'", token);
          return WASORA_PARSER_ERROR;
        }
      }

      if (wasora_define_matrix(matrixname, rows, rows_expr, cols, cols_expr, datas) == NULL) {
        return WASORA_PARSER_ERROR;
      }
      
      free(matrixname);
      return WASORA_PARSER_OK;

// ---------------------------------------------------------------------
///kw+FUNCTION+usage FUNCTION
///kw+FUNCTION+desc Define a function of one or more variables.
    } else if (strcasecmp(token, "FUNCTION") == 0 || ((dummy = strchr(wasora.line, '=')) != NULL && *(dummy-1) == ':')) {

      function_t *function = NULL;
      char *argument = NULL;
      char *dummy_argument = NULL;
      char **arg_name = NULL;
      char *backup1 = NULL;
      char *backup2 = NULL;
      char char_backup = 0;
      int level, nargs;

///kw+FUNCTION+usage <name>(<var_1>[,var2,...,var_n])
///kw+FUNCTION+detail The number of variables $n$ is given by the number of arguments given between parenthesis after the function name.
///kw+FUNCTION+detail The arguments are defined as new variables if they had not been already defined as variables.
      // si nos dieron el keyword FUNCTION tenemos que hacer un wasora_get_next_token
      // para obtener el nombre de la funcion, sino ya esta en token
      if (strcasecmp(token, "FUNCTION") == 0) {
        if ((token = wasora_get_next_token(NULL)) == NULL) {
          wasora_push_error_message("expected function name");
          return WASORA_PARSER_ERROR;
        }
      }

      // parseamos los argumentos
      if ((dummy = strchr(token, '(')) == NULL) {
        wasora_push_error_message("expected opening parenthesis '(' after function name '%s' (no spaces allowed)", token);
        return WASORA_PARSER_ERROR;
      }

      argument = strdup(dummy);
      wasora_strip_blanks(argument);

      if ((nargs = wasora_count_arguments(argument)) <= 0) {
        return WASORA_PARSER_ERROR;
      }

      dummy = strchr(token, '(');
      *dummy = '\0';

      if ((function = wasora_define_function(token, nargs)) == NULL) {
        return WASORA_PARSER_ERROR;
      }

      arg_name = calloc(nargs, sizeof(char *));
      function->var_argument = calloc(nargs, sizeof(var_t *));
      function->var_argument_alloced = 1;
      function->column = calloc((nargs+1), sizeof(int));
      // por default ponemos en las columnas 1 2 3 4 ...
      for (i = 0; i < nargs+1; i++) {
        function->column[i] = i+1;
      }



      if (nargs == 1) {
        // si la funcion toma un solo argumento es facil
        arg_name[0] = strdup(argument+1);
        if ((dummy = strchr(arg_name[0], ')')) == NULL) {
          wasora_push_error_message("expected ')' after function name (no spaces allowed)");
          return WASORA_PARSER_ERROR;
        }
        *dummy = '\0';

      } else {

        // si la funcion toma mas argumentos tenemos que buscarlos de a uno

        // si incrementamos argument despues no podemos hacer free, asi
        // que tenemos un apuntador dummy para argument
        dummy_argument = argument;

        for (i = 0; i < nargs; i++) {
          if (i != nargs-1) {

            // si no estamos en el ultimo argumento, tenemos que
            // poner un ")\0" donde termina el argumento
            level = 1;
            dummy = dummy_argument+1;
            while (1) {
              if (*dummy == ',' && level == 1) {
                break;
              }
              if (*dummy == '(') {
                level++;
              } else if (*dummy == ')') {
                level--;
              } else if (*dummy == '\0') {
                wasora_push_error_message("when parsing arguments");
                return WASORA_PARSER_ERROR;
              }
              dummy++;
            }

            // ponemos un \0 despues de dummy, pero necesitamos despues
            // restaurar el caracter que habia ahi
            char_backup = *(dummy);
            *(dummy) = '\0';
          } else {

            // le sacamos el ) final
            dummy[strlen(dummy)-1] = '\0';

          }


          // en dummy_argument+1 tenemos el argumento i-esimo
          arg_name[i] = strdup(dummy_argument+1);


          if (i != nargs-1) {
            *(dummy) = char_backup;
            dummy_argument = dummy;
          }
        }
      }
      free(argument);


      // linkeamos los argumentos a las variables correctas
      for (i = 0; i < nargs; i++) {
        if ((function->var_argument[i] = wasora_get_or_define_variable_ptr(arg_name[i])) == NULL) {
          return WASORA_PARSER_ERROR;
        }
      }


      while ((token = wasora_get_next_token(NULL)) != NULL && strcasecmp(token, "DATA") != 0) {
///kw+FUNCTION+usage {
///kw+FUNCTION+usage [ = <expr> |
///kw+FUNCTION+detail If the function is given as an algebraic expression, the short-hand operator `:=` can be used.
///kw+FUNCTION+detail That is to say, `FUNCTION f(x) = x^2` is equivalent to `f(x) := x^2`.
        
        if (strcasecmp(token, "=") == 0 || strcasecmp(token, ":=") == 0) {

          function->type = type_algebraic;

          // todo lo que sigue al ':=' o al '=' deberia ser la expresion
          // no funciona el ()?:*/
//          dummy = token + (token[0] == ':')? 3 : 2;
          if (token[0] == ':') {
            dummy = token + 3;
          } else {
            dummy = token + 2;
          }

          wasora_call(wasora_parse_expression(dummy, &function->algebraic_expression));

          // terminamos con esta linea
          break;

///kw+FUNCTION+usage FILE_PATH <file_path> |
        } else if (strcasecmp(token, "FILE_PATH") == 0) {
///kw+FUNCTION+detail If a `FILE_PATH` is given, an ASCII file containing at least $n+1$ columns is expected.
///kw+FUNCTION+detail By default, the first $n$ columns are the values of the arguments and the last column is the value of the function at those points.
///kw+FUNCTION+detail The order of the columns can be changed with the keyword `COLUMNS`, which expects $n+1$ expressions corresponding to the column numbers.
          
          // TODO: poder pedir que se refresquen los datos en cada paso
          function->type = type_pointwise_file;
          wasora_parser_string(&function->data_file);

///kw+FUNCTION+usage ROUTINE <name> |
        } else if (strcasecmp(token, "ROUTINE") == 0) {
///kw+FUNCTION+detail A function of type `ROUTINE` calls an already-defined user-provided routine using the `CALL` keyword and passes the values of the variables in each required evaluation as a `double *` argument.

          function->type = type_routine;

          if ((token = wasora_get_next_token(NULL)) == NULL) {
            wasora_push_error_message("expected routine name");
            return WASORA_PARSER_ERROR;
          }

          if ((function->routine = wasora_get_routine_ptr(token)) == NULL) {
            wasora_push_error_message("undefined routine '%s'", token);
            return WASORA_PARSER_ERROR;
          }

///kw+FUNCTION+usage | MESH <name> { DATA <new_vector_name> | VECTOR <existing_vector_name> } { NODES | CELLS } |
///kw+FUNCTION+detail If `MESH` is given, the definition points are the nodes or the cells of the mesh.
///kw+FUNCTION+detail The function arguments should be $(x)$, $(x,y)$ or $(x,y,z)$ matching the dimension the mesh.
///kw+FUNCTION+detail If the keyword `DATA` is used, a new empty vector of the appropriate size is defined.
///kw+FUNCTION+detail The elements of this new vector can be assigned to the values of the function at the $i$-th node or cell.
///kw+FUNCTION+detail If the keyword `VECTOR` is used, the values of the dependent variable are taken to be the values of the already-existing vector.
///kw+FUNCTION+detail Note that this vector should have the size of the number of nodes or cells the mesh has, depending on whether `NODES` or `CELLS` is given.
          
        } else if (strcasecmp(token, "MESH") == 0) {

          if ((token = wasora_get_next_token(NULL)) == NULL) {
            wasora_push_error_message("expected mesh name");
            return WASORA_PARSER_ERROR;
          }

          if ((function->mesh = wasora_get_mesh_ptr(token)) == NULL) {
            wasora_push_error_message("undefined mesh '%s'", token);
            return WASORA_PARSER_ERROR;
          }
          
          if ((token = wasora_get_next_token(NULL)) == NULL) {
            wasora_push_error_message("expected keyword DATA or VECTOR");
            return WASORA_PARSER_ERROR;
          }

          if (strcasecmp(token, "DATA") == 0) {
            if ((token = wasora_get_next_token(NULL)) == NULL) {
              wasora_push_error_message("expected data name");
              return WASORA_PARSER_ERROR;
            }
            if ((function->vector_value = wasora_define_vector(token, 0, NULL, NULL)) == NULL) {
              return WASORA_PARSER_ERROR;
            }
          } else if (strcasecmp(token, "VECTOR") == 0) {
            if ((token = wasora_get_next_token(NULL)) == NULL) {
              wasora_push_error_message("expected vector name");
              return WASORA_PARSER_ERROR;
            }
            if ((function->vector_value = wasora_define_vector(token, 0, NULL, NULL)) == NULL) {
              return WASORA_PARSER_ERROR;
            }
          } else {
            wasora_push_error_message("expected keyword DATA or VECTOR instead of '%s'", token);
            return WASORA_PARSER_ERROR;
          }
          
          if ((token = wasora_get_next_token(NULL)) == NULL) {
            wasora_push_error_message("expected either NODES or CELLS");
            return WASORA_PARSER_ERROR;
          }
          if (strcasecmp(token, "NODES") == 0) {
            function->type = type_pointwise_mesh_node;
          } else if (strcasecmp(token, "CELLS") == 0) {
            function->type = type_pointwise_mesh_cell;
          } else {
            wasora_push_error_message("expected either NODES or CELLS instead of '%s'", token);
            return WASORA_PARSER_ERROR;
          }
          
          
          if (function->var_argument[0] != wasora_mesh.vars.x) {
            wasora_push_error_message("first argument of function '%s' should be 'x'", function->name);
            return WASORA_PARSER_ERROR;
          }
          if (function->n_arguments > 1 && function->var_argument[1] != wasora_mesh.vars.y) {
            wasora_push_error_message("second argument of function '%s' should be 'y'", function->name);
            return WASORA_PARSER_ERROR;
          }
          if (function->n_arguments > 2 && function->var_argument[2] != wasora_mesh.vars.z) {
            wasora_push_error_message("third argument of function '%s' should be 'z'", function->name);
            return WASORA_PARSER_ERROR;
          }
          if (function->n_arguments > 3) {
            wasora_push_error_message("function '%s' cannot be MESH and have more than 3 arguments", function->name);
            return WASORA_PARSER_ERROR;
          }
          
///kw+FUNCTION+usage [ VECTOR_DATA <vector_1> <vector_2> ... <vector_n> <vector_n+1> ]
///kw+FUNCTION+usage }
///kw+FUNCTION+detail If `VECTOR_DATA` is given, a set of $n+1$ vectors of the same size is expected.
///kw+FUNCTION+detail The first $n+1$ correspond to the arguments and the last one is the function value.
          
        } else if (strcasecmp(token, "VECTOR_DATA") == 0 || strcasecmp(token, "VECTORS") == 0) {

          function->type = type_pointwise_vector;
          function->vector_argument = calloc(nargs, sizeof(vector_t *));
          function->data_argument = calloc(nargs, sizeof(double *));
          function->data_size = 0;

          for (i = 0; i < nargs; i++) {
            if (wasora_parser_vector(&function->vector_argument[i]) != WASORA_PARSER_OK) {
              return WASORA_PARSER_ERROR;
            }
          }

          if (wasora_parser_vector(&function->vector_value) != WASORA_PARSER_OK) {
            return WASORA_PARSER_ERROR;
          }


///kw+FUNCTION+usage [COLUMNS <expr_1> <expr_2> ... <expr_n> <expr_n+1> ]
        } else if (strcasecmp(token, "COLUMNS") == 0) {

          if (nargs == 0) {
            wasora_push_error_message("number of arguments is equal to zero");
            return WASORA_PARSER_ERROR;
          }

          for (i = 0; i < nargs+1; i++) {
            if ((token = wasora_get_next_token(NULL)) == NULL) {
              wasora_push_error_message("expected %d columns specifications", nargs);
              return WASORA_PARSER_ERROR;
            }
            function->column[i] = (int)(wasora_evaluate_expression_in_string(token));

          }

///kw+FUNCTION+usage [ INTERPOLATION
///kw+FUNCTION+detail Interpolation schemes can be given for either one or multi-dimensional functions with `INTERPOLATION`.
        } else if (strcasecmp(token, "INTERPOLATION") == 0) {

          if ((token = wasora_get_next_token(NULL)) == NULL) {
            wasora_push_error_message("expected interpolation method");
            return WASORA_PARSER_ERROR;
          }
          
///kw+FUNCTION+detail Available schemes for $n=1$ are:
///kw+FUNCTION+detail @
///kw+FUNCTION+usage {
///kw+FUNCTION+usage linear |
///kw+FUNCTION+detail  * linear
          if (strcasecmp(token, "linear") == 0) {
            function->interp_type = *gsl_interp_linear;
///kw+FUNCTION+usage polynomial |
///kw+FUNCTION+detail  * polynomial, the grade is equal to the number of data minus one
          } else if (strcasecmp(token, "polynomial") == 0) {
            function->interp_type = *gsl_interp_polynomial;
///kw+FUNCTION+usage spline |
///kw+FUNCTION+detail  * spline, cubic (needs at least 3 points)
          } else if (strcasecmp(token, "spline") == 0 || strcasecmp(token, "cspline") == 0 || strcasecmp(token, "splines") == 0) {
            function->interp_type = *gsl_interp_cspline;
///kw+FUNCTION+usage spline_periodic |
///kw+FUNCTION+detail  * spline_periodic 
          } else if (strcasecmp(token, "spline_periodic") == 0 || strcasecmp(token, "cspline_periodic") == 0 || strcasecmp(token, "splines_periodic") == 0) {
            function->interp_type = *gsl_interp_cspline_periodic;
///kw+FUNCTION+usage akima |
///kw+FUNCTION+detail  * akima (needs at least 5 points)
          } else if (strcasecmp(token, "akima") == 0) {
            function->interp_type = *gsl_interp_akima;
///kw+FUNCTION+usage akima_periodic |
///kw+FUNCTION+detail  * akima_periodic (needs at least 5 points)
          } else if (strcasecmp(token, "akima_periodic") == 0) {
            function->interp_type = *gsl_interp_akima_periodic;
            
///kw+FUNCTION+usage steffen |
///kw+FUNCTION+detail  * steffen, always-monotonic splines-like (available only with GSL >= 2.0)
          } else if (strcasecmp(token, "steffen") == 0) {
#if (GSL_MAJOR_VERSION < 2)  
//            wasora_push_error_message("inerpolation steffen is available only for GSL >= 2.0, you have version %s", GSL_VERSION);
            function->interp_type = *gsl_interp_linear;
#else
            function->interp_type = *gsl_interp_steffen;
#endif
///kw+FUNCTION+detail @ 
///kw+FUNCTION+detail Default interpolation scheme for one-dimensional functions is `DEFAULT_INTERPOLATION`.
///kw+FUNCTION+detail @ 
///kw+FUNCTION+detail Available schemes for $n>1$ are:
///kw+FUNCTION+detail @
///kw+FUNCTION+usage nearest |
///kw+FUNCTION+detail  * nearest, $f(\vec{x})$ is equal to the value of the closest definition point
          } else if (strcasecmp(token, "nearest") == 0) {
            function->multidim_interp = nearest;
///kw+FUNCTION+usage shepard |
///kw+FUNCTION+detail  * shepard, [inverse distance weighted average definition points](https:/\/en.wikipedia.org/wiki/Inverse_distance_weighting) (might lead to inefficient evaluation)
          } else if (strcasecmp(token, "shepard") == 0) {
            function->multidim_interp = shepard;
///kw+FUNCTION+usage shepard_kd |
///kw+FUNCTION+detail  * shepard_kd, [average of definition points within a kd-tree](https:/\/en.wikipedia.org/wiki/Inverse_distance_weighting#Modified_Shepard&#39;s_method) (more efficient evaluation provided `SHEPARD_RADIUS` is set to a proper value)
          } else if (strcasecmp(token, "shepard_kd") == 0 || strcasecmp(token, "modified_shepard") == 0) {
            function->multidim_interp = shepard_kd;
///kw+FUNCTION+usage bilinear
///kw+FUNCTION+detail  * bilinear, only available if the definition points configure an structured hypercube-like grid. If $n>3$, `SIZES` should be given.
          } else if (strcasecmp(token, "bilinear") == 0 || strcasecmp(token, "rectangle") == 0 || strcasecmp(token, "rectangular") == 0) {
            function->multidim_interp = bilinear;
///kw+FUNCTION+usage } ]
          } else {
            wasora_push_error_message("undefined interpolation method '%s'", token);
            return WASORA_PARSER_ERROR;
          }
///kw+FUNCTION+detail @

///kw+FUNCTION+usage [ INTERPOLATION_THRESHOLD <expr> ]
///kw+FUNCTION+detail For $n>1$, if the euclidean distance between the arguments and the definition points is smaller than `INTERPOLATION_THRESHOLD`, the definition point is returned and no interpolation is performed.
///kw+FUNCTION+detail Default value is square root of `DEFAULT_MULTIDIM_INTERPOLATION_THRESHOLD`.
        } else if (strcasecmp(token, "INTERPOLATION_THRESHOLD") == 0) {

           wasora_parser_expression(&function->expr_multidim_threshold);
           
///kw+FUNCTION+usage [ SHEPARD_RADIUS <expr> ]
///kw+FUNCTION+detail The initial radius of points to take into account in `shepard_kd` is given by `SHEPARD_RADIUS`. If no points are found, the radius is double until at least one definition point is found.
///kw+FUNCTION+detail The radius is doubled until at least one point is found.
///kw+FUNCTION+detail Default is `DEFAULT_SHEPARD_RADIUS`.
        } else if (strcasecmp(token, "SHEPARD_RADIUS") == 0) {

           wasora_parser_expression(&function->expr_shepard_radius);

///kw+FUNCTION+usage [ SHEPARD_EXPONENT <expr> ]
///kw+FUNCTION+detail The exponent of the `shepard` method is given by `SHEPARD_EXPONENT`.
///kw+FUNCTION+detail Default is `DEFAULT_SHEPARD_EXPONENT`.
        } else if (strcasecmp(token, "SHEPARD_EXPONENT") == 0) {
           wasora_parser_expression(&function->expr_shepard_exponent);

///kw+FUNCTION+usage [ SIZES <expr_1> <expr_2> ... <expr_n> ]
///kw+FUNCTION+detail When requesting `bilinear` interpolation for $n>3$, the number of definition points for each argument variable has to be given with `SIZES`,
        } else if (strcasecmp(token, "SIZES") == 0) {
          
          function->expr_rectangular_mesh_size = calloc(function->n_arguments, sizeof(expr_t));
          
          for (i = 0; i < function->n_arguments; i++) {
            wasora_call(wasora_parser_expression(&function->expr_rectangular_mesh_size[i]));
          }
          
///kw+FUNCTION+usage [ X_INCREASES_FIRST <expr> ]
///kw+FUNCTION+detail and wether the definition data is sorted with the first argument changing first (`X_INCREASES_FIRST` evaluating to non-zero) or with the last argument changing first (zero).
        } else if (strcasecmp(token,"X_INCREASES_FIRST") == 0) {
          
          wasora_call(wasora_parser_expression(&function->expr_x_increases_first));
          
        } else {
          wasora_push_error_message("unknown keyword '%s'", token);
          return WASORA_PARSER_ERROR;
        }
      }


      // si no nos dieron expresion algebraica, hay que leer e interpolar datos
      if (function->algebraic_expression.n_tokens == 0) {
        if (function->type == type_pointwise_file) {

          // si nos dieron un archivo, leemos de ahi
          FILE *data_file;
          char data_line[BUFFER_SIZE*BUFFER_SIZE];
          int n_columns;

          if ((data_file = wasora_fopen(function->data_file, "r")) == NULL) {
            wasora_push_error_message("\"%s\" opening file '%s'", strerror(errno), function->data_file);
            return WASORA_PARSER_ERROR;
          }

          // contamos cuantas lineas no vacias hay -> ese es el tamanio del size
          function->data_size = 0;
          while (wasora_read_data_line(data_file, data_line) != 0) {
            if (data_line[0] != '\0') {
              // en la primera vuelta miramos cuantas columnas hay
              if (function->data_size == 0) {
                n_columns = 0;
                token = strtok(data_line, " \t");
                while (token != NULL) {
                  n_columns++;
                  token = strtok(NULL, " \t");
                }

                if (n_columns < (nargs+1)) {
                  wasora_push_error_message("at least %d columns expected but %d were given in  file '%s'", nargs+1, n_columns, function->data_file);
                  return WASORA_PARSER_ERROR;
                }

              }
              function->data_size++;
            }
          }

          function->data_argument = calloc(nargs, sizeof(double *));
          function->data_argument_alloced = 1;
          for (i = 0; i < nargs; i++) {
            function->data_argument[i] = calloc(function->data_size, sizeof(double));
          }
          function->data_value = calloc(function->data_size, sizeof(double));

          rewind(data_file);

          // ahora leemos los datos
          j = 0;
          while (wasora_read_data_line(data_file, data_line) != 0) {

            if (data_line[0] != '\0') {
              for (i = 0; i < nargs; i++) {
                if ((token = wasora_get_nth_token(data_line, function->column[i])) == NULL) {
                  wasora_push_error_message("wrong-formatted file '%s' at line %d", function->data_file, j+1);
                  return WASORA_PARSER_ERROR;
                }
                sscanf(token, "%lf", &function->data_argument[i][j]);
                free(token);

                //  para poder meter steps o numeros repetidos
                if (nargs == 1 && j > 0 && gsl_fcmp(function->data_argument[i][j], function->data_argument[i][j-1], 1e-12) == 0) {
                  if (j >= 2) {
                    // si es un step tratamos de manejarlo
                    function->data_argument[i][j] += 0.005*(function->data_argument[i][j-1]-function->data_argument[i][j-2]);
                  } else {
                    // si es el primer punto, lo tiramos
                    function->data_size--;
                    j--;
                  }
                }
              
              }
              if ((token = wasora_get_nth_token(data_line, function->column[i])) == NULL) {
                wasora_push_error_message("not enough columns in file '%s' at line %d", function->data_file, j+1);
                return WASORA_PARSER_ERROR;
              }
              sscanf(token, "%lf", &function->data_value[j]);
              free(token);

              j++;
            }
          }

          fclose(data_file);


///kw+FUNCTION+usage [ DATA <num_1> <num_2> ... <num_N> ]
///kw+FUNCTION+detail The function can be pointwise-defined inline in the input using `DATA`. This should be the last keyword of the line, followed by $N=k\cdot (n+1)$ expresions giving $k$ definition points: $n$ arguments and the value of the function.
///kw+FUNCTION+detail Multiline continuation using brackets `{` and `}` can be used for a clean data organization. See the examples.
        } else if (token != NULL && strcasecmp(token, "DATA") == 0) {
          // leemos del mismo archivo de entrada
          dummy = token + strlen(token)+1;

          // nos dieron los datos hard en el wasora_input
          backup1 = malloc(strlen(dummy)+8);
          snprintf(backup1, strlen(dummy)+8, "dummy %s", dummy);
          backup2 = malloc(strlen(dummy)+8);
          snprintf(backup2, strlen(dummy)+8, "dummy %s", dummy);

          // contamos cuanta informacion hay
          function->data_size = 0;
          if ((token = wasora_get_next_token(backup1)) == NULL) {
            return WASORA_PARSER_ERROR;
          }
          while ((token = wasora_get_next_token(NULL)) != NULL) {
            function->data_size++;
          }
          if (function->data_size % (nargs+1) != 0) {
            wasora_push_error_message("data mismatch for function '%s'", function->name);
            return WASORA_PARSER_ERROR;
          }
          function->data_size /= (nargs+1);

          function->data_argument = calloc(nargs, sizeof(double *));
          function->data_argument_alloced = 1;
          for (i = 0; i < nargs; i++) {
            function->data_argument[i] = calloc(function->data_size, sizeof(double));
          }
          
          function->data_value = calloc(function->data_size, sizeof(double));

          // leemos la informacion
          if ((token = wasora_get_next_token(backup2)) == NULL) {
            return WASORA_PARSER_ERROR;
          }
          for (i = 0; i < function->data_size; i++) {
            // argumentos
            for (j = 0; j < nargs; j++) {
              if ((token = wasora_get_next_token(NULL)) == NULL) {
                return WASORA_PARSER_ERROR;
              }
              function->data_argument[j][i] = wasora_evaluate_expression_in_string(token);

              //  para poder meter steps
              if (nargs == 1 && i >= 2 && function->data_argument[j][i] == function->data_argument[j][i-1]) {
                  function->data_argument[j][i] += 0.005*(function->data_argument[j][i-1]-function->data_argument[j][i-2]);
                }
              }

            // valor
            if ((token = wasora_get_next_token(NULL)) == NULL) {
              return WASORA_PARSER_ERROR;
            }
            function->data_value[i] = wasora_evaluate_expression_in_string(token);
          }

          free(backup2);
          free(backup1);
        }


        // creamos tres variables (constantes) extra: func_a, func_b y func_n
        if (nargs == 1) {
          var_t *dummy_var;
          char *dummy_aux = malloc(strlen(function->name) + 4);

          snprintf(dummy_aux, strlen(function->name) + 4, "%s_a", function->name);
          if ((dummy_var = wasora_define_variable(dummy_aux)) == NULL) {
            return WASORA_PARSER_ERROR;
          }
          if (function->data_size != 0) {
            wasora_realloc_variable_ptr(dummy_var, &function->data_argument[0][0], 0);
          }

          snprintf(dummy_aux, strlen(function->name) + 4, "%s_b", function->name);
          if ((dummy_var = wasora_define_variable(dummy_aux)) == NULL) {
            return WASORA_PARSER_ERROR;
          }
          if (function->data_size != 0) {
            wasora_realloc_variable_ptr(dummy_var, &function->data_argument[0][function->data_size-1], 0);
          }

          snprintf(dummy_aux, strlen(function->name) + 4, "%s_n", function->name);
          if ((dummy_var = wasora_define_variable(dummy_aux)) == NULL) {
            return WASORA_PARSER_ERROR;
          }
          if (function->data_size != 0) {
            wasora_value(dummy_var) = (double)function->data_size;
          }

          free(dummy_aux);
        }

      }

      if (arg_name != NULL) {
        for (i = 0; i < nargs; i++) {
          free(arg_name[i]);
        }
        free(arg_name);
      }
      
      return WASORA_PARSER_OK;

// --- FILE -----------------------------------------------------
///kw+FILE+desc Define a file, either as input or as output, for further usage.
///kw+FILE+usage < FILE | OUTPUT_FILE | INPUT_FILE >
    } else if (strcasecmp(token, "FILE") == 0 || strcasecmp(token, "OUTPUT_FILE") == 0 || strcasecmp(token, "INPUT_FILE") == 0) {

      file_t *file;
      char *name = NULL;
      char *format = NULL;
      char *mode = NULL;
      expr_t *arg = NULL;
      int n_args = 0;
      int do_not_open = 0;

      if (strcasecmp(token, "OUTPUT_FILE") == 0) {
        mode = strdup("w");
      } else if (strcasecmp(token, "INPUT_FILE") == 0) {
        mode = strdup("r");
      }

///kw+FILE+usage <name>
      if (wasora_parser_string(&name) != WASORA_PARSER_OK) {
        return WASORA_PARSER_ERROR;
      }

///kw+FILE+usage <printf_format>
///kw+FILE+usage [ expr_1 expr_2 ... expr_n ]
      if (wasora_parser_string_format(&format, &n_args) != WASORA_PARSER_OK) {
        return WASORA_PARSER_ERROR;
      }

      if (n_args != 0) {
        arg = calloc(n_args, sizeof(expr_t));
        for (i = 0; i < n_args; i++) {
          if (wasora_parser_expression(&arg[i]) != WASORA_PARSER_OK) {
            return WASORA_PARSER_ERROR;
          }
        }
      }

///kw+FILE+usage [ INPUT | OUTPUT | MODE <fopen_mode> ]
      while ((token = wasora_get_next_token(NULL)) != NULL) {
        if (strcasecmp(token, "MODE") == 0) {
          if ((token = wasora_get_next_token(NULL)) == NULL) {
            wasora_push_error_message("expected a mode");
            return WASORA_PARSER_ERROR;
          }
          mode = strdup(token);
        } else if (strcasecmp(token, "INPUT") == 0) {
          mode = strdup("r");
        } else if (strcasecmp(token, "OUTPUT") == 0) {
          mode = strdup("w");
///kw+FILE+usage [ OPEN | DO_NOT_OPEN ]
        } else if (strcasecmp(token, "OPEN") == 0) {
          do_not_open = 0;
        } else if (strcasecmp(token, "DO_NOT_OPEN") == 0) {
          do_not_open = 1;
        } else {
          wasora_push_error_message("unknown keyword '%s'", token);
          return WASORA_PARSER_ERROR;
        }

      }

      // si no nos dicen que es, no abrimos
      if (mode == NULL) {
        do_not_open = 1;
      }

      if ((file = wasora_define_file(name, format, n_args, arg, mode, do_not_open)) == NULL) {
        return WASORA_PARSER_ERROR;
      }

      if (wasora_define_instruction(wasora_instruction_file, file) == NULL) {
        return WASORA_PARSER_ERROR;
      }

      free(format);
      free(mode);
      free(name);
      return WASORA_PARSER_OK;

// ---- CLOSE ----------------------------------------------------
///kw+CLOSE+usage CLOSE
///kw+CLOSE+desc Explicitly close an already-`OPEN`ed file.
    } else if (strcasecmp(token, "CLOSE") == 0) {
      
      file_t *file;
      
      wasora_call(wasora_parser_file(&file) != WASORA_PARSER_OK);
      
      if (wasora_define_instruction(wasora_instruction_close_file, file) == NULL) {
        return WASORA_PARSER_ERROR;
      }
      return WASORA_PARSER_OK;
      

// --- IF -----------------------------------------------------
///kw+IF+desc Begin a conditional block.
///kw+IF+usage IF expr
///kw+IF+usage &nbsp;
///kw+IF+usage  <block_of_instructions_if_expr_is_true>
///kw+IF+usage &nbsp;
///kw+IF+usage [ ELSE ]
///kw+IF+usage &nbsp;
///kw+IF+usage  [block_of_instructions_if_expr_is_false]
///kw+IF+usage &nbsp;
///kw+IF+usage ENDIF
    } else if (strcasecmp(token, "IF") == 0) {

      conditional_block_t *conditional_block;
      conditional_block = calloc(1, sizeof(conditional_block_t));
      LL_APPEND(wasora.conditional_blocks, conditional_block);

      conditional_block->condition = malloc(sizeof(expr_t));
      if (wasora_parser_expression(conditional_block->condition) != WASORA_PARSER_OK) {
        return WASORA_PARSER_ERROR;
      }

      if (wasora_get_next_token(NULL) != NULL) {
        wasora_push_error_message("conditional blocks should start in a separate line");
        return WASORA_PARSER_ERROR;
      }

      conditional_block->father = wasora.active_conditional_block;

      if (wasora_define_instruction(wasora_instruction_if, conditional_block) == NULL) {
        return WASORA_PARSER_ERROR;
      }

      wasora.active_conditional_block = conditional_block;

      return WASORA_PARSER_OK;

    // ---- ELSE  ----------------------------------------------------
    } else if (strcasecmp(token, "ELSE") == 0) {

      instruction_t *instruction;

      conditional_block_t *conditional_block;
      conditional_block = calloc(1, sizeof(conditional_block_t));
      LL_APPEND(wasora.conditional_blocks, conditional_block);

      if (wasora.active_conditional_block->else_of != NULL) {
        wasora_push_error_message("more than one ELSE clause for a single IF clause", token);
        return WASORA_PARSER_ERROR;
      }

      if ((instruction = wasora_define_instruction(wasora_instruction_else, conditional_block)) == NULL) {
        return WASORA_PARSER_ERROR;
      }

      wasora.active_conditional_block->first_false_instruction = instruction;

      conditional_block->else_of = wasora.active_conditional_block;
      conditional_block->father = wasora.active_conditional_block->father;
      wasora.active_conditional_block = conditional_block;

      return WASORA_PARSER_OK;

// ---- ENDIF  ----------------------------------------------------
    } else if (strcasecmp(token, "ENDIF") == 0) {

      instruction_t *instruction;

      if ((instruction = wasora_define_instruction(wasora_instruction_endif, wasora.active_conditional_block)) == NULL) {
        return WASORA_PARSER_ERROR;
      }

      if (wasora.active_conditional_block->else_of == NULL) {
        wasora.active_conditional_block->first_false_instruction = instruction;
      } else {
        wasora.active_conditional_block->first_true_instruction = instruction;
      }

      wasora.active_conditional_block = wasora.active_conditional_block->father;

      return WASORA_PARSER_OK;

// ---- SEMAPHORE ----------------------------------------------------
///kw+SEMAPHORE+desc Perform either a wait or a post operation on a named shared semaphore.
///kw+SEMAPHORE+usage [ SEMAPHORE | SEM ]
    } else if ((strcasecmp(token, "SEMAPHORE") == 0) || (strcasecmp(token, "SEM") == 0)) {

      struct semaphore_t *semaphore;
      semaphore = calloc(1, sizeof(struct semaphore_t));
      LL_APPEND(wasora.semaphores, semaphore);

///kw+SEMAPHORE+usage <name>
      if (wasora_parser_string(&semaphore->name) != WASORA_PARSER_OK) {
        return WASORA_PARSER_ERROR;
      }

///kw+SEMAPHORE+usage { WAIT | POST }
      char *keywords[] = {"WAIT", "POST", ""};
      int values[] = {wasora_sem_wait, wasora_sem_post, 0};
      wasora_call(wasora_parser_keywords_ints(keywords, values, (int *)&semaphore->operation));

      if (wasora_define_instruction(wasora_instruction_sem, semaphore) == NULL) {
        return WASORA_PARSER_ERROR;
      }

      return WASORA_PARSER_OK;

// ---- READ / WRITE  ----------------------------------------------------
///kw+READ+desc Read data (variables, vectors o matrices) from files or shared-memory segments.
///kw+WRITE+desc Write data (variables, vectors o matrices) to files or shared-memory segments.
///kw+WRITE+desc See the `READ` keyword for usage details.
///kw+READ+usage [ READ | WRITE ]
    } else if ((strcasecmp(token, "READ") == 0) || (strcasecmp(token, "WRITE") == 0)) {

      io_t *io = calloc(1, sizeof(io_t));
      LL_APPEND(wasora.ios, io);

      if (strcasecmp(token, "READ") == 0) {
        io->direction = io_read;
      } else if (strcasecmp(token, "WRITE") == 0) {
        io->direction = io_write;
      }

      while ((token = wasora_get_next_token(NULL)) != NULL) {
        // ---- SHM_OBJECT ---------------------------------------------------------
///kw+READ+usage [ SHM <name> ]
        if (strcasecmp(token, "SHM") == 0 || strcasecmp(token, "SHM_OBJECT") == 0) {

          io->type = io_shm;

          if (wasora_parser_string(&io->shm_name) != WASORA_PARSER_OK) {
            return WASORA_PARSER_ERROR;
          }

        // ---- FILE_PATH ----------------------------------------------------
///kw+READ+usage [ { ASCII_FILE_PATH | BINARY_FILE_PATH } <file_path> ]
        } else if (strcasecmp(token, "ASCII_FILE_PATH") == 0 || strcasecmp(token, "BINARY_FILE_PATH") == 0) {

          char mode[2];
          if (io->direction == io_read) {
            sprintf(mode, "r");
          } else if (io->direction == io_write) {
            sprintf(mode, "w");
          }
            
          if (strcasecmp(token, "ASCII_FILE_PATH") == 0) {
            io->type = io_file_ascii;
          } else if (strcasecmp(token, "BINARY_FILE_PATH") == 0) {
            io->type = io_file_binary;
          }

          wasora_call(wasora_parser_file_path(&io->file, mode));

        // ---- FILE ----------------------------------------------------
///kw+READ+usage [ { ASCII_FILE | BINARY_FILE } <identifier> ]
        } else if (strcasecmp(token, "ASCII_FILE") == 0 || strcasecmp(token, "BINARY_FILE") == 0) {

          if (strcasecmp(token, "ASCII_FILE") == 0) {
            io->type = io_file_ascii;
          } else if (strcasecmp(token, "BINARY_FILE") == 0) {
            io->type = io_file_binary;
          }

          if (wasora_parser_file(&io->file) != WASORA_PARSER_OK) {
            return WASORA_PARSER_ERROR;
          }

          // ---- FILE ----------------------------------------------------
///kw+READ+usage [ IGNORE_NULL ]
          } else if (strcasecmp(token, "IGNORE_NULL") == 0) {

            io->ignorenull = 1;

          } else  {
///kw+READ+usage [ object_1 object_2 ... object_n ]

          // cosas
          io->n_things++;
          io_thing_t *thing = calloc(1, sizeof(io_thing_t));
          LL_APPEND(io->things, thing);

          // si pusieron un corchete quieren un pedazo solamente
          if ((dummy = strchr(token, '<')) != NULL) {
            *dummy = '\0';
          }

          if ((thing->variable = wasora_get_variable_ptr(token)) != NULL) {

            // manzana, despues sumamos uno al tamanio en wasora_io_init()
            ;

          } else if ((thing->vector = wasora_get_vector_ptr(token)) != NULL) {

            if (dummy != NULL) {
              *dummy = '<';
              wasora_call(wasora_parse_range(dummy, '<', ':', '>', &thing->expr_row_min, &thing->expr_row_max));
            }

          } else if ((thing->matrix = wasora_get_matrix_ptr(token)) != NULL) {

            if (dummy != NULL) {
              *dummy = '<';
              wasora_call(wasora_parse_range(dummy, '<', ':', ';', &thing->expr_row_min, &thing->expr_row_max));
              wasora_call(wasora_parse_range(strchr(dummy, ';'), ';', ':', '>', &thing->expr_col_min, &thing->expr_col_max));
            }

 
          } else {
            
            if (wasora_parse_expression(token, &thing->expr) != WASORA_PARSER_OK) {
              wasora_push_error_message("undefined keyword, variable, vector, matrix, alias or invalid expression '%s'", token);
              return WASORA_PARSER_ERROR;
            }
            
            if (io->direction == io_read) {
              wasora_push_error_message("expressions cannot be used in a READ instruction", token);
              return WASORA_PARSER_ERROR;
            }
          
          }
        }
      }


      if (io->type == io_undefined)  {
        wasora_push_error_message("undefined I/O resource type");
        return WASORA_PARSER_ERROR;
      }

      if (wasora_define_instruction(wasora_instruction_io, io) == NULL) {
        return WASORA_PARSER_ERROR;
      }

      return WASORA_PARSER_OK;

// --- PRINT -----------------------------------------------------
///kw+PRINT+desc Print plain-text and/or formatted data to the standard output or into an output file.
///kw+PRINT+usage PRINT
    } else if (strcasecmp(token, "PRINT") == 0) {

      print_t *print;
      matrix_t *dummy_matrix;
      vector_t *dummy_vector;
      int n;

      print = calloc(1, sizeof(print_t));
      LL_APPEND(wasora.prints, print);

      char *keywords[] = {"SKIP_STEP", "SKIP_STATIC_STEP", "SKIP_TIME", "SKIP_HEADER_STEP", ""};
      expr_t *expressions[] = {
        &print->skip_step,
        &print->skip_static_step,
        &print->skip_time,
        &print->skip_header_step,
        NULL
      };

///kw+PRINT+detail Each argument `object` that is not a keyword is expected to be part of the output, can be either a matrix, a vector, an scalar algebraic expression.
///kw+PRINT+detail If the given object cannot be solved into a valid matrix, vector or expression, it is treated as a string literal if `IMPLICIT` is `ALLOWED`, otherwise a parser error is raised.
///kw+PRINT+detail To explicitly interpret an object as a literal string even if it resolves to a valid numerical expression, it should be prefixed with the `TEXT` keyword.
///kw+PRINT+detail Hashes `#` appearing literal in text strings have to be quoted to prevent the parser to treat them as comments within the wasora input file and thus ignoring the rest of the line.
///kw+PRINT+detail Whenever an argument starts with a porcentage sign  `%`, it is treated as a C `printf`-compatible format definition and all the objects that follow it are printed using the given format until a new format definition is found.
///kw+PRINT+detail The objects are treated as double-precision floating point numbers, so only floating point formats should be given. The default format is `DEFAULT_PRINT_FORMAT`.
///kw+PRINT+detail Matrices, vectors, scalar expressions, format modifiers and string literals can be given in any desired order, and are processed from left to right.
///kw+PRINT+detail Vectors are printed element-by-element in a single row. See `PRINT_VECTOR` to print vectors column-wise.
///kw+PRINT+detail Matrices are printed element-by-element in a single line using row-major ordering if mixed with other objects but in the natural row and column fashion if it is the only given object.
      
      while ((token = wasora_get_next_token(NULL)) != NULL) {
///kw+PRINT+usage [ FILE <file_id> |
///kw+PRINT+detail If the `FILE` keyword is not provided, default is to write to stdout.
        if (strcasecmp(token, "FILE") == 0) {
          if (wasora_parser_file(&print->file) != WASORA_PARSER_OK) {
            return WASORA_PARSER_ERROR;
          }
///kw+PRINT+usage FILE_PATH <file_path> ]
        } else if (strcasecmp(token, "FILE_PATH") == 0) {
            wasora_call(wasora_parser_file_path(&print->file, "w"));

///kw+PRINT+usage [ NONEWLINE ]
        } else if (strcasecmp(token, "NONEWLINE") == 0) {
///kw+PRINT+detail If the `NONEWLINE` keyword is not provided, default is to write a newline `\n` character after all the objects are processed.
          print->nonewline = 1;

///kw+PRINT+usage [ SEP <string> ]
        } else if (strcasecmp(token, "SEP") == 0 || strcasecmp(token, "SEPARATOR") == 0) {
          if (wasora_parser_string(&print->separator) != WASORA_PARSER_OK) {
            return WASORA_PARSER_ERROR;
          }
///kw+PRINT+detail The `SEP` keywords expects a string used to separate printed objects, the default is a tab 'DEFAULT_PRINT_SEPARATOR' character.

///kw+PRINT+usage [ NOSEP ]
        } else if (strcasecmp(token, "NOSEP") == 0) {
          print->separator = strdup("");
///kw+PRINT+detail Use the `NOSEP` keyword to define an empty string as object separator.
          
///kw+PRINT+usage [ HEADER ]
        } else if (strcasecmp(token, "HEADER") == 0) {
          print->header = 1;
///kw+PRINT+detail If the `HEADER` keyword is given, a single line containing the literal text
///kw+PRINT+detail given for each object is printed at the very first time the `PRINT` instruction is
///kw+PRINT+detail processed, starting with a hash `#` character.           

///kw+PRINT+usage [ SKIP_STEP <expr> ]
///kw+PRINT+detail If the `SKIP_STEP` (`SKIP_STATIC_STEP`)keyword is given, the instruction is processed only every
///kw+PRINT+detail the number of transient (static) steps that results in evaluating the expression,
///kw+PRINT+detail which may not be constant. By default the `PRINT` instruction is evaluated every
///kw+PRINT+detail step. The `SKIP_HEADER_STEP` keyword works similarly for the optional `HEADER` but
///kw+PRINT+detail by default it is only printed once. The `SKIP_TIME` keyword use time advancements
///kw+PRINT+detail to choose how to skip printing and may be useful for non-constant time-step problems.
          
///kw+PRINT+usage [ SKIP_STATIC_STEP <expr> ]
///kw+PRINT+usage [ SKIP_TIME <expr> ]
///kw+PRINT+usage [ SKIP_HEADER_STEP <expr> ]
        } else if ((n = wasora_parser_match_keyword_expression(token, keywords, expressions, sizeof(expressions)/sizeof(expr_t *))) != WASORA_PARSER_UNHANDLED) {
          if (n == WASORA_PARSER_ERROR) {
            return WASORA_PARSER_ERROR;
          }

        } else {
///kw+PRINT+usage [ <object_1> <object_2> ... <object_n> ]
///kw+PRINT+usage [ TEXT <string_1> ... TEXT <string_n> ]
          // es un formato, una expresion o texto
          // asi que agregamos un eslabon a la lista de tokens
          print_token_t *print_token = calloc(1, sizeof(print_token_t));
          LL_APPEND(print->tokens, print_token);

          if (token[0] == '%') {
            print_token->format = strdup(token);

          } else if (strcasecmp(token, "STRING") == 0 || strcasecmp(token, "TEXT") == 0) {
            if (wasora_parser_string(&print_token->text) != WASORA_PARSER_OK) {
              return WASORA_PARSER_ERROR;
            }

          } else if ((dummy_matrix = wasora_get_matrix_ptr(token)) != NULL) {
            print_token->text = strdup(token);   // nos quedamos con el texto para el header
            print_token->matrix = dummy_matrix;

          } else if ((dummy_vector = wasora_get_vector_ptr(token)) != NULL) {
            print_token->text = strdup(token);   // nos quedamos con el texto para el header
            print_token->vector = dummy_vector;

          } else {
            if (wasora_parse_expression(token, &print_token->expression) != WASORA_PARSER_OK) {

              // dejamos que las expresiones no resueltas se transformen en texto
              // solo si no tenemos implicit none
              if (wasora.implicit_none) {
                wasora_push_error_message("implicit definition is not allowed");
                return WASORA_PARSER_ERROR;
              } else {
                print_token->expression.n_tokens = 0;
                print_token->text = strdup(token);
                wasora_pop_error_message();
              }
            } else {
              print_token->text = strdup(token);   // nos quedamos con el texto para el header
            }
          }
        }
      }

      // llenamos defaults
      if (print->separator == NULL) {
        print->separator = strdup(DEFAULT_PRINT_SEPARATOR);
      }

      if (print->file == NULL) {
        print->file = wasora.special_files.stdout_;
      }

      if (wasora_define_instruction(wasora_instruction_print, print) == NULL) {
        return WASORA_PARSER_ERROR;
      }

      return WASORA_PARSER_OK;


// --- PRINT_FUNCTION -----------------------------------------------------
///kw+PRINT_FUNCTION+desc Print one or more functions as a table of values of dependent and independent variables.
///kw+PRINT_FUNCTION+usage PRINT_FUNCTION
///kw+PRINT_FUNCTION+usage <function_1> [ { function_2 | expr_1 } ... { function_n | expr_n-1 } ]
    } else if (strcasecmp(token, "PRINT_FUNCTION") == 0) {

      print_function_t *print_function;
      function_t *dummy_function;

      print_function = calloc(1, sizeof(print_function_t));
      LL_APPEND(wasora.print_functions, print_function);


      while ((token = wasora_get_next_token(NULL)) != NULL) {
///kw+PRINT_FUNCTION+usage [ FILE <file_id> |
        if (strcasecmp(token, "FILE") == 0) {
          wasora_call(wasora_parser_file(&print_function->file));
///kw+PRINT_FUNCTION+usage FILE_PATH <file_path> ]
        } else if (strcasecmp(token, "FILE_PATH") == 0) {
          wasora_call(wasora_parser_file_path(&print_function->file, "w"));

///kw+PRINT_FUNCTION+usage [ HEADER ]
        } else if (strcasecmp(token, "HEADER") == 0) {
          print_function->header = 1;
          
///kw+PRINT_FUNCTION+usage [ MIN <expr_1> <expr_2> ... <expr_m> ]
        } else if (strcasecmp(token, "MIN") == 0) {

          if (print_function->first_function == NULL) {
            wasora_push_error_message("MIN before actual function, cannot determine number of arguments");
            return WASORA_PARSER_ERROR;
          }

          if (wasora_parser_expressions(&print_function->range.min, print_function->first_function->n_arguments) != WASORA_PARSER_OK) {
            return WASORA_PARSER_ERROR;
          }


///kw+PRINT_FUNCTION+usage [ MAX <expr_1> <expr_2> ... <expr_m> ]
        } else if (strcasecmp(token, "MAX") == 0) {

          if (print_function->first_function == NULL) {
            wasora_push_error_message("MAX before actual function, cannot determine number of arguments");
            return WASORA_PARSER_ERROR;
          }

          if (wasora_parser_expressions(&print_function->range.max, print_function->first_function->n_arguments) != WASORA_PARSER_OK) {
            return WASORA_PARSER_ERROR;
          }

///kw+PRINT_FUNCTION+usage [ STEP <expr_1> <expr_2> ... <expr_m> ]
        } else if (strcasecmp(token, "STEP") == 0) {

          if (print_function->first_function == NULL) {
            wasora_push_error_message("STEP before actual function, cannot determine number of arguments");
            return WASORA_PARSER_ERROR;
          }

          if (wasora_parser_expressions(&print_function->range.step, print_function->first_function->n_arguments) != WASORA_PARSER_OK) {
            return WASORA_PARSER_ERROR;
          }

///kw+PRINT_FUNCTION+usage [ NSTEPs <expr_1> <expr_2> ... <expr_m> ]
        } else if (strcasecmp(token, "NSTEPS") == 0) {

          if (print_function->first_function == NULL) {
            wasora_push_error_message("NSTEPS before actual function, cannot determine number of arguments");
            return WASORA_PARSER_ERROR;
          }

          if (wasora_parser_expressions(&print_function->range.nsteps, print_function->first_function->n_arguments) != WASORA_PARSER_OK) {
            return WASORA_PARSER_ERROR;
          }

///kw+PRINT_FUNCTION+usage [ FORMAT <print_format> ]
        } else if (strcasecmp(token, "FORMAT") == 0) {
          if (wasora_parser_string(&print_function->format) != WASORA_PARSER_OK) {
            return WASORA_PARSER_ERROR;
          }
          
///kw+PRINT_FUNCTION+usage [ PHYSICAL_ENTITY <name> ]
        } else if (strcasecmp(token, "PHYSICAL_ENTITY") == 0) {
          char *name;
          wasora_call(wasora_parser_string(&name));
          if ((print_function->physical_entity = wasora_get_physical_entity_ptr(name, NULL)) == NULL) {
            wasora_push_error_message("unknown physical entity '%s'", name);
            free(name);
            return WASORA_PARSER_ERROR;
          }

        } else {

          // agregamos un eslabon a la lista de tokens
          print_token_t *print_token = calloc(1, sizeof(print_token_t));
          LL_APPEND(print_function->tokens, print_token);

          print_token->text = strdup(token);   // nos quedamos con el texto para el header
          
          if ((dummy_function = wasora_get_function_ptr(token)) != NULL) {
            print_token->function = dummy_function;
            if (print_function->first_function == NULL) {
              // es la primer funcion
              print_function->first_function = dummy_function;

            } else if (dummy_function->n_arguments !=  print_function->first_function->n_arguments) {
              wasora_push_error_message("functions do not have the same number of arguments");
              return WASORA_PARSER_ERROR;

            }

          } else {
            wasora_call(wasora_parse_expression(token, &print_token->expression));

          }
        }
      }

      if (print_function->first_function == NULL) {
        wasora_push_error_message("at least one function expected");
        return WASORA_PARSER_ERROR;
      }
      
      if (print_function->first_function->type == type_algebraic || print_function->first_function->type == type_routine) {
        if (print_function->range.min == NULL) {
          wasora_push_error_message("need MIN keyword (function %s is not point-wise defined)", print_function->first_function->name);
          return WASORA_PARSER_ERROR;
        }
        if (print_function->range.max == NULL) {
          wasora_push_error_message("need MAX keyword (function %s is not point-wise defined)", print_function->first_function->name);
          return WASORA_PARSER_ERROR;
        }
        if (print_function->range.step == NULL && print_function->range.nsteps == NULL) {
          wasora_push_error_message("need either STEP or NSTEPS keyword (function %s is not point-wise defined)", print_function->first_function->name);
          return WASORA_PARSER_ERROR;
        }
      }
      
      if (print_function->separator == NULL) {
        print_function->separator = strdup(DEFAULT_PRINT_SEPARATOR);
      }

      if (print_function->format == NULL) {
        print_function->format = strdup(DEFAULT_PRINT_FORMAT);
      }

      if (print_function->file == NULL) {
        print_function->file = wasora.special_files.stdout_;
      }

      if (wasora_define_instruction(wasora_instruction_print_function, print_function) == NULL) {
        return WASORA_PARSER_ERROR;
      }

      return WASORA_PARSER_OK;

// --- PRINT_VECTOR -----------------------------------------------------
///kw+PRINT_VECTOR+desc Print the elements of one or more vectors.
///kw+PRINT_VECTOR+usage PRINT_VECTOR
    } else if (strcasecmp(token, "PRINT_VECTOR") == 0) {

      print_vector_t *print_vector;

      print_vector = calloc(1, sizeof(print_vector_t));
      LL_APPEND(wasora.print_vectors, print_vector);

      while ((token = wasora_get_next_token(NULL)) != NULL) {

///kw+PRINT_VECTOR+usage [ FILE <file_id> ]
        if (strcasecmp(token, "FILE") == 0) {
          wasora_call(wasora_parser_file(&print_vector->file));
///kw+PRINT_VECTOR+usage FILE_PATH <file_path> ]
        } else if (strcasecmp(token, "FILE_PATH") == 0) {
          wasora_call(wasora_parser_file_path(&print_vector->file, "w"));

///kw+PRINT_VECTOR+usage [ { VERTICAL | HORIZONTAL } ]
  } else if (strcasecmp(token, "VERTICAL") == 0) {
          print_vector->horizontal = 0;

        } else if (strcasecmp(token, "HORIZONTAL") == 0) {
          print_vector->horizontal = 1;

///kw+PRINT_VECTOR+usage [ ELEMS_PER_LINE <expr> ]
        } else if (strcasecmp(token, "ELEMS_PER_LINE") == 0) {
          print_vector->horizontal = 1;
          if (wasora_parser_expression(&print_vector->elems_per_line) != WASORA_PARSER_OK) {
            return WASORA_PARSER_ERROR;
          }

///kw+PRINT_VECTOR+usage [ FORMAT <print_format> ]
        } else if (strcasecmp(token, "FORMAT") == 0) {
          if (wasora_parser_string(&print_vector->format) != WASORA_PARSER_OK) {
            return WASORA_PARSER_ERROR;
          }

        } else {

///kw+PRINT_VECTOR+usage <vector_1> [ vector_2 ... vector_n ]
          // agregamos un eslabon a la lista de tokens
          print_token_t *print_token = calloc(1, sizeof(print_token_t));
          LL_APPEND(print_vector->tokens, print_token);

          if ((print_token->vector = wasora_get_vector_ptr(token)) != NULL) {
            if (print_vector->first_vector == NULL) {
              // es el primer vector
              print_vector->first_vector = print_token->vector;
            } 
            
          } else {
            wasora_call(wasora_parse_expression(token, &print_token->expression));
            
          }
        }
      }


      // llenamos defaults
      if (print_vector->format == NULL) {
        print_vector->format = strdup(DEFAULT_PRINT_FORMAT);
      }

      if (print_vector->separator == NULL) {
        print_vector->separator = strdup(DEFAULT_PRINT_SEPARATOR);
      }

      if (print_vector->file == NULL) {
        print_vector->file = wasora.special_files.stdout_;
      }

      if (wasora_define_instruction(wasora_instruction_print_vector, print_vector) == NULL) {
        return WASORA_PARSER_ERROR;
      }

      return WASORA_PARSER_OK;
      
// --- SOLVE -----------------------------------------------------
///kw+SOLVE+desc Solve a non-linear system of\ $n$ equations with\ $n$ unknowns.
///kw+SOLVE+example solve1.was solve2.was
///kw+SOLVE+usage SOLVE
      
    } else if (strcasecmp(token, "SOLVE") == 0) {

      double xi;
      int i;
      solve_t *solve;
      solve = calloc(1, sizeof(solve_t));
      LL_APPEND(wasora.solves, solve);

///kw+SOLVE+usage <n> 
      wasora_call(wasora_parser_expression_in_string(&xi));
      if ((solve->n = (int)(xi)) <= 0) {
        wasora_push_error_message("expected a positive number of unknowns instead of %d", solve->n);
        return WASORA_PARSER_ERROR;
      }

      solve->unknown = calloc(solve->n, sizeof(var_t *));
      solve->residual = calloc(solve->n, sizeof(expr_t));
      solve->guess = calloc(solve->n, sizeof(expr_t));

      while ((token = wasora_get_next_token(NULL)) != NULL) {
///kw+SOLVE+usage UNKNOWNS <var_1> <var_2> ... <var_n>
        if (strcasecmp(token, "UNKNOWNS") == 0) {
          for (i = 0; i < solve->n; i++) {
            if ((token = wasora_get_next_token(NULL)) == NULL) {
              wasora_push_error_message("expected %d variables and found only %d", solve->n, i);
            }
            if ((solve->unknown[i] = wasora_get_or_define_variable_ptr(token)) == NULL) {
              return WASORA_PARSER_ERROR;
            }
          }
          
///kw+SOLVE+usage RESIDUALS <expr_1> <expr_2> ... <expr_n> ]
        } else if (strcasecmp(token, "RESIDUALS") == 0) {
          for (i = 0; i < solve->n; i++) {
            if ((token = wasora_get_next_token(NULL)) == NULL) {
              wasora_push_error_message("expected %d expressions and found only %d", solve->n, i);
            }
            wasora_call(wasora_parse_expression(token, &solve->residual[i]));
          }
      
///kw+SOLVE+usage GUESS <expr_1> <expr_2> ... <expr_n> ]
        } else if (strcasecmp(token, "GUESS") == 0) {
          for (i = 0; i < solve->n; i++) {
            if ((token = wasora_get_next_token(NULL)) == NULL) {
              wasora_push_error_message("expected %d expressions and found only %d", solve->n, i);
            }
            wasora_call(wasora_parse_expression(token, &solve->guess[i]));
          }
      
///kw+SOLVE+usage [ METHOD
        } else if (strcasecmp(token, "METHOD") == 0) {

          if ((token = wasora_get_next_token(NULL)) == NULL) {
            wasora_push_error_message("expected method name");
            return WASORA_PARSER_ERROR;
          }

///kw+SOLVE+usage {
///kw+SOLVE+usage dnewton |
          if (strcasecmp(token, "dnewton") == 0) {
            solve->type = gsl_multiroot_fsolver_dnewton;
///kw+SOLVE+usage hybrid |
          } else if (strcasecmp(token, "hybrid") == 0) {
            solve->type = gsl_multiroot_fsolver_hybrid;
///kw+SOLVE+usage hybrids |
          } else if (strcasecmp(token, "hybrids") == 0) {
            solve->type = gsl_multiroot_fsolver_hybrids;
///kw+SOLVE+usage broyden }
          } else if (strcasecmp(token, "broyden") == 0) {
            solve->type = gsl_multiroot_fsolver_hybrid;
          }
///kw+SOLVE+usage ]
          
///kw+SOLVE+usage [ EPSABS <expr> ]
        } else if (strcasecmp(token, "EPSABS") == 0) {

          wasora_call(wasora_parser_expression(&solve->epsabs));

///kw+SOLVE+usage [ EPSREL <expr> ]
        } else if (strcasecmp(token, "EPSREL") == 0) {

          wasora_call(wasora_parser_expression(&solve->epsrel));

///kw+SOLVE+usage [ MAX_ITER <expr> ]
        } else if (strcasecmp(token, "MAX_ITER") == 0) {

          wasora_call(wasora_parser_expression_in_string(&xi));
          if ((solve->max_iter = (int)xi) < 0) {
            wasora_push_error_message("expected a positive integer for MAX_ITER");
            return WASORA_PARSER_ERROR;
          }

///kw+SOLVE+usage [ VERBOSE ]
        } else if (strcasecmp(token, "VERBOSE") == 0) {

          solve->verbose = 1;          

        } else {
          wasora_push_error_message("unkown keyword '%s'", token);
          return WASORA_PARSER_ERROR;
        }
        
      }
        
      if (solve->residual[0].n_tokens == 0) {
        wasora_push_error_message("no RESIDUALs to solve");
        return WASORA_PARSER_ERROR;
      }
      
      if (solve->type == NULL) {
        solve->type = DEFAULT_SOLVE_METHOD;
      }

      if (wasora_define_instruction(wasora_instruction_solve, solve) == NULL) {
        return WASORA_PARSER_ERROR;
      }

      return WASORA_PARSER_OK;      

// --- M4 -----------------------------------------------------
///kw+M4+desc Call the `m4` macro processor with definitions from wasora variables or expressions.
///kw+M4+usage M4
    } else if (strcasecmp(token, "M4") == 0) {

      m4_t *m4;
      m4 = calloc(1, sizeof(m4_t));
      LL_APPEND(wasora.m4s, m4);

      while ((token = wasora_get_next_token(NULL)) != NULL) {
        
///kw+M4+usage { INPUT_FILE <file_id> |
        if (strcasecmp(token, "INPUT_FILE") == 0) {
          wasora_call(wasora_parser_file(&m4->input_file));
          
///kw+M4+usage FILE_PATH <file_path> }
        } else if (strcasecmp(token, "INPUT_FILE_PATH") == 0) {
          wasora_call(wasora_parser_file_path(&m4->input_file, "r"));

///kw+M4+usage { OUTPUT_FILE <file_id> |
        } else if (strcasecmp(token, "OUTPUT_FILE") == 0) {
          wasora_call(wasora_parser_file(&m4->output_file));
          
///kw+M4+usage OUTPUT_FILE_PATH <file_path> }
        } else if (strcasecmp(token, "OUTPUT_FILE_PATH") == 0) {
          wasora_call(wasora_parser_file_path(&m4->output_file, "w"));

///kw+M4+usage [ EXPAND <name> ] ... }
        } else if (strcasecmp(token, "EXPAND") == 0) {
          
          m4_macro_t *macro = calloc(1, sizeof(m4_macro_t));
          LL_APPEND(m4->macros, macro);
          
          wasora_call(wasora_parser_string(&macro->name));
          macro->print_token.format = strdup(DEFAULT_M4_FORMAT);
          if (wasora_parse_expression(macro->name, &macro->print_token.expression) != WASORA_PARSER_OK) {
            wasora_push_error_message("m4 expansion of '%s' failed", macro->name);
            return WASORA_PARSER_ERROR;
          }

///kw+M4+usage [ MACRO <name> [ <format> ] <definition> ] ... }
        } else if (strcasecmp(token, "MACRO") == 0) {
          
          m4_macro_t *macro = calloc(1, sizeof(m4_macro_t));
          LL_APPEND(m4->macros, macro);
          
          wasora_call(wasora_parser_string(&macro->name));
          
          if ((token = wasora_get_next_token(NULL)) == NULL) {
            wasora_push_error_message("expected either macro format or definition");
            return WASORA_PARSER_ERROR;
          }

          if (token[0] == '%') {
            macro->print_token.format = strdup(token);
            if ((token = wasora_get_next_token(NULL)) == NULL) {
              wasora_push_error_message("expected macro definition");
              return WASORA_PARSER_ERROR;
            }
            if (macro->print_token.format[0] == 'd') {
              macro->print_token.format[0] = 'g';
            }
          } else {
            macro->print_token.format = strdup(DEFAULT_M4_FORMAT);
          }

          // si el formato es %s o la expresion no evalua, lo tomamos como string
          if (strcasecmp(macro->print_token.format, "%s") == 0 ||
              wasora_parse_expression(token, &macro->print_token.expression) != WASORA_PARSER_OK) {
            macro->print_token.text = strdup(token);
          }
          
        } else {
          wasora_push_error_message("unkown keyword '%s'", token);
          return WASORA_PARSER_ERROR;
        }
      }

      if (wasora_define_instruction(wasora_instruction_m4, m4) == NULL) {
        return WASORA_PARSER_ERROR;
      }

      return WASORA_PARSER_OK;

// --- SHELL -----------------------------------------------------
///kw+SHELL+desc Execute a shell command.
///kw+SHELL+usage SHELL
    } else if (strcasecmp(token, "SHELL") == 0) {

      shell_t *shell;
      shell = calloc(1, sizeof(shell_t));
      LL_APPEND(wasora.shells, shell);

///kw+SHELL+usage <print_format> [ expr_1 expr_2 ... expr_n ]
      if (wasora_parser_string_format(&shell->format, &shell->n_args) != WASORA_PARSER_OK) {
        return WASORA_PARSER_ERROR;
      }

      shell->arg = calloc(shell->n_args, sizeof(expr_t));
      for (i = 0; i < shell->n_args; i++) {
        if (wasora_parser_expression(&shell->arg[i]) != WASORA_PARSER_OK) {
          return WASORA_PARSER_ERROR;
        }
      }

      if (wasora_define_instruction(wasora_instruction_shell, shell) == NULL) {
        return WASORA_PARSER_ERROR;
      }

      return WASORA_PARSER_OK;

// --- CALL -----------------------------------------------------
///kw+CALL+desc Call a previously dynamically-loaded user-provided routine.
///kw+CALL+usage CALL <name>
    } else if (strcasecmp(token, "CALL") == 0) {

      call_t *call;
      call = calloc(1, sizeof(call_t));
      LL_APPEND(wasora.calls, call);


      if (wasora_parser_string(&call->name) != WASORA_PARSER_OK) {
        return WASORA_PARSER_ERROR;
      }

      if ((call->function = wasora_get_routine_ptr(call->name)) == NULL) {
        wasora_push_error_message("undefined routine '%s'", call->name);
        return WASORA_PARSER_ERROR;
      }

///kw+CALL+usage [ expr_1 expr_2 ... expr_n ]
      call->n_arguments = 0;
      while ((token = wasora_get_next_token(NULL)) != NULL) {

      expr_t *argument = calloc(1, sizeof(expr_t));
      LL_APPEND(call->arguments, argument);

        wasora_call(wasora_parse_expression(token, argument));
        call->n_arguments++;
      }

      call->x = malloc(call->n_arguments*sizeof(double));

      if (wasora_define_instruction(wasora_instruction_call, call) == NULL) {
        return WASORA_PARSER_ERROR;
      }

      return WASORA_PARSER_OK;

// ----- HISTORY  -----------------------------------------------------------------
///kw+HISTORY+desc Record the time history of a variable as a function of time.
///kw+HISTORY+usage HISTORY
    } else if ((strcasecmp(token, "HISTORY") == 0)) {

      history_t *history;
      history = calloc(1, sizeof(history_t));
      LL_APPEND(wasora.histories, history);

///kw+HISTORY+usage <variable>
      // el nombre de la variable
      if (wasora_parser_variable(&history->variable)) {
        return WASORA_PARSER_ERROR;
      }

///kw+HISTORY+usage <function>
      // el nombre de la funcion
      if ((token = wasora_get_next_token(NULL)) == NULL) {
        wasora_push_error_message("expected function name");
        return WASORA_PARSER_ERROR;
      }

      if ((history->function = wasora_define_function(token, 1)) == NULL) {
        return WASORA_PARSER_ERROR;
      }
      // proposed by rvignolo
//      history->function->arg_name = malloc(1 * sizeof(char *));
//      history->function->arg_name[0] = strdup(wasora.special_vars.t->name);

      if (wasora_define_instruction(wasora_instruction_history, history) == NULL) {
        return WASORA_PARSER_ERROR;
      }

      return WASORA_PARSER_OK;

    // ----- PARAMETRIC  -----------------------------------------------------------------
///kw+PARAMETRIC+desc Systematically sweep a zone of the parameter space, i.e. perform a parametric run.
    } else if ((strcasecmp(token, "PARAMETRIC") == 0)) {

      varlist_t *varlist = NULL;
      varlist_t *varitem, *tmp;
      
      double xi;
      // suponemos que type = linear
      wasora.parametric.type = parametric_linear;
      wasora.parametric_mode = 1;

///kw+PARAMETRIC+usage PARAMETRIC
///kw+PARAMETRIC+usage <var_1> [ ... <var_n> ]
      while ((token = wasora_get_next_token(NULL)) != NULL) {
        
        if (strcasecmp(token, "TYPE") == 0) {
///kw+PARAMETRIC+usage [ TYPE { linear  | logarithmic  | random  | gaussianrandom  | sobol  | niederreiter  | halton  | reversehalton } ]
          char *keywords[] = {"linear", "logarithmic", "random", "gaussianrandom", "sobol", "niederreiter", "halton", "reversehalton", ""};
          int values[] = {parametric_linear,
                          parametric_logarithmic,
                          parametric_random,
                          parametric_gaussianrandom,
                          parametric_sobol,
                          parametric_niederreiter,
                          parametric_halton,
                          parametric_reversehalton,
                          0};
          
          wasora_call(wasora_parser_keywords_ints(keywords, values, (int *)&wasora.parametric.type));
          
///kw+PARAMETRIC+usage [ MIN <num_expr_1> ... <num_expr_n> ]
        } else if (strcasecmp(token, "MIN") == 0) {

          if (wasora.parametric.dimensions == 0) {
            wasora_push_error_message("MIN before variables, cannot determine number of arguments");
            return WASORA_PARSER_ERROR;
          }

          wasora_call(wasora_parser_expressions(&wasora.parametric.range.min, wasora.parametric.dimensions));

///kw+PARAMETRIC+usage [ MAX <num_expr_1> ... <num_expr_n> ]
        } else if (strcasecmp(token, "MAX") == 0) {

          if (wasora.parametric.dimensions == 0) {
            wasora_push_error_message("MAX before variables, cannot determine number of arguments");
            return WASORA_PARSER_ERROR;
          }

          wasora_call(wasora_parser_expressions(&wasora.parametric.range.max, wasora.parametric.dimensions));
          
///kw+PARAMETRIC+usage [ STEP <num_expr_1> ... <num_expr_n> ]
        } else if (strcasecmp(token, "STEP") == 0) {

          if (wasora.parametric.dimensions == 0) {
            wasora_push_error_message("STEP before variables, cannot determine number of arguments");
            return WASORA_PARSER_ERROR;
          }

          wasora_call(wasora_parser_expressions(&wasora.parametric.range.step, wasora.parametric.dimensions));

///kw+PARAMETRIC+usage [ NSTEPS <num_expr_1> ... <num_expr_n> ]
        } else if (strcasecmp(token, "NSTEPS") == 0) {

          if (wasora.parametric.dimensions == 0) {
            wasora_push_error_message("NSTEPS before variables, cannot determine number of arguments");
            return WASORA_PARSER_ERROR;
          }

          wasora_call(wasora_parser_expressions(&wasora.parametric.range.nsteps, wasora.parametric.dimensions));

///kw+PARAMETRIC+usage [ OUTER_STEPS <num_expr> ]
        } else if (strcasecmp(token, "OUTER_STEPS") == 0) {

          wasora_call(wasora_parser_expression_in_string(&xi));
          if ((wasora.parametric.outer_steps = (int)xi) <= 0) {
            wasora_push_error_message("OUTER_STEPS has to be positive");
            return WASORA_PARSER_ERROR;
          }

///kw+PARAMETRIC+usage [ MAX_DAUGHTERS <num_expr>  ]
        } else if (strcasecmp(token, "MAX_DAUGHTERS") == 0) {

          wasora_call(wasora_parser_expression_in_string(&xi));
          wasora.parametric.max_daughters = (int)(ceil(xi));

///kw+PARAMETRIC+usage [ OFFSET <num_expr> ]
        } else if (strcasecmp(token, "OFFSET") == 0) {
          wasora_call(wasora_parser_expression_in_string(&xi));
          if ((wasora.parametric.offset = (int)(round(xi))) < 0) {
            wasora_push_error_message("expected a non-negative offset");
            return WASORA_PARSER_ERROR;
          }

///kw+PARAMETRIC+usage [ ADIABATIC ]
        } else if (strcasecmp(token, "ADIABATIC") == 0) {
            wasora.parametric.adiabatic = 1;

        } else {

          if (wasora.parametric.range.min != NULL ||
              wasora.parametric.range.max != NULL ||
              wasora.parametric.range.step != NULL  ||
              wasora.parametric.range.nsteps != NULL) {

            wasora_push_error_message("already given MIN, MAX, STEP or NSTEPS, cannot add variable '%s' to the parametric list", token);
            return WASORA_PARSER_ERROR;
          }
          
          
          varitem = calloc(1, sizeof(varlist_t));
          if ((varitem->var = wasora_get_variable_ptr(token)) == NULL) {
            if ((varitem->var = wasora_define_variable(token)) == NULL) {
              return WASORA_PARSER_ERROR;
            }
          } 
          
          LL_APPEND(varlist, varitem);
          wasora.parametric.dimensions++;

        }
      }

      if (wasora.parametric.dimensions == 0) {
        wasora_push_error_message("no parametric variables given");
        return WASORA_PARSER_ERROR;
      }
      
      wasora.parametric.variable = calloc(wasora.parametric.dimensions, sizeof(var_t *));
          
      varitem = varlist;
      for (i = 0; i < wasora.parametric.dimensions; i++) {
        wasora.parametric.variable[i] = varitem->var;
        if ((varitem = varitem->next) == NULL && i != wasora.parametric.dimensions-1) {
          wasora_push_error_message("internal mismatch in number of fit parameter %d", i);
          return WASORA_PARSER_ERROR;
        }
      }
      
      if (wasora_define_instruction(wasora_instruction_parametric, NULL) == NULL) {
        return WASORA_PARSER_ERROR;
      }
      LL_FOREACH_SAFE(varlist, varitem, tmp) {
        LL_DELETE(varlist, varitem);
        free(varitem);
      }
      return WASORA_PARSER_OK;

    // ----- FIT  -----------------------------------------------------------------
    } else if ((strcasecmp(token, "FIT") == 0)) {

///kw+FIT+desc Fit a function of one or more arguments to a set of pointwise-defined data.
///kw+FIT+detail The function with the data has to be point-wise defined
///kw+FIT+detail (i.e. a `FUNCTION` read from a file with inline `DATA`).
///kw+FIT+detail The function to be fitted has to be parametrized with at least one of
///kw+FIT+detail the variables provided after the `VIA` keyword.
///kw+FIT+detail Only the names of the functions have to be given, not the arguments.
///kw+FIT+detail Both functions have to have the same number of arguments.
      
///kw+FIT+usage FIT
      char *sigma_name;
      varlist_t *varlist = NULL;
      varlist_t *varitem;

      wasora.fit_mode = 1;

///kw+FIT+usage <function_to_be_fitted> 
      // la funcion cuyos parametros vamos a ajustar
      if ((token = wasora_get_next_token(NULL)) == NULL) {
        wasora_push_error_message("expected function name");
        return WASORA_PARSER_ERROR;
      }

      if ((wasora.fit.function = wasora_get_function_ptr(token)) == NULL) {
        if (strchr(token, '(') != NULL) {
          wasora_push_error_message("function '%s' undefined (only the function name is to be provided, do not include its arguments)", token);
        } else {
          wasora_push_error_message("function '%s' undefined", token);
        }
        return WASORA_PARSER_ERROR;
      }

///kw+FIT+usage TO <function_with_data>
      if ((token = wasora_get_next_token(NULL)) == NULL) {
        wasora_push_error_message("expected function name");
        return WASORA_PARSER_ERROR;
      }
      if (strcasecmp(token, "TO") != 0) {
        wasora_push_error_message("expected keyword 'TO' instead of '%s'", token);
      }

// la funcion con los datos experimentales
      if ((token = wasora_get_next_token(NULL)) == NULL) {
        wasora_push_error_message("expected function name");
        return WASORA_PARSER_ERROR;
      }
      if ((wasora.fit.data = wasora_get_function_ptr(token)) == NULL) {
        if (strchr(token, '(') != NULL) {
          wasora_push_error_message("function '%s' undefined (only the function name is to be provided, do not include its arguments)", token);
        } else {
          wasora_push_error_message("function '%s' undefined", token);
        }
        return WASORA_PARSER_ERROR;
      }

      if (wasora.fit.function->n_arguments != wasora.fit.data->n_arguments) {
        wasora_push_error_message("function '%s' has %d arguments and '%s' has %d", wasora.fit.function->name, wasora.fit.function->n_arguments, wasora.fit.data->name, wasora.fit.data->n_arguments);
        return WASORA_PARSER_ERROR;
      }

      if ((wasora.fit.n = wasora.fit.data->data_size) == 0) {
        wasora_push_error_message("function '%s' has to be point-wise defined", wasora.fit.data->name);
        return WASORA_PARSER_ERROR;
      }

///kw+FIT+usage VIA
///kw+FIT+detail The initial guess of the solution is given by the initial value of the variables listed in the `VIA` keyword.
      if ((token = wasora_get_next_token(NULL)) == NULL) {
        wasora_push_error_message("expected function name");
        return WASORA_PARSER_ERROR;
      }
      if (strcasecmp(token, "VIA") != 0) {
        wasora_push_error_message("expected keyword 'VIA' instead of '%s'", token);
        return WASORA_PARSER_ERROR;
      }

///kw+FIT+usage <var_1> <var_2> ... <var_n>@
      while ((token = wasora_get_next_token(NULL)) != NULL) {

        // TODO: convergencia por gradiente, incertezas, covarianza, incrementos para gradiente automatico
///kw+FIT+usage [ GRADIENT <expr_1> <expr_2> ... <expr_n> ]@
///kw+FIT+detail Analytical expressions for the gradient of the function to be fitted with respect
///kw+FIT+detail to the parameters to be fitted can be optionally given with the `GRADIENT` keyword.
///kw+FIT+detail If none is provided, the gradient will be computed numerically using finite differences.
        if (strcasecmp(token, "GRADIENT") == 0) {
 
          if (wasora.fit.p == 0) {
            wasora_push_error_message("GRADIENT keyword before parameters");
            return WASORA_PARSER_ERROR;
          }
          
          wasora.fit.gradient = malloc(wasora.fit.p * sizeof(expr_t));
          for (i = 0; i < wasora.fit.p; i++) {
            if ((token = wasora_get_next_token(NULL)) == NULL) {
              wasora_push_error_message("expected an expression");
              return WASORA_PARSER_ERROR;
            }

            wasora_call(wasora_parse_expression(token, &wasora.fit.gradient[i]));
          }

///kw+FIT+usage [ RANGE_MIN <expr_1> <expr_2> ... <expr_j> ]@
///kw+FIT+detail A range over which the residuals are to be minimized can be given with `RANGE_MIN` and `RANGE_MAX`.
///kw+FIT+detail The expressions give the range of the arguments of the functions, not of the parameters.
///kw+FIT+detail For multidimensional fits, the range is an hypercube.
///kw+FIT+detail If no range is given, all the definition points of the function with the data are used for the fit.
        } else if (strcasecmp(token, "RANGE_MIN") == 0) {

          if (wasora.fit.data->n_arguments == 0) {
            wasora_push_error_message("RANGE_MIN before target function, cannot determine number of arguments");
            return WASORA_PARSER_ERROR;
          }

          wasora_call(wasora_parser_expressions(&wasora.fit.range.min, wasora.fit.data->n_arguments));

///kw+FIT+usage [ RANGE_MAX <expr_1> <expr_2> ... <expr_n> ]@
        } else if (strcasecmp(token, "RANGE_MAX") == 0) {

          if (wasora.fit.data->n_arguments == 0) {
            wasora_push_error_message("RANGE_MAX before target function, cannot determine number of arguments");
            return WASORA_PARSER_ERROR;
          }

          wasora_call(wasora_parser_expressions(&wasora.fit.range.max, wasora.fit.data->n_arguments));
          
///kw+FIT+usage [ DELTAEPSREL <expr> ]
///kw+FIT+detail Convergence can be controlled by giving the relative and absolute tolreances with
///kw+FIT+detail `DELTAEPSREL` (default `DEFAULT_NLIN_FIT_EPSREL`) and `DELTAEPSABS` (default `DEFAULT_NLIN_FIT_EPSABS`),
///kw+FIT+detail and with the maximum number of iterations `MAX_ITER` (default DEFAULT_NLIN_FIT_MAX_ITER).
          
        } else if (strcasecmp(token, "DELTAEPSREL") == 0) {

          wasora_call(wasora_parser_expression(&wasora.fit.deltaepsrel));

///kw+FIT+usage [ DELTAEPSABS <expr> ]
        } else if (strcasecmp(token, "DELTAEPSABS") == 0) {

          wasora_call(wasora_parser_expression(&wasora.fit.deltaepsabs));

///kw+FIT+usage [ MAX_ITER <expr> ]@
        } else if (strcasecmp(token, "MAX_ITER") == 0) {

          double xi;
          
          wasora_call(wasora_parser_expression_in_string(&xi));
          if ((wasora.fit.max_iter = (int)xi) < 0) {
            wasora_push_error_message("expected a positive integer for MAX_ITER");
            return WASORA_PARSER_ERROR;
          }

///kw+FIT+usage [ VERBOSE ]
        } else if (strcasecmp(token, "VERBOSE") == 0) {
///kw+FIT+detail If the optional keyword `VERBOSE` is given, some data of the intermediate steps is written in the standard output.

          wasora.fit.verbose = 1;

///kw+FIT+usage [ RERUN | DO_NOT_RERUN ]@
        } else if (strcasecmp(token, "DO_NOT_RERUN") == 0 || strcasecmp(token, "NORERUN") == 0) {

          wasora.fit.norerun = 1;

        } else if (strcasecmp(token, "RERUN") == 0) {

          wasora.fit.norerun = 0;

        } else {

          varitem = calloc(1, sizeof(varlist_t));
          if ((varitem->var = wasora_get_variable_ptr(token)) == NULL) {
            wasora_push_error_message("unknown variable '%s'", token);
            return WASORA_PARSER_ERROR;
          } 
          
          LL_APPEND(varlist, varitem);
          wasora.fit.p++;

        }
      }

      if (wasora.fit.p == 0) {
        wasora_push_error_message("no fit parameters given");
        return WASORA_PARSER_ERROR;
      }

      wasora.fit.param = malloc(wasora.fit.p * sizeof(var_t *));
      wasora.fit.sigma = malloc(wasora.fit.p * sizeof(var_t *));
      varitem = varlist;
      for (i = 0; i < wasora.fit.p; i++) {
        wasora.fit.param[i] = varitem->var;
        
        sigma_name = malloc(strlen(varitem->var->name)+strlen("sigma_")+8);
        snprintf(sigma_name, strlen(varitem->var->name)+strlen("sigma_")+8, "sigma_%s", varitem->var->name);
        if ((wasora.fit.sigma[i] = wasora_define_variable(sigma_name)) == NULL) {
          return WASORA_PARSER_ERROR;
        }
        free(sigma_name);
        
        if ((varitem = varitem->next) == NULL && i != wasora.fit.p-1) {
          wasora_push_error_message("internal mismatch in number of fit parameter %d", i);
          return WASORA_PARSER_ERROR;
        }
      }

      if (wasora.fit.algorithm == NULL) {
        wasora.fit.algorithm = DEFAULT_NLIN_FIT_METHOD;
      }
      if (wasora.fit.max_iter == 0) {
        wasora.fit.max_iter = DEFAULT_NLIN_FIT_MAX_ITER;
      }

      return WASORA_PARSER_OK;

    // ----- MINIMIZE  -----------------------------------------------------------------
///kw+MINIMIZE+usage MINIMIZE
///kw+MINIMIZE+desc Find the combination of arguments that give a (relative) minimum of a function.
    } else if (strcasecmp(token, "MINIMIZE") == 0 || strcasecmp(token, "OPTIMIZE") == 0) {

      // ponemos esto en true porque normalmente no queremos que imprima todo el chorizo
      wasora.min_mode = 1;

///kw+MINIMIZE+usage <function>@
///kw+FIT+detail The combination of arguments that minimize the function are computed and stored in the variables.
///kw+FIT+detail So if `f(x,y)` is to be minimized, after a `MINIMIZE f` both `x` and `y` would have the appropriate values.
      if ((token = wasora_get_next_token(NULL)) == NULL) {
        wasora_push_error_message("expected a function");
        return WASORA_PARSER_ERROR;
      }
      if ((wasora.min.function = wasora_get_function_ptr(token)) == NULL) {
        if (strchr(token, '(') != NULL) {
          wasora_push_error_message("function '%s' undefined (only the function name should be provided, do not include its arguments)", token);
        } else {
          wasora_push_error_message("function '%s' undefined", token);
        }
        return WASORA_PARSER_ERROR;
      }

      wasora.min.n = wasora.min.function->n_arguments;
      wasora.min.x = calloc(wasora.min.n, sizeof(var_t *));
      for (i = 0; i < wasora.min.n; i++) {
        wasora.min.x[i] = wasora.min.function->var_argument[i];
      }

/*        
        } else if (strcasecmp(token, "SIMAN_EFUNC") == 0) {
          if ((token = wasora_get_next_token(NULL)) == NULL) {
            wasora_push_error_message("expected a routine");
            return WASORA_PARSER_ERROR;
          }
          if ((wasora.min.siman_Efunc = (double (*)(void *)) wasora_get_routine_ptr(token)) == NULL) {
            wasora_push_error_message("unknown routine '%s'", token);
            return WASORA_PARSER_ERROR;
          }
*/
        
      while ((token = wasora_get_next_token(NULL)) != NULL) {
///kw+MINIMIZE+usage [ METHOD {
///kw+FIT+detail The details of the method used can be found in [GSL’s documentation](https:\/\/www.gnu.org/software/gsl/doc/html/multimin.html).
///kw+FIT+detail Some of them use derivatives and some of them do not.
///kw+FIT+detail Default method is `DEFAULT_MINIMIZER_METHOD`, which does not need derivatives.
        if (strcasecmp(token, "METHOD") == 0 || strcasecmp(token, "ALGORITHM") == 0) {
          if ((token = wasora_get_next_token(NULL)) == NULL) {
            wasora_push_error_message("expected algorithm name");
            return WASORA_PARSER_ERROR;
          }

///kw+MINIMIZE+usage nmsimplex2 |
          } else if (strcasecmp(token, "nmsimplex2") == 0) {
            wasora.min.f_type = gsl_multimin_fminimizer_nmsimplex2;
///kw+MINIMIZE+usage nmsimplex |
          } else if (strcasecmp(token, "nmsimplex") == 0) {
            wasora.min.f_type = gsl_multimin_fminimizer_nmsimplex;
///kw+MINIMIZE+usage nmsimplex2rand |
          } else if (strcasecmp(token, "nmsimplex2rand") == 0) {
            wasora.min.f_type = gsl_multimin_fminimizer_nmsimplex2rand;
///kw+MINIMIZE+usage conjugate_fr |
          if (strcasecmp(token, "conjugate_fr") == 0) {
            wasora.min.fdf_type = gsl_multimin_fdfminimizer_conjugate_fr;
///kw+MINIMIZE+usage conjugate_pr |
          } else if (strcasecmp(token, "conjugate_pr") == 0) {
            wasora.min.fdf_type = gsl_multimin_fdfminimizer_conjugate_pr;
///kw+MINIMIZE+usage vector_bfgs2 |
          } else if (strcasecmp(token, "vector_bfgs2") == 0) {
            wasora.min.fdf_type = gsl_multimin_fdfminimizer_vector_bfgs2;
///kw+MINIMIZE+usage vector_bfgs |
          } else if (strcasecmp(token, "vector_bfgs") == 0) {
            wasora.min.fdf_type = gsl_multimin_fdfminimizer_vector_bfgs;
///kw+MINIMIZE+usage steepest_descent}@
          } else if (strcasecmp(token, "steepest_descent") == 0) {
            wasora.min.fdf_type = gsl_multimin_fdfminimizer_steepest_descent;
/*
          } else if (strcasecmp(token, "genetic") == 0) {
            wasora.min.genetic = 1;
          } else if (strcasecmp(token, "pure_genetic") == 0) {
            wasora.min.genetic = 2;
*/
          } else {
            wasora_push_error_message("unknown minimization method '%s'", token);
            return WASORA_PARSER_ERROR;
          }

///kw+MINIMIZE+usage [ GRADIENT <expr_1> <expr_2> ... <expr_n> ]@
        } else if (strcasecmp(token, "GRADIENT") == 0) {

          wasora.min.gradient = malloc(wasora.min.n * sizeof(expr_t));
          for (i = 0; i < wasora.min.n; i++) {
            if ((token = wasora_get_next_token(NULL)) == NULL) {
              wasora_push_error_message("expected an expression");
              return WASORA_PARSER_ERROR;
            }

            wasora_call(wasora_parse_expression(token, &wasora.min.gradient[i]));
          }

///kw+MINIMIZE+usage [ GUESS <expr_1> <expr_2> ... <expr_n> ]@
        } else if (strcasecmp(token, "GUESS") == 0) {

          wasora.min.guess = malloc(wasora.min.n * sizeof(expr_t));
          for (i = 0; i < wasora.min.n; i++) {
            if ((token = wasora_get_next_token(NULL)) == NULL) {
              wasora_push_error_message("expected an expression");
              return WASORA_PARSER_ERROR;
            }

            wasora_call(wasora_parse_expression(token, &wasora.min.guess[i]));
          }

///kw+MINIMIZE+usage [ MIN <expr_1> <expr_2> ... <expr_n> ]@
        } else if (strcasecmp(token, "MIN") == 0) {

          if (wasora.min.n == 0) {
            wasora_push_error_message("MIN before target function, cannot determine number of arguments");
            return WASORA_PARSER_ERROR;
          }

          if (wasora_parser_expressions(&wasora.min.range.min, wasora.min.n) != WASORA_PARSER_OK) {
            return WASORA_PARSER_ERROR;
          }

///kw+MINIMIZE+usage [ MAX <expr_1> <expr_2> ... <expr_n> ]@
        } else if (strcasecmp(token, "MAX") == 0) {

          if (wasora.min.n == 0) {
            wasora_push_error_message("MAX before target function, cannot determine number of arguments");
            return WASORA_PARSER_ERROR;
          }

          if (wasora_parser_expressions(&wasora.min.range.max, wasora.min.n) != WASORA_PARSER_OK) {
            return WASORA_PARSER_ERROR;
          }

///kw+MINIMIZE+usage [ STEP <expr_1> <expr_2> ... <expr_n> ]@
        } else if (strcasecmp(token, "STEP") == 0) {

          if (wasora.min.n == 0) {
            wasora_push_error_message("STEP before target function, cannot determine number of arguments");
            return WASORA_PARSER_ERROR;
          }
          
          // los algoritmos con derivadas necesitan un solo step
          // los sin derivadas necesitan n steps
          // si fdf_type no es null, entonces estamos con derivadas
          // igual despues chequeamos que no se mezclen
          wasora.min.n_steps = (wasora.min.f_type != NULL) ? wasora.min.n : 1;
          wasora.min.range.step = malloc(wasora.min.n_steps * sizeof(expr_t));

          if (wasora_parser_expressions(&wasora.min.range.step, wasora.min.n_steps) != WASORA_PARSER_OK) {
            return WASORA_PARSER_ERROR;
          }
        } else if (strcasecmp(token, "VERBOSE") == 0) {
          wasora.min.verbose = 1;

        } else if (strcasecmp(token, "NORERUN") == 0) {
          wasora.min.norerun = 1;

        } else {

///kw+MINIMIZE+usage [ MAX_ITER <expr> ]
///kw+MINIMIZE+usage [ TOL <expr> ]
///kw+MINIMIZE+usage [ GRADTOL <expr> ]@
///kw+MINIMIZE+usage [ VERBOSE ]
///kw+MINIMIZE+usage [ NORERUN ]@
          // en una sola linea para el generador del lexer de pygments
          char *keywords[] = {"MAX_ITER", "TOL", "GRADTOL", ""};
          //, "POPULATION", "GA_STEPS"};
          expr_t *expressions[] = {
            &wasora.min.max_iter,
            &wasora.min.tol,
            &wasora.min.gradtol,
//            &wasora.min.population,
//            &wasora.min.ga_steps
            NULL,
          };

          if (wasora_parser_match_keyword_expression(token, keywords, expressions, sizeof(expressions)/sizeof(expr_t *)) == WASORA_PARSER_UNHANDLED) {
            wasora_push_error_message("unknown keyword '%s'", token);
            return WASORA_PARSER_ERROR;
          }
        }
      }


      if (wasora.min.function == NULL && wasora.min.siman_Efunc == NULL) {
        wasora_push_error_message("nothing to optimize!");
        return WASORA_PARSER_ERROR;
      }

      if (wasora.min.genetic != 0) {
        if (wasora.min.range.min == NULL) {
          wasora_push_error_message("need a MIN keyword for genetic algorithms");
          return WASORA_PARSER_ERROR;
        }

        if (wasora.min.range.max == NULL) {
          wasora_push_error_message("need a MAX keyword for genetic algorithms");
          return WASORA_PARSER_ERROR;
        }
      }

      if (wasora.min.fdf_type == NULL && wasora.min.f_type == NULL
       && wasora.min.siman_type == NULL && wasora.min.genetic == 0) {
        wasora.min.f_type = DEFAULT_MINIMIZER_METHOD;
      }
/*
      if (wasora.min.max_iter.n_tokens == 0) {
        parse_expr(DEFAULT_MINIMIZER_MAX_ITER, &wasora.min.max_iter);
      }

      if (wasora.min.n_steps == 0) {
        wasora.min.n_steps = (wasora.min.fdf_type != NULL) ? 1 : wasora.min.n;
      } else {
        if (wasora.min.fdf_type != NULL && wasora.min.n_steps != 1) {
          wasora_push_error_message(only one step size is expected when using a derivative-based algorithm");
          return WASORA_PARSER_ERROR;
        }
        if (wasora.min.f_type != NULL && wasora.min.n_steps != wasora.min.n) {
          wasora_push_error_message(expected %d step sizes when using a non-derivative-based algorithm", wasora.min.n);
          return WASORA_PARSER_ERROR;
        }
      }
  */

      if (wasora.min.siman_Efunc != NULL) {
        if (wasora.min.siman_init == NULL) {
          wasora_push_error_message("a siman init routine has to be provided");
          return WASORA_PARSER_ERROR;
        }
        if (wasora.min.siman_step == NULL) {
          wasora_push_error_message("a siman step routine has to be provided");
          return WASORA_PARSER_ERROR;
        }
        if (wasora.min.siman_copy == NULL) {
          wasora_push_error_message("a siman copy routine has to be provided");
          return WASORA_PARSER_ERROR;
        }
        if (wasora.min.siman_copy_construct == NULL) {
          wasora_push_error_message("a siman copy_construct routine has to be provided");
          return WASORA_PARSER_ERROR;
        }
        if (wasora.min.siman_destroy == NULL) {
          wasora_push_error_message("a siman destroy routine has to be provided");
          return WASORA_PARSER_ERROR;
        }
      }

      return WASORA_PARSER_OK;

    // ----- PHASE_SPACE -----------------------------------------------------------
    } else if (strcasecmp(token, "PHASE_SPACE") == 0) {
///kw+PHASE_SPACE+usage PHASE_SPACE { <vars> | <vectors> | <matrices> }
///kw+PHASE_SPACE+desc Define which variables, vectors and/or matrices belong to the phase space of the DAE system to be solved.

      var_t *variable;
      vector_t *vector;
      matrix_t *matrix;
      char *buffer;

      if (wasora_dae.dimension != 0) {
        wasora_push_error_message("PHASE_SPACE keyword already given");
        return WASORA_PARSER_ERROR;
      }
#ifdef HAVE_IDA
      if (sizeof(realtype) != sizeof(double)) {
        wasora_push_error_message("\nSUNDIALS was compiled using a different word size than wasora.\nPlease recompile with double precision floating point arithmetic.");
        return WASORA_PARSER_ERROR;
      }
#endif

      i = 0;
      while ((token = wasora_get_next_token(NULL)) != NULL) {

        phase_object_t *phase_object = calloc(1, sizeof(phase_object_t));
        LL_APPEND(wasora_dae.phase_objects, phase_object);

        if ((vector = wasora_get_vector_ptr(token)) != NULL) {

          expr_t *size_expr;
          
          phase_object->offset = i;
          phase_object->vector = vector;
          phase_object->name = vector->name;

          buffer = malloc(strlen(vector->name)+8);
          sprintf(buffer, "%s_dot", vector->name);
          
          // chapuceria, aca habria que duplicar un expr_t completo
          size_expr = malloc(sizeof(expr_t));
          wasora_parse_expression(vector->size_expr->string, size_expr);
          phase_object->vector_dot = wasora_define_vector(buffer, vector->size, size_expr, NULL);
          
          free(buffer);

        } else if ((matrix = wasora_get_matrix_ptr(token)) != NULL) {

          expr_t *rows_expr;
          expr_t *cols_expr;
          
          phase_object->offset = i;
          phase_object->matrix = matrix;
          phase_object->name = matrix->name;

          buffer = malloc(strlen(matrix->name)+8);
          sprintf(buffer, "%s_dot", matrix->name);
          
          // chapuceria, aca habria que duplicar los expr_ts completos
          rows_expr = malloc(sizeof(expr_t));
          wasora_parse_expression(matrix->rows_expr->string, rows_expr);
          cols_expr = malloc(sizeof(expr_t));
          wasora_parse_expression(matrix->cols_expr->string, cols_expr);
          
          phase_object->matrix_dot = wasora_define_matrix(buffer, matrix->rows, rows_expr, matrix->cols, cols_expr, NULL);
          
          free(buffer);

        } else {

          if ((variable = wasora_get_or_define_variable_ptr(token)) == NULL) {
            return WASORA_PARSER_ERROR;
          }

          phase_object->variable = variable;
          phase_object->name = variable->name;

          buffer = malloc(strlen(variable->name)+8);
          sprintf(buffer, "%s_dot", variable->name);
          phase_object->variable_dot = wasora_define_variable(buffer);
          free(buffer);

        }
      }

      return WASORA_PARSER_OK;
              
    // ----- DIFFERENTIAL -----------------------------------------------------------
    } else if (strcasecmp(token, "DIFFERENTIAL") == 0) {
///kw+DIFFERENTIAL+desc Explicitly mark variables, vectors or matrices as “differential” to compute intial conditions of DAE systems.
///kw+DIFFERENTIAL+usage DIFFERENTIAL { <var_1> <var_2> ... | <vector_1> <vector_2> ... | <matrix_1> <matrix_2> ... }
      
      phase_object_t *phase_object;
      
      while ((token = wasora_get_next_token(NULL)) != NULL) {
        LL_FOREACH(wasora_dae.phase_objects, phase_object) {
          if ((phase_object->variable != NULL && strcmp(token, phase_object->variable->name) == 0) ||
              (phase_object->vector   != NULL && strcmp(token, phase_object->vector->name) == 0) ||
              (phase_object->matrix   != NULL && strcmp(token, phase_object->matrix->name) == 0) ) {
            phase_object->differential = 1;
          }
        }          
      }
      
      return WASORA_PARSER_OK;
      
      // --- DAE -----------------------------------------------------
    } else if (token[0] == '0' || strstr(wasora.line, ".=") != NULL) {
///kw+_.=+desc Add an equation to the DAE system to be solved in the phase space spanned by `PHASE_SPACE`.
///kw+_.=+usage { 0[(i[,j]][<imin:imax[;jmin:jmax]>] | <expr1> } .= <expr2>
      char *equation;
      char *dummy;
      char *bracket;
      phase_object_t *phase_object;
      
      dae_t *dae = calloc(1, sizeof(dae_t));
      
      if (wasora_dae.daes == NULL) {
        // si es la primera dae, anotamos a que instruccion corresponde
        if ((wasora_dae.instruction = wasora_define_instruction(&wasora_instruction_dae, NULL)) == NULL) {
          return WASORA_PARSER_ERROR;
        }
        // y ponemos una finite-state machine en modo "leyendo daes"
//        wasora_dae.reading_daes = 1;
      }
      
      
      LL_APPEND(wasora_dae.daes, dae);

      if (token[0] == '0') {
        // ecuacion escrita en forma implicita
        dae->equation_type |= EQN_FLAG_IMPLICIT;
        if ((dummy = strchr((char *)(token+strlen(token)+1), '=')) == NULL) {
          wasora_push_error_message("expecting equal sign after keyword 0");
          return WASORA_PARSER_ERROR;
        }
        equation = strdup(dummy+1);

        if (strncmp(token, "0(i,j)", 6) == 0) {
          //  ecuacion implicita y matricial
          dae->equation_type |= EQN_FLAG_MATRICIAL;
          if ((dae->matrix = wasora_get_first_matrix(equation)) == NULL) {
            wasora_push_error_message("matrix equations need at least one matrix in the expression");
            return WASORA_PARSER_ERROR;
          }
          
          if (token[6] == '<') {
            wasora_call(wasora_parse_range(token+6, '<', ':', ';', &dae->expr_i_min, &dae->expr_i_max));
            wasora_call(wasora_parse_range(strchr(dummy+6, ';'), ';', ':', '>', &dae->expr_j_min, &dae->expr_j_max));
          }
          
        } else if (strncmp(token, "0(i)", 4) == 0) {
          // ecuacion implicita y vectorial
          dae->equation_type |= EQN_FLAG_VECTORIAL;
          if ((dae->vector = wasora_get_first_vector(equation)) == NULL) {
            wasora_push_error_message("vectorial equations need at least one vector in the expression");
            return WASORA_PARSER_ERROR;
          }

          if (token[4] == '<') {
            wasora_call(wasora_parse_range(token+4, '<', ':', '>', &dae->expr_i_min, &dae->expr_i_max));
          }
          
        } else {
          dae->equation_type |= EQN_FLAG_SCALAR;
        }
        
        

      } else if (strstr(wasora.line, ".=") != NULL) {

        // el tipo puso una ecuacion explicita
        dae->equation_type |= EQN_FLAG_EXPLICIT;

        if (strchr((char *)(token+strlen(token)+1), '=') == NULL) {
          wasora_push_error_message("syntax error, the equal sign should come right after the dot with no spaces between them");
          return WASORA_PARSER_ERROR;
        }
        dummy = strchr((char *)(token+strlen(token)+1), '=')+1;
        equation = malloc(strlen(dummy)+strlen(token)+16);

        // si en el miembro izquierdo hay un vector o una matriz, entonces araca
        if ((dae->vector = wasora_get_first_vector(token)) != NULL) {
          dae->equation_type |= EQN_FLAG_VECTORIAL;
          if ((bracket = strchr(token, '<')) != NULL) {
            wasora_call(wasora_parse_range(bracket, '<', ':', '>', &dae->expr_i_min, &dae->expr_i_max));
            *bracket = '\0';
          }
          
        } else if ((dae->matrix = wasora_get_first_matrix(token)) != NULL) {
          dae->equation_type |= EQN_FLAG_MATRICIAL;
          if ((bracket = strchr(token, '<')) != NULL) {
            wasora_call(wasora_parse_range(bracket, '<', ':', ';', &dae->expr_i_min, &dae->expr_i_max));
            wasora_call(wasora_parse_range(strchr(bracket, ';'), ';', ':', '>', &dae->expr_j_min, &dae->expr_j_max));
            *bracket = '\0';
          }
          
        } else {
          dae->equation_type |= EQN_FLAG_SCALAR;
        }

        // pasamos el miembro derecho para el otro lado cambiado de signo
        sprintf(equation, "(%s)-(%s)", token, dummy);

      } else {
        return WASORA_PARSER_ERROR;
      }


      // vemos si es diferencial o algebraica: si aparece alguna derivada en la
      // expresion implicita, es diferencial
      if ((dummy = wasora_get_first_dot(equation)) != NULL) {
        int found = 0;
        dae->equation_type |= EQN_FLAG_DIFFERENTIAL;
        LL_FOREACH(wasora_dae.phase_objects, phase_object) {
          if (strcmp(dummy, phase_object->name) == 0) {
            phase_object->differential = 1;
            found = 1;
          }
        }
        free(dummy);
        if (found == 0) {
          wasora_push_error_message("requested derivative of object '%s' but it is not in the phase space", dummy);
          return WASORA_PARSER_ERROR;
        }

      } else {
        dae->equation_type |= EQN_FLAG_ALGEBRAIC;
      }

      // la ecuacion propiamente dicha      
      wasora_call(wasora_parse_expression(equation, &dae->residual));

/*
      if (wasora.i_dae == wasora_dae.dimension) {
        // si es la ultima verificamos que no hubo instrucciones en el medio
        if (wasora.last_defined_instruction != wasora_dae.instruction) {
          wasora_push_error_message("cannot have instructions within DAEs\n");
          return WASORA_PARSER_ERROR;
        }
        // y sacamos la finite-state machine del modo "leyendo daes"
        wasora_dae.reading_daes = 0;
      }
*/
      free(equation);

      return WASORA_PARSER_OK;

    // ----- ASSIGNMENT -----------------------------------------------------------
    } else if ((token = wasora_get_next_token(NULL)) != NULL && strcmp(token, "=") == 0) {
///kw+=+desc Assign an expression to a variable, a vector or a matrix.
///kw+=+usage <var>[ [<expr_tmin>, <expr_tmax>] | @<expr_t> ] = <expr>
///kw+=+usage <vector>(<expr_i>)[<expr_i_min, expr_i_max>] [ [<expr_tmin>, <expr_tmax>] | @<expr_t> ] = <expr>
///kw+=+usage <matrix>(<expr_i>,<expr_j>)[<expr_i_min, expr_i_max; expr_j_min, expr_j_max>] [ [<expr_tmin>, <expr_tmax>] | @<expr_t> ] = <expr>

      // metemos de prepo un nuevo elemento en la linked list
      assignment_t *assignment = calloc(1, sizeof(assignment_t));
      LL_APPEND(wasora.assignments, assignment);
      
      wasora_call(wasora_parse_assignment(wasora.line, assignment));
      
      if (wasora_define_instruction(wasora_instruction_assignment, assignment) == NULL) {
        return WASORA_PARSER_ERROR;
      }

      return WASORA_PARSER_OK;

    } else {
      // si no entendimos la linea, probamos pasarsela al parser de mallas
      strcpy(line, wasora.line);
      return wasora_mesh_parse_line(line);    
    }

  }

  return WASORA_PARSER_UNHANDLED;

}


char *wasora_get_nth_token(char *string, int n) {

  char *backup;
  char *token;
  char *desired_token;
  int i;

  backup = strdup(string);

  if ((token = strtok(backup, UNQUOTED_DELIM)) == NULL) {
    free(backup);
    return NULL;
  }
  i = 1;

  while (i < n) {
    if ((token = strtok(NULL, UNQUOTED_DELIM)) == NULL) {
      free(backup);
      return NULL;
    }
    i++;
  }

  desired_token = strdup(token);
  free(backup);

  return desired_token;


}
