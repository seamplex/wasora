/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related triangle element routines
 *
 *  Copyright (C) 2014-2016 jeremy theler
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


int mesh_three_node_triangle_init(void) {

  element_type_t *element_type;
  gauss_t *gauss;
  
  element_type = &wasora_mesh.element_type[ELEMENT_TYPE_TRIANGLE];
  element_type->name = strdup("triangle");
  element_type->id = ELEMENT_TYPE_TRIANGLE;
  element_type->dim = 2;
  element_type->order = 1;
  element_type->nodes = 3;
  element_type->faces = 3;
  element_type->nodes_per_face = 2;
  element_type->h = mesh_three_node_triang_h;
  element_type->dhdr = mesh_three_node_triang_dhdr;
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

double mesh_three_node_triang_h(int j, gsl_vector *gsl_r) {
  double r;
  double s;

  r = gsl_vector_get(gsl_r, 0);
  s = gsl_vector_get(gsl_r, 1);

  switch (j) {
    case 0:
      return 1-r-s;
      break;
    case 1:
      return r;
      break;
    case 2:
      return s;
      break;
  }

  return 0;

}

double mesh_three_node_triang_dhdr(int j, int m, gsl_vector *gsl_r) {
  switch(j) {
    case 0:
      if (m == 0) {
        return -1;
      } else {
        return -1;
      }
      break;
    case 1:
      if (m == 0) {
        return 1;
      } else {
        return 0;
      }
      break;
    case 2:
      if (m == 0) {
        return 0;
      } else {
        return 1;
      }
      break;
  }

  return 0;

}


int mesh_point_in_triangle(element_t *element, const double *x) {

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
  return 0;
*/
  
// metodo de coordenadas baricentricas
//  http://en.wikipedia.org/wiki/Barycentric_coordinate_system  
  double lambda1, lambda2, lambda3;
  double x1 = element->node[0]->x[0];
  double x2 = element->node[1]->x[0];
  double x3 = element->node[2]->x[0];
  double y1 = element->node[0]->x[1];
  double y2 = element->node[1]->x[1];
  double y3 = element->node[2]->x[1];
  double zero, one;
  
  lambda1 = ((y2-y3)*(x[0]-x3) + (x3-x2)*(x[1]-y3))/((y2-y3)*(x1-x3) + (x3-x2)*(y1-y3));
  lambda2 = ((y3-y1)*(x[0]-x3) + (x1-x3)*(x[1]-y3))/((y2-y3)*(x1-x3) + (x3-x2)*(y1-y3));
  lambda3 = 1 - lambda1 - lambda2;
  
  zero = -wasora_var(wasora_mesh.vars.eps);
  one = 1+wasora_var(wasora_mesh.vars.eps);
  
  return (lambda1 > zero && lambda1 < one &&
          lambda2 > zero && lambda2 < one &&
          lambda3 > zero && lambda3 < one);

}


double mesh_triang_vol(element_t *element) {
  return 0.5 * fabs(mesh_subtract_cross_2d(element->node[0]->x, element->node[1]->x, element->node[2]->x));
}
