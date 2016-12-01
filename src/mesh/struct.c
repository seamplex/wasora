/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related structured mesh routines
 *
 *  Copyright (C) 2014 jeremy theler
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
#include <math.h>

#include <wasora.h>
#include <thirdparty/kdtree.h>
#include "mesh.h"

extern element_type_t element_types[16];

#define flat_index(i,j,k) ((i) + (j)*mesh->ncells_x + (k)*mesh->ncells_x*mesh->ncells_y) 

int mesh_create_structured(mesh_t *mesh) {

  int i_element, i_cell, i_node, i_entity;
  int i, j, k;
  int node_index;
  int volume_element_type;
  int surface_element_type;
  
  double length_x = 0;
  double length_y = 0;
  double length_z = 0;
  double halfeps = 0.5*wasora_var(wasora_mesh.vars.eps);
  double xi;
  physical_entity_t *physical_entity;
  neighbor_t *neighbor;
  int i_min, i_max, j_min, j_max, k_min, k_max;

  // esto es por la numeracion que eligio gmsh de los hexahedros
  int deltai[8] = {0, 1, 1, 0, 0, 1, 1, 0};
  int deltaj[8] = {0, 0, 1, 1, 0, 0, 1, 1};
  int deltak[8] = {0, 0, 0, 0, 1, 1, 1, 1};

  if (mesh->bulk_dimensions == 0) {
    wasora_push_error_message("structured mesh needs number of dimensions");
    return WASORA_RUNTIME_ERROR;
  } else if (mesh->bulk_dimensions < 0 || mesh->bulk_dimensions > 3) {
    wasora_push_error_message("invalidad number of dimensions '%d'", mesh->bulk_dimensions);
    return WASORA_RUNTIME_ERROR;
  }
  
  // evaluamos los ncells, length, etc
  mesh->ncells_x = wasora_evaluate_expression(mesh->expr_ncells_x);
  mesh->ncells_y = wasora_evaluate_expression(mesh->expr_ncells_y);
  mesh->ncells_z = wasora_evaluate_expression(mesh->expr_ncells_z);
  
  mesh->length_x = wasora_evaluate_expression(mesh->expr_length_x);
  mesh->length_y = wasora_evaluate_expression(mesh->expr_length_y);
  mesh->length_z = wasora_evaluate_expression(mesh->expr_length_z);
  
  mesh->uniform_delta_x = wasora_evaluate_expression(mesh->expr_uniform_delta_x);
  mesh->uniform_delta_y = wasora_evaluate_expression(mesh->expr_uniform_delta_y);
  mesh->uniform_delta_z = wasora_evaluate_expression(mesh->expr_uniform_delta_z);
  


  if ((mesh->ncells_x == 0 && mesh->length_x == 0) ||
      (mesh->length_x == 0 && mesh->uniform_delta_x == 0) || 
      (mesh->ncells_x == 0 && mesh->uniform_delta_x == 0)) {
      wasora_push_error_message("structured mesh '%s' needs two out of three of NCELLS_X, LENGTH_X and DELTA_X", mesh->name);
      return WASORA_RUNTIME_ERROR;
  }
  if (mesh->uniform_delta_x == 0) {
    mesh->uniform_delta_x = mesh->length_x / (double)(mesh->ncells_x);
  }
  if (mesh->length_x == 0) {
    mesh->length_x = mesh->ncells_x * mesh->uniform_delta_x;
  }
  if (mesh->ncells_x == 0) {
    xi = mesh->length_x / mesh->uniform_delta_x;
    if (fabs(xi-round(xi)) > halfeps) {
      wasora_push_error_message("LENGTH_X (%g) is not a multiple of DELTA_X (%g)", mesh->length_x, mesh->uniform_delta_x);
      return WASORA_RUNTIME_ERROR;
    }
    mesh->ncells_x = (int)xi;
  }
  
  mesh->nodes_x = malloc((mesh->ncells_x+1) * sizeof(double));
  mesh->delta_x = malloc((mesh->ncells_x) * sizeof(double));
  mesh->cells_x = malloc(mesh->ncells_x * sizeof(double));
  mesh->nodes_x[0] = 0;   // TODO: offset
  for (i = 0; i < mesh->ncells_x; i++) {
    mesh->delta_x[i] = mesh->uniform_delta_x;
    length_x += mesh->delta_x[i];
    mesh->nodes_x[i+1] = mesh->nodes_x[i] + mesh->delta_x[i];
    mesh->cells_x[i] = mesh->nodes_x[i+1] - 0.5*mesh->delta_x[i];
  }
  if (fabs(mesh->length_x-length_x) > halfeps)  {
    wasora_push_error_message("mismatch in delta_x");
    return WASORA_RUNTIME_ERROR;
  }
  
  if (mesh->bulk_dimensions > 1) {
    if ((mesh->ncells_y == 0 && mesh->length_y == 0) ||
        (mesh->length_y == 0 && mesh->uniform_delta_y == 0) || 
        (mesh->ncells_y == 0 && mesh->uniform_delta_y == 0)) {
        wasora_push_error_message("structured mesh '%s' needs two out of three of NCELLS_Y, LENGTH_Y and DELTA_Y", mesh->name);
        return WASORA_RUNTIME_ERROR;
    }
    if (mesh->uniform_delta_y == 0) {
      mesh->uniform_delta_y = mesh->length_y / (double)(mesh->ncells_y);
    }
    if (mesh->length_y == 0) {
      mesh->length_y = mesh->ncells_y * mesh->uniform_delta_y;
    }
    if (mesh->ncells_y == 0) {
      xi = mesh->length_y / mesh->uniform_delta_y;
      if (fabs(xi-round(xi)) > halfeps) {
        wasora_push_error_message("LENGTH_Y (%)g is not a multiple of DELTA_Y (%g)", mesh->length_y, mesh->uniform_delta_y);
        return WASORA_RUNTIME_ERROR;
      }
      mesh->ncells_y = (int)xi;
    }
    
    mesh->nodes_y = malloc((mesh->ncells_y+1) * sizeof(double));
    mesh->delta_y = malloc((mesh->ncells_y) * sizeof(double));
    mesh->cells_y = malloc(mesh->ncells_y * sizeof(double));
    mesh->nodes_y[0] = 0;   // TODO: offset
    for (j = 0; j < mesh->ncells_y; j++) {
      mesh->delta_y[j] = mesh->uniform_delta_y;
      length_y += mesh->delta_y[j];
      mesh->nodes_y[j+1] = mesh->nodes_y[j] + mesh->delta_y[j];
      mesh->cells_y[j] = mesh->nodes_y[j+1] - 0.5*mesh->delta_y[j];
    }
    if (fabs(mesh->length_y-length_y) > 1e-6)  {
      wasora_push_error_message("mismatch in delta_y");
      return WASORA_RUNTIME_ERROR;
    }
      
  } else {
    mesh->ncells_y = 1;
    mesh->delta_y = malloc(sizeof(double));
    mesh->nodes_y = malloc(sizeof(double));
    mesh->cells_y = malloc(sizeof(double));
    mesh->delta_y[0] = 1;
    mesh->nodes_y[0] = 0;
    mesh->cells_y[0] = 0;
  }
  
  if (mesh->bulk_dimensions > 2) {
    if ((mesh->ncells_z == 0 && mesh->length_z == 0) ||
        (mesh->length_z == 0 && mesh->uniform_delta_z == 0) || 
        (mesh->ncells_z == 0 && mesh->uniform_delta_z == 0)) {
        wasora_push_error_message("structured mesh '%s' needs two out of three of NCELLS_Z, LENGTH_Z and DELTA_Z", mesh->name);
        return WASORA_RUNTIME_ERROR;
    }
    if (mesh->uniform_delta_z == 0) {
      mesh->uniform_delta_z = mesh->length_z / (double)(mesh->ncells_z);
    }
    if (mesh->length_z == 0) {
      mesh->length_z = mesh->ncells_z * mesh->uniform_delta_z;
    }
    if (mesh->ncells_z == 0) {
      xi = mesh->length_z / mesh->uniform_delta_z;
      if (fabs(xi-round(xi)) > halfeps) {
        wasora_push_error_message("LENGTH_Y (%)g is not a multiple of DELTA_Z (%g)", mesh->length_z, mesh->uniform_delta_z);
        return WASORA_RUNTIME_ERROR;
      }
      mesh->ncells_z = (int)xi;
    }
  
    mesh->nodes_z = malloc((mesh->ncells_z+1) * sizeof(double));
    mesh->delta_z = malloc((mesh->ncells_z) * sizeof(double));
    mesh->cells_z = malloc(mesh->ncells_z * sizeof(double));
    mesh->nodes_z[0] = 0;   // TODO: offset
    for (k = 0; k < mesh->ncells_z; k++) {
      mesh->delta_z[k] = mesh->uniform_delta_z;
      length_z += mesh->delta_z[k];
      mesh->nodes_z[k+1] = mesh->nodes_z[k] + mesh->delta_z[k];
      mesh->cells_z[k] = mesh->nodes_z[k+1] - 0.5*mesh->delta_z[k];

    }
    if (fabs(mesh->length_z-length_z) > 1e-6)  {
      wasora_push_error_message("mismatch in delta_z");
      return WASORA_RUNTIME_ERROR;
    }

  } else {
    mesh->ncells_z = 1;
    mesh->delta_z = malloc(sizeof(double));
    mesh->nodes_z = malloc(sizeof(double));
    mesh->cells_z = malloc(sizeof(double));
    mesh->delta_z[0] = 1;
    mesh->nodes_z[0] = 0;
    mesh->cells_z[0] = 0;
  }
  

  // fabricamos los nodos
  // como vamos a hacer volumenes finitos y queremos hacer la menor cantidad de lio
  // con switces de la cantidad de dimensiones posibles, hacemos el lio aca con los
  // nodos y ya, el resto del tiempo manejamos celdas que funcionan bien
  
  switch (mesh->bulk_dimensions) {
    case 1:
      mesh->n_nodes = (mesh->ncells_x+1);
      mesh->n_elements = mesh->ncells_x + 2;
      mesh->max_nodes_per_element = 2;
      mesh->max_faces_per_element = 2;
      mesh->max_first_neighbor_nodes = 3;
      volume_element_type = ELEMENT_TYPE_LINE;
      surface_element_type = ELEMENT_TYPE_POINT;
      
      mesh->node = calloc(mesh->n_nodes, sizeof(node_t));
      i_node = 0;
      for (i = 0; i < mesh->ncells_x+1; i++) {
        mesh->node[i_node].id = i_node+1;
        mesh->node[i_node].x[0] = mesh->nodes_x[i];
        i_node++;
      }
    break;
    case 2:
      mesh->n_nodes = (mesh->ncells_x+1) * (mesh->ncells_y+1);
      mesh->n_elements = mesh->ncells_x*mesh->ncells_y + 2*(mesh->ncells_x + mesh->ncells_y);
      mesh->max_nodes_per_element = 4;
      mesh->max_faces_per_element = 4;
      mesh->max_first_neighbor_nodes = 9;
      volume_element_type = ELEMENT_TYPE_QUADRANGLE;
      surface_element_type = ELEMENT_TYPE_LINE;
      
      mesh->node = calloc(mesh->n_nodes, sizeof(node_t));
      i_node = 0;
      for (j = 0; j < mesh->ncells_y+1; j++) {
        for (i = 0; i < mesh->ncells_x+1; i++) {
          mesh->node[i_node].id = i_node+1;
          mesh->node[i_node].x[0] = mesh->nodes_x[i];
          mesh->node[i_node].x[1] = mesh->nodes_y[j];
          i_node++;
        }
      }
    break;
    case 3:
      mesh->n_nodes = (mesh->ncells_x+1) * (mesh->ncells_y+1) * (mesh->ncells_z+1);
      mesh->n_elements = mesh->ncells_x*mesh->ncells_y*mesh->ncells_z + 2*(mesh->ncells_x*mesh->ncells_y + mesh->ncells_y*mesh->ncells_z + mesh->ncells_x*mesh->ncells_z);
      mesh->max_nodes_per_element = 8;
      mesh->max_faces_per_element = 6;
      mesh->max_first_neighbor_nodes = 27;
      volume_element_type = ELEMENT_TYPE_HEXAHEDRON;
      surface_element_type = ELEMENT_TYPE_QUADRANGLE;
      
      mesh->node = calloc(mesh->n_nodes, sizeof(node_t));
      i_node = 0;
      for (k = 0; k < mesh->ncells_z+1; k++) {
        for (j = 0; j < mesh->ncells_y+1; j++) {
          for (i = 0; i < mesh->ncells_x+1; i++) {
            mesh->node[i_node].id = i_node+1;
            mesh->node[i_node].x[0] = mesh->nodes_x[i];
            mesh->node[i_node].x[1] = mesh->nodes_y[j];
            mesh->node[i_node].x[2] = mesh->nodes_z[k];
            i_node++;
          }
        }
      }
    break;
    default:
      wasora_push_error_message("dimensions > 3");
      return WASORA_RUNTIME_ERROR;
    break;
  }    
  
  // entidades fisicas de prepo
  mesh->left =   wasora_define_physical_entity("left",   0, mesh, mesh->bulk_dimensions-1, NULL, NULL, structured_direction_left);
  mesh->right =  wasora_define_physical_entity("right",  0, mesh, mesh->bulk_dimensions-1, NULL, NULL, structured_direction_right);
  if (mesh->bulk_dimensions > 1) {
    mesh->front =  wasora_define_physical_entity("front",  0, mesh, mesh->bulk_dimensions-1, NULL, NULL, structured_direction_front);
    mesh->rear =   wasora_define_physical_entity("back",   0, mesh, mesh->bulk_dimensions-1, NULL, NULL, structured_direction_rear);
  }
  if (mesh->bulk_dimensions > 2) {
    mesh->bottom = wasora_define_physical_entity("bottom", 0, mesh, mesh->bulk_dimensions-1, NULL, NULL, structured_direction_bottom);
    mesh->top =    wasora_define_physical_entity("top",    0, mesh, mesh->bulk_dimensions-1, NULL, NULL, structured_direction_top);
  }
  
  
  // elementos volumetricos
  mesh->element = calloc(mesh->n_elements, sizeof(element_t));
  i_element = 0;
  for (k = 0; k < mesh->ncells_z; k++) {
    for (j = 0; j < mesh->ncells_y; j++) {
      for (i = 0; i < mesh->ncells_x; i++) {
        
        mesh_create_element(&mesh->element[i_element], i_element+1, volume_element_type, NULL);
        for (i_node = 0; i_node < mesh->element[i_element].type->nodes; i_node++) {

          // este es el orden del vtk, asi que debe estar bien
          switch (mesh->bulk_dimensions) {
            case 1:
              node_index = (i + deltai[i_node]);
            break;
            case 2:
              node_index = (i + deltai[i_node]) + 
                      (j + deltaj[i_node]) * (mesh->ncells_x+1);
            break;
            case 3:
              node_index = (i + deltai[i_node]) + 
                      (j + deltaj[i_node]) * (mesh->ncells_x+1) +
                      (k + deltak[i_node]) * (mesh->ncells_x+1)*(mesh->ncells_y+1);
            break;
            default:
              wasora_push_error_message("dimensions > 3");
              return WASORA_RUNTIME_ERROR;
            break; 
          }
          
          mesh->element[i_element].node[i_node] = &mesh->node[node_index];
          mesh_add_element_to_list(&mesh->element[i_element].node[i_node]->associated_elements, &mesh->element[i_element]);
        }
                
        i_element++;
      }
    }
  }
  
  // barremos las physical entities y las aplicamos a los elementos
  i_entity = 0;
  LL_FOREACH(wasora_mesh.physical_entities, physical_entity) {
    if (physical_entity->material != NULL) {
      
      if (physical_entity->id == 0) {
        physical_entity->id = ++i_entity;
      }
      if (physical_entity->name == NULL) {
        physical_entity->name = malloc(strlen(physical_entity->material->name)+32);
        sprintf(physical_entity->name, "%s-%d", physical_entity->material->name, physical_entity->id);
      }
      if (physical_entity->dimension == 0) {
        physical_entity->dimension = mesh->bulk_dimensions; 
      }
    
      if (physical_entity->pos[structured_direction_left-1].n_tokens != 0 || physical_entity->pos[structured_direction_right-1].n_tokens != 0) {
        if ((i_min = wasora_mesh_struct_find_cell(mesh->ncells_x, mesh->cells_x, mesh->delta_x, wasora_evaluate_expression(&physical_entity->pos[0])+halfeps)) < 0) {
          wasora_push_error_message("plane x = %g of entity %d '%s' does not coincide with a cell border", wasora_evaluate_expression(&physical_entity->pos[0]), physical_entity->id, physical_entity->name);
          return WASORA_RUNTIME_ERROR;
        }
        if ((i_max = wasora_mesh_struct_find_cell(mesh->ncells_x, mesh->cells_x, mesh->delta_x, wasora_evaluate_expression(&physical_entity->pos[1])-halfeps)) < 0) {
          wasora_push_error_message("plane x = %g of entity %d '%s' does not coincide with a cell border", wasora_evaluate_expression(&physical_entity->pos[1]), physical_entity->id, physical_entity->name);
          return WASORA_RUNTIME_ERROR;
        }
      } else {
        i_min = 0;
        i_max = mesh->ncells_x-1;
      }
      if (physical_entity->pos[structured_direction_front-1].n_tokens != 0 || physical_entity->pos[structured_direction_rear-1].n_tokens != 0) {
        if ((j_min = wasora_mesh_struct_find_cell(mesh->ncells_y, mesh->cells_y, mesh->delta_y, wasora_evaluate_expression(&physical_entity->pos[2])+halfeps)) < 0) {
          wasora_push_error_message("plane y = %g of entity %d '%s' does not coincide with a cell border", wasora_evaluate_expression(&physical_entity->pos[2]), physical_entity->id, physical_entity->name);
          return WASORA_RUNTIME_ERROR;
        }
        if ((j_max = wasora_mesh_struct_find_cell(mesh->ncells_y, mesh->cells_y, mesh->delta_y, wasora_evaluate_expression(&physical_entity->pos[3])-halfeps)) < 0) {
          wasora_push_error_message("plane y = %g of entity %d '%s' does not coincide with a cell border", wasora_evaluate_expression(&physical_entity->pos[3]), physical_entity->id, physical_entity->name);
          return WASORA_RUNTIME_ERROR;
        }
      } else {
        j_min = 0;
        j_max = mesh->ncells_y-1;
      }
      if (physical_entity->pos[structured_direction_bottom-1].n_tokens != 0 || physical_entity->pos[structured_direction_top-1].n_tokens != 0) {
        if ((k_min = wasora_mesh_struct_find_cell(mesh->ncells_z, mesh->cells_z, mesh->delta_z, wasora_evaluate_expression(&physical_entity->pos[4])+halfeps)) < 0) {
          wasora_push_error_message("plane z = %g of entity %d '%s' does not coincide with a cell border", wasora_evaluate_expression(&physical_entity->pos[4]), physical_entity->id, physical_entity->name);
          return WASORA_RUNTIME_ERROR;
        }
        if ((k_max = wasora_mesh_struct_find_cell(mesh->ncells_z, mesh->cells_z, mesh->delta_z, wasora_evaluate_expression(&physical_entity->pos[5])-halfeps)) < 0) {
          wasora_push_error_message("plane z = %g of entity %d '%s' does not coincide with a cell border", wasora_evaluate_expression(&physical_entity->pos[5]), physical_entity->id, physical_entity->name);
          return WASORA_RUNTIME_ERROR;
        }
      } else {
        k_min = 0;
        k_max = mesh->ncells_z-1;
      }

      switch (mesh->bulk_dimensions) {
        case 1:
          for (i = i_min; i <= i_max; i++) {
            mesh->element[i].physical_entity = physical_entity;
          }
        break;  
        case 2:
          for (i = i_min; i <= i_max; i++) {
            for (j = j_min; j <= j_max; j++) {
              mesh->element[i + mesh->ncells_x*j].physical_entity = physical_entity;
            }
          }
        break;
        case 3: 
          for (i = i_min; i <= i_max; i++) {
            for (j = j_min; j <= j_max; j++) {
              for (k = k_min; k <= k_max; k++) {
                mesh->element[i + mesh->ncells_x*j + mesh->ncells_x*mesh->ncells_y*k].physical_entity = physical_entity;
              }
            }
          }
        break;
      }
    }
  }
  

  // armamos un kd-tree de nodos
  mesh->kd_nodes = kd_create(mesh->bulk_dimensions);
  for (i = 0; i < mesh->n_nodes; i++) {
    kd_insert(mesh->kd_nodes, mesh->node[i].x, &mesh->node[i]);
  }
 
  // hacemos un kd-tree de celdas
  mesh->kd_cells = kd_create(mesh->bulk_dimensions);

  // hacemos mesh_find_neighbors y mesh_fill_neighbors al mismo tiempo (porque podemos, como el duque)
  wasora_call(mesh_element2cell(mesh));
  i_cell = 0;
  for (k = 0; k < mesh->ncells_z; k++) {
    for (j = 0; j < mesh->ncells_y; j++) {
      for (i = 0; i < mesh->ncells_x; i++) {

        kd_insert(mesh->kd_cells, mesh->cell[i_cell].x, &mesh->cell[i]);

        // esto funciona para todas las dimensions porque hicimos 1 los que no estan
        mesh->cell[i_cell].volume = mesh->delta_x[i] * mesh->delta_y[j] * mesh->delta_z[k];
        mesh->cell[i_cell].n_neighbors = mesh->max_faces_per_element;

        if (mesh->cell[i_cell].element->physical_entity != NULL) {
        
          mesh->cell[i_cell].neighbor = calloc(mesh->cell[i_cell].n_neighbors, sizeof(struct neighbor_t));          

          // en unstructured buscamos elementos vecinos y hacemos apuntar las celdas
          // aca buscamos celdas vecinas y hacemos apuntar elementos
          // x_min == left
          neighbor = &mesh->cell[i_cell].neighbor[structured_direction_left-1];
          neighbor->n_ij[0] = -1;
          neighbor->n_ij[1] = 0;
          neighbor->n_ij[2] = 0;
          neighbor->x_ij[0] = mesh->cell[i_cell].x[0] + neighbor->n_ij[0] * 0.5*mesh->delta_x[i];
          neighbor->x_ij[1] = mesh->cell[i_cell].x[1] + neighbor->n_ij[1] * 0.5*mesh->delta_y[j];
          neighbor->x_ij[2] = mesh->cell[i_cell].x[2] + neighbor->n_ij[2] * 0.5*mesh->delta_z[k];
          neighbor->S_ij = mesh->delta_y[j] * mesh->delta_z[k];
          if (i > 0 && mesh->cell[flat_index(i-1,j,k)].element->physical_entity != NULL) {
            neighbor->cell = &mesh->cell[flat_index(i-1,j,k)];
            neighbor->element = neighbor->cell->element;
          } else {
            mesh_create_element(&mesh->element[i_element], i_element+1, surface_element_type, mesh->left);
            
            // el 0 siempre a la izquierda
            mesh->element[i_element].node[0] = mesh->cell[i_cell].element->node[0];
            mesh_add_element_to_list(&mesh->element[i_element].node[0]->associated_elements, &mesh->element[i_element]);                
            
            // en 2d el numero 3 tambien
            if (mesh->bulk_dimensions > 1) {
              mesh->element[i_element].node[1] = mesh->cell[i_cell].element->node[3];
              mesh_add_element_to_list(&mesh->element[i_element].node[1]->associated_elements, &mesh->element[i_element]);                
            }
            // en 3d el 4 y el 7
            if (mesh->bulk_dimensions > 2) {
              mesh->element[i_element].node[2] = mesh->cell[i_cell].element->node[7];
              mesh_add_element_to_list(&mesh->element[i_element].node[2]->associated_elements, &mesh->element[i_element]);                
              
              mesh->element[i_element].node[3] = mesh->cell[i_cell].element->node[4];
              mesh_add_element_to_list(&mesh->element[i_element].node[3]->associated_elements, &mesh->element[i_element]);                
            }
            i_element++;
          }

          // x_max == right          
          neighbor = &mesh->cell[i_cell].neighbor[structured_direction_right-1];
          neighbor->n_ij[0] = +1;
          neighbor->n_ij[1] = 0;
          neighbor->n_ij[2] = 0;
          neighbor->x_ij[0] = mesh->cell[i_cell].x[0] + neighbor->n_ij[0] * 0.5*mesh->delta_x[i];
          neighbor->x_ij[1] = mesh->cell[i_cell].x[1] + neighbor->n_ij[1] * 0.5*mesh->delta_y[j];
          neighbor->x_ij[2] = mesh->cell[i_cell].x[2] + neighbor->n_ij[2] * 0.5*mesh->delta_z[k];
          neighbor->S_ij = mesh->delta_y[j] * mesh->delta_z[k];
          if (i < mesh->ncells_x-1 && mesh->cell[flat_index(i+1,j,k)].element->physical_entity != NULL) {
            neighbor->cell = &mesh->cell[flat_index(i+1,j,k)];
            neighbor->element = neighbor->cell->element;
          } else {
            mesh_create_element(&mesh->element[i_element], i_element+1, surface_element_type, mesh->right);
            // el 1 siempre a la derecha
            mesh->element[i_element].node[0] = mesh->cell[i_cell].element->node[1];
            mesh_add_element_to_list(&mesh->element[i_element].node[0]->associated_elements, &mesh->element[i_element]);                
            
            // en 2d el numero 2 tambien
            if (mesh->bulk_dimensions > 1) {
              mesh->element[i_element].node[1] = mesh->cell[i_cell].element->node[2];
              mesh_add_element_to_list(&mesh->element[i_element].node[1]->associated_elements, &mesh->element[i_element]);                
            }
            // en 3d el 4 y el 7
            if (mesh->bulk_dimensions > 2) {
              mesh->element[i_element].node[2] = mesh->cell[i_cell].element->node[6];
              mesh_add_element_to_list(&mesh->element[i_element].node[2]->associated_elements, &mesh->element[i_element]);                
              
              mesh->element[i_element].node[3] = mesh->cell[i_cell].element->node[5];
              mesh_add_element_to_list(&mesh->element[i_element].node[3]->associated_elements, &mesh->element[i_element]);                
            }
            i_element++;
          }

          if (mesh->bulk_dimensions > 1) {
            // y_min = front
            neighbor = &mesh->cell[i_cell].neighbor[structured_direction_front-1];
            neighbor->n_ij[0] = 0;
            neighbor->n_ij[1] = -1;
            neighbor->n_ij[2] = 0;
            neighbor->x_ij[0] = mesh->cell[i_cell].x[0] + neighbor->n_ij[0] * 0.5*mesh->delta_x[i];
            neighbor->x_ij[1] = mesh->cell[i_cell].x[1] + neighbor->n_ij[1] * 0.5*mesh->delta_y[j];
            neighbor->x_ij[2] = mesh->cell[i_cell].x[2] + neighbor->n_ij[2] * 0.5*mesh->delta_z[k];
            neighbor->S_ij = mesh->delta_x[i] * mesh->delta_z[k];
            if (j > 0 && mesh->cell[flat_index(i,j-1,k)].element->physical_entity != NULL) {
              neighbor->cell = &mesh->cell[flat_index(i,j-1,k)];
              neighbor->element = neighbor->cell->element;
            } else {
              mesh_create_element(&mesh->element[i_element], i_element+1, surface_element_type, mesh->front);
              // en 2d los nodos 0 y 1
              mesh->element[i_element].node[0] = mesh->cell[i_cell].element->node[0];
              mesh_add_element_to_list(&mesh->element[i_element].node[0]->associated_elements, &mesh->element[i_element]);                

              mesh->element[i_element].node[1] = mesh->cell[i_cell].element->node[1];
              mesh_add_element_to_list(&mesh->element[i_element].node[1]->associated_elements, &mesh->element[i_element]);                

              // en 3d el 4 y el 5
              if (mesh->bulk_dimensions > 2) {
                mesh->element[i_element].node[2] = mesh->cell[i_cell].element->node[5];
                mesh_add_element_to_list(&mesh->element[i_element].node[2]->associated_elements, &mesh->element[i_element]);                

                mesh->element[i_element].node[3] = mesh->cell[i_cell].element->node[4];
                mesh_add_element_to_list(&mesh->element[i_element].node[3]->associated_elements, &mesh->element[i_element]);                
              }
              i_element++;
            }

            // y_max = rear
            neighbor = &mesh->cell[i_cell].neighbor[structured_direction_rear-1];
            neighbor->n_ij[0] = 0;
            neighbor->n_ij[1] = +1;
            neighbor->n_ij[2] = 0;
            neighbor->x_ij[0] = mesh->cell[i_cell].x[0] + neighbor->n_ij[0] * 0.5*mesh->delta_x[i];
            neighbor->x_ij[1] = mesh->cell[i_cell].x[1] + neighbor->n_ij[1] * 0.5*mesh->delta_y[j];
            neighbor->x_ij[2] = mesh->cell[i_cell].x[2] + neighbor->n_ij[2] * 0.5*mesh->delta_z[k];
            neighbor->S_ij = mesh->delta_x[i] * mesh->delta_z[k];
            if (j < mesh->ncells_y-1 && mesh->cell[flat_index(i,j+1,k)].element->physical_entity != NULL) {
              neighbor->cell = &mesh->cell[flat_index(i,j+1,k)];
              neighbor->element = neighbor->cell->element;
            } else {
              mesh_create_element(&mesh->element[i_element], i_element+1, surface_element_type, mesh->rear);
              // en 2d los nodos 2 y 3
              mesh->element[i_element].node[0] = mesh->cell[i_cell].element->node[2];
              mesh_add_element_to_list(&mesh->element[i_element].node[0]->associated_elements, &mesh->element[i_element]);                

              mesh->element[i_element].node[1] = mesh->cell[i_cell].element->node[3];
              mesh_add_element_to_list(&mesh->element[i_element].node[1]->associated_elements, &mesh->element[i_element]);                

              // en 3d el 6 y el 7
              if (mesh->bulk_dimensions > 2) {
                mesh->element[i_element].node[2] = mesh->cell[i_cell].element->node[7];
                mesh_add_element_to_list(&mesh->element[i_element].node[2]->associated_elements, &mesh->element[i_element]);                

                mesh->element[i_element].node[3] = mesh->cell[i_cell].element->node[6];
                mesh_add_element_to_list(&mesh->element[i_element].node[3]->associated_elements, &mesh->element[i_element]);                
              }
              i_element++;
            }
          }

          if (mesh->bulk_dimensions > 2) {
            // z_min = bottom
            neighbor = &mesh->cell[i_cell].neighbor[structured_direction_bottom-1];
            neighbor->n_ij[0] = 0;
            neighbor->n_ij[1] = 0;
            neighbor->n_ij[2] = -1;
            neighbor->x_ij[0] = mesh->cell[i_cell].x[0] + neighbor->n_ij[0] * 0.5*mesh->delta_x[i];
            neighbor->x_ij[1] = mesh->cell[i_cell].x[1] + neighbor->n_ij[1] * 0.5*mesh->delta_y[j];
            neighbor->x_ij[2] = mesh->cell[i_cell].x[2] + neighbor->n_ij[2] * 0.5*mesh->delta_z[k];
            neighbor->S_ij = mesh->delta_x[i] * mesh->delta_y[j];
            if (k > 0 && mesh->cell[flat_index(i,j,k-1)].element->physical_entity != NULL) {
              neighbor->cell = &mesh->cell[flat_index(i,j,k-1)];
              neighbor->cell->element = neighbor->cell->element;
            } else {
              mesh_create_element(&mesh->element[i_element], i_element+1, surface_element_type, mesh->bottom);
              // los nodos 0 1 2 3
              mesh->element[i_element].node[0] = mesh->cell[i_cell].element->node[0];
              mesh_add_element_to_list(&mesh->element[i_element].node[0]->associated_elements, &mesh->element[i_element]);                

              mesh->element[i_element].node[1] = mesh->cell[i_cell].element->node[1];
              mesh_add_element_to_list(&mesh->element[i_element].node[1]->associated_elements, &mesh->element[i_element]);                

              mesh->element[i_element].node[2] = mesh->cell[i_cell].element->node[2];
              mesh_add_element_to_list(&mesh->element[i_element].node[2]->associated_elements, &mesh->element[i_element]);                

              mesh->element[i_element].node[3] = mesh->cell[i_cell].element->node[3];
              mesh_add_element_to_list(&mesh->element[i_element].node[3]->associated_elements, &mesh->element[i_element]);                
              i_element++;
            }

            neighbor = &mesh->cell[i_cell].neighbor[structured_direction_top-1];
            neighbor->n_ij[0] = 0;
            neighbor->n_ij[1] = 0;
            neighbor->n_ij[2] = +1;
            neighbor->x_ij[0] = mesh->cell[i_cell].x[0] + neighbor->n_ij[0] * 0.5*mesh->delta_x[i];
            neighbor->x_ij[1] = mesh->cell[i_cell].x[1] + neighbor->n_ij[1] * 0.5*mesh->delta_y[j];
            neighbor->x_ij[2] = mesh->cell[i_cell].x[2] + neighbor->n_ij[2] * 0.5*mesh->delta_z[k];
            neighbor->S_ij = mesh->delta_x[i] * mesh->delta_y[j];
            if (k < mesh->ncells_z-1 && mesh->cell[flat_index(i,j,k+1)].element->physical_entity != NULL) {
              neighbor->cell = &mesh->cell[flat_index(i,j,k+1)];
              neighbor->cell->element = neighbor->cell->element;
            } else {
              mesh_create_element(&mesh->element[i_element], i_element+1, surface_element_type, mesh->bottom);
              // los nodos 4 5 6 7
              mesh->element[i_element].node[0] = mesh->cell[i_cell].element->node[4];
              mesh_add_element_to_list(&mesh->element[i_element].node[0]->associated_elements, &mesh->element[i_element]);                

              mesh->element[i_element].node[1] = mesh->cell[i_cell].element->node[5];
              mesh_add_element_to_list(&mesh->element[i_element].node[1]->associated_elements, &mesh->element[i_element]);                

              mesh->element[i_element].node[2] = mesh->cell[i_cell].element->node[6];
              mesh_add_element_to_list(&mesh->element[i_element].node[2]->associated_elements, &mesh->element[i_element]);                

              mesh->element[i_element].node[3] = mesh->cell[i_cell].element->node[7];
              mesh_add_element_to_list(&mesh->element[i_element].node[3]->associated_elements, &mesh->element[i_element]);                
              i_element++;
            }
          }
        }
        i_cell++;
      }
    }
  }
  
  // no necesariamente la cantidad de elementos volumetricos mas los de
  // superficie son iguales al maximo alocado, asi que re-calculamos el total de elementos
  mesh->n_elements = i_element;
  
  return WASORA_RUNTIME_OK;

}


