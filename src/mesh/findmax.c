/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related max-finding routines
 *
 *  Copyright (C) 2016 jeremy theler
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
#ifdef WASORA_MESH
#include <stdio.h>

#include <wasora.h>
#include "mesh.h"

int wasora_instruction_mesh_find_max(void *arg) {

  double max = -INFTY;
  double x_max = 0;
  double y_max = 0;
  double z_max = 0;
  double y;
  int i;
  int i_max = 0;
  
  mesh_find_max_t *mesh_find_max = (mesh_find_max_t *)arg;
  mesh_t *mesh = mesh_find_max->mesh;
  function_t *function = mesh_find_max->function;
  expr_t *expr = &mesh_find_max->expr;
  
  // ver si esto es lo optimo en terminos de condicionales y loops
  if (function != NULL) {
    if (mesh_find_max->centering == centering_cells) {
      if (function->type == type_pointwise_mesh_cell && function->mesh == mesh) {
        for (i = 0; i < function->data_size; i++) {
          if ((y = function->data_value[i]) > max) {
            // TODO: SPOT!
            max = y;
            i_max = i;
            x_max = function->data_argument[0][i];
            y_max = (function->n_arguments > 1)?function->data_argument[1][i] : 0;
            z_max = (function->n_arguments > 2)?function->data_argument[2][i] : 0;
          }
        }
      } else {
        for (i = 0; i < mesh->n_cells; i++) {
          if ((y = wasora_evaluate_function(function, mesh->cell[i].x)) > max) {
            max = y;
            i_max = i;
            x_max = mesh->cell[i].x[0];
            y_max = (mesh->bulk_dimensions > 1)?mesh->cell[i].x[1] : 0;
            z_max = (function->n_arguments > 2)?mesh->cell[i].x[2] : 0;
          }
        }
      }
    } else {
      if (function->type == type_pointwise_mesh_node && function->mesh == mesh) {
        for (i = 0; i < function->data_size; i++) {
          if ((y = function->data_value[i]) > max) {
            max = y;
            i_max = i;
            x_max = function->data_argument[0][i];
            y_max = (function->n_arguments > 1)?function->data_argument[1][i] : 0;
            z_max = (function->n_arguments > 2)?function->data_argument[2][i] : 0;
          }
        }
      } else {
        for (i = 0; i < mesh->n_nodes; i++) {
          if ((y = wasora_evaluate_function(function, mesh->node[i].x)) > max) {
            max = y;
            i_max = i;
            x_max = mesh->node[i].x[0];
            y_max = (mesh->bulk_dimensions > 1)?mesh->node[i].x[1] : 0;
            z_max = (mesh->bulk_dimensions > 2)?mesh->node[i].x[2] : 0;
          }
        }
      }
    }
  } else {
    if (mesh_find_max->centering == centering_cells) {
      for (i = 0; i < mesh->n_cells; i++) {
        wasora_var(wasora_mesh.vars.x) = mesh->cell[i].x[0];
        wasora_var(wasora_mesh.vars.y) = mesh->cell[i].x[1];
        wasora_var(wasora_mesh.vars.z) = mesh->cell[i].x[2];
        if ((y = wasora_evaluate_expression(expr)) > max) {
          max = y;
          i_max = i;
          x_max = mesh->cell[i].x[0];
          y_max = (mesh->bulk_dimensions > 1)?mesh->cell[i].x[1] : 0;
          z_max = (mesh->bulk_dimensions > 2)?mesh->cell[i].x[2] : 0;
        }
      }
    } else {
      for (i = 0; i < mesh->n_nodes; i++) {
        wasora_var(wasora_mesh.vars.x) = mesh->node[i].x[0];
        wasora_var(wasora_mesh.vars.y) = mesh->node[i].x[1];
        wasora_var(wasora_mesh.vars.z) = mesh->node[i].x[2];
        if ((y = wasora_evaluate_expression(expr)) > max) {
          max = y;
          i_max = i;
          x_max = mesh->node[i].x[0];
          y_max = (mesh->bulk_dimensions > 1) ? mesh->node[i].x[1] : 0;
          z_max = (mesh->bulk_dimensions > 2) ? mesh->node[i].x[2] : 0;
        }
      }
    }
  }

  if (mesh_find_max->max != NULL) {
    wasora_value(mesh_find_max->max) = max;
  }
  if (mesh_find_max->i_max != NULL) {
    wasora_value(mesh_find_max->i_max) = (double)i_max;
  }
  if (mesh_find_max->x_max != NULL) {
    wasora_value(mesh_find_max->x_max) = x_max;
  }
  if (mesh_find_max->y_max != NULL) {
    wasora_value(mesh_find_max->y_max) = y_max;
  }
  if (mesh_find_max->z_max != NULL) {
    wasora_value(mesh_find_max->z_max) = z_max;
  }
  

  return WASORA_RUNTIME_OK;
}
#endif
