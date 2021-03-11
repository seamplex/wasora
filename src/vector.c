/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora vector routines
 *
 *  Copyright (C) 2015--2017 jeremy theler
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
#include <gsl/gsl_sort_vector.h>


double wasora_vector_get(vector_t *vector, const size_t i) {
  
  if (!vector->initialized) {
    wasora_call(wasora_vector_init(vector));
  }
  
  return gsl_vector_get(wasora_value_ptr(vector), i);
}

double wasora_vector_get_initial_static(vector_t *vector, const size_t i) {
  
  if (!vector->initialized) {
    wasora_call(wasora_vector_init(vector));
  }
  
  return gsl_vector_get(vector->initial_static, i);
}

double wasora_vector_get_initial_transient(vector_t *vector, const size_t i) {
  
  if (!vector->initialized) {
    wasora_call(wasora_vector_init(vector));
  }
  
  return gsl_vector_get(vector->initial_transient, i);
}

int wasora_vector_set(vector_t *vector, const size_t i, double value) {
  
  if (!vector->initialized) {
    wasora_call(wasora_vector_init(vector));
  }
  
  gsl_vector_set(wasora_value_ptr(vector), i, value);
  
  return 0;
}


int wasora_vector_init(vector_t *vector) {

  int size;
  int i;
  expr_t *data;

  if (vector->initialized) {
    return WASORA_RUNTIME_OK;
  }

  if (vector->function_data != NULL) {
    
    if (!vector->function_data->initialized) {
      wasora_call(wasora_function_init(vector->function_data));
    }
    if (vector->size_expr->n_tokens == 0) {
      size = vector->function_data->data_size;
    } else if ((size = (int)(round(wasora_evaluate_expression(vector->size_expr)))) != vector->function_data->data_size) {
      wasora_push_error_message("vector '%s' has size mismatch, SIZE = %d and FUNCTION_DATA = %d", vector->name, size, vector->function_data->data_size);
      return WASORA_PARSER_ERROR;
    }

  } else if (vector->function_arg != NULL) {
    if (!vector->function_arg->initialized) {
      wasora_call(wasora_function_init(vector->function_arg));
    }
    if (vector->size_expr->n_tokens == 0) {
      size = vector->function_arg->data_size;
    } else if ((size = (int)(round(wasora_evaluate_expression(vector->size_expr)))) != vector->function_arg->data_size) {
      wasora_push_error_message("vector '%s' has size mismatch, SIZE = %d and FUNCTION_ARG = %d", vector->name, size, vector->function_arg->data_size);
      return WASORA_PARSER_ERROR;
    }
    
  } else if ((size = vector->size) == 0 &&
      (size = (int)(round(wasora_evaluate_expression(vector->size_expr)))) == 0) {
    wasora_push_error_message("vector '%s' has zero size", vector->name);
    return WASORA_PARSER_ERROR;
  } else if (size < 0) {
    wasora_push_error_message("vector '%s' has negative size %d", vector->name, size);
    return WASORA_PARSER_ERROR;
  }
  
  vector->size = size;
  wasora_value_ptr(vector) = gsl_vector_calloc(size);
  vector->initial_static = gsl_vector_calloc(size);
  vector->initial_transient = gsl_vector_calloc(size);

  if (vector->datas != NULL) {
    i = 0;
    LL_FOREACH(vector->datas, data) {
      gsl_vector_set(wasora_value_ptr(vector), i++, wasora_evaluate_expression(data));
    }
  }
  
  if (vector->function_data != NULL) {
    wasora_realloc_vector_ptr(vector, vector->function_data->data_value, 0);
  } else if (vector->function_arg != NULL) {
    wasora_realloc_vector_ptr(vector, vector->function_arg->data_argument[vector->function_n_arg-1], 0);
  }
  
  vector->initialized = 1;
  
  return WASORA_RUNTIME_OK;

}


int wasora_instruction_vector_sort(void *arg) {
  
  vector_sort_t *vector_sort = (vector_sort_t *)arg;
  
  if (vector_sort->v2 == NULL)
    gsl_sort_vector(vector_sort->v1->value);
  else
    gsl_sort_vector2(vector_sort->v1->value, vector_sort->v2->value);
  
  if (vector_sort->descending) {
    gsl_vector_reverse(vector_sort->v1->value);
    
    if (vector_sort->v2 != NULL)
      gsl_vector_reverse(vector_sort->v2->value);
  }
  
  return WASORA_RUNTIME_OK;
}
