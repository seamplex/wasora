#include <dlfcn.h>
#include <ctype.h>
#include <string.h>

#include "wasora.h"

// los traemos de algebra.c
extern const char factorseparators[];

void wasora_realloc_variable_ptr(var_t *var, double *newptr, int copy_contents) {
  
  // si copy_contents es true copiamos el contenido del vector de wasora
  // antes de tirar el apuntador a la basura
  if (copy_contents) {
    *newptr = wasora_value(var);
  }
  
  // si el puntero es de wasora, lo liberamos
  if (var->realloced == 0) {
    free(wasora_value_ptr(var));
  }
  
  var->realloced = 1;
  wasora_value_ptr(var) = newptr;
  
  return;
}

void wasora_realloc_vector_ptr(vector_t *vector, double *newptr, int copy_contents) {
  
  double *oldptr = gsl_vector_ptr(wasora_value_ptr(vector), 0);

  // si copy_contents es true copiamos el contenido del vector de wasora
  // antes de tirar el apuntador a la basura
  if (copy_contents) {
    memcpy(newptr, oldptr, vector->size * sizeof(double));
  }

  // si el puntero es de wasora, lo liberamos
  if (vector->realloced == 0) {
    if (wasora_value_ptr(vector)->stride != 1) {
      wasora_push_error_message("vector '%s' cannot be realloced: stride not equal to 1", vector->name);
      wasora_runtime_error();
    }
    if (wasora_value_ptr(vector)->owner == 0) {
      wasora_push_error_message("vector '%s' cannot be realloced: not the data owner", vector->name);
      wasora_runtime_error();
    }
    if (wasora_value_ptr(vector)->block->data != wasora_value_ptr(vector)->data) {
      wasora_push_error_message("vector '%s' cannot be realloced: data not pointing to block", vector->name);
      wasora_runtime_error();
    }

    free(oldptr);
  }
  
  vector->realloced = 1;
  wasora_value_ptr(vector)->data = newptr;
  
  return;
}

void wasora_realloc_matrix_ptr(matrix_t *matrix, double *newptr, int copy_contents) {
  
  double *oldptr = gsl_matrix_ptr(wasora_value_ptr(matrix), 0, 0);
  
  // si copy_contents es true copiamos el contenido del vector de wasora
  // antes de tirar el apuntador a la basura
  if (copy_contents) {
    memcpy(newptr, oldptr, matrix->rows*matrix->cols * sizeof(double));
  }
  
  // si el puntero es de wasora, lo liberamos
  if (matrix->realloced == 0) {
    free(oldptr);
  }
  
  matrix->realloced = 1;
  wasora_value_ptr(matrix)->data = newptr;
  
  return;
}

var_t *wasora_define_variable(char *name) {

  var_t *var;

  if (wasora_check_name(name) != WASORA_PARSER_OK) {
    if ((var = wasora_get_variable_ptr(name)) != NULL) {
      wasora_pop_error_message();
      return var;
    } else {
      return NULL;
    }
  }

  var = calloc(1, sizeof(var_t));
  var->name = strdup(name);
  wasora_value_ptr(var) = calloc(1, sizeof(double));
  var->initial_transient = calloc(1, sizeof(double));
  var->initial_static = calloc(1, sizeof(double));
  HASH_ADD_KEYPTR(hh, wasora.vars, var->name, strlen(var->name), var);

  return var;
}

vector_t *wasora_define_vector(char *name, int size, expr_t *size_expr, expr_t *datas) {

  vector_t *vector;
  char *dummy;
  
  // como hay cosas que pueden venir de physical names que pueden tener espacios, los
  // reemplazamos por underscores
  while ((dummy = strchr(name, ' ')) != NULL) {
    *dummy = '_';
  }

  if (wasora_check_name(name) != WASORA_PARSER_OK) {
    return NULL;
  }

  vector = calloc(1, sizeof(vector_t));
  vector->name = strdup(name);
  vector->size_expr = size_expr;
  if (size != 0) {
    vector->size = size;
  }
  vector->datas = datas;

  HASH_ADD_KEYPTR(hh, wasora.vectors, vector->name, strlen(vector->name), vector);

  return vector;

}

