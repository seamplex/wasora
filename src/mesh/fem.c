/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related FEM routines
 *
 *  Copyright (C) 2014--2020 jeremy theler
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


// evalua el jacobiano de las r con respecto a las x invirtiendo el jacobiano directo
void mesh_inverse(gsl_matrix *direct, gsl_matrix *inverse) {

  double invdet;

  switch(direct->size1) {
    case 1:
    	gsl_matrix_set(inverse, 0, 0, 1.0/gsl_matrix_get(direct, 0, 0));
      break;
    case 2:
      invdet = 1.0/mesh_determinant(direct);
      gsl_matrix_set(inverse, 0, 0, +invdet*gsl_matrix_get(direct, 1, 1));
      gsl_matrix_set(inverse, 0, 1, -invdet*gsl_matrix_get(direct, 0, 1));
      gsl_matrix_set(inverse, 1, 0, -invdet*gsl_matrix_get(direct, 1, 0));
      gsl_matrix_set(inverse, 1, 1,  invdet*gsl_matrix_get(direct, 0, 0));
      break;
    case 3:
      invdet = 1.0/mesh_determinant(direct);
      gsl_matrix_set(inverse, 0, 0, +invdet*(gsl_matrix_get(direct, 2, 2)*gsl_matrix_get(direct, 1, 1) -
                                             gsl_matrix_get(direct, 2, 1)*gsl_matrix_get(direct, 1, 2)));
      gsl_matrix_set(inverse, 0, 1, -invdet*(gsl_matrix_get(direct, 2, 2)*gsl_matrix_get(direct, 0, 1) -
                                             gsl_matrix_get(direct, 2, 1)*gsl_matrix_get(direct, 0, 2)));
      gsl_matrix_set(inverse, 0, 2, +invdet*(gsl_matrix_get(direct, 1, 2)*gsl_matrix_get(direct, 0, 1) -
                                             gsl_matrix_get(direct, 1, 1)*gsl_matrix_get(direct, 0, 2)));
      
      gsl_matrix_set(inverse, 1, 0, -invdet*(gsl_matrix_get(direct, 2, 2)*gsl_matrix_get(direct, 1, 0) -
                                             gsl_matrix_get(direct, 2, 0)*gsl_matrix_get(direct, 1, 2)));
      gsl_matrix_set(inverse, 1, 1, +invdet*(gsl_matrix_get(direct, 2, 2)*gsl_matrix_get(direct, 0, 0) -
                                             gsl_matrix_get(direct, 2, 0)*gsl_matrix_get(direct, 0, 2)));
      gsl_matrix_set(inverse, 1, 2, -invdet*(gsl_matrix_get(direct, 1, 2)*gsl_matrix_get(direct, 0, 0) -
                                             gsl_matrix_get(direct, 1, 0)*gsl_matrix_get(direct, 0, 2)));

      gsl_matrix_set(inverse, 2, 0, +invdet*(gsl_matrix_get(direct, 2, 1)*gsl_matrix_get(direct, 1, 0) -
                                             gsl_matrix_get(direct, 2, 0)*gsl_matrix_get(direct, 1, 1)));
      gsl_matrix_set(inverse, 2, 1, -invdet*(gsl_matrix_get(direct, 2, 1)*gsl_matrix_get(direct, 0, 0) -
                                             gsl_matrix_get(direct, 2, 0)*gsl_matrix_get(direct, 0, 1)));
      gsl_matrix_set(inverse, 2, 2, +invdet*(gsl_matrix_get(direct, 1, 1)*gsl_matrix_get(direct, 0, 0) -
                                             gsl_matrix_get(direct, 1, 0)*gsl_matrix_get(direct, 0, 1)));
      break;
  }

/*  
static void matrix_inverse_3x3(PetscScalar A[3][3],PetscScalar B[3][3])
{
  PetscScalar t4, t6, t8, t10, t12, t14, t17;

  t4  = A[2][0] * A[0][1];
  t6  = A[2][0] * A[0][2];
  t8  = A[1][0] * A[0][1];
  t10 = A[1][0] * A[0][2];
  t12 = A[0][0] * A[1][1];
  t14 = A[0][0] * A[1][2];
  t17 = 0.1e1 / (t4 * A[1][2] - t6 * A[1][1] - t8 * A[2][2] + t10 * A[2][1] + t12 * A[2][2] - t14 * A[2][1]);

  B[0][0] = (A[1][1] * A[2][2] - A[1][2] * A[2][1]) * t17;
  B[0][1] = -(A[0][1] * A[2][2] - A[0][2] * A[2][1]) * t17;
  B[0][2] = (A[0][1] * A[1][2] - A[0][2] * A[1][1]) * t17;
  B[1][0] = -(-A[2][0] * A[1][2] + A[1][0] * A[2][2]) * t17;
  B[1][1] = (-t6 + A[0][0] * A[2][2]) * t17;
  B[1][2] = -(-t10 + t14) * t17;
  B[2][0] = (-A[2][0] * A[1][1] + A[1][0] * A[2][1]) * t17;
  B[2][1] = -(-t4 + A[0][0] * A[2][1]) * t17;
  B[2][2] = (-t8 + t12) * t17;
}
  
  return;
*/
}


