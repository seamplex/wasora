/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora alias routines
 *
 *  Copyright (C) 2015 jeremy theler
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
#include "wasora.h"
#endif

int wasora_instruction_alias(void *arg) {
  alias_t *alias = (alias_t *)arg;
  int row, col;
  
  if (!alias->initialized) {

    alias->initialized = 1;
    row = (int)(wasora_evaluate_expression(&alias->row)) - 1;
    col = (int)(wasora_evaluate_expression(&alias->col)) - 1;
    
    if (alias->matrix != NULL) {
      
      if (!alias->matrix->initialized) {
        wasora_call(wasora_matrix_init(alias->matrix));
      }
      if (row >= alias->matrix->rows) {
        wasora_push_error_message("row %d greater than matrix size %dx%d in alias '%s'", row, alias->matrix->rows, alias->matrix->cols, alias->new_variable->name);
      }
      if (col >= alias->matrix->cols) {
        wasora_push_error_message("col %d greater than matrix size %dx%d in alias '%s'", col, alias->matrix->rows, alias->matrix->cols, alias->new_variable->name);
      }
      

      wasora_realloc_variable_ptr(alias->new_variable, gsl_matrix_ptr(wasora_value_ptr(alias->matrix), row, col), 0);
      free(alias->new_variable->initial_static);
      alias->new_variable->initial_static = gsl_matrix_ptr(alias->matrix->initial_static, row, col);
      free(alias->new_variable->initial_transient);
      alias->new_variable->initial_transient = gsl_matrix_ptr(alias->matrix->initial_transient, row, col);
      
    } else if (alias->vector != NULL) {
      
      if (!alias->vector->initialized) {
        wasora_call(wasora_vector_init(alias->vector));
      }
      if (row >= alias->vector->size) {
        wasora_push_error_message("subscript %d greater than vector size %d in alias '%s'", row, alias->vector->size, alias->new_variable->name);
      }
      
      wasora_realloc_variable_ptr(alias->new_variable, gsl_vector_ptr(wasora_value_ptr(alias->vector), row), 0);
      free(alias->new_variable->initial_static);
      alias->new_variable->initial_static = gsl_vector_ptr(alias->vector->initial_static, row);
      free(alias->new_variable->initial_transient);
      alias->new_variable->initial_transient = gsl_vector_ptr(alias->vector->initial_transient, row);
      
    } else if (alias->variable != NULL) {
      
      wasora_realloc_variable_ptr(alias->new_variable, wasora_value_ptr(alias->variable), 0);
      free(alias->new_variable->initial_static);
      alias->new_variable->initial_static = alias->variable->initial_static;
      free(alias->new_variable->initial_transient);
      alias->new_variable->initial_transient = alias->variable->initial_transient;
      
    }
  }
  
  return WASORA_RUNTIME_OK;
}

