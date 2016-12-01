/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora text output routines
 *
 *  Copyright (C) 2009--2015 jeremy theler
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

#include <stdio.h>
#include <string.h>


#include "wasora.h"


int wasora_instruction_print(void *arg) {
  print_t *print = (print_t *)arg;
  print_token_t *print_token;
  char *current_format = (print->tokens != NULL)?print->tokens->format:NULL;

  char default_print_format[] = DEFAULT_PRINT_FORMAT;
  int have_to_print = 1;
  int have_to_header = 0;
  int i, j;
  int flag = 1;  // flag para saber si ya imprimos algo o no (para matrices)
  
  if ((int)wasora_value(wasora_special_var(in_static)) != 0) {
    if (print->skip_static_step.n_tokens != 0 && print->last_static_step != 0 &&
        ((int)(wasora_value(wasora_special_var(step_static))) == 1 || ((int)(wasora_var(wasora_special_var(step_static))) - print->last_static_step) < wasora_evaluate_expression(&print->skip_static_step))) {
      have_to_print = 0;
    }
  } else {
    if (print->skip_step.n_tokens != 0 && 
        ((int)(wasora_value(wasora_special_var(step_transient))) - print->last_step) < wasora_evaluate_expression(&print->skip_step)) {
      have_to_print = 0;
    }
    if (print->skip_time.n_tokens != 0 && 
        (wasora_var(wasora_special_var(time)) == 0 || (wasora_var(wasora_special_var(time)) - print->last_time) < wasora_evaluate_expression(&print->skip_time))) {
      have_to_print = 0;
    }
  }

  if (have_to_print == 0) {
    return WASORA_RUNTIME_OK;
  }

  if (print->file->pointer == NULL) {
    wasora_call(wasora_instruction_open_file(print->file));
  }

  if (print->tokens == NULL || print->tokens->format == NULL) {
    current_format = default_print_format;
  }

  print->last_step = (int)(wasora_value(wasora_special_var(step_transient)));
  print->last_time = wasora_var(wasora_special_var(time));
  print->last_static_step = (int)(wasora_var(wasora_special_var(step_static)));

  if (print->header && (wasora.parametric_mode == 0 || (int)wasora_var(wasora.special_vars.step_outer) == 1)) {
    if (print->skip_header_step.n_tokens == 0 && print->header_already_printed == 0) {
      have_to_header = 1;
    } else if (((int)(wasora_value(wasora_special_var(step_transient))) - print->last_header_step) < wasora_evaluate_expression(&print->skip_header_step)) {
      have_to_header = 1;
    }
  }
  
// primero vemos si hay que escribir un header
  if (have_to_header) {
    fprintf(print->file->pointer, "# ");
    LL_FOREACH(print->tokens, print_token) {
      if (print_token->text != NULL) {
        fprintf(print->file->pointer, "%s", print_token->text);
        if (print_token->next != NULL) {
          fprintf(print->file->pointer, "%s", print->separator);
        }
      }
    }
    fprintf(print->file->pointer, "\n");
    print->header_already_printed = 1;
    print->last_header_step = (int)(wasora_value(wasora_special_var(step_transient)));
  }


  LL_FOREACH(print->tokens, print_token) {

    if (print_token->expression.n_tokens != 0) {
      fprintf(print->file->pointer, current_format, wasora_evaluate_expression(&print_token->expression));
      // si no es lo ultimo que hay que imprimir o no hay newline,
      // escribimos el separador
      if ((print_token->next != NULL) || (print->nonewline)) {
        fprintf(print->file->pointer, "%s", print->separator);
      }

      flag = 0;
      
    } else if (print_token->vector != NULL) {
      if (!print_token->vector->initialized) {
        wasora_call(wasora_vector_init(print_token->vector));
      }
      for (i = 0; i < print_token->vector->size; i++) {
        fprintf(print->file->pointer, current_format, gsl_vector_get(wasora_value_ptr(print_token->vector), i));
        if (i != print_token->vector->size-1) {
          fprintf(print->file->pointer, "%s", print->separator);
        }
      }
      if ((print_token->next != NULL) || (print->nonewline)) {
        fprintf(print->file->pointer, "%s", print->separator);
      }

      flag = 0;
      
    } else if (print_token->matrix != NULL) {
      if (!print_token->matrix->initialized) {
        wasora_call(wasora_matrix_init(print_token->matrix));
      }
      for (i = 0; i < print_token->matrix->rows; i++) {
        for (j = 0; j < print_token->matrix->cols; j++) {
          fprintf(print->file->pointer, current_format, gsl_matrix_get(wasora_value_ptr(print_token->matrix), i, j));
          if (j != print_token->matrix->cols-1) {
            fprintf(print->file->pointer, "%s", print->separator);
          }
        }
        if (flag == 1 && i != print_token->matrix->rows-1) {
          fprintf(print->file->pointer, "\n");
        } else {
          fprintf(print->file->pointer, "%s", print->separator);
        }
      }

      flag = 0;
      
    } else if (print_token->text != NULL) {
      fprintf(print->file->pointer, "%s%s", print_token->text, print->separator);
      
      flag = 0;
      
    } else if (print_token->format != NULL) {
      current_format = print_token->format;
    }
    
  }

  if (!print->nonewline) {
    fprintf(print->file->pointer, "\n");
  }

  // siempre flusheamos
  fflush(print->file->pointer);
    

  return WASORA_RUNTIME_OK;

}