void wasora_mesh_struct_init_rectangular_for_cells(mesh_t *mesh) {
  
  int i, j, k;
  
  mesh->rectangular_mesh_size[0] = mesh->ncells_x;
  mesh->rectangular_mesh_point[0] = calloc(mesh->ncells_x, sizeof(double));
  for (i = 0; i < mesh->ncells_x; i++) {
    mesh->rectangular_mesh_point[0][i] = mesh->cells_x[i];
  }
  if (mesh->bulk_dimensions >= 2) {
    mesh->rectangular_mesh_size[1] = mesh->ncells_y;
    mesh->rectangular_mesh_point[1] = calloc(mesh->ncells_y, sizeof(double));
    for (j = 0; j < mesh->ncells_y; j++) {
      mesh->rectangular_mesh_point[1][j] = mesh->cells_y[j];
    }
    if (mesh->bulk_dimensions == 3) {
      mesh->rectangular_mesh_size[2] = mesh->ncells_z;
      mesh->rectangular_mesh_point[2] = calloc(mesh->ncells_z, sizeof(double));
      for (k = 0; k < mesh->ncells_z; k++) {
        mesh->rectangular_mesh_point[2][k] = mesh->cells_z[k];
      }
    }
  }

  return;  
}

void wasora_mesh_struct_init_rectangular_for_nodes(mesh_t *mesh) {
  
  int i, j, k;
  
  mesh->rectangular_mesh_size[0] = mesh->ncells_x+1;
  mesh->rectangular_mesh_point[0] = calloc(mesh->ncells_x+1, sizeof(double));
  for (i = 0; i < mesh->ncells_x+1; i++) {
    mesh->rectangular_mesh_point[0][i] = mesh->nodes_x[i];
  }
  if (mesh->bulk_dimensions >= 2) {
    mesh->rectangular_mesh_size[1] = mesh->ncells_y+1;
    mesh->rectangular_mesh_point[1] = calloc(mesh->ncells_y+1, sizeof(double));
    for (j = 0; j < mesh->ncells_y+1; j++) {
      mesh->rectangular_mesh_point[1][j] = mesh->nodes_y[j];
    }
    if (mesh->bulk_dimensions >= 3) {
      mesh->rectangular_mesh_size[2] = mesh->ncells_z+1;
      mesh->rectangular_mesh_point[2] = calloc(mesh->ncells_z+1, sizeof(double));
      for (k = 0; k < mesh->ncells_z+1; k++) {
        mesh->rectangular_mesh_point[2][k] = mesh->nodes_z[k];
      }
    }
  }
}
int wasora_mesh_struct_find_cell(int n, double *x, double *dx, double x0) {

  int a, b, c;
  
  a = 0;
  b = n-1;
  c = a;

  while ((b-a) > 1) {
    c = (a+b)/2;

    if (x[c] > x0) {
      b = c;
    } else {
      a = c;
    }
  }
  
  // la biseccion le puede errar el vizcachazo por culpa de la aritmetica de enteros
  if (c > 0 && (fabs(x0-x[c-1]) < fabs(x0-x[c]))) {
    c--;
  } else if (c < n-1 && (fabs(x0-x[c+1]) < fabs(x0-x[c]))) {
    c++;
  }
  
  // el indice c tiene el numero de celda correcto, solo tenemos que chequear que
  // el plano coincida con alguno de los dos bordes
  
  if ((fabs(x0 - (x[c]-0.5*dx[c])) > wasora_value(wasora_mesh.vars.eps)) && (fabs(x0 - (x[c]+0.5*dx[c])) > wasora_value(wasora_mesh.vars.eps))) {
    wasora_push_error_message("(should be either %g or %g)", (x[c]-0.5*dx[c]), (x[c]+0.5*dx[c]));
    return -1;
  }
  
  return c;

}
#endif
