/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related gmsh routines
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
#include <stdlib.h>
#include <string.h>



int mesh_gmsh_readmesh(mesh_t *mesh) {

  char buffer[BUFFER_SIZE];
  int *tags = NULL;
  int *tag2index = NULL;

  char *dummy = NULL;
  char *name = NULL;
  physical_entity_t *physical_entity = NULL;
  geometrical_entity_t *geometrical_entity = NULL;
  // TODO: usar size_t en lugar de ints
  int i, j, k, l;
  int version_maj;
  int version_min;
  int blocks, geometrical, tag, dimension, parametric, num;
  int first, second; // this are buffers because 4.0 and 4.1 swapped tag,dim to dim,tag
  int type, physical;
  int ntags, tag_min, tag_max;
  int node;
  int cell_id;


  if (mesh->file->pointer == NULL) {
    wasora_call(wasora_instruction_open_file(mesh->file));
  }
  
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
        version_min = 2;
      } else if (strcmp("4", buffer) == 0) {
        version_maj = 4;
        version_min = 0;       
      } else if (strcmp("4.1", buffer) == 0) {
        version_maj = 4;
        version_min = 1;       
      } else {
        wasora_push_error_message("mesh '%s' has an incompatible version '%s', only versions 2.2, 4.0 and 4.1 are supported", mesh->file->path, buffer);
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
       
        if ((physical_entity = wasora_get_physical_entity_ptr(name, mesh)) == NULL) {
          // creamos una de prepo
          if ((physical_entity = wasora_define_physical_entity(name, mesh, dimension)) == NULL) {
            return WASORA_RUNTIME_ERROR;
          }
          physical_entity->tag = tag;
        } else {
          // verificamos que no tenga id numerica
          if (physical_entity->tag == 0) {
            physical_entity->tag = tag;
          } else if (physical_entity->tag != tag) {
            // o que tenga la correcta
            wasora_push_error_message("physical entity '%s' has tag %d in input and %d in mesh '%s'", name, physical_entity->tag, tag, mesh->name);
            return WASORA_PARSER_ERROR;
          }
          // y la dimension correcta
          if (physical_entity->dimension <= 0) {
            // si tiene 0 le ponemos la que acabamos de leer
            physical_entity->dimension = dimension;
          } else if (physical_entity->dimension != dimension) {
            // si no coincide nos quejamos
            wasora_push_error_message("physical entity '%s' has dimension %d in input and %d in mesh '%s'", name, physical_entity->dimension, dimension, mesh->name);
            return WASORA_PARSER_ERROR;
          }
          
        }

        // agregamos la entity a un hash local para despues
        // resolver el apuntador desde cada elemento "mas o menos" facil
        HASH_ADD(hh_tag[dimension], mesh->physical_entities_by_tag[dimension], tag, sizeof(int), physical_entity);

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
        
        geometrical_entity = calloc(1, sizeof(geometrical_entity_t));
        // a partir de 4.1 los puntos tienen solo 3 valores, no 6 de bounding box
        if (dimension == 0 && version_maj == 4 && version_min >= 1) {
          if (fscanf(mesh->file->pointer, "%d %lf %lf %lf %d",
                     &geometrical_entity->tag,
                     &geometrical_entity->boxMinX,
                     &geometrical_entity->boxMinY,
                     &geometrical_entity->boxMinZ,
                     &geometrical_entity->num_physicals) < 5) {
            wasora_push_error_message("not enough data in physical entities");
            return WASORA_RUNTIME_ERROR;
          }
        } else {
          if (fscanf(mesh->file->pointer, "%d %lf %lf %lf %lf %lf %lf %d",
                     &geometrical_entity->tag,
                     &geometrical_entity->boxMinX,
                     &geometrical_entity->boxMinY,
                     &geometrical_entity->boxMinZ,
                     &geometrical_entity->boxMaxX,
                     &geometrical_entity->boxMaxY,
                     &geometrical_entity->boxMaxZ,
                     &geometrical_entity->num_physicals) < 8) {
            wasora_push_error_message("not enough data in physical entities");
            return WASORA_RUNTIME_ERROR;
          }
        }        
        
        if (geometrical_entity->num_physicals != 0) {
          geometrical_entity->physical = calloc(geometrical_entity->num_physicals, sizeof(int));
          for (j = 0; j < geometrical_entity->num_physicals; j++) {
            if (fscanf(mesh->file->pointer, "%d", &geometrical_entity->physical[j]) == 0) {
              return WASORA_RUNTIME_ERROR;
            }
          }
        }
        
        // los puntos no tienen bounding
        if (dimension != 0) {
          if (fscanf(mesh->file->pointer, "%d", &geometrical_entity->num_bounding) < 1) {
            return WASORA_RUNTIME_ERROR;
          }          
          if (geometrical_entity->num_bounding != 0) {
            geometrical_entity->bounding = calloc(geometrical_entity->num_bounding, sizeof(int));
            for (j = 0; j < geometrical_entity->num_bounding; j++) {
              if (fscanf(mesh->file->pointer, "%d", &geometrical_entity->bounding[j]) == 0) {
                return WASORA_RUNTIME_ERROR;
              }
            }
          }
        }
        
        // agregamos la entity al hash de la dimension que corresponda
        HASH_ADD(hh[dimension], mesh->geometrical_entities[dimension], tag, sizeof(int), geometrical_entity);
        
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

        for (j = 0; j < mesh->n_nodes; j++) {
          if (fscanf(mesh->file->pointer, "%d %lf %lf %lf",
                  &tag,
                  &mesh->node[j].x[0],
                  &mesh->node[j].x[1],
                  &mesh->node[j].x[2]) < 4) {
            return WASORA_RUNTIME_ERROR;
          }
          
          // en msh2 los tags son indices
          if (j+1 != tag) {
            wasora_push_error_message("nodes in file '%s' are sparse", mesh->file->path);
            return WASORA_RUNTIME_ERROR;
          }
          mesh->node[j].tag = tag;
          mesh->node[j].index_mesh = j;
          
          // si nos dieron degrees of freedom entonces tenemos que allocar
          // lugar para la solucion phi de alguna PDE
          if (mesh->degrees_of_freedom != 0) {
            mesh->node[j].phi = calloc(mesh->degrees_of_freedom, sizeof(double));
          }
        }
        
      } else if (version_maj == 4) {
        // la cantidad de bloques y de nodos
        if (version_min == 0) {
          // en 4.0 no tenemos min y max
          tag_min = 0;
          tag_max = 0;
          if (fscanf(mesh->file->pointer, "%d %d", &blocks, &mesh->n_nodes) < 2) {
            wasora_push_error_message("error reading node blocks");
            return WASORA_RUNTIME_ERROR;
          }
        } else {
          if (fscanf(mesh->file->pointer, "%d %d %d %d", &blocks, &mesh->n_nodes, &tag_min, &tag_max) < 4) {
            wasora_push_error_message("error reading node blocks");
            return WASORA_RUNTIME_ERROR;
          }
        }
        if (mesh->n_nodes == 0) {
          wasora_push_error_message("no nodes found in mesh '%s'", mesh->file->path);
          return WASORA_RUNTIME_ERROR;
        }

        mesh->node = calloc(mesh->n_nodes, sizeof(node_t));

        if (tag_max != 0) {
          // podemos hacer este mapeo en una sola pasada porque tenemos tag_max
          // TODO: offsetear con tag_min?            
          tag2index = malloc((tag_max+1) * sizeof(int));
          for (k = 0; k <= tag_max; k++) {
            tag2index[k] = -1;
          }
        }
        
        i = 0;
        for (l = 0; l < blocks; l++) {
          if (fscanf(mesh->file->pointer, "%d %d %d %d", &first, &second, &parametric, &num) < 4) {
            wasora_push_error_message("not enough data in node block");
            return WASORA_RUNTIME_ERROR;
          }
          if (version_min == 0) {
            geometrical = first;
            dimension = second;
          } else {
            dimension = first;
            geometrical = second;
          }

          if (parametric) {
            wasora_push_error_message("mesh '%s' contains parametric data, which is unsupported yet", mesh->file->path);
            return WASORA_RUNTIME_ERROR;
          }
          
          if (version_min == 0) {
            // aca esta tag y coordenada en una sola linea
            for (k = 0; k < num; k++) {
              if (fscanf(mesh->file->pointer, "%d %lf %lf %lf",
                         &mesh->node[i].tag,
                         &mesh->node[i].x[0],
                         &mesh->node[i].x[1],
                         &mesh->node[i].x[2]) < 4) {
                wasora_push_error_message("reading node data");
                return WASORA_RUNTIME_ERROR;
              }
              
              if (mesh->node[i].tag > tag_max) {
                tag_max = mesh->node[i].tag;
              }
              
              // en msh4 los tags son los indices de la malla global
              mesh->node[i].index_mesh = i;
              
              // si nos dieron degrees of freedom entonces tenemos que allocar
              // lugar para la solucion phi de alguna PDE
              if (mesh->degrees_of_freedom != 0) {
                mesh->node[i].phi = calloc(mesh->degrees_of_freedom, sizeof(double));
              }
              
              i++;
            }
          } else {

            // aca primero todos los tags y despues las coordenadas (supuestamente para no mezclar ints y doubles)
            for (k = 0; k < num; k++) {
              if (fscanf(mesh->file->pointer, "%d", &mesh->node[i+k].tag) < 1) {
                wasora_push_error_message("reading node tag");
                return WASORA_RUNTIME_ERROR;
              }
            }
            for (k = 0; k < num; k++) {
              if (fscanf(mesh->file->pointer, "%lf %lf %lf",
                         &mesh->node[i].x[0],
                         &mesh->node[i].x[1],
                         &mesh->node[i].x[2]) < 3) {
                wasora_push_error_message("reading node coordinates");
                return WASORA_RUNTIME_ERROR;
              }
              
              // en msh4 los tags son los indices de la malla global
              mesh->node[i].index_mesh = i;
              tag2index[mesh->node[i].tag] = i;
              
              // si nos dieron degrees of freedom entonces tenemos que allocar
              // lugar para la solucion phi de alguna PDE
              if (mesh->degrees_of_freedom != 0) {
                mesh->node[i].phi = calloc(mesh->degrees_of_freedom, sizeof(double));
              }
              
              i++;
            }
          }
          
        }
        
        if (version_min == 0) {
          // tengo que hacer un loop extra en nodos porque no tuve el tamaño posta
          tag2index = malloc((tag_max+1) * sizeof(int));
          for (k = 0; k <= tag_max; k++) {
            tag2index[k] = -1;
          }
          for (i = 0; i < mesh->n_nodes; i++) {
            tag2index[mesh->node[i].tag] = i;
          }
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
              dimension = mesh->element[i].type->dim;
              HASH_FIND(hh_tag[dimension], mesh->physical_entities_by_tag[dimension], &tags[0], sizeof(int), physical_entity);
              if ((mesh->element[i].physical_entity = physical_entity) == NULL) {
                // si no encontramos ninguna, hay que crear una
                snprintf(buffer, BUFFER_SIZE-1, "%s_%d_%d", mesh->name, dimension, tags[0]);
                if ((mesh->element[i].physical_entity = wasora_define_physical_entity(buffer, mesh, mesh->element[i].type->dim)) == NULL) {
                  return WASORA_RUNTIME_ERROR;
                }
                mesh->element[i].physical_entity->tag = tags[0];
                HASH_ADD(hh_tag[dimension], mesh->physical_entities_by_tag[dimension], tag, sizeof(int), mesh->element[i].physical_entity);
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
            if (node < 1 || node > mesh->n_nodes) {
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
        if (version_min == 0) {
          // en 4.0 no tenemos min y max
          tag_min = 0;
          tag_max = 0;
          if (fscanf(mesh->file->pointer, "%d %d", &blocks, &mesh->n_elements) < 2) {
            wasora_push_error_message("error reading element blocks");
            return WASORA_RUNTIME_ERROR;
          }
        } else {
          if (fscanf(mesh->file->pointer, "%d %d %d %d", &blocks, &mesh->n_elements, &tag_min, &tag_max) < 4) {
            wasora_push_error_message("error reading node blocks");
            return WASORA_RUNTIME_ERROR;
          }
        }
        
        if (mesh->n_elements == 0) {
          wasora_push_error_message("no elements found in mesh file '%s'", mesh->file->path);
          return WASORA_RUNTIME_ERROR;
        }
        mesh->element = calloc(mesh->n_elements, sizeof(element_t));

        i = 0;
        for (l = 0; l < blocks; l++) {
          if (fscanf(mesh->file->pointer, "%d %d %d %d", &first, &second, &type, &num) < 4) {
            return WASORA_RUNTIME_ERROR;
          }
          if (version_min == 0) {
            geometrical = first;
            dimension = second;
          } else {
            dimension = first;
            geometrical = second;
          }
          
          if (type >= NUMBER_ELEMENT_TYPE) {
            wasora_push_error_message("elements of type '%d' are not supported in this version :-(", type);
            return WASORA_RUNTIME_ERROR;
          }
          if (wasora_mesh.element_type[type].nodes == 0) {
            wasora_push_error_message("elements of type '%s' are not supported in this version :-(", wasora_mesh.element_type[type].name);
            return WASORA_RUNTIME_ERROR;
          }
          
          // todo el bloque tiene la misma entidad fisica, la encontramos una vez y ya
          HASH_FIND(hh[dimension], mesh->geometrical_entities[dimension], &geometrical, sizeof(int), geometrical_entity);
          if (geometrical_entity->num_physicals > 0) {
            // que hacemos si hay mas de una? la primera? la ultima?
            physical = geometrical_entity->physical[0];
            HASH_FIND(hh_tag[dimension], mesh->physical_entities_by_tag[dimension], &physical, sizeof(int), physical_entity);
            if ((mesh->element[i].physical_entity = physical_entity) == NULL) {
              snprintf(buffer, BUFFER_SIZE-1, "%s_%d_%d", mesh->name, dimension, physical);
              if ((mesh->element[i].physical_entity = wasora_define_physical_entity(buffer, mesh, wasora_mesh.element_type[type].dim)) == NULL) {
                return WASORA_RUNTIME_ERROR;
              }
              mesh->element[i].physical_entity->tag = physical;
              HASH_ADD(hh_tag[dimension], mesh->physical_entities_by_tag[dimension], tag, sizeof(int), mesh->element[i].physical_entity);
            }
          } else {
            physical_entity = NULL;
          }
                    
          for (k = 0; k < num; k++) {
            if (fscanf(mesh->file->pointer, "%d", &tag) == 0) {
              return WASORA_RUNTIME_ERROR;
            }
            
            mesh->element[i].tag = tag;
            mesh->element[i].index = i;
            mesh->element[i].type = &(wasora_mesh.element_type[type]);
            
            if ((mesh->element[i].physical_entity = physical_entity) != NULL) {
              mesh->element[i].physical_entity->n_elements++;
            }
            
            mesh->element[i].node = calloc(mesh->element[i].type->nodes, sizeof(node_t *));
            for (j = 0; j < mesh->element[i].type->nodes; j++) {
              if (fscanf(mesh->file->pointer, "%d", &node) == 0) {
                return WASORA_RUNTIME_ERROR;
              }
              // ojo al piojo en msh4, hay que usar el maneje del tag2index
              if ((mesh->element[i].node[j] = &mesh->node[tag2index[node]]) == 0) {
                wasora_push_error_message("node %d in element %d does not exist", node, tag);
                return WASORA_RUNTIME_ERROR;
              }
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
      
      // si no tenemos funcion seguimos de largo e inogramos todo el bloque
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
  // limpiar hashes
  
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
    n = HASH_COUNT(mesh->physical_entities);
    if (n != 0) {
      fprintf(file, "$PhysicalNames\n");
      fprintf(file, "%d\n", n);
  
      // y despues barrerlas
      for (physical_entity = mesh->physical_entities; physical_entity != NULL; physical_entity = physical_entity->hh.next) {
        fprintf(file, "%d %d \"%s\"\n", physical_entity->dimension, physical_entity->tag, physical_entity->name);
      }
      fprintf(file, "$EndPhysicalNames\n");
    }
  }
  
  
  fprintf(file, "$Nodes\n");
  fprintf(file, "%d\n", mesh->n_nodes);
  for (i = 0; i < mesh->n_nodes; i++) {
    fprintf(file, "%d %g %g %g\n", mesh->node[i].tag, mesh->node[i].x[0], mesh->node[i].x[1], mesh->node[i].x[2]);
  }
  fprintf(file, "$EndNodes\n");

  fprintf(file, "$Elements\n");
  fprintf(file, "%d\n", mesh->n_elements);
  for (i = 0; i < mesh->n_elements; i++) {
    fprintf(file, "%d ", mesh->element[i].tag);
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
      fprintf(file, " %d", mesh->element[i].node[j]->tag);
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

    if (function->type == type_pointwise_mesh_cell && function->mesh == mesh) {
      for (i = 0; i < function->data_size; i++) {
        fprintf(mesh_post->file->pointer, "%d %g\n", mesh->cell[i].element->tag, function->data_value[i]);
      }
    } else {
      for (i = 0; i < mesh->n_cells; i++) {
        fprintf(mesh_post->file->pointer, "%d %g\n", mesh->cell[i].element->tag, wasora_evaluate_function(function, mesh->cell[i].x));
      }
    }
    fprintf(mesh_post->file->pointer, "$EndElementData\n");
  } else  {
    // numero de datos
    fprintf(mesh_post->file->pointer, "%d\n", mesh->n_nodes);              
  
    if (function->type == type_pointwise_mesh_node && function->mesh == mesh) {
      for (i = 0; i < function->data_size; i++) {
        fprintf(mesh_post->file->pointer, "%d %g\n", mesh->node[i].tag, function->data_value[i]);
      }
    } else {
      for (i = 0; i < mesh->n_nodes; i++) {
        fprintf(mesh_post->file->pointer, "%d %g\n", mesh->node[i].tag, wasora_evaluate_function(function, mesh->node[i].x));
      }
    }
    fprintf(mesh_post->file->pointer, "$EndNodeData\n");    
  }
  
  fflush(mesh_post->file->pointer);
  

  return WASORA_RUNTIME_OK;

}

int mesh_gmsh_write_vector(mesh_post_t *mesh_post, function_t **function, centering_t centering) {

  int i, j;
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
  fprintf(mesh_post->file->pointer, "\"%s_%s_%s\"\n", function[0]->name, function[1]->name, function[2]->name);
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
  
    for (j = 0; j < mesh->n_nodes; j++) {
      fprintf(mesh_post->file->pointer, "%d ", mesh->node[j].tag);
      
      if (function[0]->type == type_pointwise_mesh_node && function[0]->data_size == mesh_post->mesh->n_nodes) {
        fprintf(mesh_post->file->pointer, "%g ", function[0]->data_value[j]);
      } else {
        fprintf(mesh_post->file->pointer, "%g ", wasora_evaluate_function(function[0], mesh->node[j].x));
      }

      if (function[1]->type == type_pointwise_mesh_node && function[1]->data_size == mesh_post->mesh->n_nodes) {
        fprintf(mesh_post->file->pointer, "%g ", function[1]->data_value[j]);
      } else {
        fprintf(mesh_post->file->pointer, "%g ", wasora_evaluate_function(function[1], mesh->node[j].x));
      }

      if (function[2]->type == type_pointwise_mesh_node && function[2]->data_size == mesh_post->mesh->n_nodes) {
        fprintf(mesh_post->file->pointer, "%g\n", function[2]->data_value[j]);
      } else {
        fprintf(mesh_post->file->pointer, "%g\n", wasora_evaluate_function(function[2], mesh->node[j].x));
      }
    }
    fprintf(mesh_post->file->pointer, "$EndNodeData\n");
  }
 
  fflush(mesh_post->file->pointer);
  

  return WASORA_RUNTIME_OK;

}
