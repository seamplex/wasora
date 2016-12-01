/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora user-provided routines tools
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
#include <stdlib.h>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>

#include "wasora.h"
#include "interface.h"

/* funciones de variables */
int wasora_exists_var(const char *name) {
  return (wasora_get_variable_ptr(name) != NULL);
}

double wasoraexistsvar_(const char*fortranname, int length) {
  double y;
  char *cname = strdup(fortranname);
  cname[length] = '\0';
  y = wasora_exists_var(cname);
  free(cname);
  return y;
}

double wasora_get_var_value(const char *name) {
  var_t *var;

  
  if ((var = wasora_get_variable_ptr(name)) == NULL) {
    wasora_push_error_message("wasora_get_var_value: requested variable \"%s\" does not exist", name);
    wasora_runtime_error();
    return 0;
  }
  
  return wasora_value(var);
}

double *wasora_get_var_value_ptr(const char *name) {
  var_t *var;
  
  if ((var = wasora_get_variable_ptr(name)) == NULL) {
    wasora_push_error_message("wasora_get_var_value: requested variable \"%s\" does not exist", name);
    wasora_runtime_error();
    return NULL;
  }
  
  return wasora_value_ptr(var); 
}

/* fortran wrapper */
double wasoragetvarvalue_(const char *fortranname, int length) {
  double y;
  char *cname = strdup(fortranname);
  cname[length] = '\0';
  y = wasora_get_var_value(cname);
  free(cname);
  return y;
}



void wasora_set_var_value(const char *name, double value) {
  var_t *var;

  if ((var = wasora_get_variable_ptr(name)) == NULL) {
    wasora_push_error_message("wasora_set_var_value: requested variable \"%s\" does not exist", name);
    wasora_runtime_error();
    return;
  }

  wasora_value(var) = value;
  
  return;
}

/* fortran wrapper */
int wasorasetvarvalue_(const char *fortranname, double *value_ptr, int length) {
  double value = *value_ptr;
  char *cname = strdup(fortranname);
  cname[length] = '\0';
  wasora_set_var_value(cname, value);
  free(cname);
  return 0;
}




/* funciones de vectores */
int wasora_exists_vector(const char *name) {
  return (wasora_get_vector_ptr(name) == NULL)?0:1;
}

double wasoraexistsvector_(const char*fortranname, int length) {
  double y;
  char *cname = strdup(fortranname);
  cname[length] = '\0';
  y = wasora_exists_vector(cname);
  free(cname);
  return y;
}

double wasora_get_vector_value(const char *name, int i) {
  vector_t *vector;

  if ((vector = wasora_get_vector_ptr(name)) == NULL) {
    wasora_push_error_message("wasora_get_vector_value: requested vector \"%s\" does not exist", name);
    wasora_runtime_error();
    return 0;
  }
  
  if (!vector->initialized) {
    wasora_vector_init(vector);
  }
  
  if (i == 0) {
    wasora_push_error_message("wasora_get_vector_value: vector indices start from one, not from zero");
    wasora_runtime_error();
    return 0;
  }

  if (i > vector->size) {
    wasora_push_error_message("wasora_get_vector_value: requested index %d is greater thant vector \"%s\" size %d", i, name, vector->size);
    wasora_runtime_error();
    return 0;
  }
  

  return gsl_vector_get(wasora_value_ptr(vector), i-1);
  
}

/* fortran wrapper */
double wasoragetvectorvalue_(const char *fortranname, int *i, int length) {
  double y;
  char *cname = strdup(fortranname);
  cname[length] = '\0';
  y = wasora_get_vector_value(cname, *i);
  free(cname);
  return y;
}


gsl_vector *wasora_get_vector_gsl_ptr(const char *name) {
  vector_t *vector;

  if ((vector = wasora_get_vector_ptr(name)) == NULL) {
    wasora_push_error_message("wasora_get_vector_gsl_ptr: requested variable \"%s\" does not exist", name);
    wasora_runtime_error();
    return NULL;
  }
  
  if (!vector->initialized) {
    wasora_vector_init(vector);
  }
  
  return wasora_value_ptr(vector);
}

double *wasora_get_vector_raw_ptr(const char *name) {
  vector_t *vector;
  
  if ((vector = wasora_get_vector_ptr(name)) == NULL) {
    wasora_push_error_message("wasora_get_vector_gsl_ptr: requested variable \"%s\" does not exist", name);
    wasora_runtime_error();
    return NULL;
  }
  
  if (!vector->initialized) {
    wasora_vector_init(vector);
  }
  
  return  gsl_vector_ptr(wasora_value_ptr(vector), 0);
}


