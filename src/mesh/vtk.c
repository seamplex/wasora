/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related vtk output routines
 *
 *  Copyright (C) 2014--2015 jeremy theler
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// conversion de gmsh a vtk
int vtkfromgmsh_types[16] = {
  0,    // ELEMENT_TYPE_UNDEFINED
  3,    // ELEMENT_TYPE_LINE
  5,    // ELEMENT_TYPE_TRIANGLE
  9,    // ELEMENT_TYPE_QUADRANGLE
 10,    // ELEMENT_TYPE_TETRAHEDRON
 12,    // ELEMENT_TYPE_HEXAHEDRON
 13,    // ELEMENT_TYPE_PRISM 
 14,    // ELEMENT_TYPE_PYRAMID 
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  1     // ELEMENT_TYPE_POINT
};


int mesh_vtk_write_header(FILE *file) {
  fprintf(file, "# vtk DataFile Version 2.0\n");
  fprintf(file, "wasora vtk output\n");
  fprintf(file, "ASCII\n");

  return WASORA_RUNTIME_OK;
}


int mesh_vtk_write_mesh(mesh_t *mesh, FILE *file) {
  
  if (mesh->structured) {
    wasora_call(mesh_vtk_write_structured_mesh(mesh, file));
  } else {
    wasora_call(mesh_vtk_write_unstructured_mesh(mesh, file));
  }
  
  return WASORA_RUNTIME_OK;
}

int mesh_vtk_write_structured_mesh(mesh_t *mesh, FILE *file) {
  
  int i, j, k;
  int effective_y, effective_z;
  
  assert(mesh->structured != 0);
  
  effective_y = (mesh->bulk_dimensions > 1) ? mesh->ncells_y+1 : 1;
  effective_z = (mesh->bulk_dimensions > 2) ? mesh->ncells_z+1 : 1;

  fprintf(file, "DATASET RECTILINEAR_GRID\n");
  fprintf(file, "DIMENSIONS %d %d %d\n", mesh->ncells_x+1, effective_y, effective_z);

  fprintf(file, "X_COORDINATES %d double\n", mesh->ncells_x+1);
  for (i = 0; i < mesh->ncells_x+1; i++) {
    fprintf(file, "%g ", mesh->nodes_x[i]);
  }
  fprintf(file, "\n");

  fprintf(file, "Y_COORDINATES %d double\n", effective_y);
  for (j = 0; j < effective_y; j++) {
    fprintf(file, "%g ", mesh->nodes_y[j]);
  }
  fprintf(file, "\n");

  fprintf(file, "Z_COORDINATES %d double\n", effective_z);
  for (k = 0; k < effective_z; k++) {
    fprintf(file, "%g ", mesh->nodes_z[k]);
  }
  fprintf(file, "\n");
  
  return WASORA_RUNTIME_OK;  
}

int mesh_vtk_write_unstructured_mesh(mesh_t *mesh, FILE *file) {
  
  int i, j;
  int size, volumelements;
  
  assert(mesh->structured == 0);

  fprintf(file, "DATASET UNSTRUCTURED_GRID\n");
  fprintf(file, "POINTS %d double\n", mesh->n_nodes);
  for (j = 0; j < mesh->n_nodes; j++) { 
    fprintf(file, "%g %g %g\n", mesh->node[j].x[0], mesh->node[j].x[1], mesh->node[j].x[2]);
  }
  fprintf(file, "\n");

  size = 0;
  volumelements = 0;
  for (i = 0; i < mesh->n_elements; i++) {
    if (mesh->element[i].type->dim == mesh->bulk_dimensions) {
      size += 1 + mesh->element[i].type->nodes;
      volumelements++;
    }
  }
  
  fprintf(file, "CELLS %d %d\n", volumelements, size);
  for (i = 0; i < mesh->n_elements; i++) {
    if (mesh->element[i].type->dim == mesh->bulk_dimensions) {
      fprintf(file, "%d ", mesh->element[i].type->nodes);
      // ojo! capaz que no funcione si no estan ordenados los indices
      for (j = 0; j < mesh->element[i].type->nodes; j++) {
        fprintf(file, " %d", mesh->element[i].node[j]->id-1);
      }
      fprintf(file, "\n");
    }
  }
  fprintf(file, "\n");
  
  fprintf(file, "CELL_TYPES %d\n", volumelements);
  for (i = 0; i < mesh->n_elements; i++) {
    if (mesh->element[i].type->dim == mesh->bulk_dimensions) {
      fprintf(file, "%d\n", vtkfromgmsh_types[mesh->element[i].type->id]);
    }
  }
  
  return WASORA_RUNTIME_OK;  
}


