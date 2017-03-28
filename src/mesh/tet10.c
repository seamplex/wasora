/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related second-order tetrahedron element routines
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

#include <gsl/gsl_linalg.h>

// ---------------------------------------------------------------------
// tetrahedro isoparametrico de cuatro nodos sobre el triangulo unitario
// ---------------------------------------------------------------------

int mesh_ten_node_tetrahedron_init(void) {
  
  element_type_t *element_type;
  gauss_t *gauss;
  
  element_type = &wasora_mesh.element_type[ELEMENT_TYPE_TETRAHEDRON10];
  element_type->name = strdup("tetrahedron10");
  element_type->id = ELEMENT_TYPE_TETRAHEDRON10;
  element_type->dim = 3;
  element_type->nodes = 10;
  element_type->faces = 4;
  element_type->nodes_per_face = 6;
  element_type->h = mesh_ten_node_tetrahedron_h;
  element_type->dhdr = mesh_ten_node_tetrahedron_dhdr;
  element_type->point_in_element = mesh_point_in_tetrahedron;
  element_type->element_volume = mesh_tetrahedron_vol;
  
  // tres juegos de puntos de gauss
  element_type->gauss = calloc(3, sizeof(gauss_t));
  
  // el primero es el default
  // ---- cuatro puntos de Gauss sobre el elemento unitario ----  
    gauss = &element_type->gauss[GAUSS_POINTS_CANONICAL];
    mesh_alloc_gauss(gauss, element_type, 4);
  
    gauss->w[0] = 1.0/6.0 * 1.0/4.0;
    gauss->r[0][0] = 1.0/6.0;
    gauss->r[0][1] = 1.0/6.0;
    gauss->r[0][2] = 1.0/6.0;
  
    gauss->w[1] = 1.0/6.0 * 1.0/4.0;
    gauss->r[1][0] = 2.0/3.0;
    gauss->r[1][1] = 1.0/6.0;
    gauss->r[1][2] = 1.0/6.0;
  
    gauss->w[2] = 1.0/6.0 * 1.0/4.0;
    gauss->r[2][0] = 1.0/6.0;
    gauss->r[2][1] = 2.0/3.0;
    gauss->r[2][2] = 1.0/6.0;

    gauss->w[3] = 1.0/6.0 * 1.0/4.0;
    gauss->r[3][0] = 1.0/6.0;
    gauss->r[3][1] = 1.0/6.0;
    gauss->r[3][2] = 2.0/3.0;
    
    mesh_init_shape_at_gauss(gauss, element_type);
    
  // ---- un punto de Gauss sobre el elemento unitario ----  
    gauss = &element_type->gauss[GAUSS_POINTS_SINGLE];
    mesh_alloc_gauss(gauss, element_type, 1);
  
    gauss->w[0] = 1.0/6.0 * 1.0;
    gauss->r[0][0] = 1.0/3.0;
    gauss->r[0][1] = 1.0/3.0;
    gauss->r[0][2] = 1.0/3.0;

    mesh_init_shape_at_gauss(gauss, element_type);  

  return WASORA_RUNTIME_OK;
}

double mesh_ten_node_tetrahedron_h(int j, gsl_vector *gsl_r) {
  double r;
  double s;
  double t;

  r = gsl_vector_get(gsl_r, 0);
  s = gsl_vector_get(gsl_r, 1);
  t = gsl_vector_get(gsl_r, 2);

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
      return 4*t*(1-r-s-t);
      break;
    case 7:
      return 4*(1-r-s-t);
      break;
    case 8:
      return 4*r*t;
      break;
    case 9:
      return 4*s*t;
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
          return 0;
        break;
        case 1:
          return 0;
        break;
        case 2:
          return 4*t-1;
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
          return -4*t;
        break;
        case 1:
          return -4*t;
        break;
        case 2:
          return 4*(1-r-s-t) - 4*t;
        break;
      }
    case 7:
      switch(m) {
        case 0:
          return -4;
        break;
        case 1:
          return -4;
        break;
        case 2:
          return -4;
        break;
      }
    case 8:
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
    case 9:
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

      break;
  }

  return 0;


}

