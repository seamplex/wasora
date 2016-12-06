/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related prism element routines
 *
 *  Copyright (C) 2015--2016 jeremy theler & ezequiel manavela chiapero
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
#include <gsl/gsl_linalg.h>


// --------------------------------------------------------------
// prisma de seis nodos
// --------------------------------------------------------------

int mesh_six_node_prism_init(void) {

  element_type_t *element_type;
  gauss_t *gauss;

  element_type = &wasora_mesh.element_type[ELEMENT_TYPE_PRISM];
  element_type->name = strdup("prism");
  element_type->id = ELEMENT_TYPE_PRISM;
  element_type->dim = 3;
  element_type->nodes = 6;
  element_type->faces = 5;
  element_type->nodes_per_face = 4;   // Ojo aca que en nodos por cara pusimos el maximo valor (4) ya que depende de la cara
  element_type->h = mesh_six_node_prism_h;
  element_type->dhdr = mesh_six_node_prism_dhdr;
  element_type->point_in_element = mesh_point_in_prism;
  element_type->element_volume = mesh_prism_vol;

  // tres juegos de puntos de gauss
  element_type->gauss = calloc(3, sizeof(gauss_t));
  
  // el primero es el default
  // ---- seis puntos de Gauss sobre el elemento unitario ----  
    gauss = &element_type->gauss[GAUSS_POINTS_CANONICAL];
    mesh_alloc_gauss(gauss, element_type, 6);
  
    gauss->w[0] = 0.5 * 1.0/6.0;
    gauss->r[0][0] = 1.0/6.0;
    gauss->r[0][1] = 1.0/6.0;
    gauss->r[0][2] = 1.0/6.0;
  
    gauss->w[1] = 0.5 * 1.0/6.0;
    gauss->r[1][0] = 2.0/3.0;
    gauss->r[1][1] = 1.0/6.0;
    gauss->r[1][2] = 1.0/6.0;
  
    gauss->w[2] = 0.5 * 1.0/6.0;
    gauss->r[2][0] = 1.0/6.0;
    gauss->r[2][1] = 2.0/3.0;
    gauss->r[2][2] = 1.0/6.0;

    gauss->w[3] = 0.5 * 1.0/6.0;
    gauss->r[3][0] = 1.0/6.0;
    gauss->r[3][1] = 1.0/6.0;
    gauss->r[3][2] = 2.0/3.0;
  
    gauss->w[4] = 0.5 * 1.0/6.0;
    gauss->r[4][0] = 2.0/3.0;
    gauss->r[4][1] = 1.0/6.0;
    gauss->r[4][2] = 2.0/3.0;
  
    gauss->w[5] = 0.5 * 1.0/6.0;
    gauss->r[5][0] = 1.0/6.0;
    gauss->r[5][1] = 2.0/3.0;
    gauss->r[5][2] = 2.0/3.0;
    
    mesh_init_shape_at_gauss(gauss, element_type);
    
  // ---- un punto de Gauss sobre el elemento unitario ----  
    gauss = &element_type->gauss[GAUSS_POINTS_SINGLE];
    mesh_alloc_gauss(gauss, element_type, 1);
  
    gauss->w[0] = 0.5 * 1.0;
    gauss->r[0][0] = 0;
    gauss->r[0][1] = 0;

    mesh_init_shape_at_gauss(gauss, element_type);  
    
  // ---- seis puntos de Gauss sobre el elemento unitario ----  
/*    
    gauss = &element_type->gauss[2];
    mesh_alloc_gauss(gauss, element_type, 6, "6-tensor");

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

    gauss->w[3] = 1.0;
    gauss->r[3][0] = -1.0/M_SQRT3;
    gauss->r[3][1] = -1.0/M_SQRT3;
    gauss->r[3][2] = +1.0/M_SQRT3;

    gauss->w[4] = 1.0;
    gauss->r[4][0] = +1.0/M_SQRT3;
    gauss->r[4][1] = -1.0/M_SQRT3;
    gauss->r[4][2] = +1.0/M_SQRT3;

    gauss->w[5] = 2.0;
    gauss->r[5][0] = 0;
    gauss->r[5][1] = +1.0/M_SQRT3;
    gauss->r[5][2] = +1.0/M_SQRT3;
  
    mesh_init_shape_at_gauss(gauss, element_type);
*/   
  
  return WASORA_RUNTIME_OK;
}

double mesh_six_node_prism_h(int j, gsl_vector *gsl_r) {
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
      return 1.0/8.0*(1-r)*(1-s)*(1+t);
      break;
    case 4:
      return 1.0/8.0*(1+r)*(1-s)*(1+t);
      break;
    case 5:
      return 1.0/4.0*(1+s)*(1+t);
      break;
  }

  return 0;

}

double mesh_six_node_prism_dhdr(int j, int m, gsl_vector *gsl_r) {
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
          return -1.0/8.0*(1-s)*(1+t);
        break;
        case 1:
          return -1.0/8.0*(1-r)*(1+t);
        break;
        case 2:
          return +1.0/8.0*(1-r)*(1-s);
        break;
      }
    break;
    case 4:
      switch(m) {
        case 0:
          return +1.0/8.0*(1-s)*(1+t);
        break;
        case 1:
          return -1.0/8.0*(1+r)*(1+t);
        break;
        case 2:
          return +1.0/8.0*(1+r)*(1-s);
        break;
      }
    break;
    case 5:
      switch(m) {
        case 0:
          return 0;
        break;
        case 1:
          return +1.0/4.0*(1+t);
        break;
        case 2:
          return +1.0/4.0*(1+s);
        break;
      }
    break;
  }

  return 0;


}


