/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related routines
 *
 *  Copyright (C) 2014--2015 jeremy theler
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
#include <errno.h>
#include <string.h>

#include <wasora.h>
#include <thirdparty/kdtree.h>
#include "mesh.h"


int wasora_instruction_mesh(void *arg) {

  mesh_t *mesh = (mesh_t *)arg;
  physical_entity_t *physical_entity;
  function_t *function, *tmp_function;
  int i, j;
  double x_min[3];
  double x_max[3];

  // TODO: ver como hacer para refrescar la malla tiempo a tiempo
  if (mesh->initialized) {
    return WASORA_RUNTIME_OK;
  }
  
  if (mesh->structured) {
    wasora_call(mesh_create_structured(mesh));
  } else {
    wasora_call(mesh_gmsh_readmesh(mesh));
  }
  
  // barremos los nodos y definimos la bounding box
  mesh->bounding_box_min.id = -1;
  mesh->bounding_box_max.id = -1;
  mesh->bounding_box_min.index = NULL;
  mesh->bounding_box_max.index = NULL;
  mesh->bounding_box_min.associated_elements = NULL;
  mesh->bounding_box_max.associated_elements = NULL;
  for (i = 0; i < 3; i++) {
    x_min[i] = mesh->node[0].x[i];
    x_max[i] = mesh->node[0].x[i];
  }
  for (i = 0; i < mesh->n_nodes; i++) {
    for (j = 0; j < 3; j++) {
      if (mesh->node[i].x[j] < x_min[j]) {
        x_min[j] = mesh->node[i].x[j];
        mesh->bounding_box_min.x[j] = x_min[j];
      }
      if (mesh->node[i].x[j] > x_max[j]) {
        x_max[j] = mesh->node[i].x[j];
        mesh->bounding_box_max.x[j] = x_max[j];
      }
    }
  }

  // barremos los elementos y resolvemos la physical entity asociada
  for (i = 0; i < mesh->n_elements; i++) {
    if (mesh->element[i].tag != NULL && mesh->element[i].tag[0] != 0) {
      HASH_FIND(hh_id, wasora_mesh.physical_entities_by_id, &mesh->element[i].tag[0], sizeof(int), physical_entity);
      mesh->element[i].physical_entity = physical_entity;
    }
  }
  
  // rellenamos un array de nodos que pueda ser usado como argumento de funciones
  mesh->nodes_argument = malloc(mesh->spatial_dimensions * sizeof(double *));
  for (i = 0; i < mesh->spatial_dimensions; i++) {
    mesh->nodes_argument[i] = malloc(mesh->n_nodes * sizeof(double));
    for (j = 0; j < mesh->n_nodes; j++) {
      mesh->nodes_argument[i][j] = mesh->node[j].x[i]; 
    }
  }
  
  // idem de celdas
  // TODO: ver si hay que hacerlo siempre
  wasora_call(mesh_element2cell(mesh));
  mesh->cells_argument = malloc(mesh->spatial_dimensions * sizeof(double *));
  for (i = 0; i < mesh->spatial_dimensions; i++) {
    mesh->cells_argument[i] = malloc(mesh->n_cells * sizeof(double));
    for (j = 0; j < mesh->n_cells; j++) {
      mesh->cells_argument[i][j] = mesh->cell[j].x[i]; 
    }
  }

  wasora_var(wasora_mesh.vars.cells) = (double)mesh->n_cells;
  wasora_var(wasora_mesh.vars.nodes) = (double)mesh->n_nodes;
  wasora_var(wasora_mesh.vars.elements) = (double)mesh->n_elements;
  
  // barremos todas las funciones, si encontramos alguna que tenga una malla la linkeamos a esta
  HASH_ITER(hh, wasora.functions, function, tmp_function) {
    if (function->mesh != NULL && function->mesh == mesh) {

      function->initialized = 0;
      
      if (function->type == type_pointwise_mesh_node) {
        function->data_size = mesh->n_nodes;
        function->data_argument = mesh->nodes_argument;
      } else if (function->type == type_pointwise_mesh_cell) {
        function->data_size = mesh->n_cells;
        function->data_argument = mesh->cells_argument;
      }
      
      if (function->vector_value != NULL) {
        function->vector_value->size = function->data_size;
      }
    }
  }
  
  
  // esto es todo amigos!
  mesh->initialized = 1;

  return WASORA_RUNTIME_OK;

}

