/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related initialization routines
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


int wasora_mesh_init_before_parser(void) {
  
  // vector con las coordenadas globales
  wasora_mesh.vars.vec_x = wasora_define_vector("x_global", 3, NULL, NULL);
  // como ahora hacemos alias de a los elementos del vector x_global lo inicializamos
  wasora_call(wasora_vector_init(wasora_mesh.vars.vec_x));
  
//va+x+name x
//va+x+desc Holder variable for spatial dependance of functions, such spatial distribution
//va+x+desc of physical properties or results of partial differential equations.
  wasora_mesh.vars.x = wasora_define_variable("x");
  wasora_realloc_variable_ptr(wasora_mesh.vars.x, gsl_vector_ptr(wasora_value_ptr(wasora_mesh.vars.vec_x), 0), 0);

//va+y+name y
//va+y+desc Idem as `x`.
  wasora_mesh.vars.y = wasora_define_variable("y");
  wasora_realloc_variable_ptr(wasora_mesh.vars.y, gsl_vector_ptr(wasora_value_ptr(wasora_mesh.vars.vec_x), 1), 0);

//va+z+name z
//va+z+desc Idem as `x`.
  wasora_mesh.vars.z = wasora_define_variable("z");
  wasora_realloc_variable_ptr(wasora_mesh.vars.z, gsl_vector_ptr(wasora_value_ptr(wasora_mesh.vars.vec_x), 2), 0);

// y ya que estamos las ponemos en un array numerico
  wasora_mesh.vars.arr_x[0] = wasora_mesh.vars.x;
  wasora_mesh.vars.arr_x[1] = wasora_mesh.vars.y;
  wasora_mesh.vars.arr_x[2] = wasora_mesh.vars.z;

///va+nodes+name nodes
///va+nodes+desc Number of nodes of the unstructured grid.
  wasora_mesh.vars.nodes = wasora_define_variable("nodes");

///va+elements+name elements
///va+elements+desc Number of total elements of the unstructured grid. This number
///va+elements+desc include those surface elements that belong to boundary physical entities.
  
  wasora_mesh.vars.elements = wasora_define_variable("elements");

///va+cells+name cells
///va+cells+desc Number of cells of the unstructured grid. This number is the actual
///va+cells+desc quantity of volumetric elements in which the domain was discretized.
  
  wasora_mesh.vars.cells = wasora_define_variable("cells");
  
//va+eps+name eps
//va+eps+desc Small value. Default is $10^{-6}$.
  wasora_mesh.vars.eps = wasora_define_variable("eps");
  wasora_var(wasora_mesh.vars.eps) = 1e-6;

// inicializamos la informacion estatica de los elementos
// faltaria los puntos de gauss y la evalaucion en los mismos
  wasora_call(wasora_mesh_element_types_init());
  
  wasora_mesh.initialized = 1;
  
  return WASORA_RUNTIME_OK;
}

void wasora_mesh_add_node_parent(node_relative_t *parents, int index) {
  node_relative_t *parent = calloc(1, sizeof(node_relative_t));
  parent->index = index;
  LL_APPEND(parents, parent);
  return;
}

void wasora_mesh_compute_coords_from_parent(element_type_t *element_type, int j) {
  node_relative_t *parent;
  int m;
  int den = 0;
  
  LL_FOREACH(element_type->node_parents[j], parent) {
    den++;
    for (m = 0; m < element_type->dim; m++) {
      element_type->node_coords[j][m] += element_type->node_coords[parent->index][m];  
    }
  }
  
  for (m = 0; m < element_type->dim; m++) {
    element_type->node_coords[j][m] /= den;
  }
  return;
}