matrix_t *wasora_define_matrix(char *name, int rows, expr_t *rows_expr, int cols, expr_t *cols_expr, expr_t *datas) {

  matrix_t *matrix;

  if (wasora_check_name(name) != WASORA_PARSER_OK) {
      return NULL;
    }

    matrix = calloc(1, sizeof(matrix_t));
    matrix->name = strdup(name);
  matrix->rows_expr = rows_expr;
  matrix->cols_expr = cols_expr;
  
  matrix->datas = datas;

  if (rows != 0 && cols != 0) {
    matrix->initialized = 1;
    matrix->rows = rows;
    matrix->cols = cols;
    
    wasora_value_ptr(matrix) = gsl_matrix_calloc(rows, cols);
    matrix->initial_transient = gsl_matrix_calloc(rows, cols);
    matrix->initial_static = gsl_matrix_calloc(rows, cols);
  }

  HASH_ADD_KEYPTR(hh, wasora.matrices, matrix->name, strlen(matrix->name), matrix);

  return matrix;

}

// ojo! esto es una mezcla de decisiones de diseno que hay que
// revisar, porque la idea es que los define_algo tomen como argumento
// lo que despues se va a meter en la estructura administrativa para
// que los callers no tengan por que saber que hay adentro de las
// estructuras. Pero esta es tan complicada que no conviene pedir
// cualquier cosa que se pueda meter adentro por argumento, asi que
// pedimos solamente el nombre y la cantidad de argumentos y que el caller
// rellene lo que le falta de la estrutura que devuelve el define_function
function_t *wasora_define_function(const char *name, int n_arguments) {

  function_t *function;

  if (wasora_check_name(name) != WASORA_PARSER_OK) {
    return NULL;
  }

  function = calloc(1, sizeof(function_t));
  function->name = strdup(name);
  function->n_arguments = n_arguments;

  HASH_ADD_KEYPTR(hh, wasora.functions, function->name, strlen(function->name), function);

  return function;
}

file_t *wasora_define_file(char *name, char *format, int n_args, expr_t *arg, char *mode, int do_not_open) {

  file_t *file;

  HASH_FIND_STR(wasora.files, name, file);
  if (file != NULL) {
    wasora_push_error_message("there already exists a file named '%s'", name);
    return NULL;
  }

  file = calloc(1, sizeof(file_t));
  file->name = strdup(name);
  file->format = strdup(format);
  file->n_args = n_args;
  file->arg = arg;
  if (mode != NULL) {
    file->mode = strdup(mode);
  }
  file->do_not_open = do_not_open;

  HASH_ADD_KEYPTR(hh, wasora.files, file->name, strlen(file->name), file);

  return file;
}

loadable_routine_t *wasora_define_loadable_routine(char *name, void *library) {

  loadable_routine_t *loadable_routine;
  char *error;

  HASH_FIND_STR(wasora.loadable_routines, name, loadable_routine);
  if (loadable_routine != NULL) {
    wasora_push_error_message("there already exists a loadable routine named '%s'", name);
    return NULL;
  }

  loadable_routine = calloc(1, sizeof(loadable_routine_t));
  loadable_routine->name = strdup(name);

  // reseteamos errores como dice el manual
  dlerror();
  // cargamos la rutina
  loadable_routine->routine = dlsym(library, loadable_routine->name);
  // y vemos si se prendio algun error
   if ((error = dlerror()) != NULL) {
     wasora_push_error_message("'%s' when loading routine '%s'", error, loadable_routine->name);
     dlclose(library);
     return NULL;
   }

  HASH_ADD_KEYPTR(hh, wasora.loadable_routines, loadable_routine->name, strlen(loadable_routine->name), loadable_routine);

  return loadable_routine;
}

