/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related vector-filling routines
 *
 *  Copyright (C) 2015--2020 jeremy theler
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
#include <wasora.h>

#include <stdio.h>


int wasora_instruction_mesh_fill_vector(void *arg) {

  int i;
  mesh_fill_vector_t *mesh_fill_vector = (mesh_fill_vector_t *)arg;
  mesh_t *mesh = mesh_fill_vector->mesh;
  vector_t *vector = mesh_fill_vector->vector;
  function_t *function = mesh_fill_vector->function;
  expr_t *expr = &mesh_fill_vector->expr;
  
  if (!vector->initialized) {
    wasora_call(wasora_vector_init(mesh_fill_vector->vector));
  }
  
  if (mesh_fill_vector->centering == centering_default) {
    mesh_fill_vector->centering = centering_nodes;
  }
  
  if        (mesh_fill_vector->centering == centering_cells && mesh->n_cells != vector->size) {
    wasora_push_error_message("size mismatch between mesh '%s' cells (%d) and vector '%s' size (%d)", mesh->name, mesh->n_cells, vector->name, vector->size);
    return WASORA_RUNTIME_ERROR;
  } else if (mesh_fill_vector->centering == centering_nodes && mesh->n_nodes != vector->size) {
    wasora_push_error_message("size mismatch between mesh '%s' nodes (%d) and vector '%s' size (%d)", mesh_fill_vector->mesh->name, mesh_fill_vector->mesh->n_nodes, vector->name, vector->size);
    return WASORA_RUNTIME_ERROR;
  }

  
  // ver si esto es lo optimo en terminos de condicionales y loops
  if (function != NULL) {
    if (mesh_fill_vector->centering == centering_cells) {
      if (function->type == type_pointwise_mesh_cell && function->mesh == mesh) {
        for (i = 0; i < function->data_size; i++) {
          gsl_vector_set(wasora_value_ptr(vector), i, function->data_value[i]);
        }
      } else {
        for (i = 0; i < mesh->n_cells; i++) {
          gsl_vector_set(wasora_value_ptr(vector), i, wasora_evaluate_function(function, mesh->cell[i].x));
        }
      }
    } else {
      if (function->type == type_pointwise_mesh_node && function->mesh == mesh) {
        for (i = 0; i < function->data_size; i++) {
          gsl_vector_set(wasora_value_ptr(vector), i, function->data_value[i]);
        }
      } else {
        for (i = 0; i < mesh->n_nodes; i++) {
          gsl_vector_set(wasora_value_ptr(vector), i, wasora_evaluate_function(function, mesh->node[i].x));
        }
      }
    }
  } else {
    if (mesh_fill_vector->centering == centering_cells) {
      for (i = 0; i < mesh->n_cells; i++) {
        mesh_update_coord_vars(mesh->cell[i].x);
        gsl_vector_set(wasora_value_ptr(vector), i, wasora_evaluate_expression(expr));
      }
    } else {
      for (i = 0; i < mesh->n_nodes; i++) {
        mesh_update_coord_vars(mesh->node[i].x);
        gsl_vector_set(wasora_value_ptr(vector), i, wasora_evaluate_expression(expr));
      }
    }
  }

  return WASORA_RUNTIME_OK;
}
