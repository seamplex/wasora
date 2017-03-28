/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related second-order triangle element routines
 *
 *  Copyright (C) 2017 jeremy theler
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

int mesh_six_node_triangle_init(void) {

  element_type_t *element_type;
  gauss_t *gauss;
  
  element_type = &wasora_mesh.element_type[ELEMENT_TYPE_TRIANGLE6];
  element_type->name = strdup("triangle6");
  element_type->id = ELEMENT_TYPE_TRIANGLE6;
  element_type->dim = 2;
  element_type->nodes = 6;
  element_type->faces = 3;
  element_type->nodes_per_face = 3;
  element_type->h = mesh_six_node_triang_h;
  element_type->dhdr = mesh_six_node_triang_dhdr;
  element_type->point_in_element = mesh_point_in_triangle;
  element_type->element_volume = mesh_triang_vol;

  // tres juegos de puntos de gauss
  element_type->gauss = calloc(3, sizeof(gauss_t));
  
  // ---- un punto de Gauss sobre el elemento unitario ----  
    gauss = &element_type->gauss[GAUSS_POINTS_SINGLE];
    mesh_alloc_gauss(gauss, element_type, 1);
  
    gauss->w[0] = 0.5 * 1.0;
    gauss->r[0][0] = 1.0/3.0;
    gauss->r[0][1] = 1.0/3.0;

    mesh_init_shape_at_gauss(gauss, element_type);  
    
  // ---- tres puntos de Gauss sobre el elemento unitario ----  
    gauss = &element_type->gauss[GAUSS_POINTS_CANONICAL];
    mesh_alloc_gauss(gauss, element_type, 3);
  
    gauss->w[0] = 1.0/2.0 * 1.0/3.0;
    gauss->r[0][0] = 1.0/6.0;
    gauss->r[0][1] = 1.0/6.0;
  
    gauss->w[1] = 1.0/2.0 * 1.0/3.0;
    gauss->r[1][0] = 2.0/3.0;
    gauss->r[1][1] = 1.0/6.0;
  
    gauss->w[2] = 1.0/2.0 * 1.0/3.0;
    gauss->r[2][0] = 1.0/6.0;
    gauss->r[2][1] = 2.0/3.0;

    mesh_init_shape_at_gauss(gauss, element_type);
  
  return WASORA_RUNTIME_OK;
}

double mesh_six_node_triang_h(int j, gsl_vector *gsl_r) {
  double r;
  double s;

  r = gsl_vector_get(gsl_r, 0);
  s = gsl_vector_get(gsl_r, 1);

  switch (j) {
    case 0:
      return (1-r-s)*(2*(1-r-s)-1);
      break;
    case 1:
      return r*(2*r-1);
      break;
    case 2:
      return s*(2*s-1);
      break;
      
    case 3:
      return 4*(1-r-s)*r;
      break;
    case 4:
      return 4*r*s;
      break;
    case 5:
      return 4*s*(1-r-s);
      break;
  }

  return 0;

}

double mesh_six_node_triang_dhdr(int j, int m, gsl_vector *gsl_r) {
  double r;
  double s;

  r = gsl_vector_get(gsl_r, 0);
  s = gsl_vector_get(gsl_r, 1);

  switch(j) {
    case 0:
      if (m == 0) {
        return 1-4*(1-r-s);
      } else {
        return 1-4*(1-r-s);
      }
      break;
    case 1:
      if (m == 0) {
        return 4*r-1;
      } else {
        return 0;
      }
      break;
    case 2:
      if (m == 0) {
        return 0;
      } else {
        return 4*s-1;
      }
      break;
      
    case 3:
      if (m == 0) {
        return 4*(1-r-s)-4*r;
      } else {
        return -4*r;
      }
      break;
    case 4:
      if (m == 0) {
        return 4*s;
      } else {
        return 4*r;
      }
      break;
    case 5:
      if (m == 0) {
        return -4*s;
      } else {
        return 4*(1-r-s)-4*s;
      }
      break;

  }

  return 0;


}