// calcula el determinante de una matriz de 2x2 //
/*
The inverse of a 2x2 matrix:

| a11 a12 |-1             |  a22 -a12 |
| a21 a22 |    =  1/DET * | -a21  a11 |

with DET  =  a11a22-a12a21

The inverse of a 3x3 matrix:

| a11 a12 a13 |-1             |   a33a22-a32a23  -(a33a12-a32a13)   a23a12-a22a13  |
| a21 a22 a23 |    =  1/DET * | -(a33a21-a31a23)   a33a11-a31a13  -(a23a11-a21a13) |
| a31 a32 a33 |               |   a32a21-a31a22  -(a32a11-a31a12)   a22a11-a21a12  |

with DET  =  a11(a33a22-a32a23)-a21(a33a12-a32a13)+a31(a23a12-a22a13)
*/

//}

// calcula el determinante de una matriz //
double mesh_determinant(gsl_matrix *A) {

  switch(A->size1) {
    case 0:
      return 1.0;
    break;
    case 1:
      return gsl_matrix_get(A, 0, 0);
    break;
    case 2:
      return + gsl_matrix_get(A, 0, 0)*gsl_matrix_get(A, 1, 1)
             - gsl_matrix_get(A, 0, 1)*gsl_matrix_get(A, 1, 0);
    break;
    case 3:
      return + gsl_matrix_get(A, 0, 0)*gsl_matrix_get(A, 1, 1)*gsl_matrix_get(A, 2, 2)
             + gsl_matrix_get(A, 0, 1)*gsl_matrix_get(A, 1, 2)*gsl_matrix_get(A, 2, 0)
             + gsl_matrix_get(A, 0, 2)*gsl_matrix_get(A, 1, 0)*gsl_matrix_get(A, 2, 1) 
             - gsl_matrix_get(A, 0, 2)*gsl_matrix_get(A, 1, 1)*gsl_matrix_get(A, 2, 0)
             - gsl_matrix_get(A, 0, 1)*gsl_matrix_get(A, 1, 0)*gsl_matrix_get(A, 2, 2) 
             - gsl_matrix_get(A, 0, 0)*gsl_matrix_get(A, 1, 2)*gsl_matrix_get(A, 2, 1);  
    break;
  }
  
  return 0.0;
}

// calcula los gradientes de las h con respecto a las x evaluadas en r
void mesh_compute_dhdx(element_t *element, double *r, gsl_matrix *drdx_ref, gsl_matrix *dhdx) {

  gsl_matrix *dxdr;
  gsl_matrix *drdx;
  int j, m, m_prime;
  
  if (drdx_ref != NULL) {
    // si ya nos dieron drdx usamos esa
    drdx = drdx_ref;
    
  } else {
    // sino la calculamos
    drdx = gsl_matrix_calloc(element->type->dim, element->type->dim);
    dxdr = gsl_matrix_calloc(element->type->dim, element->type->dim);
    
    mesh_compute_dxdr(element, r, dxdr);
    mesh_inverse(dxdr, drdx);
  }
  
  
  gsl_matrix_set_zero(dhdx);
  for (j = 0; j < element->type->nodes; j++) {
    for (m = 0; m < element->type->dim; m++) {
      for (m_prime = 0; m_prime < element->type->dim; m_prime++) {
        gsl_matrix_add_to_element(dhdx, j, m, element->type->dhdr(j, m_prime, r) * gsl_matrix_get(drdx, m_prime, m));
      }
    }
  }

  if (drdx_ref == NULL) {
    gsl_matrix_free(drdx);
    gsl_matrix_free(dxdr);
  }
  
  return;

}


