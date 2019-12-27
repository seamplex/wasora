/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related integration routines
 *
 *  Copyright (C) 2016,2018 jeremy theler
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

int wasora_instruction_mesh_integrate(void *arg) {

  double integral = 0;
  double xi;
  int i, j, v;
  mesh_integrate_t *mesh_integrate = (mesh_integrate_t *)arg;
  mesh_t *mesh = mesh_integrate->mesh;
  element_t *element;
  function_t *function = mesh_integrate->function;
  expr_t *expr = &mesh_integrate->expr;
  physical_entity_t *physical_entity = mesh_integrate->physical_entity;
  var_t *result = mesh_integrate->result;
  
  // esto es lo mismo que findmax y que fillvector
  // TODO: pelar ojo a las expresiones de x y de nx
  if (function != NULL) {
    if (mesh_integrate->centering == centering_cells) {
      if (function->type == type_pointwise_mesh_cell && function->mesh == mesh) {
        for (i = 0; i < mesh->n_cells; i++) {
          if (mesh->cell[i].element->physical_entity == physical_entity) {
            integral += function->data_value[i] * mesh->cell[i].element->type->element_volume(mesh->cell[i].element);
          }
        }
      } else {
        for (i = 0; i < mesh->n_cells; i++) {
          if (mesh->cell[i].element->physical_entity == physical_entity) {
            integral += wasora_evaluate_function(function, mesh->cell[i].x) * mesh->cell[i].element->type->element_volume(mesh->cell[i].element);
          }
        }
      }
    } else {
      if (function->type == type_pointwise_mesh_node && function->mesh == mesh) {
        if (physical_entity->n_elements != 0) {
          for (i = 0; i < physical_entity->n_elements; i++) {
            element = &mesh->element[physical_entity->element[i]];
            for (v = 0; v < element->type->gauss[GAUSS_POINTS_CANONICAL].V; v++) {
              mesh_compute_integration_weight_at_gauss(element, v);

              xi = 0;
              for (j = 0; j < element->type->nodes; j++) {
                xi += element->type->gauss[GAUSS_POINTS_CANONICAL].h[v][j] * function->data_value[element->node[j]->tag - 1];
              }

              integral += element->w[v] * xi;
            }
          }
        } else {
          for (i = 0; i < mesh->n_elements; i++) {
            element = &mesh->element[i];
            if (element->physical_entity != NULL && element->physical_entity == physical_entity) {
              for (v = 0; v < element->type->gauss[GAUSS_POINTS_CANONICAL].V; v++) {
                mesh_compute_integration_weight_at_gauss(element, v);

                xi = 0;
                for (j = 0; j < element->type->nodes; j++) {
                  xi += element->type->gauss[GAUSS_POINTS_CANONICAL].h[v][j] * function->data_value[element->node[j]->tag - 1];
                }

                integral += element->w[v] * xi;
              }
            }
          }          
        }
      } else {
        for (i = 0; i < physical_entity->n_elements; i++) {
          element = &mesh->element[physical_entity->element[i]];
          for (v = 0; v < element->type->gauss[GAUSS_POINTS_CANONICAL].V; v++) {
            mesh_compute_integration_weight_at_gauss(element, v);

            xi = 0;
            for (j = 0; j < element->type->nodes; j++) {
              xi += element->type->gauss[GAUSS_POINTS_CANONICAL].h[v][j] * wasora_evaluate_function(mesh_integrate->function, element->node[j]->x);
            }

            integral += element->w[v] * xi;
          }
        }
      }
    }
  } else {
    if (mesh_integrate->centering == centering_cells) {
      for (i = 0; i < mesh->n_cells; i++) {
        if (mesh->cell[i].element->physical_entity == physical_entity) {
          mesh_update_coord_vars(mesh->cell[i].x);
          integral += wasora_evaluate_expression(expr) * mesh->cell[i].element->type->element_volume(mesh->cell[i].element);
        }
      }
    } else {
      if (physical_entity != NULL && physical_entity->n_elements != 0) {
        for (i = 0; i < physical_entity->n_elements; i++) {
          element = &mesh->element[physical_entity->element[i]];
          for (v = 0; v < element->type->gauss[GAUSS_POINTS_CANONICAL].V; v++) {
            mesh_compute_integration_weight_at_gauss(element, v);
            mesh_compute_x_at_gauss(element, v);
            mesh_update_coord_vars(element->x[v]);
            mesh_compute_normal(element);
            integral += element->w[v] * wasora_evaluate_expression(expr);
          }
        }
      } else {
        for (i = 0; i < mesh->n_elements; i++) {
          element = &mesh->element[i];
          if (element->physical_entity != NULL && element->physical_entity == physical_entity) {
            for (v = 0; v < element->type->gauss[GAUSS_POINTS_CANONICAL].V; v++) {
              mesh_compute_integration_weight_at_gauss(element, v);
              mesh_compute_x_at_gauss(element, v);
              mesh_update_coord_vars(element->x[v]);
              mesh_compute_normal(element);
              integral += element->w[v] * wasora_evaluate_expression(expr);
            }
          }
        }        
      }
      
    }
  }  
  
  wasora_var_value(result) = integral;
  

  return WASORA_RUNTIME_OK;
}
