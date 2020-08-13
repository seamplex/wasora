/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related max-finding routines
 *
 *  Copyright (C) 2016--2020 jeremy theler
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

int wasora_instruction_mesh_find_minmax(void *arg) {

  double min = +INFTY;
  double x_min = 0;
  double y_min = 0;
  double z_min = 0;
  
  double max = -INFTY;
  double x_max = 0;
  double y_max = 0;
  double z_max = 0;
  
  double y;
  int e_i;
  int j; // es que i ya lo usamos
  int i;
  int i_max = 0;
  int i_min = 0;
  
  mesh_find_minmax_t *mesh_find_minmax = (mesh_find_minmax_t *)arg;
  mesh_t *mesh = mesh_find_minmax->mesh;
  physical_entity_t *physical_entity = mesh_find_minmax->physical_entity;
  function_t *function = mesh_find_minmax->function;
  expr_t *expr = &mesh_find_minmax->expr;
  element_t *element = NULL;
  
  // ver si esto es lo optimo en terminos de condicionales y loops
  if (physical_entity == NULL) {
    if (function != NULL) {
      if (mesh_find_minmax->centering == centering_cells) {
        if (function->type == type_pointwise_mesh_cell && function->mesh == mesh) {
          for (i = 0; i < function->data_size; i++) {
            y = function->data_value[i];
            if (y > max) {
              // TODO: SPOT!
              max = y;
              i_max = i;
              x_max = function->data_argument[0][i];
              y_max = (function->n_arguments > 1)?function->data_argument[1][i] : 0;
              z_max = (function->n_arguments > 2)?function->data_argument[2][i] : 0;
            }
            if (y < min) {
              // TODO: SPOT!
              min = y;
              i_min = i;
              x_min = function->data_argument[0][i];
              y_min = (function->n_arguments > 1)?function->data_argument[1][i] : 0;
              z_min = (function->n_arguments > 2)?function->data_argument[2][i] : 0;
            }
          }
        } else {
          for (i = 0; i < mesh->n_cells; i++) {
            y = wasora_evaluate_function(function, mesh->cell[i].x);
            if (y > max) {
              max = y;
              i_max = i;
              x_max = mesh->cell[i].x[0];
              y_max = mesh->cell[i].x[1];
              z_max = mesh->cell[i].x[2];
            }
            if (y < min) {
              min = y;
              i_min = i;
              x_min = mesh->cell[i].x[0];
              y_min = mesh->cell[i].x[1];
              z_min = mesh->cell[i].x[2];
            }
          }
        }
      } else {
        if (function->type == type_pointwise_mesh_node && function->mesh == mesh) {
          for (i = 0; i < function->data_size; i++) {
            y = function->data_value[i];
            if (y > max) {
              max = y;
              i_max = i;
              x_max = function->data_argument[0][i];
              y_max = (function->n_arguments > 1)?function->data_argument[1][i] : 0;
              z_max = (function->n_arguments > 2)?function->data_argument[2][i] : 0;
            }
            if (y < min) {
              min = y;
              i_min = i;
              x_min = function->data_argument[0][i];
              y_min = (function->n_arguments > 1)?function->data_argument[1][i] : 0;
              z_min = (function->n_arguments > 2)?function->data_argument[2][i] : 0;
            }
          }
        } else {
          for (i = 0; i < mesh->n_nodes; i++) {
            y = wasora_evaluate_function(function, mesh->node[i].x);
            if (y > max) {
              max = y;
              i_max = i;
              x_max = mesh->node[i].x[0];
              y_max = mesh->node[i].x[1];
              z_max = mesh->node[i].x[2];
            }
            if (y < min) {
              min = y;
              i_min = i;
              x_min = mesh->node[i].x[0];
              y_min = mesh->node[i].x[1];
              z_min = mesh->node[i].x[2];
            }
          }
        }
      }
    } else {
      if (mesh_find_minmax->centering == centering_cells) {
        for (i = 0; i < mesh->n_cells; i++) {
          mesh_update_coord_vars(mesh->cell[i].x);
          y = wasora_evaluate_expression(expr);
          if (y > max) {
            max = y;
            i_max = i;
            x_max = mesh->cell[i].x[0];
            y_max = mesh->cell[i].x[1];
            z_max = mesh->cell[i].x[2];
          }
          if (y < min) {
            min = y;
            i_min = i;
            x_min = mesh->cell[i].x[0];
            y_min = mesh->cell[i].x[1];
            z_min = mesh->cell[i].x[2];
          }
        }
      } else {
        for (i = 0; i < mesh->n_nodes; i++) {
          wasora_var(wasora_mesh.vars.x) = mesh->node[i].x[0];
          wasora_var(wasora_mesh.vars.y) = mesh->node[i].x[1];
          wasora_var(wasora_mesh.vars.z) = mesh->node[i].x[2];
          y = wasora_evaluate_expression(expr);
          if (y > max) {
            max = y;
            i_max = i;
            x_max = mesh->node[i].x[0];
            y_max = mesh->node[i].x[1];
            z_max = mesh->node[i].x[2];
          }
          if (y < min) {
            min = y;
            i_min = i;
            x_min = mesh->node[i].x[0];
            y_min = mesh->node[i].x[1];
            z_min = mesh->node[i].x[2];
          }
        }
      }
    }
  } else {
    
    if (function != NULL) {
      if (mesh_find_minmax->centering == centering_cells) {
        if (function->type == type_pointwise_mesh_cell && function->mesh == mesh) {
          wasora_push_error_message("MESH_FIND_MINMAX with OVER on a cell-centered function not implemented yet.");
          return WASORA_RUNTIME_ERROR;
        } else {
          wasora_push_error_message("MESH_FIND_MINMAX with OVER on a cell-centered generic function not implemented yet.");
          return WASORA_RUNTIME_ERROR;
        }
      } else {
        if (function->type == type_pointwise_mesh_node && function->mesh == mesh) {
          wasora_push_error_message("MESH_FIND_MINMAX with OVER on a node-centered function not implemented yet.");
          return WASORA_RUNTIME_ERROR;
        } else {
          wasora_push_error_message("MESH_FIND_MINMAX with OVER on a node-centered generic function not implemented yet.");
          return WASORA_RUNTIME_ERROR;
        }
      }
    } else {
      if (mesh_find_minmax->centering == centering_cells) {
        wasora_push_error_message("MESH_FIND_MINMAX with OVER on a cell-centered expression not implemented yet.");
        return WASORA_RUNTIME_ERROR;
      } else {
        for (e_i = 0; e_i < physical_entity->n_elements; e_i++) {
          element = &mesh->element[physical_entity->element[e_i]];
          for (j = 0; j < element->type->nodes; j++) {
            
            wasora_var(wasora_mesh.vars.x) = element->node[j]->x[0];
            wasora_var(wasora_mesh.vars.y) = element->node[j]->x[1];
            wasora_var(wasora_mesh.vars.z) = element->node[j]->x[2];
            y = wasora_evaluate_expression(expr);
            if (y > max) {
              max = y;
              i_max = element->node[j]->index_mesh;
              x_max = element->node[j]->x[0];
              y_max = element->node[j]->x[1];
              z_max = element->node[j]->x[2];
            }
            if (y < min) {
              min = y;
              i_min = element->node[j]->index_mesh;
              x_min = element->node[j]->x[0];
              y_min = element->node[j]->x[1];
              z_min = element->node[j]->x[2];
            }
        
          }
        }
      }
    }

    
  }  

  if (mesh_find_minmax->min != NULL) {
    wasora_value(mesh_find_minmax->min) = min;
  }
  if (mesh_find_minmax->i_min != NULL) {
    wasora_value(mesh_find_minmax->i_min) = (double)i_min;
  }
  if (mesh_find_minmax->x_min != NULL) {
    wasora_value(mesh_find_minmax->x_min) = x_min;
  }
  if (mesh_find_minmax->y_min != NULL) {
    wasora_value(mesh_find_minmax->y_min) = y_min;
  }
  if (mesh_find_minmax->z_min != NULL) {
    wasora_value(mesh_find_minmax->z_min) = z_min;
  }
  
  if (mesh_find_minmax->max != NULL) {
    wasora_value(mesh_find_minmax->max) = max;
  }
  if (mesh_find_minmax->i_max != NULL) {
    wasora_value(mesh_find_minmax->i_max) = (double)i_max;
  }
  if (mesh_find_minmax->x_max != NULL) {
    wasora_value(mesh_find_minmax->x_max) = x_max;
  }
  if (mesh_find_minmax->y_max != NULL) {
    wasora_value(mesh_find_minmax->y_max) = y_max;
  }
  if (mesh_find_minmax->z_max != NULL) {
    wasora_value(mesh_find_minmax->z_max) = z_max;
  }
  

  return WASORA_RUNTIME_OK;
}