// TODO: generalizar a prismas no paralelos
int mesh_point_in_prism(element_t *element, const double *x) {
// // metodo de coordenadas baricentricas
////  http://en.wikipedia.org/wiki/Barycentric_coordinate_system  
//  double lambda1, lambda2, lambda3;
//  double x1 = element->node[0]->x[0];
//  double x2 = element->node[1]->x[0];
//  double x3 = element->node[2]->x[0];
//  double y1 = element->node[0]->x[1];
//  double y2 = element->node[1]->x[1];
//  double y3 = element->node[2]->x[1];
//  double z1 = element->node[0]->x[2];
//  double z4 = element->node[3]->x[2];
//    double zero, one;
//  
//  lambda1 = ((y2-y3)*(x[0]-x3) + (x3-x2)*(x[1]-y3))/((y2-y3)*(x1-x3) + (x3-x2)*(y1-y3));
//  lambda2 = ((y3-y1)*(x[0]-x3) + (x1-x3)*(x[1]-y3))/((y2-y3)*(x1-x3) + (x3-x2)*(y1-y3));
//  lambda3 = 1 - lambda1 - lambda2;
//  
//  zero = -wasora_var(wasora_mesh.vars.eps);
//  one = 1+wasora_var(wasora_mesh.vars.eps);
//  
//  return (lambda1 > zero && lambda1 < one &&
//          lambda2 > zero && lambda2 < one &&
//          lambda3 > zero && lambda3 < one &&
//          (x[2]+1e-4)  > z1 && (x[2]-1e-4) < z4);
//
//}

    //here we define 3 tetrahedrons which compose the original prism
    //These tetrahedrons are (0,1,2,3), (4,3,5,1), (2,3,5,1)
  double zero, one, lambda1, lambda2, lambda3, lambda4;
  gsl_vector *tetra_aux_index = gsl_vector_alloc(4);
  gsl_matrix *T = gsl_matrix_alloc(3, 3);
  gsl_vector *xx4 = gsl_vector_alloc(3);
  gsl_vector *lambda = gsl_vector_alloc(3);
  gsl_permutation *p = gsl_permutation_alloc(3);
  int s, flag;
  int i, j, jj, jj_end;
  
  
  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      gsl_matrix_set(T, i, j, element->node[j]->x[i] - element->node[3]->x[i]);
    }
    gsl_vector_set(xx4, i, x[i] - element->node[3]->x[i]);
  }
  gsl_linalg_LU_decomp (T, p, &s);
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
  
  if (flag == 0){
    gsl_vector_set(tetra_aux_index, 0, 4);
    gsl_vector_set(tetra_aux_index, 1, 3);
    gsl_vector_set(tetra_aux_index, 2, 5);
    gsl_vector_set(tetra_aux_index, 3, 1);
    jj_end = gsl_vector_get(tetra_aux_index, 3);
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            jj = gsl_vector_get(tetra_aux_index, j);
            gsl_matrix_set(T, i, j, element->node[jj]->x[i] - element->node[jj_end]->x[i]);
        }
        gsl_vector_set(xx4, i, x[i] - element->node[jj_end]->x[i]);
      }
      gsl_linalg_LU_decomp (T, p, &s);
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

  }
  
  if (flag == 0){
    gsl_vector_set(tetra_aux_index, 0, 2);
    gsl_vector_set(tetra_aux_index, 1, 3);
    gsl_vector_set(tetra_aux_index, 2, 5);
    gsl_vector_set(tetra_aux_index, 3, 1);
    jj_end = gsl_vector_get(tetra_aux_index, 3);
   for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
          jj = gsl_vector_get(tetra_aux_index, j);
          gsl_matrix_set(T, i, j, element->node[jj]->x[i] - element->node[jj_end]->x[i]);
        }
        gsl_vector_set(xx4, i, x[i] - element->node[jj_end]->x[i]);
      }
      gsl_linalg_LU_decomp (T, p, &s);
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

  }
  
  
  gsl_matrix_free(T);
  gsl_vector_free(xx4);
  gsl_vector_free(lambda);
  gsl_permutation_free(p);
  
  return flag;
}
    
// TODO: generalizar a prismas no paralelos
double mesh_prism_vol(element_t *element) {

//  return 0.5 * fabs(element->node[0]->x[2]-element->node[3]->x[2])* fabs(mesh_subtract_cross_2d(element->node[0]->x, element->node[1]->x, element->node[2]->x));

  double a[3], b[3], c[3];
  double v1, v2, v3;
  
  mesh_subtract(element->node[0]->x, element->node[1]->x, a);
  mesh_subtract(element->node[0]->x, element->node[2]->x, b);
  mesh_subtract(element->node[0]->x, element->node[3]->x, c);
  v1 = fabs(mesh_cross_dot(a, b, c));
  
  mesh_subtract(element->node[4]->x, element->node[3]->x, a);
  mesh_subtract(element->node[4]->x, element->node[5]->x, b);
  mesh_subtract(element->node[4]->x, element->node[1]->x, c);
  v2 = fabs(mesh_cross_dot(a, b, c));
  
  mesh_subtract(element->node[2]->x, element->node[3]->x, a);
  mesh_subtract(element->node[2]->x, element->node[5]->x, b);
  mesh_subtract(element->node[2]->x, element->node[1]->x, c);
  v3 = fabs(mesh_cross_dot(a, b, c));
  
  return 1.0/(1.0*2.0*3.0) * (v1+v2+v3);

}
