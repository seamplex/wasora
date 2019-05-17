/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora pointer lookup functions
 *
 *  Copyright (C) 2009--2013 jeremy theler
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


#include <string.h>

#include "wasora.h"

extern const char factorseparators[];


// devuelve la direccion de la estructura de la variable que se llama name
var_t *wasora_get_variable_ptr(const char *name) {
  var_t *var;
  HASH_FIND_STR(wasora.vars, name, var);
  return var;
}

// devuelve la direccion de la estructura del vector que se llama name
vector_t *wasora_get_vector_ptr(const char *name) {
  vector_t *vector;
  HASH_FIND_STR(wasora.vectors, name, vector);
  return vector;
}

// devuelve la direccion de la estructura de la matriz que se llama name
matrix_t *wasora_get_matrix_ptr(const char *name) {
  matrix_t *matrix;
  HASH_FIND_STR(wasora.matrices, name, matrix);
  return matrix;
}


// devuelve la direccion de la estructura de la funcion que se llama name
function_t *wasora_get_function_ptr(const char *name) {
  function_t *function;
  HASH_FIND_STR(wasora.functions, name, function);
  return function;
}


// devuelve la direccion del archivo que se llama name
file_t  *wasora_get_file_ptr(const char *name) {
  file_t *file;
  HASH_FIND_STR(wasora.files, name, file);
  return file;
}

// devuelve la direccion de la rutina de usuario que se llama name
double (*wasora_get_routine_ptr(const char *name))(const double *, const char *) {
  loadable_routine_t *loadable_routine;
  HASH_FIND_STR(wasora.loadable_routines, name, loadable_routine);
  if (loadable_routine == NULL) {
    return NULL;
  }
  return loadable_routine->routine;
}

// devuelve la direccion de la rutina de usuario que se llama name
loadable_routine_t *wasora_get_loadable_routine(const char *name) {
  loadable_routine_t *loadable_routine;
  HASH_FIND_STR(wasora.loadable_routines, name, loadable_routine);
  return loadable_routine;
}

/* devuelve la direccion de la funcion builtin que se llama name */
builtin_function_t *wasora_get_builtin_function_ptr(const char *name) {
  int i;

  for (i = 0; i < N_BUILTIN_FUNCTIONS; i++) {
    if (strcmp(name, builtin_function[i].name) == 0) {
      return &builtin_function[i];
    }
  }

  return NULL;

}


/* devuelve la direccion de la funcion sobre vectores builtin que se llama name */
builtin_vectorfunction_t *wasora_get_builtin_vectorfunction_ptr(const char *name) {
  int i;

  for (i = 0; i < N_BUILTIN_VECTOR_FUNCTIONS; i++) {
    if (strcmp(name, builtin_vectorfunction[i].name) == 0) {
      return &builtin_vectorfunction[i];
    }
  }

  return NULL;

}

/* devuelve la direccion del funcional builtin que se llama name */
builtin_functional_t *wasora_get_builtin_functional_ptr(const char *name) {
  int i;

  for (i = 0; i < N_BUILTIN_FUNCTIONALS; i++) {
    if (strcmp(name, builtin_functional[i].name) == 0) {
      return &builtin_functional[i];
    }
  }
  
  return NULL;

}


vector_t *wasora_get_first_vector(const char *s) {
  
  char *line = strdup(s);
  char *factor;
  vector_t *wanted;
  
  factor = strtok(line, factorseparators);
  while (factor != NULL) {
    if ((wanted = wasora_get_vector_ptr(factor)) != NULL) {
      free(line);
      return wanted;
    }
    factor = strtok(NULL, factorseparators);
  }
  
  free(line);
  return NULL;
  
}

matrix_t *wasora_get_first_matrix(const char *s) {
  
  char *line = strdup(s);
  char *factor;
  matrix_t *wanted;
  
  factor = strtok(line, factorseparators);
  while (factor != NULL) {
    if ((wanted = wasora_get_matrix_ptr(factor)) != NULL) {
      free(line);
      return wanted;
    }
    factor = strtok(NULL, factorseparators);
  }
  
  free(line);
  return NULL;
  
}

char *wasora_get_first_dot(const char *s) {

  char *line = strdup(s);
  char *token;
  char *dummy;
  char *wanted;

  token = strtok(line, factorseparators);
  while (token != NULL) {

    if ((dummy = strstr(token, "_dot")) != NULL) {
      *dummy = '\0';
      wanted = strdup(token);
      free(line);
      return wanted;
    }

    token = strtok(NULL, factorseparators);
  }

  free(line);
  return NULL;

}