// compute the gradient of the shape functions with respect to x evalauted at gauss point v of scheme integration
void mesh_compute_dhdx_at_gauss(element_t *element, int v, int integration) {

  int m, m_prime;    // dimensions
  int j;             // nodes
  int V_changed = 0;
  gsl_matrix *dhdx;
  
  
  if (element->dhdx == NULL || element->V_dhdx != element->type->gauss[integration].V) {
    int v_prime;
    for (v_prime = 0; v_prime < element->V_dhdx; v_prime++) {
      gsl_matrix_free(element->dhdx[v_prime]);
    }
    free(element->dhdx);

    element->V_dhdx = element->type->gauss[integration].V;
    element->dhdx = calloc(element->V_dhdx, sizeof(gsl_matrix *));
    V_changed = 1;
  }
  
  if (element->dhdx[v] == NULL) {
    element->dhdx[v] = gsl_matrix_calloc(element->type->nodes, element->type->dim);
  } else {
    return;
  }
  
  if (element->drdx == NULL || element->drdx[v] == NULL || V_changed) {
    mesh_compute_drdx_at_gauss(element, v, integration);
  }

  dhdx = element->dhdx[v];
  
  // TODO: matrix-matrix multiplication with blas?
  for (j = 0; j < element->type->nodes; j++) {
    for (m = 0; m < element->type->dim; m++) {
      for (m_prime = 0; m_prime < element->type->dim; m_prime++) {
      	gsl_matrix_add_to_element(dhdx, j, m, gsl_matrix_get(element->type->gauss[integration].dhdr[v], j, m_prime) * gsl_matrix_get(element->drdx[v], m_prime, m));
      }
    }
  }
  
#ifdef FEM_DUMP  
  printf("dhdx(%d,%d) = \n", element->index, v);
  gsl_matrix_fprintf(stdout, element->dhdx[v], "%g");  
#endif
  

  return;

}

void mesh_compute_h(element_t *element, double *r, double *h) {

  int j;

  for (j = 0; j < element->type->nodes; j++) {
    h[j] = element->type->h(j, r);
  }

  return;

}


void mesh_compute_drdx_at_gauss(element_t *element, int v, int integration) {
  
  int V_changed = 0;
  
  if (element->drdx == NULL || element->V_drdx != element->type->gauss[integration].V) {
    int v_prime;
    for (v_prime = 0; v_prime < element->V_drdx; v_prime++) {
      gsl_matrix_free(element->drdx[v_prime]);
    }
    free(element->drdx);

    element->V_drdx = element->type->gauss[integration].V;
    element->drdx = calloc(element->V_drdx, sizeof(gsl_matrix *));
    V_changed = 1;
  }
  if (element->drdx[v] == NULL) {
    element->drdx[v] = gsl_matrix_calloc(element->type->dim, element->type->dim);
  } else {
    return;
  }
  
  if (element->dxdr == NULL || element->dxdr[v] == NULL || V_changed) {
    mesh_compute_dxdr_at_gauss(element, v, integration);
  }
  
  mesh_inverse(element->dxdr[v], element->drdx[v]);

#ifdef FEM_DUMP  
  printf("drdx(%d,%d) = \n", element->index, v);
  gsl_matrix_fprintf(stdout, element->drdx[v], "%g");  
#endif
  
  return;
  
}



void mesh_compute_dxdr(element_t *element, double *r, gsl_matrix *dxdr) {

  int m, m_prime, j;
  
  // OJO! esto solo camina en elementos volumetricos, ver dxdr_at_gauss
  for (m = 0; m < element->type->dim; m++) {
    for (m_prime = 0; m_prime < element->type->dim; m_prime++) {
      for (j = 0; j < element->type->nodes; j++) {
        gsl_matrix_add_to_element(dxdr, m, m_prime, element->type->dhdr(j, m_prime, r) * element->node[j]->x[m]);
      }
    }
  }
  
  return;
}