int wasora_instruction_print_function(void *arg) {
  print_function_t *print_function = (print_function_t *)arg;
  print_token_t *print_token;
  
  int j, k;
  double *x, *x_min, *x_max, *x_step;
  
  if (print_function->file->pointer == NULL) {
    wasora_call(wasora_instruction_open_file(print_function->file));
  }
  
  if (!print_function->first_function->initialized) {
    wasora_call(wasora_function_init(print_function->first_function));
  }
  
  // vemos si hay que escribir un header
  if (print_function->header && (wasora.parametric_mode == 0 || (int)wasora_var(wasora.special_vars.step_outer) == 1)) {
    fprintf(print_function->file->pointer, "# ");
    
    // primero los argumentos de la primera funcion
    for (k = 0; k < print_function->first_function->n_arguments; k++) {
      fprintf(print_function->file->pointer, "%s", print_function->first_function->var_argument[k]->name);
      fprintf(print_function->file->pointer, "%s", print_function->separator);
    }
    
    LL_FOREACH(print_function->tokens, print_token) {
      if (print_token->text != NULL) {
        fprintf(print_function->file->pointer, "%s", print_token->text);
        if (print_token->next != NULL) {
          fprintf(print_function->file->pointer, "%s", print_function->separator);
        }
      }
    }
    fprintf(print_function->file->pointer, "\n");
  }
  

  if (print_function->range.min != NULL && print_function->range.max != NULL && 
      (print_function->range.step != NULL || print_function->range.nsteps != NULL)) {
    // nos dieron los puntos donde se quiere dibujar las funciones

    x = calloc(print_function->first_function->n_arguments, sizeof(double));
    x_min = calloc(print_function->first_function->n_arguments, sizeof(double));
    x_max = calloc(print_function->first_function->n_arguments, sizeof(double));
    x_step = calloc(print_function->first_function->n_arguments, sizeof(double));

    for (j = 0; j < print_function->first_function->n_arguments; j++) {
      x_min[j] = wasora_evaluate_expression(&print_function->range.min[j]);
      x_max[j] = wasora_evaluate_expression(&print_function->range.max[j]);
      x_step[j] = (print_function->range.step != NULL) ? wasora_evaluate_expression(&print_function->range.step[j]) :
                                   (x_max[j]-x_min[j])/wasora_evaluate_expression(&print_function->range.nsteps[j]);

      x[j] = x_min[j];
    }

    // hasta que el primer argumento llegue al maximo y se pase un
    // poquito para evitar que por el redondeo se nos escape el ultimo punto
    while (x[0] <= x_max[0] + wasora_value(wasora_special_var(zero))) {

      // imprimimos los argumentos
      for (j = 0; j < print_function->first_function->n_arguments; j++) {
        fprintf(print_function->file->pointer, print_function->format, x[j]);
        fprintf(print_function->file->pointer, "%s", print_function->separator);
      }

      LL_FOREACH (print_function->tokens, print_token) {
        // imprimimos lo que nos pidieron
        if (print_token->function != NULL) {
          fprintf(print_function->file->pointer, print_function->format, wasora_evaluate_function(print_token->function, x));
          
        } else if (print_token->expression.n_tokens != 0) {
          wasora_set_function_args(print_function->first_function, x);
          fprintf(print_function->file->pointer, print_function->format, wasora_evaluate_expression(&print_token->expression));
          
        }
        
        if (print_token->next != NULL) {
          fprintf(print_function->file->pointer, "%s", print_function->separator);
        } else {
          fprintf(print_function->file->pointer, "\n");
          // siempre flusheamos
          fflush(print_function->file->pointer);
        }
      }

      // incrementamos el ultimo argumento
      x[print_function->first_function->n_arguments-1] += x_step[print_function->first_function->n_arguments-1];
      // y vamos mirando si hay que reiniciarlos
      for (j = print_function->first_function->n_arguments-2; j >= 0; j--) {
        if (x[j+1] > x_max[j+1]) {
          x[j+1] = x_min[j+1];
          x[j] += x_step[j];

          // si estamos en 2d y reiniciamos el primer argumento imprimimos una linea
          // en blanco para que plotear con gnuplot with lines salga lindo
          if (print_function->first_function->n_arguments == 2 && j == 0) {
            fprintf(print_function->file->pointer, "\n");
          }

        }
      }
    }

    free(x);
    free(x_min);
    free(x_max);
    free(x_step);

  } else if (print_function->first_function != NULL && print_function->first_function->data_size > 0) {

    // imprimimos en los puntos de definicion de la primera
    x = calloc(print_function->first_function->n_arguments, sizeof(double));

    for (j = 0; j < print_function->first_function->data_size; j++) {

      // los argumentos de la primera funcion
      for (k = 0; k < print_function->first_function->n_arguments; k++) {
        // nos acordamos los argumentos para las otras funciones que vienen despues
        x[k] = print_function->first_function->data_argument[k][j];
        fprintf(print_function->file->pointer, print_function->format, x[k]);
        fprintf(print_function->file->pointer, "%s", print_function->separator);
      }

      // las cosas que nos pidieron
      LL_FOREACH(print_function->tokens, print_token) {
      
        // imprimimos lo que nos pidieron
        if (print_token->function != NULL) {
          if (print_token->function == print_function->first_function) {
            // la primera funcion tiene los puntos posta asi que no hay que interpolar
            fprintf(print_function->file->pointer, print_function->format, print_token->function->data_value[j]);
          } else {
            fprintf(print_function->file->pointer, print_function->format, wasora_evaluate_function(print_token->function, x));
          }
          
        } else if (print_token->expression.n_tokens != 0) {
          wasora_set_function_args(print_function->first_function, x);
          fprintf(print_function->file->pointer, print_function->format, wasora_evaluate_expression(&print_token->expression));
          
        }
        
        if (print_token->next != NULL) {
          fprintf(print_function->file->pointer, "%s", print_function->separator);
        } else {
          fprintf(print_function->file->pointer, "\n");
          // siempre flusheamos
          fflush(print_function->file->pointer);
        }

        // si estamos en 2d y cambiamos los dos primeros argumentos imprimimos una linea
        // en blanco para que plotear con gnuplot with lines salga lindo        
        if (print_token->next == NULL &&
            print_function->first_function->n_arguments == 2 &&
            print_function->first_function->rectangular_mesh &&
            j != print_function->first_function->data_size &&
            gsl_fcmp(print_function->first_function->data_argument[0][j], print_function->first_function->data_argument[0][j+1], print_function->first_function->multidim_threshold) != 0 &&
            gsl_fcmp(print_function->first_function->data_argument[1][j], print_function->first_function->data_argument[1][j+1], print_function->first_function->multidim_threshold) != 0) {
          fprintf(print_function->file->pointer, "\n");
        }

      }
//      fprintf(print_function->file->pointer, "\n");

    }

    free(x);

  } else {
    wasora_push_error_message("mandatory range needed for PRINT_FUNCTION instruction (function %s is not point-wise defined)", print_function->first_function->name);
    return WASORA_RUNTIME_ERROR;
      
  }

  fflush(print_function->file->pointer);
  
  return 0;

}



