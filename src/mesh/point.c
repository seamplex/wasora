/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related point element routines
 *
 *  Copyright (C) 2014--2017 jeremy theler
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

// --------------------------------------------------------------
// punto de un nodo
// --------------------------------------------------------------
int mesh_one_node_point_init(void) {
  
  element_type_t *element_type;
  gauss_t *gauss;
  
  element_type = &wasora_mesh.element_type[ELEMENT_TYPE_POINT];
  element_type->name = strdup("point");
  element_type->id = ELEMENT_TYPE_POINT;
  element_type->dim = 0;
  element_type->order = 0;
  element_type->nodes = 1;
  element_type->faces = 0;
  element_type->nodes_per_face = 0;
  element_type->h = mesh_one_node_point_h;
  element_type->dhdr = mesh_one_node_point_dhdr;
  element_type->element_volume = mesh_point_vol;
  element_type->point_in_element = NULL;

  element_type->gauss = calloc(2, sizeof(gauss_t));
  
  // el primero es el default
    gauss = &element_type->gauss[GAUSS_POINTS_CANONICAL];
    mesh_alloc_gauss(gauss, element_type, 1);
  
    gauss->w[0] = 1.0;
    gauss->r[0][0] = 0.0;
    gauss->r[0][0] = 0.0;
    gauss->r[0][0] = 0.0;

    gauss->h[0][0] = 1;  
    gauss->dhdr[0][0][0] = 1;
    
    
  // ---- un punto de Gauss sobre el elemento unitario ----  
    gauss = &element_type->gauss[GAUSS_POINTS_SINGLE];
    mesh_alloc_gauss(gauss, element_type, 1);
  
    gauss->w[0] = 1.0;
    gauss->r[0][0] = 0.0;
    gauss->r[0][0] = 0.0;
    gauss->r[0][0] = 0.0;

    gauss->h[0][0] = 1;  
    gauss->dhdr[0][0][0] = 1;

  
  return WASORA_RUNTIME_OK;
}
double mesh_one_node_point_h(int i, gsl_vector *gsl_r) {
  return 1;
}

double mesh_one_node_point_dhdr(int i, int j, gsl_vector *gsl_r) {
  return 0;
}

double mesh_point_vol(element_t *element) {
  return 0;
}
