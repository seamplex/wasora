/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related evaluation routines
 *
 *  Copyright (C) 2017 jeremy theler
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

int wasora_instruction_mesh_evaluate(void *arg) {

  int j;
  int belongs_to_entity = 0;
  mesh_evaluate_t *mesh_evaluate = (mesh_evaluate_t *)arg;
  mesh_t *mesh = mesh_evaluate->mesh;
  element_list_item_t *element_list_item;
  function_t *function = mesh_evaluate->function;
//  expr_t *expr = &mesh_evaluate->expr;
  
  if (function != NULL) {
    if (function->type == type_pointwise_mesh_node && function->mesh == mesh) {
      for (j = 0; j < mesh->n_nodes; j++) {
        belongs_to_entity = 0;
        LL_FOREACH(mesh->node[j].associated_elements, element_list_item) {
          if (element_list_item->element->physical_entity == mesh_evaluate->physical_entity) {
            belongs_to_entity = 1;
          }
        }
        if (belongs_to_entity) {
          printf("%g %g %g  %g\n", mesh->node[j].x[0], mesh->node[j].x[1], mesh->node[j].x[2], function->data_value[j]);
        }
      }
    }
  } else {
    wasora_push_error_message("TODO!");
    return WASORA_RUNTIME_ERROR;
  }  
  
  

  return WASORA_RUNTIME_OK;
}
