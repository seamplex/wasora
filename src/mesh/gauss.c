/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related gauss integration routines
 *
 *  Copyright (C) 2014--2019 jeremy theler
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

double mesh_integral_over_element(function_t *function, element_t *element, expr_t *weight) {

  double integral = 0;
  double xi;

  int v;
  int j;
  
  for (v = 0; v < element->type->gauss[GAUSS_POINTS_CANONICAL].V; v++) {
    mesh_compute_integration_weight_at_gauss(element, v);

    xi = 0;
    for (j = 0; j < element->type->nodes; j++) {
      xi += element->type->gauss[GAUSS_POINTS_CANONICAL].h[v][j] * function->data_value[element->node[j]->index_mesh];
    }

    if (weight == NULL) {
      integral += element->w[v] * xi;
    } else {
    	mesh_compute_x_at_gauss(element, v);
      mesh_update_coord_vars(element->x[v]);
      integral += wasora_evaluate_expression(weight) * element->w[v] * xi;
    }
  }

  return integral;

}