void mesh_compute_x(element_t *element, double *r, double *x) {

  int j, m;

  
  // solo para elementos volumetricos
  for (m = 0; m < 3; m++) {
    x[m] = 0;
    for (j = 0; j < element->type->nodes; j++) {
      x[m] += element->type->h(j, r) * element->node[j]->x[m];
    }
  }

  return;
}

/*
void mesh_inverse(int, gsl_matrix *dxdr, gsl_matrix *drdx) {
  return;
}
*/
//void mesh_compute_dhdx(element, fino.mesh->fem.r, fino.mesh->fem.drdx, fino.mesh->fem.dhdx);

double mesh_integration_weight(mesh_t *mesh, element_t *element, int v) {
  return 0;
}


void mesh_compute_integration_weight_at_gauss(element_t *element, int v, int integration) {
  
  if (element->w == NULL) {
    element->w = calloc(element->type->gauss[integration].V, sizeof(double));
  }
  
  if (element->dxdr == NULL || element->dxdr[v] == NULL) {
    mesh_compute_dxdr_at_gauss(element, v, integration);
  }
  
  element->w[v] = element->type->gauss[integration].w[v] * fabs(mesh_determinant(element->dxdr[v]));
#ifdef FEM_DUMP  
  printf("w(%d,%d) = %g\n", element->index, v, element->w[v]);
#endif

  return;
}



