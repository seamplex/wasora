/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related routines
 *
 *  Copyright (C) 2014--2020 jeremy theler
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
  node_data_t *node_data;
  int i, j, d, v;
  int first_neighbor_nodes;
  int bulk_dimensions = 0;
  double scale_factor;
  double offset[3];
  double vol;
  double cog[3];
  double x_min[3];
  double x_max[3];

  // TODO: ver como hacer para refrescar la malla tiempo a tiempo
  if (mesh->initialized) {
    return WASORA_RUNTIME_OK;
  }
  
  if (mesh->structured) {
    wasora_call(mesh_create_structured(mesh));
  } else if (mesh->format == mesh_format_gmsh) {
    wasora_call(mesh_gmsh_readmesh(mesh));
  } else if (mesh->format == mesh_format_vtk) {
    wasora_call(mesh_vtk_readmesh(mesh));
  } else if (mesh->format == mesh_format_frd) {
    wasora_call(mesh_frd_readmesh(mesh));
  } else {
    wasora_push_error_message("unsupported mesh format");
    return WASORA_RUNTIME_ERROR;
  }
  
  // barremos los nodos y definimos la bounding box (capaz se pueda meter esto en el loop del kd_tree)
  mesh->bounding_box_min.index_mesh = -1;
  mesh->bounding_box_max.index_mesh = -1;
  mesh->bounding_box_min.index_dof = NULL;
  mesh->bounding_box_max.index_dof = NULL;
  mesh->bounding_box_min.associated_elements = NULL;
  mesh->bounding_box_max.associated_elements = NULL;
  
  
  if (mesh->scale_factor->n_tokens != 0) {
    scale_factor = wasora_evaluate_expression(mesh->scale_factor);
  } else {
    scale_factor = 1;
  }
  offset[0] = wasora_evaluate_expression(mesh->offset_x);
  offset[1] = wasora_evaluate_expression(mesh->offset_y);
  offset[2] = wasora_evaluate_expression(mesh->offset_z);

  for (d = 0; d < 3; d++) {
    x_min[d] = +1e22;
    x_max[d] = -1e22;
  }
  
  for (j = 0; j < mesh->n_nodes; j++) {
    for (d = 0; d < 3; d++) {
      if (scale_factor != 0 || offset[d] != 0) {
        mesh->node[j].x[d] *= scale_factor;
        mesh->node[j].x[d] -= offset[d];
      }
      
      if (mesh->spatial_dimensions < 1 && fabs(mesh->node[j].x[0]) > 1e-6) {
        mesh->spatial_dimensions = 1;
      }
      if (mesh->spatial_dimensions < 2 && fabs(mesh->node[j].x[1]) > 1e-6) {
        mesh->spatial_dimensions = 2;
      }
      if (mesh->spatial_dimensions < 3 && fabs(mesh->node[j].x[2]) > 1e-6) {
        mesh->spatial_dimensions = 3;
      }
      
      if (mesh->node[j].x[d] < x_min[d]) {
        x_min[d] = mesh->bounding_box_min.x[d] = mesh->node[j].x[d];
      }
      if (mesh->node[j].x[d] > x_max[d]) {
        x_max[d] = mesh->bounding_box_max.x[d] = mesh->node[j].x[d];
      }
    }
  }
  
  wasora_call(wasora_vector_init(wasora_mesh.vars.bbox_min));
  wasora_call(wasora_vector_init(wasora_mesh.vars.bbox_max));
  
  gsl_vector_set(wasora_mesh.vars.bbox_min->value, 0, x_min[0]);
  gsl_vector_set(wasora_mesh.vars.bbox_min->value, 1, x_min[1]);
  gsl_vector_set(wasora_mesh.vars.bbox_min->value, 2, x_min[2]);
  gsl_vector_set(wasora_mesh.vars.bbox_max->value, 0, x_max[0]);
  gsl_vector_set(wasora_mesh.vars.bbox_max->value, 1, x_max[1]);
  gsl_vector_set(wasora_mesh.vars.bbox_max->value, 2, x_max[2]);
  
  
  // alocamos los arrays de los elementos que pertenecen a cada entidad fisica
  // (un array es mas eficiente que una linked list)
  for (physical_entity = mesh->physical_entities; physical_entity != NULL; physical_entity = physical_entity->hh.next) {
    if (physical_entity->n_elements != 0) {
      physical_entity->element = malloc(physical_entity->n_elements * sizeof(int));
    }
    // miramos cual es el tag mas grande de las entities porque nos puede servir para alocar arrays temporales
    if (physical_entity->tag > mesh->physical_tag_max) {
      mesh->physical_tag_max = physical_entity->tag;
    }
  }
  
  for (i = 0; i < mesh->n_elements; i++) {
    
    // vemos la dimension del elemento -> la mayor es la de la malla
    if (mesh->element[i].type->dim > bulk_dimensions) {
      bulk_dimensions = mesh->element[i].type->dim;
    }

    // el orden
    if (mesh->element[i].type->order > mesh->order) {
      mesh->order = mesh->element[i].type->order;
    }

    // nos acordamos del elemento que tenga el mayor numero de nodos
    if (mesh->element[i].type->nodes > mesh->max_nodes_per_element) {
      mesh->max_nodes_per_element = mesh->element[i].type->nodes;
    }

    // y del que tenga mayor cantidad de vecinos
    if (mesh->element[i].type->faces > mesh->max_faces_per_element) {
      mesh->max_faces_per_element = mesh->element[i].type->faces;
    }

    // armamos la lista de elementos de cada entidad
    physical_entity = mesh->element[i].physical_entity;
    if (physical_entity != NULL && physical_entity->i_element < physical_entity->n_elements) {
      physical_entity->element[physical_entity->i_element++] = i;
    }
  }
 
  // verificamos que la malla tenga la dimension esperada
  if (mesh->bulk_dimensions == 0) {
    mesh->bulk_dimensions = bulk_dimensions;
  } else if (mesh->bulk_dimensions != bulk_dimensions) {
    wasora_push_error_message("mesh '%s' is expected to have %d dimensions but it has %d", mesh->file->path, mesh->bulk_dimensions, bulk_dimensions);
    return WASORA_RUNTIME_ERROR;
  }
    
  // rellenamos un array de nodos que pueda ser usado como argumento de funciones
  // TODO: poner esto en el loop de arriba?
  mesh->nodes_argument = malloc(mesh->spatial_dimensions * sizeof(double *));
  for (d = 0; d < mesh->spatial_dimensions; d++) {
    mesh->nodes_argument[d] = malloc(mesh->n_nodes * sizeof(double));
    for (j = 0; j < mesh->n_nodes; j++) {
      mesh->nodes_argument[d][j] = mesh->node[j].x[d]; 
    }
  }
  
  // idem de celdas
  if (wasora_mesh.need_cells) {
    wasora_call(mesh_element2cell(mesh));
    mesh->cells_argument = malloc(mesh->spatial_dimensions * sizeof(double *));
    for (d = 0; d < mesh->spatial_dimensions; d++) {
      mesh->cells_argument[d] = malloc(mesh->n_cells * sizeof(double));
      for (i = 0; i < mesh->n_cells; i++) {
        mesh->cells_argument[d][i] = mesh->cell[i].x[d]; 
      }
    }
  }

  if (wasora_mesh.main_mesh == mesh) {
    wasora_var(wasora_mesh.vars.cells) = (double)mesh->n_cells;
    wasora_var(wasora_mesh.vars.nodes) = (double)mesh->n_nodes;
    wasora_var(wasora_mesh.vars.elements) = (double)mesh->n_elements;
  }
  
  // vemos si nos quedo alguna funcion sin leer
  LL_FOREACH(mesh->node_datas, node_data) {
    if (node_data->function->mesh == NULL) {
      wasora_push_error_message("cannot find function '%s' in mesh '%s", node_data->name_in_mesh, mesh->name);
      return WASORA_RUNTIME_ERROR;
    }
    
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
  if (mesh->bulk_dimensions != 0) {
    for (physical_entity = mesh->physical_entities; physical_entity != NULL; physical_entity = physical_entity->hh.next) {
      vol = cog[0] = cog[1] = cog[2] = 0;
      for (i = 0; i < physical_entity->n_elements; i++) {
        element = &mesh->element[physical_entity->element[i]];
        for (v = 0; v < element->type->gauss[mesh->integration].V; v++) {
          mesh_compute_integration_weight_at_gauss(element, v, mesh->integration);

          for (j = 0; j < element->type->nodes; j++) {
            vol += element->w[v] * element->type->gauss[mesh->integration].h[v][j];
            cog[0] += element->w[v] * element->type->gauss[mesh->integration].h[v][j] * element->node[j]->x[0];
            cog[1] += element->w[v] * element->type->gauss[mesh->integration].h[v][j] * element->node[j]->x[1];
            cog[2] += element->w[v] * element->type->gauss[mesh->integration].h[v][j] * element->node[j]->x[2];
          }
        }
      }
      physical_entity->volume = vol;
      physical_entity->cog[0] = cog[0]/vol;
      physical_entity->cog[1] = cog[1]/vol;
      physical_entity->cog[2] = cog[2]/vol;

      // las pasamos a wasora para que esten disponibles en el input
      if (physical_entity->var_vol != NULL) {
        wasora_var_value(physical_entity->var_vol) = vol;
      }

      if (physical_entity->vector_cog != NULL) {
        if (!physical_entity->vector_cog->initialized) {
          wasora_call(wasora_vector_init(physical_entity->vector_cog));
        }
        gsl_vector_set(physical_entity->vector_cog->value, 0, physical_entity->cog[0]);
        gsl_vector_set(physical_entity->vector_cog->value, 1, physical_entity->cog[1]);
        gsl_vector_set(physical_entity->vector_cog->value, 2, physical_entity->cog[2]);
      }
    }
  }

  // create a k-dimensional tree and try to figure out what the maximum number of neighbours each node has
  if (mesh->kd_nodes == NULL) {
    mesh->kd_nodes = kd_create(mesh->spatial_dimensions);
    for (j = 0; j < mesh->n_nodes; j++) {
      kd_insert(mesh->kd_nodes, mesh->node[j].x, &mesh->node[j]);
    
      first_neighbor_nodes = 1;  // el nodo mismo
      LL_FOREACH(mesh->node[j].associated_elements, associated_element) {
        if (associated_element->element->type->dim == mesh->bulk_dimensions) {
          if (associated_element->element->type->id == ELEMENT_TYPE_TETRAHEDRON4 ||
              associated_element->element->type->id == ELEMENT_TYPE_TETRAHEDRON10) {
            // los tetrahedros son "buenos" y con esta cuenta nos ahorramos memoria
            first_neighbor_nodes += (associated_element->element->type->nodes) - (associated_element->element->type->nodes_per_face);
          } else {
            // si tenemos elementos generales, hay que allocar mas memoria
            first_neighbor_nodes += (associated_element->element->type->nodes) - 1;
          }
        }
      }
      if (first_neighbor_nodes > mesh->max_first_neighbor_nodes) {
        mesh->max_first_neighbor_nodes = first_neighbor_nodes;
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
    // pedimos los que estén en un radio de algunas veces (cuantas?) veces el anterior
    presults = kd_nearest_range(mesh->kd_nodes, x, 7*mesh_subtract_module(x, x_nearest));
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
// esta en el input (dimensiones, grados de libertad, entidades fisicas, etc)
int mesh_free(mesh_t *mesh) {

  physical_entity_t *physical_entity;
//  physical_entity_t *physical_entity_tmp;
  element_list_item_t *element_item, *element_tmp;
  int i, j, d, v;
  
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
    for (d = 0; d < mesh->spatial_dimensions; d++) {
      free(mesh->cells_argument[d]);
    }
    free(mesh->cells_argument);
    free(mesh->cell);
  }
  mesh->cell = NULL;
  mesh->n_cells = 0;
  mesh->max_faces_per_element = 0;

  // elements  
  if (mesh->element != NULL) {
    for (i = 0; i < mesh->n_elements; i++) {
      if (mesh->element[i].node != NULL) {
        for (j = 0; j < mesh->element[i].type->nodes; j++) {
          LL_FOREACH_SAFE(mesh->element[i].node[j]->associated_elements, element_item, element_tmp) {
            LL_DELETE(mesh->element[i].node[j]->associated_elements, element_item);
            free(element_item);
          }
          
          if (mesh->element[i].dphidx_node != NULL) {
            gsl_matrix_free(mesh->element[i].dphidx_node[j]);
          }
          
        }
        free(mesh->element[i].node);
        
        if (mesh->element[i].dphidx_node != NULL) {
          free(mesh->element[i].dphidx_node);
        }
      }
      
      if (mesh->element[i].type != NULL && mesh->element[i].type->gauss != NULL) {
        for (v = 0; v < mesh->element[i].type->gauss[mesh->integration].V; v++) {

          if (mesh->element[i].x != NULL && mesh->element[i].x[v] != NULL) {
            free(mesh->element[i].x[v]);
          }
          if (mesh->element[i].H != NULL && mesh->element[i].H[v] != NULL) {
            gsl_matrix_free(mesh->element[i].H[v]);
          }  
          if (mesh->element[i].B != NULL && mesh->element[i].B[v] != NULL) {
            gsl_matrix_free(mesh->element[i].B[v]);
          }  
          if (mesh->element[i].dxdr != NULL && mesh->element[i].dxdr[v] != NULL) {
            gsl_matrix_free(mesh->element[i].dxdr[v]);
          }
          if (mesh->element[i].drdx != NULL && mesh->element[i].drdx[v] != NULL) {
            gsl_matrix_free(mesh->element[i].drdx[v]);
          }  
          if (mesh->element[i].dhdx != NULL && mesh->element[i].dhdx[v] != NULL) {
            gsl_matrix_free(mesh->element[i].dhdx[v]);
          }  
          if (mesh->element[i].dphidx_gauss != NULL && mesh->element[i].dphidx_gauss[v] != NULL) {
            gsl_matrix_free(mesh->element[i].dphidx_gauss[v]);
          }  
        }
      }  
      
      if (mesh->element[i].w != NULL) {
        free(mesh->element[i].w);
      }  
      if (mesh->element[i].x != NULL) {
        free(mesh->element[i].x);
      }  
      if (mesh->element[i].H != NULL) {
        free(mesh->element[i].H);
      }  
      if (mesh->element[i].B != NULL) {
        free(mesh->element[i].B);
      }  
      if (mesh->element[i].dxdr != NULL) {
        free(mesh->element[i].dxdr);
      }  
      if (mesh->element[i].drdx != NULL) {
        free(mesh->element[i].drdx);
      }  
      if (mesh->element[i].dhdx != NULL) {
        free(mesh->element[i].dhdx);
      }  
      if (mesh->element[i].dphidx_gauss != NULL) {
        free(mesh->element[i].dphidx_gauss);
      }  
      if (mesh->element[i].l != NULL) {
        free(mesh->element[i].l);
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

  // nodes
  if (mesh->node != NULL) {
    for (j = 0; j < mesh->n_nodes; j++) {
      if (mesh->node[j].index_dof != NULL) {
        free (mesh->node[j].index_dof);
      }
      if (mesh->node[j].phi != NULL) {
        free(mesh->node[j].phi);
      }
      if (mesh->node[j].dphidx != NULL) {
        gsl_matrix_free(mesh->node[j].dphidx);
      }
      if (mesh->node[j].delta_dphidx != NULL) {
        gsl_matrix_free(mesh->node[j].delta_dphidx);
      }
      if (mesh->node[j].f != NULL) {
        free(mesh->node[j].f);
      }
    }
    free(mesh->node);
  }
  mesh->node = NULL;
  mesh->n_nodes = 0;
  mesh->max_first_neighbor_nodes = 1;

  for (physical_entity = mesh->physical_entities; physical_entity != NULL; physical_entity = physical_entity->hh.next) {
    physical_entity->n_elements = 0;
    physical_entity->i_element = 0;
    free(physical_entity->element);
    physical_entity->element = NULL;
  }
/*
  for (d = 0; d < 4; d++) {
    HASH_ITER(hh_tag[d], mesh->physical_entities_by_tag[d], physical_entity, physical_entity_tmp) {
      HASH_DELETE(hh_tag[d], mesh->physical_entities_by_tag[d], physical_entity);
    }
  }
  HASH_ITER(hh, mesh->physical_entities, physical_entity, physical_entity_tmp) {
    HASH_DEL(mesh->physical_entities, physical_entity);
    // si hacemos free de la entidad en si entonces perdemos la informacion sobre BCs
    // TODO: pensar!    
    // free(physical_entity->name);
    // free(physical_entity->element);
    // free(physical_entity);
  }
*/    
  
  mesh->initialized = 0;

  return WASORA_RUNTIME_OK;
}


// devuelve la direccion de la estructura de la variable que se llama name
mesh_t *wasora_get_mesh_ptr(const char *name) {
  mesh_t *mesh;
  HASH_FIND_STR(wasora_mesh.meshes, name, mesh);
  return mesh;
}
