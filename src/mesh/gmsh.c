/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related gmsh routines
 *
 *  Copyright (C) 2014--2018 jeremy theler
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



int mesh_gmsh_readmesh(mesh_t *mesh) {

  char buffer[BUFFER_SIZE];
  int *tags = NULL;
  int *tag2index = NULL;

  char *dummy = NULL;
  char *name = NULL;
  physical_entity_t *physical_entity = NULL;
  physical_entity_t *physical_entities_by_dim_tag = NULL; // hash table

  int i, j, k, l;
  int i_entity[4];
  int version_maj;
//  int version_min;
  int blocks, tag_geo, tag, dim, parametric, num;
  int type;
  int ntags, tag_max;
  int node;
  int dimension;
  int cell_id;
  int spatial_dimensions;
  int bulk_dimensions;
  int order;


  if (mesh->file->pointer == NULL) {
    wasora_call(wasora_instruction_open_file(mesh->file));
  }

  // empezamos suponiendo cero dimensiones y vamos viendo cual es el elemento
  // de mayor dimension que aparece -> esa es la de la malla
  bulk_dimensions = 0;
  spatial_dimensions = 0;
  order = 0; // idem con el orden
  
  while (fgets(buffer, BUFFER_SIZE-1, mesh->file->pointer) != NULL) {

    if (strncmp("\n", buffer, 1) == 0) {
      ;
    
    // ------------------------------------------------------  
    } else if (strncmp("$MeshFormat", buffer, 11) == 0) {
  
      // la version
      if (fscanf(mesh->file->pointer, "%s", buffer) == 0) {
        return WASORA_RUNTIME_ERROR;
      }
      if (strcmp("2.2", buffer) == 0) {
        version_maj = 2;
//        version_min = 2;
      } else if (strcmp("4", buffer) == 0) {
        version_maj = 4;
//        version_min = 0;       
      } else {
        wasora_push_error_message("mesh '%s' has an incompatible version '%s', only version 2.2 or 4 are supported", mesh->file->path, buffer);
        return WASORA_RUNTIME_ERROR;
      }
  
      // el tipo (0 = ASCII)
      if (fscanf(mesh->file->pointer, "%s", buffer) == 0) {
        return WASORA_RUNTIME_ERROR;
      }
      if (strcmp("0", buffer) != 0) {
        wasora_push_error_message("mesh '%s' is binary, only ASCII files are supported", mesh->file->path);
        return WASORA_RUNTIME_ERROR;
      }
  
      // el tamano de un double es irrelevante en ASCII
      if (fscanf(mesh->file->pointer, "%s", buffer) == 0) {
        return WASORA_RUNTIME_ERROR;
      }
/*      
      if (strcmp("8", buffer) != 0) {
        wasora_push_error_message("mesh '%s' has an incompatible data size '%s', only 8-byte files are supported", mesh->file->path, buffer);
        return WASORA_RUNTIME_ERROR;
      }
*/
      // el newline
      if (fgets(buffer, BUFFER_SIZE-1, mesh->file->pointer) == NULL) {
        wasora_push_error_message("corrupted mesh '%s'", mesh->file->path);
        return WASORA_RUNTIME_ERROR;
      } 
      
      // la linea $EndMeshFormat
      if (fgets(buffer, BUFFER_SIZE-1, mesh->file->pointer) == NULL) {
        wasora_push_error_message("corrupted mesh '%s'", mesh->file->path);
        return WASORA_RUNTIME_ERROR;
      } 
      if (strncmp("$EndMeshFormat", buffer, 14) != 0) {
        wasora_push_error_message("$EndMeshFormat not found in mesh '%s'", mesh->file->path);
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

        if (fscanf(mesh->file->pointer, "%d %d", &dimension, &tag) < 2) {
          return WASORA_RUNTIME_ERROR;
        }
        if (dimension < 0 || dimension > 4) {
          wasora_push_error_message("invalid dimension %d for physical entity %d in mesh file '%s'", dimension, tag, mesh->file->path);
          return WASORA_RUNTIME_ERROR;
        }
        
        if (fgets(buffer, BUFFER_SIZE-1, mesh->file->pointer) == NULL) {
          wasora_push_error_message("corrupted mesh file '%s'", mesh->file->path);
          return WASORA_RUNTIME_ERROR;
        }
        if ((dummy = strrchr(buffer, '\"')) == NULL) {
          wasora_push_error_message("end quote not found in physical name %d in '%s'", tag, mesh->file->path);
          return WASORA_RUNTIME_ERROR;
        }
        *dummy = '\0';
        if ((dummy = strchr(buffer, '\"')) == NULL) {
          wasora_push_error_message("beginning quote not found in physical name %d in '%s'", tag, mesh->file->path);
          return WASORA_RUNTIME_ERROR;
        }
        name = strdup(dummy+1);

        HASH_FIND_STR(wasora_mesh.physical_entities_by_name, name, physical_entity);
       
        if (physical_entity == NULL) {
          // creamos una de prepo
          if ((physical_entity = wasora_define_physical_entity(name, mesh, dimension)) == NULL) {
            return WASORA_RUNTIME_ERROR;
          }
          physical_entity->tag = tag;
        } else {
          // si ya hay una, primero miramos que corresponda a esta misma malla
          if (physical_entity->mesh == NULL) {
            physical_entity->mesh = mesh;
          } else if (physical_entity->mesh != mesh) {
            wasora_push_error_message("physical entity '%s' in mesh '%s' already belongs to mesh '%s'", name, mesh->name, physical_entity->mesh->name);
          }
          // y despues verificamos que no tenga id numerica
          if (physical_entity->tag == 0) {
            physical_entity->tag = tag;
          } else if (physical_entity->tag != tag) {
            // o que tenga la correcta
            wasora_push_error_message("physical entity '%s' has tag %d in input and %d in mesh '%s'", name, physical_entity->tag, tag, mesh->name);
            return WASORA_PARSER_ERROR;
          }
          
        }

        // agregamos la entity a un hash local para despues
        // resolver el apuntador desde cada elemento "mas o menos" facil
        snprintf(buffer, BUFFER_SIZE-1, "%d,%d", dimension, tag);
        HASH_ADD_KEYPTR(hh_dim_tag, physical_entities_by_dim_tag, buffer, (unsigned)uthash_strlen(buffer), physical_entity);

        // si la physical entity no tiene material, buscamos uno que se llame igual
        if (physical_entity->material == NULL) {
          // si es NULL es porque la physical entity es una BC o algo,
          // pero eso no nos molesta, las BCs las vemos despues
          HASH_FIND_STR(wasora_mesh.materials, name, physical_entity->material);
        }

        free(name);

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
    } else if (strncmp("$Entities", buffer, 9) == 0) {
 
      // la cantidad de cosas
      if (fscanf(mesh->file->pointer, "%d %d %d %d", &mesh->points, &mesh->curves, &mesh->surfaces, &mesh->volumes) < 4) {
        return WASORA_RUNTIME_ERROR;
      }
      
      mesh->geometrical_entity[0] = calloc(mesh->points, sizeof(geometrical_entity_t));
      mesh->geometrical_entity[1] = calloc(mesh->curves, sizeof(geometrical_entity_t));
      mesh->geometrical_entity[2] = calloc(mesh->surfaces, sizeof(geometrical_entity_t));
      mesh->geometrical_entity[3] = calloc(mesh->volumes, sizeof(geometrical_entity_t));
      
      for (dimension = 0; dimension < 4; dimension++) {
        i_entity[dimension] = 0;
      }
      
      for (i = 0; i < mesh->points+mesh->curves+mesh->surfaces+mesh->volumes; i++) {
        if (i < mesh->points) {
          dimension = 0;
        } else if (i < mesh->points+mesh->curves) {
          dimension = 1;          
        } else if (i < mesh->points+mesh->curves+mesh->surfaces) {
          dimension = 2;          
        } else {
          dimension = 3;
        }
        if (fscanf(mesh->file->pointer, "%d %lf %lf %lf %lf %lf %lf %d",
                &mesh->geometrical_entity[dimension][i_entity[dimension]].tag,
                &mesh->geometrical_entity[dimension][i_entity[dimension]].boxMinX,
                &mesh->geometrical_entity[dimension][i_entity[dimension]].boxMinY,
                &mesh->geometrical_entity[dimension][i_entity[dimension]].boxMinZ,
                &mesh->geometrical_entity[dimension][i_entity[dimension]].boxMaxX,
                &mesh->geometrical_entity[dimension][i_entity[dimension]].boxMaxY,
                &mesh->geometrical_entity[dimension][i_entity[dimension]].boxMaxZ,
                &mesh->geometrical_entity[dimension][i_entity[dimension]].num_physicals) < 8) {
          return WASORA_RUNTIME_ERROR;
        }
        
        if (mesh->geometrical_entity[dimension][i_entity[dimension]].num_physicals != 0) {
          mesh->geometrical_entity[dimension][i_entity[dimension]].physical = calloc(mesh->geometrical_entity[dimension][i_entity[dimension]].num_physicals, sizeof(int));
          for (j = 0; j < mesh->geometrical_entity[dimension][i_entity[dimension]].num_physicals; j++) {
            if (fscanf(mesh->file->pointer, "%d", &mesh->geometrical_entity[dimension][i_entity[dimension]].physical[j]) == 0) {
              return WASORA_RUNTIME_ERROR;
            }
          }
        }
        
        // los puntos no tienen bounding
        if (dimension != 0) {
          if (fscanf(mesh->file->pointer, "%d", &mesh->geometrical_entity[dimension][i_entity[dimension]].num_bounding) < 1) {
            return WASORA_RUNTIME_ERROR;
          }          
          if (mesh->geometrical_entity[dimension][i_entity[dimension]].num_bounding != 0) {
            mesh->geometrical_entity[dimension][i_entity[dimension]].bounding = calloc(mesh->geometrical_entity[dimension][i_entity[dimension]].num_bounding, sizeof(int));
            for (j = 0; j < mesh->geometrical_entity[dimension][i_entity[dimension]].num_bounding; j++) {
              if (fscanf(mesh->file->pointer, "%d", &mesh->geometrical_entity[dimension][i_entity[dimension]].bounding[j]) == 0) {
                return WASORA_RUNTIME_ERROR;
              }
            }
          }
        }
      }

      // queda un blanco y un newline
      if (fgets(buffer, BUFFER_SIZE-1, mesh->file->pointer) == NULL) {
        wasora_push_error_message("corrupted mesh file '%s'", mesh->file->path);
        return -3;
      } 

      // la linea $EndEntities
      if (fgets(buffer, BUFFER_SIZE-1, mesh->file->pointer) == NULL) {
        wasora_push_error_message("corrupted mesh file '%s'", mesh->file->path);
        return -3;
      } 
      if (strncmp("$EndEntities", buffer, 12) != 0) {
        wasora_push_error_message("$EndEntities not found in mesh file '%s'", mesh->file->path);
        return -2;
      }
      
    // ------------------------------------------------------      
    } else if (strncmp("$Nodes", buffer, 6) == 0) {

      if (version_maj == 2) {
        // la cantidad de nodos
        if (fscanf(mesh->file->pointer, "%d", &(mesh->n_nodes)) == 0) {
          return WASORA_RUNTIME_ERROR;
        }
        if (mesh->n_nodes == 0) {
          wasora_push_error_message("no nodes found in mesh '%s'", mesh->file->path);
          return WASORA_RUNTIME_ERROR;
        }

        mesh->node = calloc(mesh->n_nodes, sizeof(node_t));

        for (i = 0; i < mesh->n_nodes; i++) {
          if (fscanf(mesh->file->pointer, "%d %lf %lf %lf",
                  &tag,
                  &mesh->node[i].x[0],
                  &mesh->node[i].x[1],
                  &mesh->node[i].x[2]) < 4) {
            return WASORA_RUNTIME_ERROR;
          }
          // en msh2 los tags son indices
          if (i+1 != tag) {
            wasora_push_error_message("nodes in file '%s' are sparse", mesh->file->path);
            return WASORA_RUNTIME_ERROR;
          }
          mesh->node[i].tag = tag;
          mesh->node[i].index = i;
        }
        
      } else if (version_maj == 4) {
        // la cantidad de bloques y de nodos
        if (fscanf(mesh->file->pointer, "%d %d", &blocks, &mesh->n_nodes) < 2) {
          return WASORA_RUNTIME_ERROR;
        }
        if (mesh->n_nodes == 0) {
          wasora_push_error_message("no nodes found in mesh '%s'", mesh->file->path);
          return WASORA_RUNTIME_ERROR;
        }

        mesh->node = calloc(mesh->n_nodes, sizeof(node_t));
        
        tag_max = 0;
        i = 0;
        for (l = 0; l < blocks; l++) {
          if (fscanf(mesh->file->pointer, "%d %d %d %d", &tag_geo, &dim, &parametric, &num) < 4) {
            return WASORA_RUNTIME_ERROR;
          }
          if (parametric) {
            wasora_push_error_message("mesh '%s' contains parametric data, which is unsupported yet", mesh->file->path);
            return WASORA_RUNTIME_ERROR;
          }
          for (k = 0; k < num; k++) {
            if (fscanf(mesh->file->pointer, "%d %lf %lf %lf",
                  &tag,
                  &mesh->node[i].x[0],
                  &mesh->node[i].x[1],
                  &mesh->node[i].x[2]) < 4) {
              return WASORA_RUNTIME_ERROR;
            }

            if (tag > tag_max) {
              tag_max = tag;
            }
            // en msh4 los tags son los indices de la malla global
            mesh->node[i].tag = tag;
            mesh->node[i].index = i;
            i++;
          }
        }
        
        // tengo que hacer un loop extra en nodos porque no tuve el tamaño posta
        tag2index = malloc((tag_max+1) * sizeof(int));
        for (k = 0; k <= tag_max; k++) {
          tag2index[k] = -1;
        }
        for (i = 0; i < mesh->n_nodes; i++) {
          tag2index[mesh->node[i].tag] = i;
        }
        
      }
           
      // el newline
      if (fgets(buffer, BUFFER_SIZE-1, mesh->file->pointer) == NULL) {
        wasora_push_error_message("corrupted mesh file '%s'", mesh->file->path);
        return -3;
      } 
      
      // la linea $EndNodes
      if (fgets(buffer, BUFFER_SIZE-1, mesh->file->pointer) == NULL) {
        wasora_push_error_message("corrupted mesh file '%s'", mesh->file->path);
        return -3;
      } 
      if (strncmp("$EndNodes", buffer, 9) != 0) {
        wasora_push_error_message("$EndNodes not found in mesh file '%s'", mesh->file->path);
        return -2;
      }

    // ------------------------------------------------------      
    } else if (strncmp("$Elements", buffer, 9) == 0) {              
      
      if (version_maj == 2) {
        // la cantidad de elementos
        if (fscanf(mesh->file->pointer, "%d", &(mesh->n_elements)) == 0) {
          return WASORA_RUNTIME_ERROR;
        }
        if (mesh->n_elements == 0) {
          wasora_push_error_message("no elements found in mesh file '%s'", mesh->file->path);
          return -2;
        }
        mesh->element = calloc(mesh->n_elements, sizeof(element_t));

        for (i = 0; i < mesh->n_elements; i++) {

          if (fscanf(mesh->file->pointer, "%d %d %d", &tag, &type, &ntags) < 3) {
            return WASORA_RUNTIME_ERROR;
          }

          // en msh2 los tags son indices
          if (i+1 != tag) {
            wasora_push_error_message("nodes in file '%s' are sparse", mesh->file->path);
            return WASORA_RUNTIME_ERROR;
          }
          mesh->element[i].tag = tag;
          mesh->element[i].index = i;

          // tipo de elemento
          if (type >= NUMBER_ELEMENT_TYPE) {
            wasora_push_error_message("elements of type '%d' are not supported in this version :-(", type);
            return WASORA_RUNTIME_ERROR;
          }
          if (wasora_mesh.element_type[type].nodes == 0) {
            wasora_push_error_message("elements of type '%s' are not supported in this version :-(", mesh->element[i].type->name);
            return WASORA_RUNTIME_ERROR;
          }
          mesh->element[i].type = &(wasora_mesh.element_type[type]);

          // format v2.2
          // cada elemento tiene un tag que es un array de enteros
          // el primero es el id de la entidad fisica
          // el segundo es el id de la entidad geometrica (no nos interesa)
          // despues siguen cosas opcionales como particiones, padres, dominios, etc
          if (ntags > 0) {
            tags = malloc(ntags * sizeof(int));
            for (k = 0; k < ntags; k++) {
              if (fscanf(mesh->file->pointer, "%d", &tags[k]) == 0) {
                return WASORA_RUNTIME_ERROR;
              }
            }
            
            
            if (ntags > 1) {
              // buscamos en el hash local tag,dim
              snprintf(buffer, BUFFER_SIZE-1, "%d,%d", mesh->element[i].type->dim, tags[1]);
              HASH_FIND(hh_dim_tag, physical_entities_by_dim_tag, buffer, (unsigned)uthash_strlen(buffer), physical_entity);
              if ((mesh->element[i].physical_entity = physical_entity) == NULL) {
                // si no encontramos ninguna, hay que crear una
                if ((mesh->element[i].physical_entity = wasora_define_physical_entity(buffer, mesh, mesh->element[i].type->dim)) == NULL) {
                  return WASORA_RUNTIME_ERROR;
                }
                HASH_ADD_KEYPTR(hh_dim_tag, physical_entities_by_dim_tag, buffer, strlen(buffer), physical_entity);
                mesh->element[i].physical_entity->tag = tags[1];
              }
              mesh->element[i].physical_entity->n_elements++;
            }
            free(tags);
          }
          
          mesh->element[i].node = calloc(mesh->element[i].type->nodes, sizeof(node_t *));
          for (j = 0; j < mesh->element[i].type->nodes; j++) {
            if (fscanf(mesh->file->pointer, "%d", &node) < 1) {
              return WASORA_RUNTIME_ERROR;
            }
            if (node > mesh->n_nodes) {
              wasora_push_error_message("node %d in element %d does not exist", node, tag);
              return WASORA_RUNTIME_ERROR;
            }
            // en msh2 todo manzana
            mesh->element[i].node[j] = &mesh->node[node-1];
            mesh_add_element_to_list(&mesh->element[i].node[j]->associated_elements, &mesh->element[i]);
          }
        }
        
      } else if (version_maj == 4) {
        // la cantidad de bloques y de elementos
        if (fscanf(mesh->file->pointer, "%d %d", &blocks, &mesh->n_elements) < 2) {
          return WASORA_RUNTIME_ERROR;
        }
        if (mesh->n_elements == 0) {
          wasora_push_error_message("no elements found in mesh file '%s'", mesh->file->path);
          return -2;
        }
        mesh->element = calloc(mesh->n_elements, sizeof(element_t));

        i = 0;
        for (l = 0; l < blocks; l++) {
          if (fscanf(mesh->file->pointer, "%d %d %d %d", &tag_geo, &dim, &type, &num) < 4) {
            return WASORA_RUNTIME_ERROR;
          }
          if (type >= NUMBER_ELEMENT_TYPE) {
            wasora_push_error_message("elements of type '%d' are not supported in this version :-(", type);
            return WASORA_RUNTIME_ERROR;
          }
          if (wasora_mesh.element_type[type].nodes == 0) {
            wasora_push_error_message("elements of type '%s' are not supported in this version :-(", wasora_mesh.element_type[type].name);
            return WASORA_RUNTIME_ERROR;
          }
          
          for (k = 0; k < num; k++) {
            if (fscanf(mesh->file->pointer, "%d", &tag) == 0) {
              return WASORA_RUNTIME_ERROR;
            }
            
            mesh->element[i].tag = tag;
            mesh->element[i].index = i;
            mesh->element[i].type = &(wasora_mesh.element_type[type]);
            
            // aca hay que linkear a la entidad fisica de las entidades geometricas
            if (mesh->geometrical_entity[dim][tag_geo].num_physicals > 0) {
              // que hacemos si hay mas de una? la primera? la ultima?
              tag = mesh->geometrical_entity[dim][tag_geo].physical[0];
              // TODO: SPOT
              snprintf(buffer, BUFFER_SIZE-1, "%d,%d", mesh->element[i].type->dim, tag);
              HASH_FIND(hh_dim_tag, physical_entities_by_dim_tag, buffer, strlen(buffer), physical_entity);
              if ((mesh->element[i].physical_entity = physical_entity) == NULL) {
                if ((mesh->element[i].physical_entity = wasora_define_physical_entity(buffer, mesh, mesh->element[i].type->dim)) == NULL) {
                  return WASORA_RUNTIME_ERROR;
                }
                mesh->element[i].physical_entity->tag = tag;
              }
              physical_entity->n_elements++;
            }
            
            mesh->element[i].node = calloc(mesh->element[i].type->nodes, sizeof(node_t *));
            for (j = 0; j < mesh->element[i].type->nodes; j++) {
              if (fscanf(mesh->file->pointer, "%d", &node) == 0) {
                return WASORA_RUNTIME_ERROR;
              }
              if (node > mesh->n_nodes) {
                wasora_push_error_message("node %d in element %d does not exist", node, tag);
                return WASORA_RUNTIME_ERROR;
              }
              // ojo al piojo en msh4, hay que usar el maneje del tag2index
              mesh->element[i].node[j] = &mesh->node[tag2index[node]];
              mesh_add_element_to_list(&mesh->element[i].node[j]->associated_elements, &mesh->element[i]);
            }
            i++;
          }
        }
      }

      // el newline
      if (fgets(buffer, BUFFER_SIZE-1, mesh->file->pointer) == NULL) {
        wasora_push_error_message("corrupted mesh file '%s'", mesh->file->path);
        return -3;
      } 

      // la linea $EndElements
      if (fgets(buffer, BUFFER_SIZE-1, mesh->file->pointer) == NULL) {
        wasora_push_error_message("corrupted mesh file '%s'", mesh->file->path);
        return -3;
      } 
      if (strncmp("$EndElements", buffer, 12) != 0) {
        wasora_push_error_message("$EndElements not found in mesh file '%s'", mesh->file->path);
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
    } else {
        
      do {
        if (fgets(buffer, BUFFER_SIZE-1, mesh->file->pointer) == NULL || buffer == NULL) {
          wasora_push_error_message("corrupted mesh file '%s'", mesh->file->path);
          return -3;
        }
      } while(strncmp("$End", buffer, 4) != 0);
        
    }
  }

  fclose(mesh->file->pointer);
  mesh->file->pointer = NULL;
  
  free(tag2index);
  
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
          fprintf(file, "%d %d \"%s\"\n", physical_entity->dimension, physical_entity->tag, physical_entity->name);
        }
      }
      fprintf(file, "$EndPhysicalNames\n");
    }
  }
  
  
  fprintf(file, "$Nodes\n");
  fprintf(file, "%d\n", mesh->n_nodes);
  for (i = 0; i < mesh->n_nodes; i++) {
    fprintf(file, "%d %g %g %g\n", mesh->node[i].index, mesh->node[i].x[0], mesh->node[i].x[1], mesh->node[i].x[2]);
  }
  fprintf(file, "$EndNodes\n");

  fprintf(file, "$Elements\n");
  fprintf(file, "%d\n", mesh->n_elements);
  for (i = 0; i < mesh->n_elements; i++) {
    fprintf(file, "%d ", mesh->element[i].index);
    fprintf(file, "%d ", mesh->element[i].type->id);

    // en principio aca tendriamos que poner los tags y que se yo
//    fprintf(file, "%d ", mesh->element[i].ntags);
    
    // por ahora ponemos solo dos tags que son los que al menos requiere gmsh
    // el primero es la entidad fisica y el segundo la geometria, que no nos interesa
    if (mesh->element[i].physical_entity != NULL) {
      fprintf(file, "2 %d 0", mesh->element[i].physical_entity->tag);
    } else {
      fprintf(file, "2 0 0");
    }
    // los nodos
    for (j = 0; j < mesh->element[i].type->nodes; j++) {
      fprintf(file, " %d", mesh->element[i].node[j]->index);
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
        fprintf(mesh_post->file->pointer, "%d %g\n", mesh->cell[i].element->index, function->data_value[i]);
      }
    } else {
      for (i = 0; i < mesh->n_cells; i++) {
        fprintf(mesh_post->file->pointer, "%d %g\n", mesh->cell[i].element->index, wasora_evaluate_function(function, mesh->cell[i].x));
      }
    }
    fprintf(mesh_post->file->pointer, "$EndElementData\n");
  } else  {
    // numero de datos
    fprintf(mesh_post->file->pointer, "%d\n", mesh->n_nodes);              
  
    if (function->type == type_pointwise_mesh_node) {
      for (i = 0; i < function->data_size; i++) {
        fprintf(mesh_post->file->pointer, "%d %g\n", mesh->node[i].index, function->data_value[i]);
      }
    } else {
      for (i = 0; i < mesh->n_nodes; i++) {
        fprintf(mesh_post->file->pointer, "%d %g\n", mesh->node[i].index, wasora_evaluate_function(function, mesh->node[i].x));
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
      fprintf(mesh_post->file->pointer, "%d %g %g %g\n", mesh->cell[i].element->index,
                                                         wasora_evaluate_function(function[0], mesh->cell[i].x),
                                                         wasora_evaluate_function(function[1], mesh->cell[i].x),
                                                         wasora_evaluate_function(function[2], mesh->cell[i].x));
    }
    fprintf(mesh_post->file->pointer, "$EndElementData\n");
  } else {
    // numero de datos
    fprintf(mesh_post->file->pointer, "%d\n", mesh->n_nodes);              
  
    for (i = 0; i < mesh->n_nodes; i++) {
      fprintf(mesh_post->file->pointer, "%d %g %g %g\n", mesh->node[i].index,
                                                         wasora_evaluate_function(function[0], mesh->node[i].x),
                                                         wasora_evaluate_function(function[1], mesh->node[i].x),
                                                         wasora_evaluate_function(function[2], mesh->node[i].x));
      }
    fprintf(mesh_post->file->pointer, "$EndNodeData\n");
  }
 
  fflush(mesh_post->file->pointer);
  

  return WASORA_RUNTIME_OK;

}