void mesh_compute_dxdr_at_gauss(element_t *element, int v, int integration) {

  int m, m_prime;
  int j;
//  int V_changed = 0;
  double *r;
  gsl_matrix *dxdr;
  
  if (element->dxdr == NULL || element->V_dxdr != element->type->gauss[integration].V) {
    int v_prime;
    for (v_prime = 0; v_prime < element->V_dxdr; v_prime++) {
      gsl_matrix_free(element->dxdr[v_prime]);
    }
    free(element->dxdr);

    element->V_dxdr = element->type->gauss[integration].V;
    element->dxdr = calloc(element->V_dxdr, sizeof(gsl_matrix *));
//    V_changed = 1;
  }
  
  if (element->dxdr[v] == NULL) {
    element->dxdr[v] = gsl_matrix_calloc(element->type->dim, element->type->dim);
  } else {
    return;
  }

  dxdr = element->dxdr[v];
  r = element->type->gauss[integration].r[v];

  if (element->type->dim == 0) {
    // a point does not have any derivative, if we are here then just keep silence
    ;
  } else if (element->type->dim == 1 && (element->node[0]->x[1] != 0 || element->node[1]->x[1] != 0 ||
                                         element->node[0]->x[2] != 0 || element->node[1]->x[2] != 0)) {
    // if we are a line but not aligned with the x axis we have to compute the axial coordinate l
    double dx, dy, dz, l;
    
    dx = element->node[1]->x[0] - element->node[0]->x[0];
    dy = element->node[1]->x[1] - element->node[0]->x[1];
    dz = element->node[1]->x[2] - element->node[0]->x[2];
    l = gsl_hypot3(dx, dy, dz);

    for (j = 0; j < element->type->nodes; j++) {
      gsl_matrix_add_to_element(dxdr, 0, 0, element->type->dhdr(j, 0, r) * (element->node[j]->x[0] * dx/l +
                                                                            element->node[j]->x[1] * dy/l +
                                                                            element->node[j]->x[2] * dz/l));
    }
    
  } else if (element->type->dim == 2 && (element->node[0]->x[2] != 0 || element->node[1]->x[2] != 0 || element->node[2]->x[2])) {
    // if we are a triangle or a quadrangle (quadrangles are double-triangles so they are alike)
    // but we do not live on the x-y plane we have to do some tricks
    double n[3];
    double xy[3];
    double a[3];
    double b[3];
    double v[3];
    double R[3][3];
    double s, c, k;
    
    // http://math.stackexchange.com/questions/180418/calculate-rotation-matrix-to-align-vector-a-to-vector-b-in-3d
    mesh_subtract(element->node[0]->x, element->node[1]->x, a);
    mesh_subtract(element->node[0]->x, element->node[2]->x, b);
    mesh_normalized_cross(a, b, n);
    xy[0] = 0;
    xy[1] = 0;
    xy[2] = 1;

    mesh_cross(n, xy, v);
    s = gsl_hypot3(v[0], v[1], v[2]);
    c = mesh_dot(n, xy);
    k = (1-c)/(s*s);

/*
 (%i4) V:matrix([0, -v2, v1],[v2, 0, -v0],[-v1, v0, 0]);
                             [  0    - v2   v1  ]
                             [                  ]
(%o4)                        [  v2    0    - v0 ]
                             [                  ]
                             [ - v1   v0    0   ]
(%i5) V . V;
                   [     2     2                           ]
                   [ - v2  - v1      v0 v1        v0 v2    ]
                   [                                       ]
(%o5)              [                  2     2              ]
                   [    v0 v1     - v2  - v0      v1 v2    ]
                   [                                       ]
                   [                               2     2 ]
                   [    v0 v2        v1 v2     - v1  - v0  ]
*/    

    if (fabs(s) < wasora_var(wasora_mesh.vars.eps)) {
      // too late, but the transformation is a translation
      R[0][0] = 1;
      R[0][1] = 0;
      R[0][2] = 0;
      R[1][0] = 0;
      R[1][1] = 1;
      R[1][2] = 0;
      R[2][0] = 0;
      R[2][1] = 0;
      R[2][2] = 1;
    } else {
      R[0][0] = 1     + k * (-v[2]*v[2] - v[1]*v[1]);
      R[0][1] = -v[2] + k * (v[0]*v[1]);
      R[0][2] = +v[1] + k * (v[0]*v[2]);
      R[1][0] = +v[2] + k * (v[0]*v[1]);
      R[1][1] = 1     + k * (-v[2]*v[2] - v[0]*v[0]);
      R[1][2] = -v[0] + k * (v[1]*v[2]);
      R[2][0] = -v[1] + k * (v[0]*v[2]);
      R[2][1] = +v[0] + k * (v[1]*v[2]);
      R[2][2] = 1     + k * (-v[1]*v[1] - v[0]*v[0]);
    }
    
    for (m = 0; m < 2; m++) {
      for (j = 0; j < element->type->nodes; j++) {
        for (m_prime = 0; m_prime < 2; m_prime++) {
          gsl_matrix_add_to_element(dxdr, m, m_prime, element->type->dhdr(j, m, r) * (element->node[j]->x[0] * R[m_prime][0] +
                                                                                      element->node[j]->x[1] * R[m_prime][1] +
                                                                                      element->node[j]->x[2] * R[m_prime][2]));
        }
      }
    }
    
  } else {
    // we can do a full traditional computation
    // i.e. lines are in the x axis
    //      surfaces are on the xy plane
    //      volumes are always volumes!
  
    for (m = 0; m < element->type->dim; m++) {
      for (m_prime = 0; m_prime < element->type->dim; m_prime++) {
        for (j = 0; j < element->type->nodes; j++) {
          // TODO: matrix-vector product
          gsl_matrix_add_to_element(dxdr, m, m_prime, gsl_matrix_get(element->type->gauss->dhdr[v], j, m_prime) * element->node[j]->x[m]);
        }
      }
    }
    
  } 

#ifdef FEM_DUMP  
  printf("dxdr(%d,%d) = \n", element->index, v);
  gsl_matrix_fprintf(stdout, element->dxdr[v], "%g");  
#endif
  
  return;
}


void mesh_compute_x_at_gauss(element_t *element, int v, int integration) {

  int j, m;
  
  if (element->x == NULL) {
    element->x = calloc(element->type->gauss[integration].V, sizeof(double *));
  }
  if (element->x[v] == NULL) {
    element->x[v] = calloc(3, sizeof(double));
  } else {
    return;
  }
  
  for (j = 0; j < element->type->nodes; j++) {
    for (m = 0; m < 3; m++) {
      element->x[v][m] += element->type->gauss[integration].h[v][j] * element->node[j]->x[m];
    }
  }
  
#ifdef FEM_DUMP  
  printf("x(%d,%d) = %g %g %g\n", element->index, v, element->x[v][0], element->x[v][1], element->x[v][2]);
#endif

  return;  
}

