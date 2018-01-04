/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related second-order tetrahedron element routines
 *
 *  Copyright (C) 2017--2018 jeremy theler
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

#include <gsl/gsl_linalg.h>

// ---------------------------------------------------------------------
// tetrahedro isoparametrico de cuatro nodos sobre el triangulo unitario
// ---------------------------------------------------------------------

int mesh_ten_node_tetrahedron_init(void) {
  
  element_type_t *element_type;
  int j;
  
  element_type = &wasora_mesh.element_type[ELEMENT_TYPE_TETRAHEDRON10];
  element_type->name = strdup("tetrahedron10");
  element_type->id = ELEMENT_TYPE_TETRAHEDRON10;
  element_type->dim = 3;
  element_type->order = 2;
  element_type->nodes = 10;
  element_type->faces = 4;
  element_type->nodes_per_face = 6;
  element_type->h = mesh_ten_node_tetrahedron_h;
  element_type->dhdr = mesh_ten_node_tetrahedron_dhdr;
  element_type->point_in_element = mesh_point_in_tetrahedron;
  element_type->element_volume = mesh_tetrahedron_vol;

  // coordenadas de los nodos
/*
Tetrahedron10:





           2                  
         ,/|`\                
       ,/  |  `\       
     ,6    '.   `5     
   ,/       8     `\   
 ,/         |       `\ 
0--------4--'.--------1
 `\.         |      ,/ 
    `\.      |    ,9   
       `7.   '. ,/     
          `\. |/       
             `3        



*/     
  element_type->node_coords = calloc(element_type->nodes, sizeof(double *));
  element_type->node_parents = calloc(element_type->nodes, sizeof(node_relative_t *));  
  for (j = 0; j < element_type->nodes; j++) {
    element_type->node_coords[j] = calloc(element_type->dim, sizeof(double));  
  }
  
  element_type->first_order_nodes++;
  element_type->node_coords[0][0] = 0;
  element_type->node_coords[0][1] = 0;
  element_type->node_coords[0][2] = 0;
  
  element_type->first_order_nodes++;
  element_type->node_coords[1][0] = 1;  
  element_type->node_coords[1][1] = 0;
  element_type->node_coords[1][2] = 0;
  
  element_type->first_order_nodes++;
  element_type->node_coords[2][0] = 0;  
  element_type->node_coords[2][1] = 1;
  element_type->node_coords[2][2] = 0;

  element_type->first_order_nodes++;
  element_type->node_coords[3][0] = 0;  
  element_type->node_coords[3][1] = 0;
  element_type->node_coords[3][2] = 1;
  
  wasora_mesh_add_node_parent(element_type->node_parents[4], 0);
  wasora_mesh_add_node_parent(element_type->node_parents[4], 1);
  wasora_mesh_compute_coords_from_parent(element_type, 4);
  
  wasora_mesh_add_node_parent(element_type->node_parents[5], 1);
  wasora_mesh_add_node_parent(element_type->node_parents[5], 2);
  wasora_mesh_compute_coords_from_parent(element_type, 5); 
 
  wasora_mesh_add_node_parent(element_type->node_parents[6], 2);
  wasora_mesh_add_node_parent(element_type->node_parents[6], 0);
  wasora_mesh_compute_coords_from_parent(element_type, 6);   
 
  wasora_mesh_add_node_parent(element_type->node_parents[7], 3);
  wasora_mesh_add_node_parent(element_type->node_parents[7], 0);
  wasora_mesh_compute_coords_from_parent(element_type, 7);

  wasora_mesh_add_node_parent(element_type->node_parents[8], 2);
  wasora_mesh_add_node_parent(element_type->node_parents[8], 3);
  wasora_mesh_compute_coords_from_parent(element_type, 8);

  wasora_mesh_add_node_parent(element_type->node_parents[9], 3);
  wasora_mesh_add_node_parent(element_type->node_parents[9], 1);
  wasora_mesh_compute_coords_from_parent(element_type, 9);  
  
  mesh_tetrahedron_gauss_init(element_type);

  return WASORA_RUNTIME_OK;
}