int mesh_vtk_write_scalar(mesh_post_t *mesh_post, function_t *function, centering_t centering) {

  int i;
  mesh_t *mesh;
  
  if (mesh_post->mesh != NULL) {
    mesh = mesh_post->mesh;
  } else if (function != NULL) {
    mesh = function->mesh;
  } else {
    return WASORA_RUNTIME_ERROR;
  }
  
  if (centering == centering_cells) {
    if (mesh_post->cell_init == 0) {
      fprintf(mesh_post->file->pointer, "CELL_DATA %d\n", mesh->n_cells);
      mesh_post->cell_init = 1;
    }
      
    fprintf(mesh_post->file->pointer, "SCALARS %s double\n", function->name);
    fprintf(mesh_post->file->pointer, "LOOKUP_TABLE default\n");

    if (function->type == type_pointwise_mesh_cell) {
      wasora_function_init(function);
      for (i = 0; i < function->data_size; i++) {
        fprintf(mesh_post->file->pointer, "%g\n", function->data_value[i]);
      }
    } else {
      for (i = 0; i < mesh->n_cells; i++) {
        fprintf(mesh_post->file->pointer, "%g\n", wasora_evaluate_function(function, mesh->cell[i].x));
      }
    }
  } else  {
    if (mesh_post->point_init == 0) {
      fprintf(mesh_post->file->pointer, "POINT_DATA %d\n", mesh->n_nodes);
      mesh_post->point_init = 1;
    }
      
    fprintf(mesh_post->file->pointer, "SCALARS %s double\n", function->name);
    fprintf(mesh_post->file->pointer, "LOOKUP_TABLE default\n");
  
    if (function->type == type_pointwise_mesh_node) {
      wasora_function_init(function);
      for (i = 0; i < function->data_size; i++) {
        fprintf(mesh_post->file->pointer, "%g\n", function->data_value[i]);
      }
    } else {
      for (i = 0; i < mesh->n_nodes; i++) {
        fprintf(mesh_post->file->pointer, "%g\n", wasora_evaluate_function(function, mesh->node[i].x));
      }
    }
  }

  fflush(mesh_post->file->pointer);
  
  return WASORA_RUNTIME_OK;
  
}


int mesh_vtk_write_vector(mesh_post_t *mesh_post, function_t **function, centering_t centering) {

  int i;
  mesh_t *mesh;
  
  if (mesh_post->mesh != NULL) {
    mesh = mesh_post->mesh;
  } else if (function[0] != NULL) {
    mesh = function[0]->mesh;
  } else {
    return WASORA_RUNTIME_ERROR;
  }
  
  
  if (centering == centering_cells) {
    if (mesh_post->cell_init == 0) {
      fprintf(mesh_post->file->pointer, "CELL_DATA %d\n", mesh->n_cells);
      mesh_post->cell_init = 1;
    }
      
    fprintf(mesh_post->file->pointer, "VECTORS %s-%s-%s double\n", function[0]->name, function[1]->name, function[1]->name);
      
    for (i = 0; i < mesh->n_cells; i++) {
      fprintf(mesh_post->file->pointer, "%g %g %g\n", wasora_evaluate_function(function[0], mesh->cell[i].x),
                                                      wasora_evaluate_function(function[1], mesh->cell[i].x),
                                                      wasora_evaluate_function(function[2], mesh->cell[i].x));
    }
  } else {
    if (mesh_post->point_init == 0) {
      fprintf(mesh_post->file->pointer, "POINT_DATA %d\n", mesh->n_nodes);
      mesh_post->point_init = 1;
    }

    wasora_function_init(function[0]);
    wasora_function_init(function[1]);
    wasora_function_init(function[2]);
    
    fprintf(mesh_post->file->pointer, "VECTORS %s-%s-%s double\n", function[0]->name, function[1]->name, function[2]->name);
      
    for (i = 0; i < mesh->n_nodes; i++) {
      fprintf(mesh_post->file->pointer, "%g %g %g\n", wasora_evaluate_function(function[0], mesh->node[i].x),
                                                      wasora_evaluate_function(function[1], mesh->node[i].x),
                                                      wasora_evaluate_function(function[2], mesh->node[i].x));
    }
  }
  
     
  fflush(mesh_post->file->pointer);
  
  return WASORA_RUNTIME_OK;
  
}