void wasora_set_vector_value(const char *name, int i, double value) {
  vector_t *vector;

  if ((vector = wasora_get_vector_ptr(name)) == NULL) {
    wasora_push_error_message("wasora_set_vector_value: requested vector \"%s\" does not exist", name);
    wasora_runtime_error();
    return;
  }

  if (!vector->initialized) {
    wasora_vector_init(vector);
  }
  
  if (i == 0) {
    wasora_push_error_message("wasora_set_vector_value: vector indices start from one, not from zero");
    wasora_runtime_error();
    return;
  }

  if (i > vector->size) {
    wasora_push_error_message("wasora_set_vector_value: requested index %d is greater thant vector \"%s\" size %d", i, name, vector->size);
    wasora_runtime_error();
    return;
  }

  gsl_vector_set(wasora_value_ptr(vector), i-1, value);
  
  return;
}

/* fortran wrapper */
int wasorasetvectorvalue_(const char *fortranname, int *i, double *value_ptr, int length) {
  double value = *value_ptr;
  char *cname = strdup(fortranname);
  cname[length] = '\0';
  wasora_set_vector_value(cname, *i, value);
  free(cname);
  return 0;
}


double *wasora_get_crisp_pointer(const char *name) {
  vector_t *vector;
  
  if ((vector = wasora_get_vector_ptr(name)) == NULL) {
    wasora_push_error_message("wasora_get_crisp_pointer: requested vector \"%s\" does not exist", name);
    wasora_runtime_error();
    return NULL;
  }
  return gsl_vector_ptr(wasora_value_ptr(vector), 0);
  
}

int wasora_get_vector_size(const char *name) {
  vector_t *vector;
  
  if ((vector = wasora_get_vector_ptr(name)) == NULL) {
    wasora_push_error_message("wasora_get_vector_size: requested vector \"%s\" does not exist", name);
    wasora_runtime_error();
    return 0;
  }
  
  return vector->size;
  
}

/* fortran wrapper */
int wasoragetvectorsize_(const char*fortranname, int length) {
  int size;
  char *cname = strdup(fortranname);
  cname[length] = '\0';
  size = wasora_get_vector_size(cname);
  free(cname);
  return size;
}




/* funciones de matrices */
int wasora_exists_matrix(const char *name) {
  return (wasora_get_matrix_ptr(name) == NULL)?0:1;
}

double wasoraexistsmatrix_(const char*fortranname, int length) {
  double y;
  char *cname = strdup(fortranname);
  cname[length] = '\0';
  y = wasora_exists_matrix(cname);
  free(cname);
  return y;
}

double wasora_get_matrix_value(const char *name, int i, int j) {
  matrix_t *matrix;

  if ((matrix = wasora_get_matrix_ptr(name)) == NULL) {
    wasora_push_error_message("wasora_get_vector_value: requested matrix \"%s\" does not exist", name);
    wasora_runtime_error();
    return 0;
  }
  
  if (!matrix->initialized) {
    wasora_call(wasora_matrix_init(matrix));
  }
  
  
  if (i > matrix->rows) {
    wasora_push_error_message("wasora_set_vector_value: requested index %d is greater thant vector \"%s\" size %d", i, name, matrix->rows);
    wasora_runtime_error();
    return 0;
  }

  if (j > matrix->cols) {
    wasora_push_error_message("wasora_set_vector_value: requested index %d is greater thant vector \"%s\" size %d", j, name, matrix->cols);
    wasora_runtime_error();
    return 0;
  }

  return gsl_matrix_get(wasora_value_ptr(matrix), i-1, j-1);
  
}

/* fortran wrapper */
double wasoragetmatrixvalue_(const char *fortranname, int *i, int *j, int length) {
  double y;
  char *cname = strdup(fortranname);
  cname[length] = '\0';
  y = wasora_get_matrix_value(cname, *i, *j);
  free(cname);
  return y;
}

gsl_matrix *wasora_get_matrix_gsl_ptr(const char *name) {
  matrix_t *matrix;
  
  if ((matrix = wasora_get_matrix_ptr(name)) == NULL) {
    wasora_push_error_message("wasora_get_matrix_gsl_ptr: requested matrix \"%s\" does not exist", name);
    wasora_runtime_error();
    return NULL;
  }
  
  if (!matrix->initialized) {
    wasora_matrix_init(matrix);
  }
    
  return wasora_value_ptr(matrix);
}