element_t *mesh_find_element(mesh_t *mesh, const double *x) {

  element_t *element = NULL;
  element_list_item_t *associated_element;
  node_t *node;
  double distance;
  double x_nearest[3] = {0, 0, 0};
  struct kdres *rset;  //Auxiliary.
  rset = kd_nearest(mesh->kd_nodes, x);
  // buscamoe el nodo mas cercano al punto x
  node = (node_t *)(kd_res_item(rset, x_nearest));
  kd_res_free(rset);

  // barremos los elementos asociados al este nodo
  // si el punto x cae dentro de estos poquitos elementos
  LL_FOREACH(node->associated_elements, associated_element) {
    if (associated_element->element->type->dim == mesh->bulk_dimensions && associated_element->element->type->point_in_element(associated_element->element, x)) {
      element = associated_element->element;
      break;
    }
  }
  
  // si no encontramos ninguno entonces capaz que la malla este deformada, probamos con otros nodos
  if (element == NULL) {
    struct kdres *presults;
    // pedimos los que estén en un radio de una vez y media veces el anterior
    presults = kd_nearest_range(mesh->kd_nodes, x, 1.5*mesh_subtract_module(x, x_nearest));
    while(element == NULL && kd_res_end(presults) == 0) {
      node = (node_t *)(kd_res_item(presults, x_nearest));
      LL_FOREACH(node->associated_elements, associated_element) {
        if (associated_element->element->type->dim == mesh->bulk_dimensions && associated_element->element->type->point_in_element(associated_element->element, x)) {
          element = associated_element->element;
          break;
        }
      }
      kd_res_next(presults);
    }
    kd_res_free(presults);
  }
  
  
  // TODO: regla SPOT
  if (element == NULL) {
    // si no encontro ninguno vemos si x esta cerquita del nodo o no
    distance = 0;
    if (mesh->spatial_dimensions == 1) {
      distance = gsl_pow_2(x[0]-node->x[0]);
    } else if (mesh->spatial_dimensions == 2) {
    	distance = mesh_subtract_squared_module2d(x, node->x);
    } else if (mesh->spatial_dimensions == 3) {
      distance = mesh_subtract_squared_module(x, node->x);
    }

    // si estamos cerquita, ponemos el primero que tenga la dimension correcta y ya
    if (distance < DEFAULT_MULTIDIM_INTERPOLATION_THRESHOLD) {
      LL_FOREACH(node->associated_elements, associated_element) {
        if (associated_element->element->type->dim == mesh->bulk_dimensions) {
          element = associated_element->element;
        }
      }
    }
  }

  return element;
}

// libera lo que allocamos al leerla, pero no lo que
// esta en el input (dimensiones, grados de libertad, etc)
int mesh_free(mesh_t *mesh) {

  element_list_item_t *item, *tmp;
  int i, j, k;

  if (mesh->cell != NULL) {
    for (i = 0; i < mesh->n_cells; i++) {
      if (mesh->cell[i].index != NULL) {
        free(mesh->cell[i].index);
      }
      if (mesh->cell[i].neighbor != NULL) {
        free(mesh->cell[i].neighbor);
      }
      if (mesh->cell[i].ifaces != NULL) {
        for (j = 0; j < mesh->cell[i].element->type->faces; j++) {
          free(mesh->cell[i].ifaces[j]);
        }
        free(mesh->cell[i].ifaces);
      }
      if (mesh->cell[i].ineighbor != NULL) {
        free(mesh->cell[i].ineighbor);
      }
    }
    free(mesh->cell);
  }
  mesh->cell = NULL;
  mesh->n_cells = 0;
  mesh->max_faces_per_element = 0;

  if (mesh->element != NULL) {
    for (i = 0; i < mesh->n_elements; i++) {
      free(mesh->element[i].tag);
      if (mesh->element[i].node != NULL) {
        for (j = 0; j < mesh->element[i].type->nodes; j++) {
          LL_FOREACH_SAFE(mesh->element[i].node[j]->associated_elements, item, tmp) {
            LL_DELETE(mesh->element[i].node[j]->associated_elements, item);
            free(item);
          }          
        }
        free(mesh->element[i].node);
      }
    }
    free(mesh->element);
  }
  mesh->element = NULL;
  mesh->n_elements = 0;
  mesh->max_nodes_per_element = 0;

  if (mesh->kd_nodes != NULL) {
    kd_free(mesh->kd_nodes);
  }

  if (mesh->node != NULL) {
    for (k = 0; k < mesh->n_nodes; k++) {
      if (mesh->node[k].index != NULL) {
        free (mesh->node[k].index);
      }
    }
    free(mesh->node);
  }
  mesh->node = NULL;
  mesh->n_nodes = 0;

  if (mesh->fem.r != NULL) {
    gsl_vector_free(mesh->fem.r);
    mesh->fem.r = NULL;
    gsl_vector_free(mesh->fem.h);
    mesh->fem.h = NULL;
    gsl_matrix_free(mesh->fem.dhdr);
    mesh->fem.dhdr = NULL;
    gsl_matrix_free(mesh->fem.dhdx);
    mesh->fem.dhdx = NULL;
    gsl_matrix_free(mesh->fem.drdx);
    mesh->fem.drdx = NULL;
    gsl_matrix_free(mesh->fem.dxdr);
    mesh->fem.dxdr = NULL;
    free(mesh->fem.l);
    mesh->fem.l = NULL;
  }
  
  mesh->initialized = 0;

  return WASORA_RUNTIME_OK;
}


// devuelve la direccion de la estructura de la variable que se llama name
mesh_t *wasora_get_mesh_ptr(const char *name) {
  mesh_t *mesh;
  HASH_FIND_STR(wasora_mesh.meshes, name, mesh);
  return mesh;
}
#endif
