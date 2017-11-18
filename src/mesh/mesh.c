/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related routines
 *
 *  Copyright (C) 2014--2017 jeremy theler
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
#include <thirdparty/kdtree.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>

int wasora_instruction_mesh(void *arg) {

  mesh_t *mesh = (mesh_t *)arg;
  physical_entity_t *physical_entity;
  function_t *function, *tmp_function;
  element_list_item_t *associated_element;
  element_t *element;
  material_t *material;
  material_list_item_t *material_item;
  int i, j, d, v;
  int first_neighbor_nodes;
  double w, vol;
  double cog[3];
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
  
  // barremos los nodos y definimos la bounding box (capaz se pueda meter esto en el loop del kd_tree)
  mesh->bounding_box_min.id = -1;
  mesh->bounding_box_max.id = -1;
  mesh->bounding_box_min.index = NULL;
  mesh->bounding_box_max.index = NULL;
  mesh->bounding_box_min.associated_elements = NULL;
  mesh->bounding_box_max.associated_elements = NULL;
  for (d = 0; d < 3; d++) {
    x_min[d] = mesh->node[0].x[d];
    x_max[d] = mesh->node[0].x[d];
  }
  for (j = 0; j < mesh->n_nodes; j++) {
    for (d = 0; d < 3; d++) {
      if (mesh->node[j].x[d] < x_min[d]) {
        x_min[d] = mesh->bounding_box_min.x[d] = mesh->node[j].x[d];
      }
      if (mesh->node[j].x[d] > x_max[d]) {
        x_max[d] = mesh->bounding_box_max.x[d] = mesh->node[j].x[d];
      }
    }
    
    // si hay muchos materiales asociados al nodo, buscamos el master
    // que es el primer material definido en el input
    if (mesh->node[j].materials_list != NULL) {
      for (material = wasora_mesh.materials; mesh->node[j].master_material == NULL && material != NULL; material = material->hh.next) {
        LL_FOREACH(mesh->node[j].materials_list, material_item) {
          if (material_item->material == material) {
            mesh->node[j].master_material = material;
          }
        }
      }
    }
  }

  // armamos un kd-tree de nodos y miramos cual es la mayor cantidad de vecinos que tiene un nodo
  if (mesh->kd_nodes == NULL) {
    mesh->kd_nodes = kd_create(mesh->spatial_dimensions);
    for (j = 0; j < mesh->n_nodes; j++) {
      kd_insert(mesh->kd_nodes, mesh->node[j].x, &mesh->node[j]);
    
      first_neighbor_nodes = 1;  // el nodo mismo
      LL_FOREACH(mesh->node[j].associated_elements, associated_element) {
        if (associated_element->element->type->dim == mesh->bulk_dimensions) {
          first_neighbor_nodes += (associated_element->element->type->nodes) - (associated_element->element->type->nodes_per_face);
        }
      }
      if (first_neighbor_nodes > mesh->max_first_neighbor_nodes) {
        mesh->max_first_neighbor_nodes = first_neighbor_nodes;
      }
    }
  }
  
  
  // barremos los elementos y resolvemos la physical entity asociada
  // esto lo hacemos solo para la primera malla
  if (wasora_mesh.meshes == mesh) {
    LL_FOREACH(wasora_mesh.physical_entities, physical_entity) {
      if (physical_entity->n_elements != 0) {
        physical_entity->element = malloc(physical_entity->n_elements * sizeof(int));
      }
    }
  }
  
  // TODO: esto esta mezclado! (por que?)
  // no se si se tiene que hacer siempre, capaz se puede meter dentro del gmsh_read o
  // o del generador de structured
  for (i = 0; i < mesh->n_elements; i++) { 
    physical_entity = mesh->element[i].physical_entity;
    if (wasora_mesh.meshes == mesh) {
      if (physical_entity != NULL && physical_entity->i_element < physical_entity->n_elements) {
        physical_entity->element[physical_entity->i_element++] = i;
      }
    }
  }
  
  // rellenamos un array de nodos que pueda ser usado como argumento de funciones
  mesh->nodes_argument = malloc(mesh->spatial_dimensions * sizeof(double *));
  for (d = 0; d < mesh->spatial_dimensions; d++) {
    mesh->nodes_argument[d] = malloc(mesh->n_nodes * sizeof(double));
    for (j = 0; j < mesh->n_nodes; j++) {
      mesh->nodes_argument[d][j] = mesh->node[j].x[d]; 
    }
  }
  
  // idem de celdas
  // TODO: ver si hay que hacerlo siempre
  wasora_call(mesh_element2cell(mesh));
  mesh->cells_argument = malloc(mesh->spatial_dimensions * sizeof(double *));
  for (d = 0; d < mesh->spatial_dimensions; d++) {
    mesh->cells_argument[d] = malloc(mesh->n_cells * sizeof(double));
    for (i = 0; i < mesh->n_cells; i++) {
      mesh->cells_argument[d][i] = mesh->cell[i].x[d]; 
    }
  }

  if (wasora_mesh.main_mesh == mesh) {
    wasora_var(wasora_mesh.vars.cells) = (double)mesh->n_cells;
    wasora_var(wasora_mesh.vars.nodes) = (double)mesh->n_nodes;
    wasora_var(wasora_mesh.vars.elements) = (double)mesh->n_elements;
  }
  
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
  
  // calculamos el volumen (o superficie o longitud) y el centro de masa de las physical entities
  // solo para la primera malla
  if (mesh == wasora_mesh.main_mesh && mesh->bulk_dimensions != 0) {
    LL_FOREACH(wasora_mesh.physical_entities, physical_entity) {
      vol = cog[0] = cog[1] = cog[2] = 0;
      for (i = 0; i < physical_entity->n_elements; i++) {
        element = &mesh->element[physical_entity->element[i]];
        for (v = 0; v < element->type->gauss[GAUSS_POINTS_CANONICAL].V; v++) {
          w = mesh_integration_weight(mesh, element, v);

          for (j = 0; j < element->type->nodes; j++) {
            vol += w * gsl_vector_get(mesh->fem.h, j);
            cog[0] += w * gsl_vector_get(mesh->fem.h, j) * element->node[j]->x[0];
            cog[1] += w * gsl_vector_get(mesh->fem.h, j) * element->node[j]->x[1];
            cog[2] += w * gsl_vector_get(mesh->fem.h, j) * element->node[j]->x[2];
          }
        }
      }
      physical_entity->volume = vol;
      physical_entity->cog[0] = cog[0]/vol;
      physical_entity->cog[1] = cog[1]/vol;
      physical_entity->cog[2] = cog[2]/vol;
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

  physical_entity_t *physical_entity;
  element_list_item_t *element_item, *element_tmp;
  material_list_item_t *material_item, *material_tmp;
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
          LL_FOREACH_SAFE(mesh->element[i].node[j]->associated_elements, element_item, element_tmp) {
            LL_DELETE(mesh->element[i].node[j]->associated_elements, element_item);
            free(element_item);
          }
          LL_FOREACH_SAFE(mesh->element[i].node[j]->materials_list, material_item, material_tmp) {
            LL_DELETE(mesh->element[i].node[j]->materials_list, material_item);
            free(material_item);
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
  mesh->kd_nodes = NULL;

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
  
  mesh->max_first_neighbor_nodes = 1;

  if (mesh == wasora_mesh.meshes) {
    LL_FOREACH(wasora_mesh.physical_entities, physical_entity) {
      physical_entity->n_elements = 0;
      physical_entity->i_element = 0;
      free(physical_entity->element);
      physical_entity->element = NULL;
    }
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
