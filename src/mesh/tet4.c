/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related tetrahedron element routines
 *
 *  Copyright (C) 2015--2018 jeremy theler
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

int mesh_four_node_tetrahedron_init(void) {
  
  element_type_t *element_type;
  int j;
  
  element_type = &wasora_mesh.element_type[ELEMENT_TYPE_TETRAHEDRON];
  element_type->name = strdup("tet4");
  element_type->id = ELEMENT_TYPE_TETRAHEDRON;
  element_type->dim = 3;
  element_type->order = 1;
  element_type->nodes = 4;
  element_type->faces = 4;
  element_type->nodes_per_face = 3;
  element_type->h = mesh_four_node_tetrahedron_h;
  element_type->dhdr = mesh_four_node_tetrahedron_dhdr;
  element_type->point_in_element = mesh_point_in_tetrahedron;
  element_type->element_volume = mesh_tetrahedron_vol;


  // coordenadas de los nodos
/*
Tetrahedron:                     

                   v
                 .
               ,/
              /
           2                     
         ,/|`\                   
       ,/  |  `\                 
     ,/    '.   `\               
   ,/       |     `\             
 ,/         |       `\           
0-----------'.--------1 --> u    
 `\.         |      ,/           
    `\.      |    ,/             
       `\.   '. ,/               
          `\. |/                 
             `3                  
                `\.
                   ` w

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
  
  mesh_tetrahedron_gauss_init(element_type);

  return WASORA_RUNTIME_OK;
}

void mesh_tetrahedron_gauss_init(element_type_t *element_type) {
  double a, b, c, d;
  double r[3];
  gauss_t *gauss;

  element_type->gauss = calloc(2, sizeof(gauss_t));
  
  // el primero es el default
  // ---- cuatro puntos de Gauss sobre el elemento unitario ----  
    gauss = &element_type->gauss[GAUSS_POINTS_CANONICAL];
    mesh_alloc_gauss(gauss, element_type, 4);
    
    a = (5.0-sqrt(5))/20.0;
    b = (5.0+3.0*sqrt(5))/20.0;
    
    gauss->w[0] = 1.0/6.0 * 1.0/4.0;
    gauss->r[0][0] = a;
    gauss->r[0][1] = a;
    gauss->r[0][2] = a;
  
    gauss->w[1] = 1.0/6.0 * 1.0/4.0;
    gauss->r[1][0] = b;
    gauss->r[1][1] = a;
    gauss->r[1][2] = a;
 
    gauss->w[2] = 1.0/6.0 * 1.0/4.0;
    gauss->r[2][0] = a;
    gauss->r[2][1] = b;
    gauss->r[2][2] = a;
    
    gauss->w[3] = 1.0/6.0 * 1.0/4.0;
    gauss->r[3][0] = a;
    gauss->r[3][1] = a;
    gauss->r[3][2] = b;
    
    
    mesh_init_shape_at_gauss(gauss, element_type);
    
    // matriz de extrapolacion
    gauss->extrap = gsl_matrix_alloc(gauss->V, gauss->V);
    
    c = -a/(b-a);
    d = 1+(1-b)/(b-a);
    
    r[0] = c;
    r[1] = c;
    r[2] = c;
    gsl_matrix_set(gauss->extrap, 0, 0, mesh_four_node_tetrahedron_h(0, r));
    gsl_matrix_set(gauss->extrap, 0, 1, mesh_four_node_tetrahedron_h(1, r));
    gsl_matrix_set(gauss->extrap, 0, 2, mesh_four_node_tetrahedron_h(2, r));
    gsl_matrix_set(gauss->extrap, 0, 3, mesh_four_node_tetrahedron_h(3, r));

    r[0] = d;
    r[1] = c;
    r[2] = c;
    gsl_matrix_set(gauss->extrap, 1, 0, mesh_four_node_tetrahedron_h(0, r));
    gsl_matrix_set(gauss->extrap, 1, 1, mesh_four_node_tetrahedron_h(1, r));
    gsl_matrix_set(gauss->extrap, 1, 2, mesh_four_node_tetrahedron_h(2, r));
    gsl_matrix_set(gauss->extrap, 1, 3, mesh_four_node_tetrahedron_h(3, r));

    r[0] = c;
    r[1] = d;
    r[2] = c;
    gsl_matrix_set(gauss->extrap, 2, 0, mesh_four_node_tetrahedron_h(0, r));
    gsl_matrix_set(gauss->extrap, 2, 1, mesh_four_node_tetrahedron_h(1, r));
    gsl_matrix_set(gauss->extrap, 2, 2, mesh_four_node_tetrahedron_h(2, r));
    gsl_matrix_set(gauss->extrap, 2, 3, mesh_four_node_tetrahedron_h(3, r));

    r[0] = c;
    r[1] = c;
    r[2] = d;
    gsl_matrix_set(gauss->extrap, 3, 0, mesh_four_node_tetrahedron_h(0, r));
    gsl_matrix_set(gauss->extrap, 3, 1, mesh_four_node_tetrahedron_h(1, r));
    gsl_matrix_set(gauss->extrap, 3, 2, mesh_four_node_tetrahedron_h(2, r));
    gsl_matrix_set(gauss->extrap, 3, 3, mesh_four_node_tetrahedron_h(3, r));    
    
    
  // ---- un punto de Gauss sobre el elemento unitario ----  
    gauss = &element_type->gauss[GAUSS_POINTS_SINGLE];
    mesh_alloc_gauss(gauss, element_type, 1);
  
    gauss->w[0] = 1.0/6.0 * 1.0;
    gauss->r[0][0] = 1.0/4.0;
    gauss->r[0][1] = 1.0/4.0;
    gauss->r[0][2] = 1.0/4.0;

    mesh_init_shape_at_gauss(gauss, element_type);  
    
  return;
}

double mesh_four_node_tetrahedron_h(int j, double *vec_r) {
  double r = vec_r[0];
  double s = vec_r[1];
  double t = vec_r[2];

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


double mesh_four_node_tetrahedron_dhdr(int j, int m, double *vec_r) {
  
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
//    wasora_push_error_message("element %d is degenerate", element->tag);
//    wasora_runtime_error();
    return 0;
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
  
  if (element->volume == 0) {
    mesh_subtract(element->node[0]->x, element->node[1]->x, a);
    mesh_subtract(element->node[0]->x, element->node[2]->x, b);
    mesh_subtract(element->node[0]->x, element->node[3]->x, c);
  
    element->volume = 1.0/(1.0*2.0*3.0) * fabs(mesh_cross_dot(c, a, b));
  }  
  
  return element->volume;

// AFEM.Ch09.pdf
// 6V = J = x 21 (y 23 z 34 − y34 z 23 ) + x32 (y34 z 12 − y12 z34 ) + x 43 (y12 z23 − y23 z 12),
  
 
}
