/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related routines to read frd files from calculix
 *
 *  Copyright (C) 2018 jeremy theler
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
#include <stdlib.h>
#include <string.h>

int frdfromgmsh_types[18] = {
 ELEMENT_TYPE_UNDEFINED,
 ELEMENT_TYPE_HEXAHEDRON,
 ELEMENT_TYPE_PRISM,
 ELEMENT_TYPE_HEXAHEDRON20,
 ELEMENT_TYPE_TETRAHEDRON,
 ELEMENT_TYPE_UNDEFINED,  // prism15
 ELEMENT_TYPE_TETRAHEDRON10,
 ELEMENT_TYPE_TRIANGLE,
 ELEMENT_TYPE_TRIANGLE6,
 ELEMENT_TYPE_QUADRANGLE,
 ELEMENT_TYPE_QUADRANGLE8,
 ELEMENT_TYPE_LINE,
 ELEMENT_TYPE_LINE3,
};
int mesh_frd_readmesh(mesh_t *mesh) {

  char buffer[BUFFER_SIZE];
  char tmp[BUFFER_SIZE];
  double scale_factor;
  double offset[3];
  int i, j, k;
  int id;
  int type;
  int node;
  int dimension;
  int cell_id;
  int spatial_dimensions;
  int bulk_dimensions;
  int order;
  
  int format;
  int minusone, minustwo, minusthree;
  
  char *dummy;
  char *name;
  physical_entity_t *physical_entity;
  material_t *material;

  
  if (mesh->file->pointer == NULL) {
    wasora_call(wasora_instruction_open_file(mesh->file));
  }
  scale_factor = wasora_evaluate_expression(mesh->scale_factor);
  offset[0] = wasora_evaluate_expression(mesh->offset_x);
  offset[1] = wasora_evaluate_expression(mesh->offset_y);
  offset[2] = wasora_evaluate_expression(mesh->offset_z);

  // empezamos suponiendo cero dimensiones y vamos viendo cual es el elemento
  // de mayor dimension que aparece -> esa es la de la malla
  bulk_dimensions = 0;
  spatial_dimensions = 0;
  order = 0;
  
  while (fgets(buffer, BUFFER_SIZE-1, mesh->file->pointer) != NULL) {

    if (strncmp("\n", buffer, 1) == 0) {
      ;
    
    // ------------------------------------------------------  
    } else if (strncmp("    2C", buffer, 6) == 0) {
 
      // la cantidad de nodos
      if (sscanf(buffer, "%s %d %d", tmp, &mesh->n_nodes, &format) != 3) {
        wasora_push_error_message("error parsing number of nodes '%s'", buffer);
        return WASORA_RUNTIME_ERROR;
      }
      if (mesh->n_nodes == 0) {
        wasora_push_error_message("no nodes found in mesh '%s'", mesh->file->path);
        return WASORA_RUNTIME_ERROR;
      }
      if (format > 1) {
        wasora_push_error_message("format %d not supported'", format);
        return WASORA_RUNTIME_ERROR;
      }
      
      mesh->node = calloc(mesh->n_nodes, sizeof(node_t));

      for (i = 0; i < mesh->n_nodes; i++) {
        if (fscanf(mesh->file->pointer, "%d", &minusone) != 1) {
          wasora_push_error_message("error parsing nodes", buffer);
          return WASORA_RUNTIME_ERROR;
        }
        if (minusone != -1) {
          wasora_push_error_message("expected minus one as line starter", buffer);
          return WASORA_RUNTIME_ERROR;
        }

        if (fscanf(mesh->file->pointer, "%d", &id) == 0) {
          return WASORA_RUNTIME_ERROR;
        }
        // nuestros id son de C
        id--;
        mesh->node[id].id = id+1;

        for (j = 0; j < 3; j++) {
          if (fscanf(mesh->file->pointer, "%lf", &mesh->node[id].x[j]) == 0) {
            return WASORA_RUNTIME_ERROR;
          }
          if (scale_factor != 0 || offset[j] != 0) {
            mesh->node[id].x[j] = scale_factor*mesh->node[id].x[j] - offset[j];
          }
        }
        
        if (spatial_dimensions < 1 && fabs(mesh->node[id].x[0]) > 1e-6) {
          spatial_dimensions = 1;
        }
        if (spatial_dimensions < 2 && fabs(mesh->node[id].x[1]) > 1e-6) {
          spatial_dimensions = 2;
        }
        if (spatial_dimensions < 3 && fabs(mesh->node[id].x[2]) > 1e-6) {
          spatial_dimensions = 3;
        }
      }
  
      // el -3
      if (fscanf(mesh->file->pointer, "%d", &minusthree) != 1) {
        wasora_push_error_message("error parsing nodes", buffer);
        return WASORA_RUNTIME_ERROR;
      }
      if (minusthree != -3) {
        wasora_push_error_message("expected minus three as line starter", buffer);
        return WASORA_RUNTIME_ERROR;
      }

    // ------------------------------------------------------      
    } else if (strncmp("    3C", buffer, 6) == 0) {

      // la cantidad de elementos
      if (sscanf(buffer, "%s %d %d", tmp, &mesh->n_elements, &format) != 3) {
        wasora_push_error_message("error parsing number of elements '%s'", buffer);
        return WASORA_RUNTIME_ERROR;
      }
      if (mesh->n_elements == 0) {
        wasora_push_error_message("no elements found in mesh '%s'", mesh->file->path);
        return WASORA_RUNTIME_ERROR;
      }
      if (format > 1) {
        wasora_push_error_message("format %d not supported'", format);
        return WASORA_RUNTIME_ERROR;
      }

      mesh->element = calloc(mesh->n_elements, sizeof(element_t));

      for (i = 0; i < mesh->n_elements; i++) {

        if (fscanf(mesh->file->pointer, "%d", &minusone) != 1) {
          wasora_push_error_message("error parsing nodes", buffer);
          return WASORA_RUNTIME_ERROR;
        }
        if (minusone != -1) {
          wasora_push_error_message("expected minus one as line starter", buffer);
          return WASORA_RUNTIME_ERROR;
        }
        
        if (fscanf(mesh->file->pointer, "%d", &id) == 0) {
          return WASORA_RUNTIME_ERROR;
        }
        id--; // nuestras id son de C
        mesh->element[id].id = id+1;
    
        if (fscanf(mesh->file->pointer, "%d", &type) == 0) {
          return WASORA_RUNTIME_ERROR;
        }
        if (type > 12) {
          wasora_push_error_message("elements type '%d' shold be less than 13", type);
          return WASORA_RUNTIME_ERROR;
        }
        mesh->element[id].type = &(wasora_mesh.element_type[frdfromgmsh_types[type]]);
        if (mesh->element[id].type->nodes == 0) {
          wasora_push_error_message("elements of type '%s' are not supported in this version :-(", mesh->element[id].type->name);
          return WASORA_RUNTIME_ERROR;
        }

        // en gmsh cada elemento tiene un tag que es un array de enteros
        // el primero es el id de la entidad fisica
        // el segundo es el id de la entidad geometrica (no nos interesa)
        // aca ponemos siempre dos tags, uno para el "grupo" y otro para el "material"
        mesh->element[id].ntags = 2;
        mesh->element[id].tag = malloc(mesh->element[id].ntags * sizeof(int));
        for (j = 0; j < mesh->element[id].ntags; j++) {
          if (fscanf(mesh->file->pointer, "%d", &mesh->element[id].tag[j]) == 0) {
            return WASORA_RUNTIME_ERROR;
          }
        }
        
        // agregamos uno a la cantidad de elementos asociados a la entidad fisica
/*        
        if (mesh == wasora_mesh.main_mesh) {
          if (mesh->element[i].tag != NULL && mesh->element[i].tag[0] != 0) {
            HASH_FIND(hh_id, wasora_mesh.physical_entities_by_id, &mesh->element[i].tag[0], sizeof(int), physical_entity);
            if ((mesh->element[i].physical_entity = physical_entity) != NULL) {
              physical_entity->n_elements++;
              // ponemos la dimension de la entidad fisica
              if (mesh->element[i].type->dim > physical_entity->dimension) {
                physical_entity->dimension = mesh->element[i].type->dim;
              }
            }
          }
        }
*/      

        // vemos la dimension del elemento -> la mayor es la de la malla
        if (mesh->element[id].type->dim > bulk_dimensions) {
          bulk_dimensions = mesh->element[id].type->dim;
        }
        
        // el orden
        if (mesh->element[id].type->order > order) {
          order = mesh->element[id].type->order;
        }
        
        // nos acordamos del elemento que tenga el mayor numero de nodos
        if (mesh->element[id].type->nodes > mesh->max_nodes_per_element) {
          mesh->max_nodes_per_element = mesh->element[id].type->nodes;
        }

        // y del que tenga mayor cantidad de vecinos
        if (mesh->element[id].type->faces > mesh->max_faces_per_element) {
          mesh->max_faces_per_element = mesh->element[id].type->faces;
        }
    
        // el -2
        if (fscanf(mesh->file->pointer, "%d", &minustwo) != 1) {
          wasora_push_error_message("error parsing elements", buffer);
          return WASORA_RUNTIME_ERROR;
        }
        if (minustwo != -2) {
          wasora_push_error_message("expected minus two as line starter", buffer);
          return WASORA_RUNTIME_ERROR;
        }
        
        mesh->element[id].node = calloc(mesh->element[id].type->nodes, sizeof(node_t *));
        for (j = 0; j < mesh->element[id].type->nodes; j++) {
          if (fscanf(mesh->file->pointer, "%d", &node) == 0) {
            return WASORA_RUNTIME_ERROR;
          }
          if (node > mesh->n_nodes) {
            wasora_push_error_message("node %d in element %d does not exist", node, id);
            return WASORA_RUNTIME_ERROR;
          }
          mesh->element[id].node[j] = &mesh->node[node-1];
          mesh_add_element_to_list(&mesh->element[id].node[j]->associated_elements, &mesh->element[id]);
          // habria que ver si la dimension es la del problema?
/*          
          if (mesh->element[id].physical_entity != NULL && mesh->element[id].physical_entity->material != NULL) {
            mesh_add_material_to_list(&mesh->element[id].node[j]->materials_list, mesh->element[id].physical_entity->material);
          }
 */
        }
      }

      // el -3
      if (fscanf(mesh->file->pointer, "%d", &minusthree) != 1) {
        wasora_push_error_message("error parsing nodes", buffer);
        return WASORA_RUNTIME_ERROR;
      }
      if (minusthree != -3) {
        wasora_push_error_message("expected minus three as line starter", buffer);
        return WASORA_RUNTIME_ERROR;
      }
      
    // ------------------------------------------------------      
    } else if (strncmp("$PhysicalNames", buffer, 14) == 0) {

      // si hay physical names entonces definimos implicitamente physical
      // entities (si es que no existen ya, si existen chequeamos los ids)

      // la cantidad de cosas
      if (fscanf(mesh->file->pointer, "%d", &mesh->n_physical_names) == 0) {
        return WASORA_RUNTIME_ERROR;
      }
      for (i = 0; i < mesh->n_physical_names; i++) {

        if (fscanf(mesh->file->pointer, "%d", &dimension) == 0) {
          return WASORA_RUNTIME_ERROR;
        }
        if (fscanf(mesh->file->pointer, "%d", &id) == 0) {
          return WASORA_RUNTIME_ERROR;
        }
        if (fgets(buffer, BUFFER_SIZE-1, mesh->file->pointer) == NULL) {
          wasora_push_error_message("corrupted mesh file '%s'", mesh->file->path);
          return WASORA_RUNTIME_ERROR;
        }
        if ((dummy = strrchr(buffer, '\"')) == NULL) {
          wasora_push_error_message("end quote not found in physical name %d in '%s'", id, mesh->file->path);
          return WASORA_RUNTIME_ERROR;
        }
        *dummy = '\0';
        if ((dummy = strchr(buffer, '\"')) == NULL) {
          wasora_push_error_message("beginning quote not found in physical name %d in '%s'", id, mesh->file->path);
          return WASORA_RUNTIME_ERROR;
        }
        name = strdup(dummy+1);

        HASH_FIND(hh_name, wasora_mesh.physical_entities_by_name, name, strlen(name), physical_entity);
        if (physical_entity == NULL) {
          LL_FOREACH(wasora_mesh.physical_entities_by_id, physical_entity) {
            if (physical_entity->id == id) {
              break;
            }
          }
        }        
        
        if (physical_entity == NULL) {
          // creamos una de prepo
          physical_entity = wasora_define_physical_entity(name, id, mesh, dimension, NULL, NULL, 0);
        } else {
          // verificamos que no tenga id numerica
          if (physical_entity->id == 0) {
            physical_entity->id = id;
            // y lo agregamos al hash de enteros
            HASH_ADD(hh_id, wasora_mesh.physical_entities_by_id, id, sizeof(int), physical_entity);
          } else if (physical_entity->id != id) {
            // o que tenga la correcta
            wasora_push_error_message("physical entity '%s' has id %d in input and %d in mesh", name, physical_entity->id, id);
            return WASORA_PARSER_ERROR;
          }
        }

        // si la physical entity no tiene material, buscamos uno que se llame igual
        if (physical_entity->material == NULL) {
          HASH_FIND_STR(wasora_mesh.materials, name, material);
          // si es NULL es porque la physical entity es una BC o algo,
          // pero eso no nos molesta, las BCs las vemos despues
          physical_entity->material = material;
        }

      }
          

      // la linea $EndPhysicalNames
      if (fgets(buffer, BUFFER_SIZE-1, mesh->file->pointer) == NULL) {
        wasora_push_error_message("corrupted mesh file '%s'", mesh->file->path);
        return -3;
      } 
      if (strncmp("$EndPhysicalNames", buffer, 17) != 0) {
        wasora_push_error_message("$EndPhysicalNames not found in mesh file '%s'", mesh->file->path);
        return -2;
      }
      
    // ------------------------------------------------------      
    } else if (strncmp("$ElementData", buffer, 12) == 0) {
      
      // TODO!
      
    // ------------------------------------------------------      
    } else if (strncmp("$ElementNodeData", buffer, 16) == 0 || strncmp("$NodeData", buffer, 9) == 0) {
      
      node_data_t *node_data;
      function_t *function = NULL;
      double time, value;
      int j, id, timestep, dofs, nodes;
      int n_string_tags, n_real_tags, n_integer_tags;
      char *string_tag = NULL;
      
      // string-tags
      fscanf(mesh->file->pointer, "%d", &n_string_tags);
      if (n_string_tags != 1) {
        continue;
      }
      fgets(buffer, BUFFER_SIZE-1, mesh->file->pointer); // el \n
      fgets(buffer, BUFFER_SIZE-1, mesh->file->pointer);
      string_tag = strtok(buffer, "\"");
      
      LL_FOREACH(mesh->node_datas, node_data) {
        if (strcmp(string_tag, node_data->name_in_mesh) == 0) {
          function = node_data->function;
        }
      }
      
      if (function == NULL) {
        continue;
      }
      
      // real-tags
      fscanf(mesh->file->pointer, "%d", &n_real_tags);
      if (n_real_tags != 1) {
        continue;
      }
      fscanf(mesh->file->pointer, "%lf", &time);
      
      // integer-tags
      fscanf(mesh->file->pointer, "%d", &n_integer_tags);
      if (n_integer_tags != 3) {
        continue;
      }
      fscanf(mesh->file->pointer, "%d", &timestep);
      fscanf(mesh->file->pointer, "%d", &dofs);
      fscanf(mesh->file->pointer, "%d", &nodes);
      
      if (dofs != 1 || nodes != mesh->n_nodes) {
        continue;
      }
      
      // si llegamos hasta aca, tenemos una funcion
      function->type = type_pointwise_mesh_node;
      function->mesh = mesh;
      function->data_argument = mesh->nodes_argument;
      function->data_size = nodes;
      function->data_value = calloc(nodes, sizeof(double));
      
      for (j = 0; j < nodes; j++) {
        fscanf(mesh->file->pointer, "%d %lf", &id, &value);
        function->data_value[id-1] = value;
      }

      // el newline
      if (fgets(buffer, BUFFER_SIZE-1, mesh->file->pointer) == NULL) {
        wasora_push_error_message("corrupted mesh file '%s'", mesh->file->path);
        return -3;
      } 

      // la linea $ElementNodeData
      if (fgets(buffer, BUFFER_SIZE-1, mesh->file->pointer) == NULL) {
        wasora_push_error_message("corrupted mesh file '%s'", mesh->file->path);
        return -3;
      } 
      if (strncmp("$EndElementNodeData", buffer, 19) != 0 && strncmp("$EndNodeData", buffer, 12) != 0) {
        wasora_push_error_message("$EndElementNodeData not found in mesh file '%s'", mesh->file->path);
        return -2;
      }

  
    // ------------------------------------------------------      
    // extension nuestra!
    } else if (strncmp("$Neighbors", buffer, 10) == 0 || strncmp("$Neighbours", buffer, 11) == 0) {

      int element_id;
      
      // la cantidad de celdas
      if (fscanf(mesh->file->pointer, "%d", &(mesh->n_cells)) == 0) {
        return WASORA_RUNTIME_ERROR;
      }
      if (mesh->n_cells == 0) {
        wasora_push_error_message("no cells found in mesh file '%s'", mesh->file->path);
        return -2;
      }
      mesh->cell = calloc(mesh->n_cells, sizeof(cell_t));

      for (i = 0; i < mesh->n_cells; i++) {
    
        if (fscanf(mesh->file->pointer, "%d", &cell_id) == 0) {
          return WASORA_RUNTIME_ERROR;
        }
        if (cell_id != i+1) {
          wasora_push_error_message("cells in mesh file '%s' are not sorted", mesh->file->path);
          return -2;
        }
        mesh->cell[i].id = cell_id;
        
        if (fscanf(mesh->file->pointer, "%d", &element_id) == 0) {
          return WASORA_RUNTIME_ERROR;
        }
        mesh->cell[i].element = &mesh->element[element_id - 1];
        
        if (fscanf(mesh->file->pointer, "%d", &mesh->cell[i].n_neighbors) == 0) {
          return WASORA_RUNTIME_ERROR;
        }
        if (fscanf(mesh->file->pointer, "%d", &j) == 0) {
          return WASORA_RUNTIME_ERROR;
        }
        if (j != mesh->cell[i].element->type->nodes_per_face) {
          wasora_push_error_message("mesh file '%s' has inconsistencies in the neighbors section", mesh->file->path);
          return -2;
        }

        mesh->cell[i].ineighbor = malloc(mesh->cell[i].n_neighbors * sizeof(int));
        mesh->cell[i].ifaces = malloc(mesh->cell[i].n_neighbors * sizeof(int *));
        for (j = 0; j < mesh->cell[i].n_neighbors; j++) {
          if (fscanf(mesh->file->pointer, "%d", &mesh->cell[i].ineighbor[j]) == 0) {
            return WASORA_RUNTIME_ERROR;
          }
          mesh->cell[i].ifaces[j] = malloc(mesh->cell[i].element->type->nodes_per_face);
          for (k = 0; k < mesh->cell[i].element->type->nodes_per_face; k++) {
            if (fscanf(mesh->file->pointer, "%d", &mesh->cell[i].ifaces[j][k]) == 0) {
              return WASORA_RUNTIME_ERROR;
            }
          }
        }
      }
        
      
      // el newline
      if (fgets(buffer, BUFFER_SIZE-1, mesh->file->pointer) == NULL) {
        wasora_push_error_message("corrupted mesh file '%s'", mesh->file->path);
        return -3;
      } 

      // la linea $EndNeighbors
      if (fgets(buffer, BUFFER_SIZE-1, mesh->file->pointer) == NULL) {
        wasora_push_error_message("corrupted mesh file '%s'", mesh->file->path);
        return -3;
      } 
      if (strncmp("$EndNeighbors", buffer, 13) != 0 && strncmp("$EndNeighbours", buffer, 14) != 0) {
        wasora_push_error_message("$EndNeighbors not found in mesh file '%s'", mesh->file->path);
        return -2;
      }
      
    // ------------------------------------------------------      
    }
  }

  fclose(mesh->file->pointer);
  mesh->file->pointer = NULL;
  
  // verificamos que la malla tenga la dimension esperada
  if (mesh->bulk_dimensions == 0) {
    mesh->bulk_dimensions = bulk_dimensions;
  } else if (mesh->bulk_dimensions != bulk_dimensions) {
    wasora_push_error_message("mesh '%s' is expected to have %d dimensions but it has %d", mesh->file->path, mesh->bulk_dimensions, bulk_dimensions);
    return WASORA_RUNTIME_ERROR;
  }
  
  mesh->spatial_dimensions = spatial_dimensions;
  mesh->order = order;  

  return WASORA_RUNTIME_OK;
}

