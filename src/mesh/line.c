/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related line element routines
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

#include <math.h>

// --------------------------------------------------------------
// linea de dos nodos
// --------------------------------------------------------------


int mesh_two_node_line_init(void) {

  element_type_t *element_type;
  gauss_t *gauss;
  int j;
  
  element_type = &wasora_mesh.element_type[ELEMENT_TYPE_LINE];
  element_type->name = strdup("line2");
  element_type->id = ELEMENT_TYPE_LINE;
  element_type->dim = 1;
  element_type->order = 1;
  element_type->nodes = 2;
  element_type->faces = 2;
  element_type->nodes_per_face = 1;
  element_type->h = mesh_two_node_line_h;
  element_type->dhdr = mesh_two_node_line_dhdr;
  element_type->point_in_element = mesh_point_in_line;
  element_type->element_volume = mesh_line_vol;

  // coordenadas de los nodos
/*
Line:              
                   
0----------1 --> u   
*/
  
  element_type->node_coords = calloc(element_type->nodes, sizeof(double *));
  element_type->node_parents = calloc(element_type->nodes, sizeof(node_relative_t *));
  for (j = 0; j < element_type->nodes; j++) {
    element_type->node_coords[j] = calloc(element_type->dim, sizeof(double));
  }

  element_type->first_order_nodes++;
  element_type->node_coords[0][0] = 0;

  element_type->first_order_nodes++;
  element_type->node_coords[1][0] = 1;
  
  // dos juegos de puntos de gauss
  element_type->gauss = calloc(2, sizeof(gauss_t));
  
  // el primero es constante
  // ---- un punto de Gauss ----  
    gauss = &element_type->gauss[GAUSS_POINTS_SINGLE];
    mesh_alloc_gauss(gauss, element_type, 1);
  
    gauss->w[0] = 2 * 1.0;
    gauss->r[0][0] = 0;

    mesh_init_shape_at_gauss(gauss, element_type);
    
  // ---- dos puntos de Gauss ----  
    gauss = &element_type->gauss[GAUSS_POINTS_CANONICAL];
    mesh_alloc_gauss(gauss, element_type, 2);
  
    gauss->w[0] = 2 * 0.5;
    gauss->r[0][0] = -1.0/M_SQRT3;

    gauss->w[1] = 2 * 0.5;
    gauss->r[1][0] = +1.0/M_SQRT3;

    mesh_init_shape_at_gauss(gauss, element_type);
    
  
  return WASORA_RUNTIME_OK;
}

double mesh_two_node_line_h(int k, double *vec_r) {
  double r = vec_r[0];

  // numeracion gmsh
  switch (k) {
    case 0:
      return 0.5*(1-r);
      break;
    case 1:
      return 0.5*(1+r);
      break;
  }

  return 0;

}

double mesh_two_node_line_dhdr(int k, int m, double *arg) {

  switch(k) {
    case 0:
      if (m == 0) {
        return -0.5;
      }
      break;
    case 1:
      if (m == 0) {
        return 0.5;
      }
      break;
  }

  return 0;
}



int mesh_point_in_line(element_t *element, const double *x) {
  return ((x[0] >= element->node[0]->x[0] && x[0] <= element->node[1]->x[0]) ||
          (x[0] >= element->node[1]->x[0] && x[0] <= element->node[0]->x[0]));
}


double mesh_line_vol(element_t *element) {
  return fabs(element->node[1]->x[0] - element->node[0]->x[0]);
}
