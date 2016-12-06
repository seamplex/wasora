/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related gauss integration routines
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

double mesh_integral_over_element(function_t *function, element_t *element, expr_t *weight) {

  double w;
  double xi;
  double integral = 0;

  int v;
  int j;
  int d;
  
  mesh_t *mesh = function->mesh;
  
  for (v = 0; v < element->type->gauss[GAUSS_POINTS_CANONICAL].V; v++) {
    w = mesh_integration_weight(mesh, element, v);

    xi = 0;
    for (j = 0; j < element->type->nodes; j++) {
      xi += gsl_vector_get(mesh->fem.h, j) * function->data_value[element->node[j]->id - 1];
    }

    if (weight == NULL) {
      integral += w * xi;
    } else {
      for (d = 0; d < element->type->dim; d++) {
        gsl_vector_set(mesh->fem.r, d, element->type->gauss[GAUSS_POINTS_CANONICAL].r[v][d]);
      }
    	mesh_compute_x(element, mesh->fem.r, wasora_value_ptr(wasora_mesh.vars.vec_x));
      integral += wasora_evaluate_expression(weight) * w * xi;
    }
  }

  return integral;

}


void mesh_alloc_fem_objects(mesh_t *mesh) {
  int M, J, G, N;

  M = mesh->bulk_dimensions;
  J = mesh->max_nodes_per_element;
  G = mesh->degrees_of_freedom;
  N = J*G;

  mesh->fem.r = gsl_vector_alloc(M);
  mesh->fem.x = wasora_value_ptr(wasora_mesh.vars.vec_x);
  mesh->fem.h = gsl_vector_alloc(J);
  mesh->fem.dhdr = gsl_matrix_alloc(J, M);
  mesh->fem.dhdx = gsl_matrix_alloc(J, M);
  mesh->fem.drdx = gsl_matrix_alloc(M, M);
  mesh->fem.dxdr = gsl_matrix_alloc(M, M);
  if (N != 0) {
    mesh->fem.l = calloc(N, sizeof(int));
  }

  return;
}

double mesh_integration_weight(mesh_t *mesh, element_t *element, int v) {

  int d;
  
  if (mesh->fem.r == NULL) {
    mesh_alloc_fem_objects(mesh);
  }
  
  // calculamos las coordenadas del punto de gauss
  for (d = 0; d < element->type->dim; d++) {
    gsl_vector_set(mesh->fem.r, d, element->type->gauss[GAUSS_POINTS_CANONICAL].r[v][d]);
  }

  // evaluamos las funciones de forma en r
  mesh_compute_h(element, mesh->fem.r, mesh->fem.h);

  // dxdr
  mesh_compute_dxdr(element, mesh->fem.r, mesh->fem.dxdr);

  // calculamos el peso de integracion por el determinante del jacobiano
  return element->type->gauss[GAUSS_POINTS_CANONICAL].w[v] * fabs(mesh_determinant(element->type->dim, mesh->fem.dxdr));
}