int mesh_compute_r(element_t *element, gsl_vector *x, gsl_vector *r) {

  int j, j_prime;
  
  
  if (element->type->id == ELEMENT_TYPE_TETRAHEDRON4 || element->type->id == ELEMENT_TYPE_TETRAHEDRON10) {
//    double xi0, one;
    double sixV;
//    double sixV01;
    double sixV02, sixV03, sixV04;

    // porque ya teniamos todo desde 1    
    double dx[5][5];
    double dy[5][5];
    double dz[5][5];
    for (j = 0; j < 4; j++) {
      for (j_prime = 0; j_prime < 4; j_prime++) {
        dx[j+1][j_prime+1] = element->node[j]->x[0] - element->node[j_prime]->x[0];
        dy[j+1][j_prime+1] = element->node[j]->x[1] - element->node[j_prime]->x[1];
        dz[j+1][j_prime+1] = element->node[j]->x[2] - element->node[j_prime]->x[2];
      }
    }
  
    // arrancan en uno
    sixV = dx[2][1] * (dy[2][3] * dz[3][4] - dy[3][4] * dz[2][3] ) + dx[3][2] * (dy[3][4] * dz[1][2] - dy[1][2] * dz[3][4] ) + dx[4][3] * (dy[1][2] * dz[2][3] - dy[2][3] * dz[1][2]);
 
    // estos si arrancan en cero
//    sixV01 = element->node[1]->x[0] * (element->node[2]->x[1]*element->node[3]->x[2] - element->node[3]->x[1]*element->node[2]->x[2]) + element->node[2]->x[0] * (element->node[3]->x[1]*element->node[1]->x[2] - element->node[1]->x[1]*element->node[3]->x[2]) + element->node[3]->x[0] * (element->node[1]->x[1]*element->node[2]->x[2] - element->node[2]->x[1]*element->node[1]->x[2]);
    sixV02 = element->node[0]->x[0] * (element->node[3]->x[1]*element->node[2]->x[2] - element->node[2]->x[1]*element->node[3]->x[2]) + element->node[2]->x[0] * (element->node[0]->x[1]*element->node[3]->x[2] - element->node[3]->x[1]*element->node[0]->x[2]) + element->node[3]->x[0] * (element->node[2]->x[1]*element->node[0]->x[2] - element->node[0]->x[1]*element->node[2]->x[2]);
    sixV03 = element->node[0]->x[0] * (element->node[1]->x[1]*element->node[3]->x[2] - element->node[3]->x[1]*element->node[1]->x[2]) + element->node[1]->x[0] * (element->node[3]->x[1]*element->node[0]->x[2] - element->node[0]->x[1]*element->node[3]->x[2]) + element->node[3]->x[0] * (element->node[0]->x[1]*element->node[1]->x[2] - element->node[1]->x[1]*element->node[0]->x[2]);
    sixV04 = element->node[0]->x[0] * (element->node[2]->x[1]*element->node[1]->x[2] - element->node[1]->x[1]*element->node[2]->x[2]) + element->node[1]->x[0] * (element->node[0]->x[1]*element->node[2]->x[2] - element->node[2]->x[1]*element->node[0]->x[2]) + element->node[2]->x[0] * (element->node[1]->x[1]*element->node[0]->x[2] - element->node[0]->x[1]*element->node[1]->x[2]);
    
    // otra vez en uno
//    xi0 =                1.0/sixV * (sixV01 * 1 + (dy[4][2]*dz[3][2] - dy[3][2]*dz[4][2])*gsl_vector_get(x, 0) + (dx[3][2]*dz[4][2] - dx[4][2]*dz[3][2])*gsl_vector_get(x, 1) + (dx[4][2]*dy[3][2] - dx[3][2]*dy[4][2])*gsl_vector_get(x, 2));
    gsl_vector_set(r, 0, 1.0/sixV * (sixV02 * 1 + (dy[3][1]*dz[4][3] - dy[3][4]*dz[1][3])*gsl_vector_get(x, 0) + (dx[4][3]*dz[3][1] - dx[1][3]*dz[3][4])*gsl_vector_get(x, 1) + (dx[3][1]*dy[4][3] - dx[3][4]*dy[1][3])*gsl_vector_get(x, 2)));
    gsl_vector_set(r, 1, 1.0/sixV * (sixV03 * 1 + (dy[2][4]*dz[1][4] - dy[1][4]*dz[2][4])*gsl_vector_get(x, 0) + (dx[1][4]*dz[2][4] - dx[2][4]*dz[1][4])*gsl_vector_get(x, 1) + (dx[2][4]*dy[1][4] - dx[1][4]*dy[2][4])*gsl_vector_get(x, 2)));
    gsl_vector_set(r, 2, 1.0/sixV * (sixV04 * 1 + (dy[1][3]*dz[2][1] - dy[1][2]*dz[3][1])*gsl_vector_get(x, 0) + (dx[2][1]*dz[1][3] - dx[3][1]*dz[1][2])*gsl_vector_get(x, 1) + (dx[1][3]*dy[2][1] - dx[1][2]*dy[3][1])*gsl_vector_get(x, 2)));
    
/*    
    one = xi0 + gsl_vector_get(r,0) + gsl_vector_get(r,1) + gsl_vector_get(r,2);
    if (gsl_fcmp(one, 1.0, 1e-3) != 0) {
      printf("internal mismatch when computing canonical coordinates in element %d\n", element->id);
      return WASORA_RUNTIME_ERROR;
    }
 */
    
  } else {
    wasora_push_error_message("not for element type %d", element->type->id) ;
    return WASORA_RUNTIME_ERROR;
  }

  return WASORA_RUNTIME_OK;
}


