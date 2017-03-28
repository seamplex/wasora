/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related tetrahedron element routines
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

#include <gsl/gsl_linalg.h>


// --------------------------------------------------------------
// tetrahedro isoparametrico de cuatro nodos
// pag 366-365 de bathe (con diferente numeracion)
// --------------------------------------------------------------
/*
double mesh_four_node_tetrahedron_h(int j, gsl_vector *gsl_r) {
  double r;
  double s;
  double t;

  r = gsl_vector_get(gsl_r, 0);
  s = gsl_vector_get(gsl_r, 1);
  t = gsl_vector_get(gsl_r, 2);

  switch (j) {
    case 0:
      return 1.0/8.0*(1-r)*(1-s)*(1-t);
      break;
    case 1:
      return 1.0/8.0*(1+r)*(1-s)*(1-t);
      break;
    case 2:
      return 1.0/4.0*(1+s)*(1-t);
      break;
    case 3:
      return 1.0/2.0*(1+t);
      break;
  }

  return 0;

}
*/

// ---------------------------------------------------------------------
// tetrahedro isoparametrico de cuatro nodos sobre el triangulo unitario
// ---------------------------------------------------------------------

int mesh_four_node_tetrahedron_init(void) {
  
  element_type_t *element_type;
  gauss_t *gauss;
  
  element_type = &wasora_mesh.element_type[ELEMENT_TYPE_TETRAHEDRON];
  element_type->name = strdup("tetrahedron");
  element_type->id = ELEMENT_TYPE_TETRAHEDRON;
  element_type->dim = 3;
  element_type->nodes = 4;
  element_type->faces = 4;
  element_type->nodes_per_face = 3;
  element_type->h = mesh_four_node_tetrahedron_h;
  element_type->dhdr = mesh_four_node_tetrahedron_dhdr;
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
  
  // ---- tres puntos de gauss producto tensorial  ----  
/*    
    gauss = &element_type->gauss[2];
    mesh_alloc_gauss(gauss, element_type, 3, "3-tensor");
  
    gauss->w[0] = 1.0;
    gauss->r[0][0] = -1.0/M_SQRT3;
    gauss->r[0][1] = -1.0/M_SQRT3;
    gauss->r[0][2] = -1.0/M_SQRT3;

    gauss->w[1] = 1.0;
    gauss->r[1][0] = +1.0/M_SQRT3;
    gauss->r[1][1] = -1.0/M_SQRT3;
    gauss->r[1][2] = -1.0/M_SQRT3;

    gauss->w[2] = 2.0;
    gauss->r[2][0] = 0;
    gauss->r[2][1] = +1.0/M_SQRT3;
    gauss->r[2][2] = -1.0/M_SQRT3;

    gauss->w[3] = 4.0;
    gauss->r[3][0] = 0/M_SQRT3;
    gauss->r[3][1] = 0/M_SQRT3;
    gauss->r[3][2] = +1.0/M_SQRT3;
  
    mesh_init_shape_at_gauss(gauss, element_type);  
*/

  return WASORA_RUNTIME_OK;
}

double mesh_four_node_tetrahedron_h(int j, gsl_vector *gsl_r) {
  double r;
  double s;
  double t;

  r = gsl_vector_get(gsl_r, 0);
  s = gsl_vector_get(gsl_r, 1);
  t = gsl_vector_get(gsl_r, 2);

  switch (j) {
    case 0:
      return 1-r-s-t;
      break;
    case 1:
      return r;
      break;
    case 2:
      return s;
      break;
    case 3:
      return t;
      break;
  }

  return 0;

}

