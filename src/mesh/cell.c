/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related cell routines
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

int mesh_element2cell(mesh_t *mesh) {
  
  int i_element, i_cell;
  
  if (mesh->cell != NULL) {
    return WASORA_RUNTIME_OK;
  }
  
  // contamos cuantas celdas hay
  // una celda = un elemento de la dimension del problema
  mesh->n_cells = 0;
  for (i_element = 0; i_element < mesh->n_elements; i_element++) {
    if (mesh->element[i_element].type != NULL && mesh->element[i_element].type->dim == mesh->bulk_dimensions) {
      mesh->n_cells++;
    }
  }
  // alocamos las celdas
  mesh->cell = calloc(mesh->n_cells, sizeof(cell_t));
  

  i_cell = 0;
  for (i_element = 0; i_element < mesh->n_elements; i_element++) {
    if (mesh->element[i_element].type != NULL && mesh->element[i_element].type->dim == mesh->bulk_dimensions) {
  
      // las id las empezamos en uno
      mesh->cell[i_cell].id = i_cell+1;
      
      // elemento (ida y vuelta)
      mesh->cell[i_cell].element = &mesh->element[i_element];
      mesh->element[i_element].cell = &mesh->cell[i_cell];
    
      // holder para vecinos
      mesh->cell[i_cell].ineighbor = calloc(mesh->element[i_element].type->faces, sizeof(int));

      // coordenadas del centro
/*      
      for (i_dim = 0; i_dim < mesh->spatial_dimensions; i_dim++) {
        mesh->cell[i_cell].x[i_dim] = 0;
        for (i_node = 0; i_node < mesh->cell[i_cell].element->type->nodes; i_node++) {
          mesh->cell[i_cell].x[i_dim] += mesh->cell[i_cell].element->node[i_node]->x[i_dim];
        }
        mesh->cell[i_cell].x[i_dim] /= (double)(mesh->cell[i_cell].element->type->nodes);
      }
 */
      wasora_call(mesh_compute_element_barycenter(mesh->cell[i_cell].element, mesh->cell[i_cell].x));
      
      mesh->cell[i_cell].volume = mesh->cell[i_cell].element->type->element_volume(mesh->cell[i_cell].element);      
      
      i_cell++;

    }
  }

  return WASORA_RUNTIME_OK;
  
}


int mesh_compute_coords(mesh_t *mesh) {

  int i;
  
  for (i = 0; i < mesh->n_cells; i++) {
    wasora_call(mesh_compute_element_barycenter(mesh->cell[i].element, mesh->cell[i].x));
    
    // esto seria mas lindo con alguna clase de encapsulacion
    mesh->cell[i].volume = mesh->cell[i].element->type->element_volume(mesh->cell[i].element);
  }
  
  return WASORA_RUNTIME_OK;
}


int mesh_cell_indexes(mesh_t *mesh, int dofs) {
  int index = 0;
  int i, g;
  
  mesh->degrees_of_freedom = dofs;
  
  for (i = 0; i < mesh->n_cells; i++) {
    // holder para indices y resultados
    mesh->cell[i].index = calloc(mesh->degrees_of_freedom, sizeof(int));
    for (g = 0; g < mesh->degrees_of_freedom; g++) {
      mesh->cell[i].index[g] = index++;
    }
  }
  
  return WASORA_RUNTIME_OK;
}