void mesh_compute_H_at_gauss(element_t *element, int v, int dofs, int integration) {
  int j;
  int d;

  if (element->H == NULL) {
    element->H = calloc(element->type->gauss[integration].V, sizeof(gsl_matrix *));
  }
  if (element->H[v] == NULL) {
    element->H[v] = gsl_matrix_calloc(dofs, dofs*element->type->nodes);
  } else {
    return;
  }
  
  for (d = 0; d < dofs; d++) {
    for (j = 0; j < element->type->nodes; j++) {
      gsl_matrix_set(element->H[v], d, dofs*j+d, element->type->gauss[integration].h[v][j]);
    }
  }
  
#ifdef FEM_DUMP  
  printf("H(%d,%d) = \n", element->index, v);
  gsl_matrix_fprintf(stdout, element->H[v], "%g");  
#endif
  
  


  return;
  
}


void mesh_compute_B_at_gauss(element_t *element, int v, int dofs, int integration) {

  int m, d, j;  
  
  if (element->B == NULL) {
    element->B = calloc(element->type->gauss[integration].V, sizeof(gsl_matrix *));
  }
  if (element->B[v] == NULL) {
    element->B[v] = gsl_matrix_calloc(dofs*element->type->dim, dofs*element->type->nodes);
  } else {
    return;
  }
  
  if (element->dhdx == NULL || element->dhdx[v] == NULL) {
    mesh_compute_dhdx_at_gauss(element, v, integration);
  }
  
  
  for (m = 0; m < element->type->dim; m++) {
    for (d = 0; d < dofs; d++) {
      for (j = 0; j < element->type->nodes; j++) {
        gsl_matrix_set(element->B[v], dofs*m+d, dofs*j+d, gsl_matrix_get(element->dhdx[v], j, m));
      }
    }
  }
 
  return;
}

void mesh_compute_l(mesh_t *mesh, element_t *element) {
  int j, d;
  
  if (element->l == NULL) {
    element->l = calloc(element->type->nodes * mesh->degrees_of_freedom, sizeof(double));
  } else {
    return;
  }
  
  // the vector l contains the global indexes of each DOF in the element
  // note that this vector is always node major independently of the global orderin
  for (j = 0; j < element->type->nodes; j++) {
    for (d = 0; d < mesh->degrees_of_freedom; d++) {
      element->l[mesh->degrees_of_freedom*j + d] = element->node[j]->index_dof[d];
    }  
  }
  
  return;
  
}

inline int mesh_update_coord_vars(double *x) {
  wasora_var(wasora_mesh.vars.x) = x[0];
  wasora_var(wasora_mesh.vars.y) = x[1];
  wasora_var(wasora_mesh.vars.z) = x[2];    
  return WASORA_RUNTIME_OK;
}

inline int mesh_compute_normal(element_t *element) {
  double n[3];

  wasora_call(mesh_compute_outward_normal(element, n));
  wasora_var_value(wasora_mesh.vars.nx) = n[0];
  wasora_var_value(wasora_mesh.vars.ny) = n[1];
  wasora_var_value(wasora_mesh.vars.nz) = n[2];
  
  return 0;
}  