void wasora_set_matrix_value(const char *name, int i, int j, double value) {
  matrix_t *matrix;

  if ((matrix = wasora_get_matrix_ptr(name)) == NULL) {
    wasora_push_error_message("wasora_set_vector_value: requested vector \"%s\" does not exist", name);
    wasora_runtime_error();
    return;
  }
  
  if (!matrix->initialized) {
    wasora_matrix_init(matrix);
  }
  
  if (i <= 0 || j <= 0) {
    wasora_push_error_message("wasora_get_vector_value: vector indices start from one, not from zero");
    wasora_runtime_error();
    return;
  }

  if (i > matrix->rows) {
    wasora_push_error_message("wasora_set_vector_value: requested index %d is greater thant vector \"%s\" size %d", i, name, matrix->rows);
    wasora_runtime_error();
    return;
  }

  if (j > matrix->cols) {
    wasora_push_error_message("wasora_set_vector_value: requested index %d is greater thant vector \"%s\" size %d", j, name, matrix->cols);
    wasora_runtime_error();
    return;
  }

  gsl_matrix_set(wasora_value_ptr(matrix), i-1, j-1, value);
  
  return;
}

/* fortran wrapper */
int wasorasetmatrixvalue_(const char *fortranname, int *i, int *j, double *value_ptr, int length) {
  double value = *value_ptr;
  char *cname = strdup(fortranname);
  cname[length] = '\0';
  wasora_set_matrix_value(cname, *i, *j, value);
  free(cname);
  return 0;
}


double *wasora_get_matrix_crisp_pointer(const char *name) {
  matrix_t *matrix;
  
  if ((matrix = wasora_get_matrix_ptr(name)) == NULL) {
    wasora_push_error_message("wasora_get_matrix_crisp_pointer: requested matrix \"%s\" does not exist", name);
    wasora_runtime_error();
    return NULL;
  }

  if (!matrix->initialized) {
    wasora_matrix_init(matrix);
  }
  
  return gsl_matrix_ptr(wasora_value_ptr(matrix), 0, 0);
  
}

int wasora_get_matrix_rows(const char *name) {
  matrix_t *matrix;
  
  if ((matrix = wasora_get_matrix_ptr(name)) == NULL) {
    wasora_push_error_message("wasora_get_matrix_rows: requested matrix \"%s\" does not exist", name);
    wasora_runtime_error();
    return 0;
  }

  if (!matrix->initialized) {
    wasora_call(wasora_matrix_init(matrix));
  }
  
  return matrix->rows;
  
}

int wasora_get_matrix_cols(const char *name) {
  matrix_t *matrix;
  
  if ((matrix = wasora_get_matrix_ptr(name)) == NULL) {
    wasora_push_error_message("wasora_get_matrix_cols: requested matrix \"%s\" does not exist", name);
    wasora_runtime_error();
    return 0;
  }

  if (!matrix->initialized) {
    wasora_call(wasora_matrix_init(matrix));
  }
  
  return matrix->cols;
  
}

/* fortran wrapper */
int wasoragetmatrixcols_(const char *fortranname, int length) {
  int size;
  char *cname = strdup(fortranname);
  cname[length] = '\0';
  size = wasora_get_matrix_rows(cname);
  free(cname);
  return size;
}

int wasoragetmatrixrows_(const char *fortranname, int length) {
  int size;
  char *cname = strdup(fortranname);
  cname[length] = '\0';
  size = wasora_get_matrix_rows(cname);
  free(cname);
  return size;
}



/* funciones de funciones */
int wasora_exists_function(const char *name) {
  if (wasora_get_function_ptr(name) != NULL) {
    return 1;
  } else if (wasora_get_builtin_function_ptr(name) != NULL) {
    return 1;
  }
  return 0;
}

double wasoraexistsfunction_(const char*fortranname, int length) {
  double y;
  char *cname = strdup(fortranname);
  cname[length] = '\0';
  y = wasora_exists_function(cname);
  free(cname);
  return y;
}



double wasoragetfunctionvalue(const char *fortranname, double *arg, int length) {
  double y;
  char *cname = strdup(fortranname);
  cname[length] = '\0';
  y = wasora_get_function_value(cname, arg);
  free(cname);
  return y;
}

double wasora_get_function_value(const char *name, double *arg) {
//  int j;
//  algebraic_token_t dummy_expr;
  function_t *function;

  if ((function = wasora_get_function_ptr(name)) != NULL) {

    return wasora_evaluate_function(function, arg);

  /* TODO: hacer expresiones temporales que sean constantes e iguales a los argumentos pedidos */
/*    
  } else if ((dummy_expr.builtin_function = get_builtin_function_ptr(name)) != NULL) {

    dummy_expr.arg_value = calloc(dummy_expr.builtin_function->max_arguments, sizeof(double));
    for (j = 0; j < dummy_expr.builtin_function->max_arguments; j++) {
      dummy_expr.arg_value[j] = arg[j];
    }
    dummy_expr.value = dummy_expr.builtin_function->routine(&dummy_expr);
    free(dummy_expr.arg_value);

    return dummy_expr.value;
*/  
  } else {

    wasora_push_error_message("wasora_get_function_value: requested function \"%s\" does not exist", name);
    wasora_runtime_error();
    return 0;
    
  }
    
  return 0;
  
  
}