int wasora_instruction_print_vector(void *arg) {
  print_vector_t *print_vector = (print_vector_t *)arg;
  print_token_t *print_token;

  int j, k;
  int n_elems_per_line;

  if (print_vector->file->pointer == NULL) {
    wasora_call(wasora_instruction_open_file(print_vector->file));
  }

  if (print_vector->first_vector == NULL) {
    return WASORA_RUNTIME_OK;
  }
  
  if (!print_vector->first_vector->initialized) {
    wasora_call(wasora_vector_init(print_vector->first_vector));
  }
  
  if ((n_elems_per_line = (int)wasora_evaluate_expression(&print_vector->elems_per_line)) || print_vector->horizontal) {

    LL_FOREACH(print_vector->tokens, print_token) {
      
      if (!print_token->vector->initialized) {
        wasora_call(wasora_vector_init(print_token->vector));
      }
      
      if (print_token->vector->size != print_vector->first_vector->size) {
        wasora_push_error_message("vectors %s and %s do not have the same size", print_token->vector->name, print_vector->first_vector->name);
        return WASORA_RUNTIME_ERROR;
      }
      
      j = 0;
      for (k = 0; k < print_vector->first_vector->size; k++) {
        
        if (print_token->vector != NULL) {
          fprintf(print_vector->file->pointer, print_vector->format, gsl_vector_get(wasora_value_ptr(print_token->vector), k));

        } else if (print_token->expression.n_tokens != 0) {
          wasora_var(wasora.special_vars.i) = k+1;
          fprintf(print_vector->file->pointer, print_vector->format, wasora_evaluate_expression(&print_token->expression));
          
        }

        if (++j == n_elems_per_line) {
          j = 0;
          fprintf(print_vector->file->pointer, "\n");
        } else {
          fprintf(print_vector->file->pointer, "%s", print_vector->separator);
        }
      }
      fprintf(print_vector->file->pointer, "\n");
    }

  } else {

    for (k = 0; k < print_vector->first_vector->size; k++) {

      LL_FOREACH(print_vector->tokens, print_token) {
        
        if (print_token->vector != NULL) {
          if (print_token->vector->initialized == 0) {
            wasora_call(wasora_vector_init(print_token->vector));
          }
          if (print_token->vector->size != print_vector->first_vector->size) {
            wasora_push_error_message("vectors %s and %s do not have the same size", print_token->vector->name, print_vector->first_vector->name);
            return WASORA_RUNTIME_ERROR;
          }
          
          fprintf(print_vector->file->pointer, print_vector->format, gsl_vector_get(wasora_value_ptr(print_token->vector), k));

        } else if (print_token->expression.n_tokens != 0) {
          wasora_var(wasora.special_vars.i) = k+1;
          fprintf(print_vector->file->pointer, print_vector->format, wasora_evaluate_expression(&print_token->expression));
          
        }

        if (print_token->next != NULL) {
          fprintf(print_vector->file->pointer, "%s", print_vector->separator);
        } else {
          fprintf(print_vector->file->pointer, "\n");
        }
      }
    }
  }

  fflush(print_vector->file->pointer);
  
  return 0;

}

/*
int wasora_instruction_print_matrix(void *arg) {
  print_matrix_t *print_matrix = (print_matrix_t *)arg;


  int j, k;

  int have_to_print = 1;

  if (have_to_print) {

    if (print_matrix->file->pointer == NULL) {
      wasora_instruction_open_file(print_matrix->file);
    }

    for (k = 0; k < print_matrix->matrix->rows; k++) {
      for (j = 0; j < print_matrix->matrix->cols; j++) {

//        if (print_matrix->matrix->is_crisp == 0) {
//          fprintf(print_matrix->file->pointer, print_matrix->format, print_matrix->matrix->element[k][j]->current[0]);
//        } else {
          fprintf(print_matrix->file->pointer, print_matrix->format, gsl_matrix_get(print_matrix->matrix->current, k, j));
//        }
        fprintf(print_matrix->file->pointer, "%s", print_matrix->separator);
      }
      fprintf(print_matrix->file->pointer, "\n");
    }
  }

  fflush(print_matrix->file->pointer);
  
  return 0;

}
*/
