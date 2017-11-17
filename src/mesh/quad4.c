/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related quadrangle element routines
 *
 *  Copyright (C) 2014--2017 jeremy theler
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
// cuadrangulo de cuatro nodos
// --------------------------------------------------------------
int mesh_four_node_quadrangle_init(void) {
  
  element_type_t *element_type;
  gauss_t *gauss;
  
  element_type = &wasora_mesh.element_type[ELEMENT_TYPE_QUADRANGLE];
  element_type->name = strdup("quadrangle");
  element_type->id = ELEMENT_TYPE_QUADRANGLE;
  element_type->dim = 2;
  element_type->order = 1;
  element_type->nodes = 4;
  element_type->faces = 4;
  element_type->nodes_per_face = 2;
  element_type->h = mesh_four_node_quad_h;
  element_type->dhdr = mesh_four_node_quad_dhdr;
  element_type->point_in_element = mesh_point_in_quadrangle;
  element_type->element_volume = mesh_quad_vol;

  // dos juegos de puntos de gauss
  element_type->gauss = calloc(2, sizeof(gauss_t));
  
  // el primero es el default
  // ---- cuatro puntos de Gauss ----  
    gauss = &element_type->gauss[GAUSS_POINTS_CANONICAL];
    mesh_alloc_gauss(gauss, element_type, 4);
  
    gauss->w[0] = 4 * 0.25;
    gauss->r[0][0] = -1.0/M_SQRT3;
    gauss->r[0][1] = -1.0/M_SQRT3;

    gauss->w[1] = 4 * 0.25;
    gauss->r[1][0] = +1.0/M_SQRT3;
    gauss->r[1][1] = -1.0/M_SQRT3;
 
    gauss->w[2] = 4 * 0.25;
    gauss->r[2][0] = +1.0/M_SQRT3;
    gauss->r[2][1] = +1.0/M_SQRT3;

    gauss->w[3] = 4 * 0.25;
    gauss->r[3][0] = -1.0/M_SQRT3;
    gauss->r[3][1] = +1.0/M_SQRT3;

    mesh_init_shape_at_gauss(gauss, element_type);
    
  // ---- un punto de Gauss  ----  
    gauss = &element_type->gauss[GAUSS_POINTS_SINGLE];
    mesh_alloc_gauss(gauss, element_type, 1);
  
    gauss->w[0] = 4 * 1.0;
    gauss->r[0][0] = 0.0;
    gauss->r[0][1] = 0.0;

    mesh_init_shape_at_gauss(gauss, element_type);  

  return WASORA_RUNTIME_OK;    
}

double mesh_four_node_quad_h(int j, gsl_vector *gsl_r) {
  double r;
  double s;

  r = gsl_vector_get(gsl_r, 0);
  s = gsl_vector_get(gsl_r, 1);

  switch (j) {
    case 0:
      return 0.25*(1-r)*(1-s);
      break;
    case 1:
      return 0.25*(1+r)*(1-s);
      break;
    case 2:
      return 0.25*(1+r)*(1+s);
      break;
    case 3:
      return 0.25*(1-r)*(1+s);
      break;
  }

  return 0;

}

double mesh_four_node_quad_dhdr(int j, int m, gsl_vector *gsl_r) {
  double r;
  double s;

  r = gsl_vector_get(gsl_r, 0);
  s = gsl_vector_get(gsl_r, 1);

  switch(j) {
    case 0:
      if (m == 0) {
        return -0.25*(1-s);
      } else {
        return -0.25*(1-r);
      }
      break;
    case 1:
      if (m == 0) {
        return 0.25*(1-s);
      } else {
        return -0.25*(1+r);
      }
      break;
    case 2:
      if (m == 0) {
        return 0.25*(1+s);
      } else {
        return 0.25*(1+r);
      }
      break;
    case 3:
      if (m == 0) {
        return -0.25*(1+s);
      } else {
        return 0.25*(1-r);
      }
      break;

  }

  return 0;


}



int mesh_point_in_quadrangle(element_t *element, const double *x) {

/*
  double z1, z2, z3;

  z1 = mesh_subtract_cross_2d(element->node[0]->x, element->node[1]->x, x);
  z2 = mesh_subtract_cross_2d(element->node[1]->x, element->node[2]->x, x);
  z3 = mesh_subtract_cross_2d(element->node[2]->x, element->node[0]->x, x);
  
  if ((GSL_SIGN(z1) == GSL_SIGN(z2) && GSL_SIGN(z2) == GSL_SIGN(z3)) ||
      (fabs(z1) < 1e-4 && GSL_SIGN(z2) == GSL_SIGN(z3)) ||
      (fabs(z2) < 1e-4 && GSL_SIGN(z1) == GSL_SIGN(z3)) ||          
      (fabs(z3) < 1e-4 && GSL_SIGN(z1) == GSL_SIGN(z2)) ) {
    return 1;
  }
  
  
  z1 = mesh_subtract_cross_2d(element->node[0]->x, element->node[2]->x, x);
  z2 = mesh_subtract_cross_2d(element->node[2]->x, element->node[3]->x, x);
  z3 = mesh_subtract_cross_2d(element->node[3]->x, element->node[0]->x, x);

  if ((GSL_SIGN(z1) == GSL_SIGN(z2) && GSL_SIGN(z2) == GSL_SIGN(z3)) ||
      (fabs(z1) < 1e-4 && GSL_SIGN(z2) == GSL_SIGN(z3)) ||
      (fabs(z2) < 1e-4 && GSL_SIGN(z1) == GSL_SIGN(z3)) ||          
      (fabs(z3) < 1e-4 && GSL_SIGN(z1) == GSL_SIGN(z2)) ) {
    return 1;
  }
  
  return 0;
*/

  int i, j;

  element_t triang;

  triang.type = &wasora_mesh.element_type[ELEMENT_TYPE_TRIANGLE];
  triang.node = calloc(triang.type->nodes, sizeof(node_t *));

  for (i = 0; i < element->type->faces; i++) {
    for (j = 0; j < triang.type->faces; j++) {
      triang.node[j] = element->node[(i+j) % element->type->faces];
    }
    if (mesh_point_in_triangle(&triang, x)) {
      free(triang.node);
      return 1;
    }
  }

  free(triang.node);

  return 0;
}

double mesh_quad_vol(element_t *element) {
  
  return 0.5*(fabs(mesh_subtract_cross_2d(element->node[0]->x, element->node[1]->x, element->node[2]->x)) +
              fabs(mesh_subtract_cross_2d(element->node[2]->x, element->node[3]->x, element->node[0]->x)) );
}

