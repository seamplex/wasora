/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related hexahedron element routines
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
#include <gsl/gsl_linalg.h>


// --------------------------------------------------------------
// hexahedro de ocho nodos
// --------------------------------------------------------------

int mesh_eight_node_hexahedron_init(void) {
  
  element_type_t *element_type;
  int j;

  element_type = &wasora_mesh.element_type[ELEMENT_TYPE_HEXAHEDRON];
  element_type->name = strdup("hex8");
  element_type->id = ELEMENT_TYPE_HEXAHEDRON;
  element_type->dim = 3;
  element_type->order = 1;
  element_type->nodes = 8;
  element_type->faces = 6;
  element_type->nodes_per_face = 4;
  element_type->h = mesh_eight_node_hexahedron_h;
  element_type->dhdr = mesh_eight_node_hexahedron_dhdr;
  element_type->point_in_element = mesh_point_in_hexahedron;
  element_type->element_volume = mesh_hexahedron_vol;

  // coordenadas de los nodos
/*
Hexahedron:           

       v
3----------2          
|\     ^   |\         
| \    |   | \        
|  \   |   |  \       
|   7------+---6      
|   |  +-- |-- | -> u 
0---+---\--1   |      
 \  |    \  \  |      
  \ |     \  \ |      
   \|      w  \|      
    4----------5      

*/     
  element_type->node_coords = calloc(element_type->nodes, sizeof(double *));
  element_type->node_parents = calloc(element_type->nodes, sizeof(node_relative_t *));
  for (j = 0; j < element_type->nodes; j++) {
    element_type->node_coords[j] = calloc(element_type->dim, sizeof(double));  
  }
  
  element_type->first_order_nodes++;  
  element_type->node_coords[0][0] = -1;
  element_type->node_coords[0][1] = -1;
  element_type->node_coords[0][2] = -1;

  element_type->first_order_nodes++;  
  element_type->node_coords[1][0] = +1;  
  element_type->node_coords[1][1] = -1;
  element_type->node_coords[1][2] = -1;
  
  element_type->first_order_nodes++;
  element_type->node_coords[2][0] = +1;  
  element_type->node_coords[2][1] = +1;
  element_type->node_coords[2][2] = -1;

  element_type->first_order_nodes++;
  element_type->node_coords[3][0] = -1;
  element_type->node_coords[3][1] = +1;
  element_type->node_coords[3][2] = -1;
 
  element_type->first_order_nodes++;
  element_type->node_coords[4][0] = -1;
  element_type->node_coords[4][1] = -1;
  element_type->node_coords[4][2] = +1;
  
  element_type->first_order_nodes++;
  element_type->node_coords[5][0] = +1;  
  element_type->node_coords[5][1] = -1;
  element_type->node_coords[5][2] = +1;
  
  element_type->first_order_nodes++;
  element_type->node_coords[6][0] = +1;  
  element_type->node_coords[6][1] = +1;
  element_type->node_coords[6][2] = +1;

  element_type->first_order_nodes++;
  element_type->node_coords[7][0] = -1;
  element_type->node_coords[7][1] = +1;
  element_type->node_coords[7][2] = +1;
  
  mesh_hexa_gauss8_init(element_type);
  
  return WASORA_RUNTIME_OK;
}

