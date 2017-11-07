/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related element routines
 *
 *  Copyright (C) 2015--2016 jeremy theler
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

/*

Line:                   Line3:           Line4:    
                                                
0----------1 --> u      0-----2----1     0----2----3----1

Triangle:               Triangle6:          Triangle9/10:          Triangle12/15:

v                                                              
^                                                                   2 
|                                                                   | \ 
2                       2                    2                      9   8
|`\                     |`\                  | \                    |     \ 
|  `\                   |  `\                7   6                 10 (14)  7
|    `\                 5    `4              |     \                |         \ 
|      `\               |      `\            8  (9)  5             11 (12) (13) 6
|        `\             |        `\          |         \            |             \
0----------1 --> u      0-----3----1         0---3---4---1          0---3---4---5---1

Quadrangle:            Quadrangle8:            Quadrangle9:

      v
      ^
      |
3-----------2          3-----6-----2           3-----6-----2 
|     |     |          |           |           |           | 
|     |     |          |           |           |           | 
|     +---- | --> u    7           5           7     8     5 
|           |          |           |           |           | 
|           |          |           |           |           | 
0-----------1          0-----4-----1           0-----4-----1 

Tetrahedron:                          Tetrahedron10:

                   v
                 .
               ,/
              /
           2                                     2                              
         ,/|`\                                 ,/|`\                          
       ,/  |  `\                             ,/  |  `\       
     ,/    '.   `\                         ,6    '.   `5     
   ,/       |     `\                     ,/       8     `\   
 ,/         |       `\                 ,/         |       `\ 
0-----------'.--------1 --> u         0--------4--'.--------1
 `\.         |      ,/                 `\.         |      ,/ 
    `\.      |    ,/                      `\.      |    ,9   
       `\.   '. ,/                           `7.   '. ,/     
          `\. |/                                `\. |/       
             `3                                    `3        
                `\.
                   ` w

Hexahedron:             Hexahedron20:          Hexahedron27:

       v
3----------2            3----13----2           3----13----2     
|\     ^   |\           |\         |\          |\         |\    
| \    |   | \          | 15       | 14        |15    24  | 14  
|  \   |   |  \         9  \       11 \        9  \ 20    11 \  
|   7------+---6        |   7----19+---6       |   7----19+---6 
|   |  +-- |-- | -> u   |   |      |   |       |22 |  26  | 23| 
0---+---\--1   |        0---+-8----1   |       0---+-8----1   | 
 \  |    \  \  |         \  17      \  18       \ 17    25 \  18
  \ |     \  \ |         10 |        12|        10 |  21    12| 
   \|      w  \|           \|         \|          \|         \| 
    4----------5            4----16----5           4----16----5 

Prism:                      Prism15:               Prism18:

           w
           ^
           |
           3                       3                      3        
         ,/|`\                   ,/|`\                  ,/|`\      
       ,/  |  `\               12  |  13              12  |  13    
     ,/    |    `\           ,/    |    `\          ,/    |    `\  
    4------+------5         4------14-----5        4------14-----5 
    |      |      |         |      8      |        |      8      | 
    |    ,/|`\    |         |      |      |        |    ,/|`\    | 
    |  ,/  |  `\  |         |      |      |        |  15  |  16  | 
    |,/    |    `\|         |      |      |        |,/    |    `\| 
   ,|      |      |\        10     |      11       10-----17-----11
 ,/ |      0      | `\      |      0      |        |      0      | 
u   |    ,/ `\    |    v    |    ,/ `\    |        |    ,/ `\    | 
    |  ,/     `\  |         |  ,6     `7  |        |  ,6     `7  | 
    |,/         `\|         |,/         `\|        |,/         `\| 
    1-------------2         1------9------2        1------9------2 

Pyramid:                     Pyramid13:                   Pyramid14:

               4                            4                            4
             ,/|\                         ,/|\                         ,/|\
           ,/ .'|\                      ,/ .'|\                      ,/ .'|\
         ,/   | | \                   ,/   | | \                   ,/   | | \
       ,/    .' | `.                ,/    .' | `.                ,/    .' | `.
     ,/      |  '.  \             ,7      |  12  \             ,7      |  12  \
   ,/       .' w |   \          ,/       .'   |   \          ,/       .'   |   \
 ,/         |  ^ |    \       ,/         9    |    11      ,/         9    |    11
0----------.'--|-3    `.     0--------6-.'----3    `.     0--------6-.'----3    `.
 `\        |   |  `\    \      `\        |      `\    \     `\        |      `\    \
   `\     .'   +----`\ - \ -> v  `5     .'        10   \      `5     .' 13     10   \
     `\   |    `\     `\  \        `\   |           `\  \       `\   |           `\  \ 
       `\.'      `\     `\`          `\.'             `\`         `\.'             `\` 
          1----------------2            1--------8-------2           1--------8-------2
                    `\
                       u
*/