/*
double mesh_four_node_tetrahedron_dhdr(int j, int m, gsl_vector *gsl_r) {
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
          return -1.0/8.0*(1-s)*(1-t);
        break;
        case 1:
          return -1.0/8.0*(1-r)*(1-t);
        break;
        case 2:
          return -1.0/8.0*(1-r)*(1-s);
        break;
      }
    break;
    case 1:
      switch(m) {
        case 0:
          return +1.0/8.0*(1-s)*(1-t);
        break;
        case 1:
          return -1.0/8.0*(1+r)*(1-t);
        break;
        case 2:
          return -1.0/8.0*(1+r)*(1-s);
        break;
      }
    break;
    case 2:
      switch(m) {
        case 0:
          return 0;
        break;
        case 1:
          return +1.0/4.0*(1-t);
        break;
        case 2:
          return -1.0/4.0*(1+s);
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
          return +1.0/2.0;
        break;
      }
    break;
  }

  return 0;


}
*/

double mesh_four_node_tetrahedron_dhdr(int j, int m, gsl_vector *gsl_r) {
/*  
  double r;
  double s;
  double t;

  r = gsl_vector_get(gsl_r, 0);
  s = gsl_vector_get(gsl_r, 1);
  t = gsl_vector_get(gsl_r, 2);
*/
  
  switch (j) {
    case 0:
      switch(m) {
        case 0:
          return -1;
        break;
        case 1:
          return -1;
        break;
        case 2:
          return -1;
        break;
      }
    break;
    case 1:
      switch(m) {
        case 0:
          return +1;
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
          return +1;
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
          return +1;
        break;
      }
    break;
  }

  return 0;


}




int mesh_point_in_tetrahedron(element_t *element, const double *x) {

// http://en.wikipedia.org/wiki/Barycentric_coordinate_system  
  double zero, one, lambda1, lambda2, lambda3, lambda4;
  double xi;
  gsl_matrix *T = gsl_matrix_alloc(3, 3);
  gsl_vector *xx4 = gsl_vector_alloc(3);
  gsl_vector *lambda = gsl_vector_alloc(3);
  gsl_permutation *p = gsl_permutation_alloc(3);
  int s, flag;
  int i, j;
  
  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      gsl_matrix_set(T, i, j, element->node[j]->x[i] - element->node[3]->x[i]);
    }
    gsl_vector_set(xx4, i, x[i] - element->node[3]->x[i]);
  }
  gsl_linalg_LU_decomp (T, p, &s);
  if ((xi = fabs(gsl_linalg_LU_det (T, s))) < 1e-20) {
    wasora_push_error_message("element %d is degenerate", element->id);
    wasora_runtime_error();
  }
  gsl_linalg_LU_solve (T, p, xx4, lambda);

  zero = -wasora_var(wasora_mesh.vars.eps);
  one = 1+wasora_var(wasora_mesh.vars.eps);
  lambda1 = gsl_vector_get(lambda, 0);
  lambda2 = gsl_vector_get(lambda, 1);
  lambda3 = gsl_vector_get(lambda, 2);
  lambda4 = 1 - gsl_vector_get(lambda, 0) - gsl_vector_get(lambda, 1) - gsl_vector_get(lambda, 2);
  
  flag = (lambda1 > zero && lambda1 < one &&
          lambda2 > zero && lambda2 < one &&
          lambda3 > zero && lambda3 < one &&
          lambda4 > zero && lambda4 < one);

  
  gsl_matrix_free(T);
  gsl_vector_free(xx4);
  gsl_vector_free(lambda);
  gsl_permutation_free(p);
  
  return flag;
}

double mesh_tetrahedron_vol(element_t *element) {

  double a[3], b[3], c[3];
  
  mesh_subtract(element->node[0]->x, element->node[1]->x, a);
  mesh_subtract(element->node[0]->x, element->node[2]->x, b);
  mesh_subtract(element->node[0]->x, element->node[3]->x, c);
  
  return 1.0/(1.0*2.0*3.0) * fabs(mesh_cross_dot(c, a, b));

// AFEM.Ch09.pdf
// 6V = J = x 21 (y 23 z 34 − y34 z 23 ) + x32 (y34 z 12 − y12 z34 ) + x 43 (y12 z23 − y23 z 12),
  
 
}