void mesh_hexa_gauss8_init(element_type_t *element_type) {

  gauss_t *gauss;
  
  // dos juegos de puntos de gauss
  element_type->gauss = calloc(2, sizeof(gauss_t));
  
  // el primero es el default
  // ---- ocho puntos de Gauss sobre el elemento unitario ----  
    gauss = &element_type->gauss[GAUSS_POINTS_CANONICAL];
    mesh_alloc_gauss(gauss, element_type, 8);

    gauss->w[0] = 8 * 1.0/8.0;
    gauss->r[0][0] = -1.0/M_SQRT3;
    gauss->r[0][1] = -1.0/M_SQRT3;
    gauss->r[0][2] = -1.0/M_SQRT3;

    gauss->w[1] = 8 * 1.0/8.0;
    gauss->r[1][0] = +1.0/M_SQRT3;
    gauss->r[1][1] = -1.0/M_SQRT3;
    gauss->r[1][2] = -1.0/M_SQRT3;

    gauss->w[2] = 8 * 1.0/8.0;
    gauss->r[2][0] = +1.0/M_SQRT3;
    gauss->r[2][1] = +1.0/M_SQRT3;
    gauss->r[2][2] = -1.0/M_SQRT3;

    gauss->w[3] = 8 * 1.0/8.0;
    gauss->r[3][0] = -1.0/M_SQRT3;
    gauss->r[3][1] = +1.0/M_SQRT3;
    gauss->r[3][2] = -1.0/M_SQRT3;

    gauss->w[4] = 8 * 1.0/8.0;
    gauss->r[4][0] = -1.0/M_SQRT3;
    gauss->r[4][1] = -1.0/M_SQRT3;
    gauss->r[4][2] = +1.0/M_SQRT3;

    gauss->w[5] = 8 * 1.0/8.0;
    gauss->r[5][0] = +1.0/M_SQRT3;
    gauss->r[5][1] = -1.0/M_SQRT3;
    gauss->r[5][2] = +1.0/M_SQRT3;

    gauss->w[6] = 8 * 1.0/8.0;
    gauss->r[6][0] = +1.0/M_SQRT3;
    gauss->r[6][1] = +1.0/M_SQRT3;
    gauss->r[6][2] = +1.0/M_SQRT3;

    gauss->w[7] = 8 * 1.0/8.0;
    gauss->r[7][0] = -1.0/M_SQRT3;
    gauss->r[7][1] = +1.0/M_SQRT3;
    gauss->r[7][2] = +1.0/M_SQRT3;
  
    mesh_init_shape_at_gauss(gauss, element_type);
    
  // ---- un punto de Gauss sobre el elemento unitario ----  
    gauss = &element_type->gauss[GAUSS_POINTS_SINGLE];
    mesh_alloc_gauss(gauss, element_type, 1);
  
    gauss->w[0] = 8 * 1.0;
    gauss->r[0][0] = 0;
    gauss->r[0][1] = 0;

    mesh_init_shape_at_gauss(gauss, element_type);  
  
  return;
}


double mesh_eight_node_hexahedron_h(int j, double *vec_r) {
  double r = vec_r[0];
  double s = vec_r[1];
  double t = vec_r[2];

  switch (j) {
    case 0:
      return 1.0/8.0*(1-r)*(1-s)*(1-t);
      break;
    case 1:
      return 1.0/8.0*(1+r)*(1-s)*(1-t);
      break;
    case 2:
      return 1.0/8.0*(1+r)*(1+s)*(1-t);
      break;
    case 3:
      return 1.0/8.0*(1-r)*(1+s)*(1-t);
      break;
    case 4:
      return 1.0/8.0*(1-r)*(1-s)*(1+t);
      break;
    case 5:
      return 1.0/8.0*(1+r)*(1-s)*(1+t);
      break;
    case 6:
      return 1.0/8.0*(1+r)*(1+s)*(1+t);
      break;
    case 7:
      return 1.0/8.0*(1-r)*(1+s)*(1+t);
      break;
  }

  return 0;

}

double mesh_eight_node_hexahedron_dhdr(int j, int m, double *vec_r) {
  double r = vec_r[0];
  double s = vec_r[1];
  double t = vec_r[2];

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
          return +1.0/8.0*(1+s)*(1-t);
        break;
        case 1:
          return +1.0/8.0*(1+r)*(1-t);
        break;
        case 2:
          return -1.0/8.0*(1+r)*(1+s);
        break;
      }
    break;
    case 3:
      switch(m) {
        case 0:
          return -1.0/8.0*(1+s)*(1-t);
        break;
        case 1:
          return +1.0/8.0*(1-r)*(1-t);
        break;
        case 2:
          return -1.0/8.0*(1-r)*(1+s);
        break;
      }
    break;
    case 4:
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
    case 5:
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
    case 6:
      switch(m) {
        case 0:
          return +1.0/8.0*(1+s)*(1+t);
        break;
        case 1:
          return +1.0/8.0*(1+r)*(1+t);
        break;
        case 2:
          return +1.0/8.0*(1+r)*(1+s);
        break;
      }
    break;
    case 7:
      switch(m) {
        case 0:
          return -1.0/8.0*(1+s)*(1+t);
        break;
        case 1:
          return +1.0/8.0*(1-r)*(1+t);
        break;
        case 2:
          return +1.0/8.0*(1-r)*(1+s);
        break;
      }
    break;
  }

  return 0;


}