int wasora_mesh_element_types_init(void) {

  element_type_t *element_type;
//  int v, j, m;
  
  wasora_mesh.element_type = calloc(16, sizeof(element_type_t));

  // undefined  ----------------------------------------------------------------
  element_type = &wasora_mesh.element_type[ELEMENT_TYPE_UNDEFINED];
  element_type->name = strdup("undefined");
  element_type->id = ELEMENT_TYPE_UNDEFINED;
  element_type->dim = 0;
  element_type->nodes = 0;
  element_type->faces = 0;
  element_type->nodes_per_face = 0;
  element_type->h = NULL;
  element_type->dhdr = NULL;
  element_type->point_in_element = NULL;
  element_type->element_volume = NULL;
  
  // line ----------------------------------------------------------------------
  mesh_two_node_line_init();
  mesh_three_node_line_init();

  // triangles -----------------------------------------------------------------
  mesh_three_node_triangle_init();
  mesh_six_node_triangle_init();
  
  // quadrangle ----------------------------------------------------------------
  mesh_four_node_quadrangle_init();
  
  // tetrahedra  ---------------------------------------------------------------
  mesh_four_node_tetrahedron_init();
  mesh_ten_node_tetrahedron_init();
  
  // hexahedron ---------------------------------------------------------------- 
  mesh_eight_node_hexahedron_init();
  mesh_twentyseven_node_hexaedron_init();
  mesh_twenty_node_hexaedron_init();
    
  // prism ---------------------------------------------------------------------
  mesh_six_node_prism_init();

  // not supported  
  element_type = &wasora_mesh.element_type[ELEMENT_TYPE_PYRAMID];
  element_type->dim = 3;
  element_type->name = strdup("pyramid");
  element_type->id = ELEMENT_TYPE_PYRAMID;
  element_type->nodes = 0;

  wasora_mesh.element_type[10].name = strdup("quad9");
//  wasora_mesh.element_type[10].nodes = 0;
  
//  wasora_mesh.element_type[12].name = strdup("hexa27");
  wasora_mesh.element_type[13].name = strdup("prism18");
  wasora_mesh.element_type[14].name = strdup("pyramid14");
  wasora_mesh.element_type[16].name = strdup("quad8");
//  wasora_mesh.element_type[16].nodes = 0;

  
  // point
  mesh_one_node_point_init();
  
  return WASORA_RUNTIME_OK;
  
};

int mesh_alloc_gauss(gauss_t *gauss, element_type_t *element_type, int V) {

  int v, j;
  int dim = (element_type->dim != 0) ? element_type->dim : 1;

  gauss->V = V;

  gauss->w = calloc(gauss->V, sizeof(double));
  gauss->r = calloc(gauss->V, sizeof(double *));
  gauss->h = calloc(gauss->V, sizeof(double *));
  gauss->dhdr = calloc(gauss->V, sizeof(double **));
  for (v = 0; v < gauss->V; v++) {
    gauss->r[v] = calloc(dim, sizeof(double));
    
    gauss->h[v] = calloc(element_type->nodes, sizeof(double));
    gauss->dhdr[v] = calloc(element_type->nodes, sizeof(double *));
    for (j = 0; j < element_type->nodes; j++) {
      gauss->dhdr[v][j] = calloc(dim, sizeof(double));
    }
  }
  
  return WASORA_RUNTIME_OK;
  
}

