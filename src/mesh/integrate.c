/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related integration routines
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
#include <wasora.h>

#include <stdio.h>

int wasora_instruction_mesh_integrate(void *arg) {

  double integral = 0;
  double w, xi;
  int i, j, v;
  mesh_integrate_t *mesh_integrate = (mesh_integrate_t *)arg;
  mesh_t *mesh = mesh_integrate->mesh;
  function_t *function = mesh_integrate->function;
  expr_t *expr = &mesh_integrate->expr;
  physical_entity_t *physical_entity = mesh_integrate->physical_entity;
  var_t *result = mesh_integrate->result;
  
  // esto es lo mismo que findmax y que fillvector
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
        for (i = 0; i < mesh->n_elements; i++) {
          if (mesh->element[i].physical_entity == physical_entity) {
            for (v = 0; v < mesh->element[i].type->gauss[GAUSS_POINTS_CANONICAL].V; v++) {
              w = mesh_integration_weight(mesh, &mesh->element[i], v);

              xi = 0;
              for (j = 0; j < mesh->element[i].type->nodes; j++) {
                xi += gsl_vector_get(mesh->fem.h, j) * function->data_value[mesh->element[i].node[j]->id - 1];
              }

              integral += w * xi;
            }
          }
        }
      } else {
        for (i = 0; i < mesh->n_elements; i++) {
          if (mesh->element[i].physical_entity == physical_entity) {
            for (v = 0; v < mesh->element[i].type->gauss[GAUSS_POINTS_CANONICAL].V; v++) {
              w = mesh_integration_weight(mesh, &mesh->element[i], v);

              xi = 0;
              for (j = 0; j < mesh->element[i].type->nodes; j++) {
                xi += gsl_vector_get(mesh->fem.h, j) * wasora_evaluate_function(mesh_integrate->function, mesh->element[i].node[j]->x);
              }

              integral += w * xi;
            }
          }
        }
      }
    }
  } else {
    if (mesh_integrate->centering == centering_cells) {
      for (i = 0; i < mesh->n_cells; i++) {
        if (mesh->cell[i].element->physical_entity == physical_entity) {
          wasora_var(wasora_mesh.vars.x) = mesh->cell[i].x[0];
          wasora_var(wasora_mesh.vars.y) = mesh->cell[i].x[1];
          wasora_var(wasora_mesh.vars.z) = mesh->cell[i].x[2];
          integral += wasora_evaluate_expression(expr) * mesh->cell[i].element->type->element_volume(mesh->cell[i].element);
        }
      }
    } else {
      for (i = 0; i < mesh->n_elements; i++) {
        if (mesh->element[i].physical_entity == physical_entity) {
          for (v = 0; v < mesh->element[i].type->gauss[GAUSS_POINTS_CANONICAL].V; v++) {
            w = mesh_integration_weight(mesh, &mesh->element[i], v);

            xi = 0;
            for (j = 0; j < mesh->element[i].type->nodes; j++) {
              wasora_var(wasora_mesh.vars.x) = mesh->element[i].node[j]->x[0];
              wasora_var(wasora_mesh.vars.y) = mesh->element[i].node[j]->x[1];
              wasora_var(wasora_mesh.vars.z) = mesh->element[i].node[j]->x[2];
              xi += gsl_vector_get(mesh->fem.h, j) * wasora_evaluate_expression(expr);
            }

            integral += w * xi;
          }
        }
      }
    }
  }  
  
  wasora_var_value(result) = integral;
  

  return WASORA_RUNTIME_OK;
}