int mesh_point_in_hexahedron(element_t *element, const double *x) {
  
  double zero, one, lambda1, lambda2, lambda3, lambda4;
  gsl_vector *tetra_aux_index = gsl_vector_alloc(4);
  gsl_matrix *T = gsl_matrix_alloc(3, 3);
  gsl_vector *xx4 = gsl_vector_alloc(3);
  gsl_vector *lambda = gsl_vector_alloc(3);
  gsl_permutation *p = gsl_permutation_alloc(3);
  int s, flag;
  int i, j, jj, jj_end;
  
  gsl_vector_set(tetra_aux_index, 0, 0);
  gsl_vector_set(tetra_aux_index, 1, 1);
  gsl_vector_set(tetra_aux_index, 2, 3);
  gsl_vector_set(tetra_aux_index, 3, 4);
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
  
  if (flag == 0){
    gsl_vector_set(tetra_aux_index, 0, 5);
    gsl_vector_set(tetra_aux_index, 1, 4);
    gsl_vector_set(tetra_aux_index, 2, 7);
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
    gsl_vector_set(tetra_aux_index, 0, 3);
    gsl_vector_set(tetra_aux_index, 1, 4);
    gsl_vector_set(tetra_aux_index, 2, 7);
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
    gsl_vector_set(tetra_aux_index, 0, 3);
    gsl_vector_set(tetra_aux_index, 1, 1);
    gsl_vector_set(tetra_aux_index, 2, 2);
    gsl_vector_set(tetra_aux_index, 3, 7);
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
    gsl_vector_set(tetra_aux_index, 0, 5);
    gsl_vector_set(tetra_aux_index, 1, 7);
    gsl_vector_set(tetra_aux_index, 2, 6);
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
    gsl_vector_set(tetra_aux_index, 1, 7);
    gsl_vector_set(tetra_aux_index, 2, 6);
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
  

  gsl_vector_free(tetra_aux_index);
  gsl_matrix_free(T);
  gsl_vector_free(xx4);
  gsl_vector_free(lambda);
  gsl_permutation_free(p);
  
  return flag;




}

double mesh_hexahedron_vol(element_t *element) {

 double a[3], b[3], c[3];
 double v1, v2, v3, v4, v5, v6;
  
  mesh_subtract(element->node[0]->x, element->node[1]->x, a);
  mesh_subtract(element->node[0]->x, element->node[3]->x, b);
  mesh_subtract(element->node[0]->x, element->node[4]->x, c);
  v1 = fabs(mesh_cross_dot(a, b, c));
  
  mesh_subtract(element->node[5]->x, element->node[4]->x, a);
  mesh_subtract(element->node[5]->x, element->node[7]->x, b);
  mesh_subtract(element->node[5]->x, element->node[1]->x, c);
  v2 = fabs(mesh_cross_dot(a, b, c));
  
  mesh_subtract(element->node[3]->x, element->node[4]->x, a);
  mesh_subtract(element->node[3]->x, element->node[7]->x, b);
  mesh_subtract(element->node[3]->x, element->node[1]->x, c);
  v3 = fabs(mesh_cross_dot(a, b, c));
  
  mesh_subtract(element->node[3]->x, element->node[1]->x, a);
  mesh_subtract(element->node[3]->x, element->node[2]->x, b);
  mesh_subtract(element->node[3]->x, element->node[7]->x, c);
  v4 = fabs(mesh_cross_dot(a, b, c));
  
  mesh_subtract(element->node[5]->x, element->node[7]->x, a);
  mesh_subtract(element->node[5]->x, element->node[6]->x, b);
  mesh_subtract(element->node[5]->x, element->node[1]->x, c);
  v5 = fabs(mesh_cross_dot(a, b, c));
  
  mesh_subtract(element->node[2]->x, element->node[7]->x, a);
  mesh_subtract(element->node[2]->x, element->node[6]->x, b);
  mesh_subtract(element->node[2]->x, element->node[1]->x, c);
  v6 = fabs(mesh_cross_dot(a, b, c));
  
 return 1.0/(1.0*2.0*3.0) * (v1+v2+v3+v4+v5+v6);

}