int mesh_init_shape_at_gauss(gauss_t *gauss, element_type_t *element_type) {
  
  int v, j, d;
  gsl_vector *r = gsl_vector_alloc(element_type->dim);
  
  for (v = 0; v < gauss->V; v++) {
    for (j = 0; j < element_type->nodes; j++) {

      for (d = 0; d < element_type->dim; d++) {
        gsl_vector_set(r, d, gauss->r[v][d]);
      }

      gauss->h[v][j] = element_type->h(j, r);
      for (d = 0; d < element_type->dim; d++) {
        gauss->dhdr[v][j][d] = element_type->dhdr(j, d, r);
      }
      
    }
  }
  gsl_vector_free(r);
  
  return WASORA_RUNTIME_OK;
}


// esta no rellena los nodos!
int mesh_create_element(element_t *element, int id, int type, physical_entity_t *physical_entity) {
 
  element->id = id;
  element->type = &(wasora_mesh.element_type[type]);
  element->node  = calloc(element->type->nodes, sizeof(node_t *));
  element->physical_entity = physical_entity;
  
  return WASORA_RUNTIME_OK;
}

int mesh_add_element_to_list(element_list_item_t **list, element_t *element) {
  
  element_list_item_t *item = calloc(1, sizeof(element_list_item_t));
  item->element = element;
  LL_APPEND(*list, item);
  
  return WASORA_RUNTIME_OK;
  
}

int mesh_add_material_to_list(material_list_item_t **list, material_t *material) {
  
  material_list_item_t *item;
  
  // solo agregamos el material si es que no esta en la lista
  LL_FOREACH(*list, item) {
    if (item->material == material) {
      return WASORA_RUNTIME_OK;
    }
  }
  
  item = calloc(1, sizeof(material_list_item_t));
  item->material = material;
  LL_APPEND(*list, item);
  
  return WASORA_RUNTIME_OK;
  
}

int mesh_compute_element_barycenter(element_t *element, double barycenter[]) {
  
  int j;
  
  barycenter[0] = barycenter[1] = barycenter[2] = 0;
  for (j = 0; j < element->type->nodes; j++) {
    barycenter[0] += element->node[j]->x[0];
    barycenter[1] += element->node[j]->x[1];
    barycenter[2] += element->node[j]->x[2];
  }
  barycenter[0] /= element->type->nodes; 
  barycenter[1] /= element->type->nodes; 
  barycenter[2] /= element->type->nodes; 
  
  return WASORA_RUNTIME_OK;
}


int mesh_node_indexes(mesh_t *mesh, int dofs) {
  int j, g;
  
  mesh->degrees_of_freedom = dofs;

  switch (mesh->ordering) {
    case ordering_node_based:
      for (j = 0; j < mesh->n_nodes; j++) {
        mesh->node[j].index = malloc(mesh->degrees_of_freedom*sizeof(int));
        for (g = 0; g < mesh->degrees_of_freedom; g++) {
          mesh->node[j].index[g] = mesh->degrees_of_freedom*j + g;
        }
      }  
    break;    
    
    case ordering_unknown_based:
      for (j = 0; j < mesh->n_nodes; j++) {
        mesh->node[j].index = malloc(mesh->degrees_of_freedom*sizeof(int));
        for (g = 0; g < mesh->degrees_of_freedom; g++) {
          mesh->node[j].index[g] = mesh->n_nodes*g + j;
        }
      }  
    break;

  }
  
  return WASORA_RUNTIME_OK;
}


int mesh_compute_local_node_index(element_t *element, int global_index) {
  int j;
  int local_index = -1;
  
  // buscamos a que indice local i corresponde el j global
  for (j = 0; j < element->type->nodes; j++) {
    if (element->node[j]->id == global_index+1) {
      local_index = j;
      break;
    }
  }
  
  return local_index;
  
}
