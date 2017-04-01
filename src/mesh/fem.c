/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related FEM routines
 *
 *  Copyright (C) 2014--2016 jeremy theler
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

#include <gsl/gsl_math.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>


// evalua el jacobiano de las r con respecto a las x invirtiendo el jacobiano directo
void mesh_inverse(int dim, gsl_matrix *direct, gsl_matrix *inverse) {

  double invdet;

  switch(dim) {
    case 1:
    	gsl_matrix_set(inverse, 0, 0, 1.0/gsl_matrix_get(direct, 0, 0));
      break;
    case 2:
      invdet = 1.0/mesh_determinant(2, direct);
      gsl_matrix_set(inverse, 0, 0, +invdet*gsl_matrix_get(direct, 1, 1));
      gsl_matrix_set(inverse, 0, 1, -invdet*gsl_matrix_get(direct, 0, 1));
      gsl_matrix_set(inverse, 1, 0, -invdet*gsl_matrix_get(direct, 1, 0));
      gsl_matrix_set(inverse, 1, 1,  invdet*gsl_matrix_get(direct, 0, 0));
      break;
    case 3:
      invdet = 1.0/mesh_determinant(3, direct);
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
// OJO! pedimos el tamanio explicitamente porque puede darse el caso
// que A sea mas grande de lo debido y solo interese una partecita
double mesh_determinant(int d, gsl_matrix *A) {

  switch(d) {
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
void mesh_compute_dhdx(element_t *element, gsl_vector *r, gsl_matrix *drdx, gsl_matrix *dhdx) {

  int j;
  int m, m_prime;

  gsl_matrix_set_zero(dhdx);
  for (j = 0; j < element->type->nodes; j++) {
    for (m = 0; m < element->type->dim; m++) {
      for (m_prime = 0; m_prime < element->type->dim; m_prime++) {
      	gsl_matrix_add_to_element(dhdx, j, m, element->type->dhdr(j, m_prime, r) * gsl_matrix_get(drdx, m_prime, m));
      }
    }
  }

  return;

}

void mesh_compute_h(element_t *element, gsl_vector *r, gsl_vector *h) {

  int j;

  for (j = 0; j < element->type->nodes; j++) {
    gsl_vector_set(h, j, element->type->h(j, r));
  }

  return;

}

void mesh_compute_dxdr(element_t *element, gsl_vector *r, gsl_matrix *dxdr) {

  int m, m_prime;
  int j;

  gsl_matrix_set_zero(dxdr);
  
  if (element->type->dim == 0) {
    // un puntito
    gsl_matrix_set(dxdr, 0, 0, 1.0);
  } else if (element->type->dim == 1 && (element->node[0]->x[1] != 0 || element->node[1]->x[1] != 0 ||
                                         element->node[0]->x[2] != 0 || element->node[1]->x[2] != 0)) {
    
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
    
    // los cuadrangulos son triangulos dobles asi  que con mirar un triangulo ya esta
  } else if (element->type->dim == 2 && (element->node[0]->x[2] != 0 || element->node[1]->x[2] != 0 || element->node[2]->x[2])) {

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
      // too late, pero la transformacion es una traslacion
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
    // la transformacion mantiene el plano original
    // i.e. los segmentos estan en la recta x
    //      las superficies estan en el plano xy    
  
    for (m = 0; m < element->type->dim; m++) {
      for (m_prime = 0; m_prime < element->type->dim; m_prime++) {
        for (j = 0; j < element->type->nodes; j++) {
          gsl_matrix_add_to_element(dxdr, m, m_prime, element->type->dhdr(j, m_prime, r) * element->node[j]->x[m]);
        }
      }
    }
    
  } 

  return;
}


void mesh_compute_x(element_t *element, gsl_vector *r, gsl_vector *x) {

  int j, m;

  gsl_vector_set_zero(x);
  for (j = 0; j < element->type->nodes; j++) {
    for (m = 0; m < 3; m++) {
      gsl_vector_add_to_element(x, m, element->type->h(j, r) * element->node[j]->x[m]);
    }
  }

  return;
}

int mesh_compute_r(element_t *element, gsl_vector *x, gsl_vector *r) {

  int j, m;
  
  if (element->type->id != ELEMENT_TYPE_TETRAHEDRON &&  element->type->id != ELEMENT_TYPE_TETRAHEDRON10) {
    wasora_push_error_message("not for element type %d", element->type->id) ;
    return WASORA_RUNTIME_ERROR;
  }

/*
 * TO BE DONE! eq 9.11 de AFEM.Ch09  
  gsl_vector_set_zero(x);
  for (j = 0; j < element->type->nodes; j++) {
    for (m = 0; m < element->type->dim; m++) {
      gsl_vector_add_to_element(x, m, element->type->h(j, r) * element->node[j]->x[m]);
    }
  }
*/

  return WASORA_RUNTIME_OK;
}

int mesh_compute_H(mesh_t *mesh, element_t *element) {
  int j;
  int d;
  
  for (d = 0; d < mesh->degrees_of_freedom; d++) {
    for (j = 0; j < element->type->nodes; j++) {
      gsl_matrix_set(mesh->fem.H, d, mesh->degrees_of_freedom*j+d, element->type->h(j, mesh->fem.r));
    }
  }
  
  return WASORA_RUNTIME_OK;
  
}

int mesh_compute_B(mesh_t *mesh, element_t *element) {

  int m, d, j;  
  
  for (m = 0; m < mesh->bulk_dimensions; m++) {
    for (d = 0; d < mesh->degrees_of_freedom; d++) {
      for (j = 0; j < element->type->nodes; j++) {
        gsl_matrix_set(mesh->fem.B, mesh->degrees_of_freedom*m+d, mesh->degrees_of_freedom*j+d, gsl_matrix_get(mesh->fem.dhdx, j, m));
      }
    }
  }
 
  return WASORA_RUNTIME_ERROR;
}

int mesh_compute_l(mesh_t *mesh, element_t *element) {
  int j, d;
  
  // armamos el vector l que indica como ensamblar
  for (j = 0; j < element->type->nodes; j++) {
    for (d = 0; d < mesh->degrees_of_freedom; d++) {
      mesh->fem.l[mesh->degrees_of_freedom*j + d] = element->node[j]->index[d];
    }
  }
  
  return WASORA_RUNTIME_OK;
  
}

double mesh_compute_fem_objects_at_gauss(mesh_t *mesh, element_t *element, int v) {
  double w;
  
  w = mesh_integration_weight(mesh, element, v);
  
  mesh_compute_x(element, mesh->fem.r, mesh->fem.x);
  mesh_compute_H(mesh, element);
  mesh_inverse(mesh->bulk_dimensions, mesh->fem.dxdr, mesh->fem.drdx);
  mesh_compute_dhdx(element, mesh->fem.r, mesh->fem.drdx, mesh->fem.dhdx);
  mesh_compute_B(mesh, element);

  // el vector con los indices globales que indica como ensamblar
  mesh_compute_l(mesh, element);
  
  return w;
  
}