/*
int mesh_gmsh_write_header(FILE *file) {
  fprintf(file, "$MeshFormat\n");
  fprintf(file, "2.2 0 8\n");
  fprintf(file, "$EndMeshFormat\n");

  return WASORA_RUNTIME_OK;
}

int mesh_gmsh_write_mesh(mesh_t *mesh, int no_physical_names, FILE *file) {
  
  int i, j, n;
  physical_entity_t *physical_entity;

  if (no_physical_names == 0) {
    // tenemos que contar las physical entities primero
    n = 0;
    LL_FOREACH(wasora_mesh.physical_entities, physical_entity) {
      if (physical_entity->name != NULL) {
        n++;
      }
    }
    if (n != 0) {
      fprintf(file, "$PhysicalNames\n");
      fprintf(file, "%d\n", n);
  
      // y despues barrerlas
      LL_FOREACH(wasora_mesh.physical_entities, physical_entity) {
        if (physical_entity->name != NULL) {
          fprintf(file, "%d %d \"%s\"\n", physical_entity->dimension, physical_entity->id, physical_entity->name);
        }
      }
      fprintf(file, "$EndPhysicalNames\n");
    }
  }
  
  
  fprintf(file, "$Nodes\n");
  fprintf(file, "%d\n", mesh->n_nodes);
  for (i = 0; i < mesh->n_nodes; i++) {
    fprintf(file, "%d %g %g %g\n", mesh->node[i].id, mesh->node[i].x[0], mesh->node[i].x[1], mesh->node[i].x[2]);
  }
  fprintf(file, "$EndNodes\n");

  fprintf(file, "$Elements\n");
  fprintf(file, "%d\n", mesh->n_elements);
  for (i = 0; i < mesh->n_elements; i++) {
    fprintf(file, "%d ", mesh->element[i].id);
    fprintf(file, "%d ", mesh->element[i].type->id);

    // en principio aca tendriamos que poner los tags y que se yo
//    fprintf(file, "%d ", mesh->element[i].ntags);
    
    // por ahora ponemos solo dos tags que son los que al menos requiere gmsh
    // el primero es la entidad fisica y el segundo la geometria, que no nos interesa
    if (mesh->element[i].physical_entity != NULL) {
      fprintf(file, "2 %d 0", mesh->element[i].physical_entity->id);
    } else {
      fprintf(file, "2 0 0");
    }
    // los nodos
    for (j = 0; j < mesh->element[i].type->nodes; j++) {
      fprintf(file, " %d", mesh->element[i].node[j]->id);
    }
    fprintf(file, "\n");
  }
  fprintf(file, "$EndElements\n");
  
  return WASORA_RUNTIME_OK;
  
}


int mesh_gmsh_write_scalar(mesh_post_t *mesh_post, function_t *function, centering_t centering) {

  int i;
  mesh_t *mesh;
  
  if (mesh_post->mesh != NULL) {
    mesh = mesh_post->mesh;
  } else if (function != NULL) {
    mesh = function->mesh;
  } else {
    wasora_push_error_message("do not know which mesh to apply to post-process function '%s'", function->name);
    return WASORA_RUNTIME_ERROR;
  }

  if (centering == centering_cells) {
    fprintf(mesh_post->file->pointer, "$ElementData\n");
    if (mesh->n_cells == 0) {
      wasora_call(mesh_element2cell(mesh));
    }
  } else {
    fprintf(mesh_post->file->pointer, "$NodeData\n");
  }

  // un tag de string  
  fprintf(mesh_post->file->pointer, "1\n");
  // nombre de la vista
  fprintf(mesh_post->file->pointer, "\"%s\"\n", function->name);
  // la otra (opcional) es el esquema de interpolacion
  
  // un tag real (el unico)  
  fprintf(mesh_post->file->pointer, "1\n");                          
  // tiempo
  fprintf(mesh_post->file->pointer, "%g\n", (wasora_var(wasora_special_var(end_time)) != 0) ? wasora_value(wasora_special_var(time)) : wasora_value(wasora_special_var(step_static)));

  // tres tags enteros
  fprintf(mesh_post->file->pointer, "3\n");
  // timestep
  fprintf(mesh_post->file->pointer, "%d\n", (int)((wasora_var(wasora_special_var(end_time)) != 0) ? wasora_var(wasora_special_var(step_transient)) : wasora_value(wasora_special_var(step_static))));
  // cantidad de datos por punto: uno (escalar)
  fprintf(mesh_post->file->pointer, "%d\n", 1);

  if (centering == centering_cells) {
    // numero de datos
    fprintf(mesh_post->file->pointer, "%d\n", mesh->n_cells);

    if (function->type == type_pointwise_mesh_cell) {
      for (i = 0; i < function->data_size; i++) {
        fprintf(mesh_post->file->pointer, "%d %g\n", mesh->cell[i].element->id, function->data_value[i]);
      }
    } else {
      for (i = 0; i < mesh->n_cells; i++) {
        fprintf(mesh_post->file->pointer, "%d %g\n", mesh->cell[i].element->id, wasora_evaluate_function(function, mesh->cell[i].x));
      }
    }
    fprintf(mesh_post->file->pointer, "$EndElementData\n");
  } else  {
    // numero de datos
    fprintf(mesh_post->file->pointer, "%d\n", mesh->n_nodes);              
  
    if (function->type == type_pointwise_mesh_node) {
      for (i = 0; i < function->data_size; i++) {
        fprintf(mesh_post->file->pointer, "%d %g\n", mesh->node[i].id, function->data_value[i]);
      }
    } else {
      for (i = 0; i < mesh->n_nodes; i++) {
        fprintf(mesh_post->file->pointer, "%d %g\n", mesh->node[i].id, wasora_evaluate_function(function, mesh->node[i].x));
      }
    }
    fprintf(mesh_post->file->pointer, "$EndNodeData\n");    
  }
  
  fflush(mesh_post->file->pointer);
  

  return WASORA_RUNTIME_OK;

}

int mesh_gmsh_write_vector(mesh_post_t *mesh_post, function_t **function, centering_t centering) {

  int i;
  mesh_t *mesh;
  
  if (mesh_post->mesh != NULL) {
    mesh = mesh_post->mesh;
  } else if (function != NULL) {
    mesh = function[0]->mesh;
  } else {
    return WASORA_RUNTIME_ERROR;
  }

  if (centering == centering_cells) {
    fprintf(mesh_post->file->pointer, "$ElementData\n");
    if (mesh->n_cells == 0) {
      wasora_call(mesh_element2cell(mesh));
    }
  } else {
    fprintf(mesh_post->file->pointer, "$NodeData\n");
  }

  // un tag de string  
  fprintf(mesh_post->file->pointer, "1\n");
  // nombre de la vista
  fprintf(mesh_post->file->pointer, "\"%s-%s-%s\"\n", function[0]->name, function[1]->name, function[2]->name);
  // la otra (opcional) es el esquema de interpolacion
  
  // un tag real (el unico)  
  fprintf(mesh_post->file->pointer, "1\n");                          
  // tiempo
  fprintf(mesh_post->file->pointer, "%g\n", (wasora_var(wasora_special_var(end_time)) != 0) ? wasora_value(wasora_special_var(time)) : wasora_value(wasora_special_var(step_static)));

  // tres tags enteros
  fprintf(mesh_post->file->pointer, "3\n");
  // timestep
  fprintf(mesh_post->file->pointer, "%d\n", (int)((wasora_var(wasora_special_var(end_time)) != 0) ? wasora_var(wasora_special_var(step_transient)) : wasora_value(wasora_special_var(step_static))));
  // cantidad de datos por punto: 3 (un vector)
  fprintf(mesh_post->file->pointer, "%d\n", 3);

  if (centering == centering_cells) {
    for (i = 0; i < mesh->n_cells; i++) {
      fprintf(mesh_post->file->pointer, "%d %g %g %g\n", mesh->cell[i].element->id,
                                                         wasora_evaluate_function(function[0], mesh->cell[i].x),
                                                         wasora_evaluate_function(function[1], mesh->cell[i].x),
                                                         wasora_evaluate_function(function[2], mesh->cell[i].x));
    }
    fprintf(mesh_post->file->pointer, "$EndElementData\n");
  } else {
    // numero de datos
    fprintf(mesh_post->file->pointer, "%d\n", mesh->n_nodes);              
  
    for (i = 0; i < mesh->n_nodes; i++) {
      fprintf(mesh_post->file->pointer, "%d %g %g %g\n", mesh->node[i].id,
                                                         wasora_evaluate_function(function[0], mesh->node[i].x),
                                                         wasora_evaluate_function(function[1], mesh->node[i].x),
                                                         wasora_evaluate_function(function[2], mesh->node[i].x));
      }
    fprintf(mesh_post->file->pointer, "$EndNodeData\n");
  }
 
  fflush(mesh_post->file->pointer);
  

  return WASORA_RUNTIME_OK;

}
*/