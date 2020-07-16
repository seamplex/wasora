/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related quality routines
 *
 *  Copyright (C) 2018--2019 jeremy theler
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

int mesh_compute_quality(mesh_t *mesh, element_t *element) {

  double det, det0;
  double det_local_min = 1e6;
  int v, m, m_prime, j;
  
  
  if (element->quality == 0) {

    // calculamos el jacobiano principal
    gsl_matrix *dxdr = gsl_matrix_calloc(element->type->dim, element->type->dim);
    for (m = 0; m < element->type->dim; m++) {
      for (m_prime = 0; m_prime < element->type->dim; m_prime++) {
        for (j = 0; j < element->type->nodes; j++) {
          gsl_matrix_add_to_element(dxdr, m, m_prime, element->type->dhdr(j, m_prime, element->type->barycenter_coords) * element->node[j]->x[m]);
        }
      }
    }
    det0 = fabs(mesh_determinant(dxdr));
    
    for (v = 0; v < element->type->gauss[GAUSS_POINTS_FULL].V; v++) {

      gsl_matrix_set_zero(dxdr);
      for (m = 0; m < element->type->dim; m++) {
        for (m_prime = 0; m_prime < element->type->dim; m_prime++) {
          for (j = 0; j < element->type->nodes; j++) {
            gsl_matrix_add_to_element(dxdr, m, m_prime, element->type->dhdr(j, m_prime, element->type->gauss[GAUSS_POINTS_FULL].r[v]) * element->node[j]->x[m]);
          }  
        }
      }
      
      det = mesh_determinant(dxdr);

      if (det < det_local_min) {
        det_local_min = det;
      }
    }

    element->quality = det_local_min/det0;
    
    gsl_matrix_free(dxdr);
  }

  return WASORA_RUNTIME_OK;  
}
