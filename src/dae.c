/*------------ -------------- -------- --- ----- ---   --       -            -
 *  dae evaluation routines
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

#include "wasora.h"


#ifdef HAVE_IDA
 #include <sundials/sundials_math.h>
 #include <sundials/sundials_types.h>
#endif

int wasora_dae_init(void) {
  
#ifdef HAVE_IDA
  int i, k, l;
  phase_object_t *phase_object;
  dae_t *dae;
  
  // primero calculamos el tamanio del phase space
  wasora_dae.dimension = 0;
  LL_FOREACH(wasora_dae.phase_objects, phase_object) {
    
    if (phase_object->variable != NULL) {
      
      phase_object->size = 1;
      
    } else if (phase_object->vector != NULL) {
      
      if (!phase_object->vector->initialized) {
        wasora_call(wasora_vector_init(phase_object->vector));
      }
      if (!phase_object->vector_dot->initialized) {
        wasora_call(wasora_vector_init(phase_object->vector_dot));
      }
      
      phase_object->size = phase_object->vector->size;
      

    } else if (phase_object->matrix != NULL) {

      if (!phase_object->matrix->initialized) {
        wasora_call(wasora_matrix_init(phase_object->matrix));
      }
      if (!phase_object->matrix_dot->initialized) {
        wasora_call(wasora_matrix_init(phase_object->matrix_dot));
      }
      
      phase_object->size = phase_object->matrix->rows * phase_object->matrix->cols;
      
    }
    
    wasora_dae.dimension += phase_object->size;
      
  }
  
  if (wasora_dae.dimension == 0) {
    wasora_push_error_message("empty phase space in PHASE_SPACE keyword");
    return WASORA_RUNTIME_ERROR;
  }

  // alocamos
  wasora_dae.phase_value = malloc(wasora_dae.dimension * sizeof(double *));
  wasora_dae.phase_derivative = malloc(wasora_dae.dimension * sizeof(double *));
  
  i = 0;  
  LL_FOREACH(wasora_dae.phase_objects, phase_object) {

    if (phase_object->variable != NULL) {
      phase_object->offset = i;
      wasora_dae.phase_value[i] = wasora_value_ptr(phase_object->variable);
      wasora_dae.phase_derivative[i] = wasora_value_ptr(phase_object->variable_dot);
      i++;
      
    } else if (phase_object->vector != NULL) {

      phase_object->offset = i;
      for (k = 0; k < phase_object->vector->size; k++) {
        wasora_dae.phase_value[i] = gsl_vector_ptr(wasora_value_ptr(phase_object->vector), k);
        wasora_dae.phase_derivative[i] = gsl_vector_ptr(wasora_value_ptr(phase_object->vector_dot), k);
        i++;
      }

    } else if (phase_object->matrix != NULL) {

      phase_object->offset = i;
    for (k = 0; k < phase_object->matrix->rows; k++) {
        for (l = 0; l < phase_object->matrix->cols; l++) {
          wasora_dae.phase_value[i] = gsl_matrix_ptr(wasora_value_ptr(phase_object->matrix), k, l);
          wasora_dae.phase_derivative[i] = gsl_matrix_ptr(wasora_value_ptr(phase_object->matrix_dot), k, l);
          i++;
        }
      }
    }
  }

  // procesamos las DAEs
  i = 0;
  LL_FOREACH(wasora_dae.daes, dae) {
    
    if (dae->matrix != NULL) {
      
      if (!dae->matrix->initialized) {
        wasora_call(wasora_matrix_init(dae->matrix));
      }
      
      dae->i_min = (dae->expr_i_min.n_tokens != 0) ? wasora_evaluate_expression(&dae->expr_i_min)-1 : 0;
      dae->i_max = (dae->expr_i_max.n_tokens != 0) ? wasora_evaluate_expression(&dae->expr_i_max)   : dae->matrix->rows;
      if (dae->i_max <= dae->i_min) {
        wasora_push_error_message("i_max %d is smaller or equal than i_min %d", dae->i_max, dae->i_min+1);
      }

      dae->j_min = (dae->expr_j_min.n_tokens != 0) ? wasora_evaluate_expression(&dae->expr_j_min)-1 : 0;
      dae->j_max = (dae->expr_j_max.n_tokens != 0) ? wasora_evaluate_expression(&dae->expr_j_max)   : dae->matrix->cols;
      if (dae->j_max <= dae->j_min) {
        wasora_push_error_message("j_max %d is smaller or equal than j_min %d", dae->j_max, dae->j_min+1);
      }
      i += (dae->i_max - dae->i_min) * (dae->j_max - dae->j_min);

    } else if (dae->vector != NULL) {
      
      if (!dae->vector->initialized) {
        wasora_call(wasora_vector_init(dae->vector));
      }
      
      dae->i_min = (dae->expr_i_min.n_tokens != 0) ? wasora_evaluate_expression(&dae->expr_i_min)-1 : 0;
      dae->i_max = (dae->expr_i_max.n_tokens != 0) ? wasora_evaluate_expression(&dae->expr_i_max)   : dae->vector->size;
      if (dae->i_max <= dae->i_min) {
        wasora_push_error_message("i_max %d is smaller or equal than i_max %d", dae->i_max, dae->i_min+1);
      }
    
      i += (dae->i_max - dae->i_min);
    } else {
     
      // scalar
      i++;
      
    }
  }
  
  if (i > wasora_dae.dimension) {
    wasora_push_error_message("more DAE equations than phase space dimension %d", wasora_dae.dimension);
    return WASORA_PARSER_ERROR;
  } else if (i < wasora_dae.dimension) {
    wasora_push_error_message("less DAE equations than phase space dimension %d", wasora_dae.dimension);
    return WASORA_PARSER_ERROR;
  }
  
  
  

  wasora_dae.x = N_VNew_Serial(wasora_dae.dimension);
  wasora_dae.dxdt = N_VNew_Serial(wasora_dae.dimension);
  wasora_dae.id = N_VNew_Serial(wasora_dae.dimension);

  wasora_dae.system = IDACreate();
  
  // llenamos el vector de ida con las variables del usuario 
  for (i = 0; i < wasora_dae.dimension; i++) {
    NV_DATA_S(wasora_dae.x)[i] = *(wasora_dae.phase_value[i]);
    NV_DATA_S(wasora_dae.dxdt)[i] = *(wasora_dae.phase_derivative[i]);
    // suponemos que son algebraicas, despues marcamos las diferenciales
    // pero hay que inicializar el vector id!
    NV_DATA_S(wasora_dae.id)[i] = DAE_ALGEBRAIC; 
  }
  // inicializamos IDA  
  if (IDAInit(wasora_dae.system, wasora_ida_dae, wasora_var(wasora_special_var(time)), wasora_dae.x, wasora_dae.dxdt) != IDA_SUCCESS) {
    return WASORA_RUNTIME_ERROR;
  }

  // seteo de tolerancias 
  if ((wasora.special_vectors.abs_error = wasora_get_vector_ptr("abs_error")) != NULL) {

    // si hay un vector de absolutas, lo usamos 
    if (wasora.special_vectors.abs_error->size != wasora_dae.dimension) {
      wasora_push_error_message("dimension of system is %d and dimension of abs_error is %d", wasora_dae.dimension, wasora.special_vectors.abs_error->size);
      return WASORA_RUNTIME_ERROR;
    }

    wasora_dae.abs_error = N_VNew_Serial(wasora_dae.dimension);

    for (i = 0; i < wasora_dae.dimension; i++) {
      if (gsl_vector_get(wasora_value_ptr(wasora.special_vectors.abs_error), i) > 0) {
        NV_DATA_S(wasora_dae.abs_error)[i] = gsl_vector_get(wasora_value_ptr(wasora.special_vectors.abs_error), i);
      } else {
        NV_DATA_S(wasora_dae.abs_error)[i] = wasora_var(wasora_special_var(rel_error));
      }
    }

    if (IDASVtolerances(wasora_dae.system, wasora_var(wasora_special_var(rel_error)), wasora_dae.abs_error) != IDA_SUCCESS) {
      return WASORA_RUNTIME_ERROR;
    }
  } else {
    // si no nos dieron vector de absolutas, ponemos absolutas igual a relativas 
    if (IDASStolerances(wasora_dae.system, wasora_var(wasora_special_var(rel_error)), wasora_var(wasora_special_var(rel_error))) != IDA_SUCCESS) {
      return WASORA_RUNTIME_ERROR;
    }
  }

  //  estas tienen que venir despues de IDAInit

  // marcamos como differential aquellos elementos del phase space que lo sean
  LL_FOREACH(wasora_dae.phase_objects, phase_object) {
    if (phase_object->differential) {
      for (i = phase_object->offset; i < phase_object->offset+phase_object->size; i++) {
        NV_DATA_S(wasora_dae.id)[i] = DAE_DIFFERENTIAL;
      }
    }
  }
        
  
  if (IDASetId(wasora_dae.system, wasora_dae.id) != IDA_SUCCESS) {
    return WASORA_RUNTIME_ERROR;
  }
 
  if (IDADense(wasora_dae.system, wasora_dae.dimension) != IDA_SUCCESS) {
    return WASORA_RUNTIME_ERROR;
  }
  if (IDASetInitStep(wasora_dae.system, wasora_var(wasora_special_var(dt))) != IDA_SUCCESS) {
    return WASORA_RUNTIME_ERROR;
  }
  if (wasora_var(wasora_special_var(max_dt)) != 0) {
    if (IDASetMaxStep(wasora_dae.system, wasora_var(wasora_special_var(max_dt))) != IDA_SUCCESS) {
      return WASORA_RUNTIME_ERROR;
    }
  }
  
  return WASORA_RUNTIME_OK;
#else

  wasora_push_error_message("wasora cannot solve DAE systems as it was not linked against SUNDIALS IDA library.");
  return WASORA_RUNTIME_ERROR;

#endif
  
}

int wasora_dae_ic(void) {
  
#ifdef HAVE_IDA
  int err;

  if (wasora_dae.initial_conditions_mode == from_variables) {
    if ((err = IDACalcIC(wasora_dae.system, IDA_YA_YDP_INIT, wasora_var(wasora_special_var(dt)))) != IDA_SUCCESS) {
      wasora_push_error_message("error computing initial conditions from variable values, error = %d", err);
      return WASORA_RUNTIME_ERROR;
    }
  } else if (wasora_dae.initial_conditions_mode == from_derivatives) {
    if ((err = IDACalcIC(wasora_dae.system, IDA_Y_INIT, wasora_var(wasora_special_var(dt)))) != IDA_SUCCESS)  {
      wasora_push_error_message("error computing initial conditions from derivative values, error = %d", err);
      return WASORA_RUNTIME_ERROR;
    }
  }  
  
#endif
  return WASORA_RUNTIME_OK;
}


#ifdef HAVE_IDA
int wasora_ida_dae(realtype t, N_Vector yy, N_Vector yp, N_Vector rr, void *params) {

  int i, j, k;
  dae_t *dae;
 
  // esta funcion se llama un monton de veces en pasos de tiempo
  // intermedios, entoces le decimos cuanto vale el tiempo nuevo por
  //  si hay residuos que dependen explicitamente del tiempo
  wasora_var(wasora_special_var(time)) = t;

  // copiamos el estado del solver de IDA a wasora
  for (k = 0; k < wasora_dae.dimension; k++) {
    *(wasora_dae.phase_value[k]) = NV_DATA_S(yy)[k];
    *(wasora_dae.phase_derivative[k]) = NV_DATA_S(yp)[k];
  }

  // evaluamos los residuos en wasora y los dejamos en IDA
  k = 0;
  LL_FOREACH(wasora_dae.daes, dae) {
    
    // fall-offs para detectar errores (i.e. una expresion vectorial que dependa de j)
    j = 0;
    i = 0;
    
    if (dae->i_max == 0) {
      // ecuacion escalar
      NV_DATA_S(rr)[k++] = wasora_evaluate_expression(&dae->residual);
      
    } else {
         
      for (i = dae->i_min; i < dae->i_max; i++) {
        wasora_var(wasora_special_var(i)) = (double)i+1;
        if (dae->j_max == 0) {
          
          // ecuacion vectorial
          NV_DATA_S(rr)[k++] = wasora_evaluate_expression(&dae->residual);
        } else {
          for (j = dae->j_min; j < dae->j_max; j++) {
            wasora_var(wasora_special_var(j)) = (double)j+1;
            
            // ecuacion matricial
            NV_DATA_S(rr)[k++] = wasora_evaluate_expression(&dae->residual);
            
          }
        }
      }
    }
  }
  
#else
 int wasora_ida_dae(void) {
#endif

   return WASORA_RUNTIME_OK;

}

// instruccion dummy
int wasora_instruction_dae(void *arg) {
  
  return WASORA_RUNTIME_OK;
  
}