instruction_t *wasora_define_instruction(int (*routine)(void *), void *argument) {

  instruction_t *instruction;
  instruction = calloc(1, sizeof(instruction_t));

  if (wasora_dae.reading_daes) {
    wasora_push_error_message("cannot have instructions within DAEs");
    return NULL;
  }

  instruction->routine = routine;
  instruction->argument = argument;
  if (wasora.active_conditional_block != NULL) {
    if (wasora.active_conditional_block->else_of == NULL && wasora.active_conditional_block->first_true_instruction == NULL) {
      wasora.active_conditional_block->first_true_instruction = instruction;
    } else if (wasora.active_conditional_block->else_of != NULL && wasora.active_conditional_block->first_false_instruction == NULL) {
      wasora.active_conditional_block->first_false_instruction = instruction;
    }
  }

  wasora.last_defined_instruction = instruction;

  LL_APPEND(wasora.instructions, instruction);

  return instruction;
}


var_t *wasora_get_or_define_variable_ptr(char *name) {
  var_t *dummy;

  if ((dummy = wasora_get_variable_ptr(name)) == NULL) {
    return wasora_define_variable(name);
  }

  return dummy;
}


// vemos si no existe ya una variable, funcion o tabla que se llame name
int wasora_check_name(const char *name) {
  int i;
  char *s;
  
  var_t *var;
  vector_t *vector;
  matrix_t *matrix;
  function_t *function;

  if (name == NULL) {
    wasora_push_error_message("(name is null)");
    return WASORA_PARSER_ERROR;
  }

  if (isdigit(name[0])) {
    wasora_push_error_message("invalid object name '%s' (it starts with a digit)", name);
    return WASORA_PARSER_ERROR;
  }
  
  if ((s = strpbrk(name, factorseparators)) != NULL) {
    wasora_push_error_message("invalid object name '%s' (it contains a '%c')", name, s[0]);
    return WASORA_PARSER_ERROR;
  }
  
  HASH_FIND_STR(wasora.vars, name, var);
  if (var != NULL) {
    wasora_push_error_message("there already exists a variable named '%s'", name);
    return WASORA_PARSER_ERROR;
  }

  HASH_FIND_STR(wasora.vectors, name, vector);
  if (vector != NULL) {
    wasora_push_error_message("there already exists a vector named '%s'", name);
    return WASORA_PARSER_ERROR;
  }

  HASH_FIND_STR(wasora.matrices, name, matrix);
  if (matrix != NULL) {
    wasora_push_error_message("there already exists a matrix named '%s'", name);
    return WASORA_PARSER_ERROR;
  }

  HASH_FIND_STR(wasora.functions, name, function);
  if (function != NULL) {
    wasora_push_error_message("there already exists a function named '%s'", name);
    return WASORA_PARSER_ERROR;
  }

  for (i = 0; i < N_BUILTIN_FUNCTIONS; i++) {
    if (builtin_function[i].name != NULL && strcmp(name, builtin_function[i].name) == 0) {
      wasora_push_error_message("there already exists a built-in function named '%s'", name);
      return WASORA_PARSER_ERROR;
    }
  }

  for (i = 0; i < N_BUILTIN_VECTOR_FUNCTIONS; i++) {
    if (builtin_vectorfunction[i].name != NULL && strcmp(name, builtin_vectorfunction[i].name) == 0) {
      wasora_push_error_message("there already exists a built-in vector function named '%s'", name);
      return WASORA_PARSER_ERROR;
    }
  }

  for (i = 0; i < N_BUILTIN_FUNCTIONALS; i++) {
    if (builtin_functional[i].name != NULL && strcmp(name, builtin_functional[i].name) == 0) {
      wasora_push_error_message("there already exists a built-in functional named '%s'", name);
      return WASORA_PARSER_ERROR;
    }
  }


  return WASORA_PARSER_OK;

}
