/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related line3 element routines
 *
 *  Copyright (C) 2017--2020 jeremy theler
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
// linea de tres nodos
// --------------------------------------------------------------


int mesh_line3_init(void) {

  double r[1];
  element_type_t *element_type;
  int j, v;
  
  element_type = &wasora_mesh.element_type[ELEMENT_TYPE_LINE3];
  element_type->name = strdup("line3");
  element_type->id = ELEMENT_TYPE_LINE3;
  element_type->dim = 1;
  element_type->order = 2;
  element_type->nodes = 3;
  element_type->faces = 2;
  element_type->nodes_per_face = 1;
  element_type->h = mesh_line3_h;
  element_type->dhdr = mesh_line3_dhdr;
  element_type->point_in_element = mesh_point_in_line;
  element_type->element_volume = mesh_line_vol;

  // coordenadas de los nodos
/*
Line3:      
            
0-----2----1   
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
  
  wasora_mesh_add_node_parent(&element_type->node_parents[2], 0);
  wasora_mesh_add_node_parent(&element_type->node_parents[2], 1);
  wasora_mesh_compute_coords_from_parent(element_type, 2);

  // ------------
  // gauss points and extrapolation matrices
  
  // full integration: three points
  mesh_gauss_init_line3(element_type, &element_type->gauss[integration_full]);
  element_type->gauss[integration_full].extrap = gsl_matrix_calloc(element_type->nodes, 3);

  // reduced integration: two points
  mesh_gauss_init_line2(element_type, &element_type->gauss[integration_reduced]);
  element_type->gauss[integration_reduced].extrap = gsl_matrix_calloc(element_type->nodes, 2);
  
  
  for (j = 0; j < element_type->first_order_nodes; j++) {
    r[0] = M_SQRT3 * element_type->node_coords[j][0];

    
    for (v = 0; v < 2; v++) {
      // full: 3 points, use the corner nodes with the first-order shape functions and average in the rest
      gsl_matrix_set(element_type->gauss[integration_full].extrap, j, v, mesh_line2_h(v, r));
    
      // reduced: 2 gauss points
      gsl_matrix_set(element_type->gauss[integration_reduced].extrap, j, v, mesh_line2_h(v, r));
    }
  }
  
  // average on the high-order of the full one
  gsl_matrix_set(element_type->gauss[integration_full].extrap, 2, 0, 0.5);
  gsl_matrix_set(element_type->gauss[integration_full].extrap, 2, 1, 0.5);
  
  
  return WASORA_RUNTIME_OK;
}

double mesh_line3_h(int k, double *vec_r) {
  double r = vec_r[0];

  // Gmsh ordering
  switch (k) {
    case 0:
      return 0.5*r*(r+1);
      break;
    case 1:
      return 0.5*r*(r-1);
      break;
    case 2:
      return (1+r)*(1-r);
      break;
  }

  return 0;

}

double mesh_line3_dhdr(int k, int m, double *vec_r) {
  double r = vec_r[0];

  switch(k) {
    case 0:
      if (m == 0) {
        return r+0.5;
      }
      break;
    case 1:
      if (m == 0) {
        return r-0.5;
      }
      break;
    case 2:
      if (m == 0) {
        return -2*r;
      }
      break;
  }

  return 0;
}
