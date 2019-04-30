/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related quality routines
 *
 *  Copyright (C) 2018 jeremy theler
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

double mesh_compute_quality(mesh_t *mesh, element_t *element) {

  double det, det0;
  double det_local_min = 1e6;
//  int node_local_min;
  int j;
  

  // calculamos el jacobiano principal
  gsl_vector_set(mesh->fem.r, 0, element->type->barycenter_coords[0]);
  gsl_vector_set(mesh->fem.r, 1, element->type->barycenter_coords[0]);
  gsl_vector_set(mesh->fem.r, 2, element->type->barycenter_coords[0]);
  mesh_compute_dxdr(element, mesh->fem.r, mesh->fem.dxdr);
  det0 = fabs(mesh_determinant(element->type->dim, mesh->fem.dxdr));

  for (j = 0; j < element->type->nodes; j++) {

    // esto da exactamente ceros o unos (o 0.5 para nodos intermedios)
    wasora_call(mesh_compute_r_at_node(element, j, mesh->fem.r));
    // calcula el jacobiano (matrix)
    mesh_compute_dxdr(element, mesh->fem.r, mesh->fem.dxdr);
    det = mesh_determinant(element->type->dim, mesh->fem.dxdr);

    if (det < det_local_min) {
      det_local_min = det;
//      node_local_min = j;
    }
//    printf("%d\t%d\t%e\t%e\t%e\n", element->id, j, det, det0, det/det0);
  }

  return det_local_min/det0;
}