double mesh_ten_node_tetrahedron_h(int j, gsl_vector *gsl_r) {
  double r;
  double s;
  double t;
//  double h[10];

  r = gsl_vector_get(gsl_r, 0);
  s = gsl_vector_get(gsl_r, 1);
  t = gsl_vector_get(gsl_r, 2);

  // bathe page 375 re-numerado para gmsh, hay que swapear 8 y 10
/*  
  h[8-1] = 4*t*(1-r-s-t);
  h[9-1] = 4*s*t;
  h[10-1] = 4*r*t;
  h[7-1] = 4*s*(1-r-s-t);
  h[6-1] = 4*r*s;
  h[5-1] = 4*r*(1-r-s-t);
  
  h[4-1] = t - 0.5*(h[8-1] + h[9-1] + h[10-1]);
  h[3-1] = s - 0.5*(h[6-1] + h[7-1] + h[9-1]);
  h[2-1] = r - 0.5*(h[5-1] + h[6-1] + h[10-1]);
  h[1-1] = (1-r-s-t) - 0.5*(h[5-1] + h[7-1] + h[8-1]);
  
  return h[j];
*/  
  switch (j) {
    case 0:
      return (1-r-s-t)*(2*(1-r-s-t)-1);
      break;
    case 1:
      return r*(2*r-1);
      break;
    case 2:
      return s*(2*s-1);
      break;
    case 3:
      return t*(2*t-1);
      break;
      
    case 4:
      return 4*(1-r-s-t)*r;
      break;
    case 5:
      return 4*r*s;
      break;
    case 6:
      return 4*s*(1-r-s-t);
      break;
    case 7:
      return 4*(1-r-s-t)*t;
      break;
    case 8:
      return 4*s*t;
      break;
    case 9:
      return 4*r*t;
      break;
      
  }

  return 0;

}

double mesh_ten_node_tetrahedron_dhdr(int j, int m, gsl_vector *gsl_r) {
  double r;
  double s;
  double t;

  r = gsl_vector_get(gsl_r, 0);
  s = gsl_vector_get(gsl_r, 1);
  t = gsl_vector_get(gsl_r, 2);
  
  switch (j) {
    case 0:
      switch(m) {
        case 0:
          return 1-4*(1-r-s-t);
        break;
        case 1:
          return 1-4*(1-r-s-t);
        break;
        case 2:
          return 1-4*(1-r-s-t);
        break;
      }
    break;
    case 1:
      switch(m) {
        case 0:
          return 4*r-1;
        break;
        case 1:
          return 0;
        break;
        case 2:
          return 0;
        break;
      }
    break;
    case 2:
      switch(m) {
        case 0:
          return 0;
        break;
        case 1:
          return 4*s-1;
        break;
        case 2:
          return 0;
        break;
      }
    break;
    case 3:
      switch(m) {
        case 0:
          return 0;
        break;
        case 1:
          return 0;
        break;
        case 2:
          return 4*t-1;
        break;
      }

    case 4:
      switch(m) {
        case 0:
          return -4*r+4*(1-r-s-t);
        break;
        case 1:
          return -4*r;
        break;
        case 2:
          return -4*r;
        break;
      }
    case 5:
      switch(m) {
        case 0:
          return 4*s;
        break;
        case 1:
          return 4*r;
        break;
        case 2:
          return 0;
        break;
      }
    case 6:
      switch(m) {
        case 0:
          return -4*s;
        break;
        case 1:
          return -4*s+4*(1-r-s-t);
        break;
        case 2:
          return -4*s;
        break;
      }
    case 7:
      switch(m) {
        case 0:
          return -4*t;
        break;
        case 1:
          return -4*t;
        break;
        case 2:
          return -4*t+4*(1-r-s-t);
        break;
      }
    case 8:
      switch(m) {
        case 0:
          return 0;
        break;
        case 1:
          return 4*t;
        break;
        case 2:
          return 4*s;
        break;
      }
    case 9:
      switch(m) {
        case 0:
          return 4*t;
        break;
        case 1:
          return 0;
        break;
        case 2:
          return 4*r;
        break;
      }

      break;
  }

  return 0;


}

