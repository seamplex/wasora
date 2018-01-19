/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related vtk output routines
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// conversion de gmsh a vtk
//Sacado de https://github.com/Kitware/VTK/blob/master/Common/DataModel/vtkCellType.h
int vtkfromgmsh_types[18] = {
  0,    // ELEMENT_TYPE_UNDEFINED
  3,    // ELEMENT_TYPE_LINE
  5,    // ELEMENT_TYPE_TRIANGLE
  9,    // ELEMENT_TYPE_QUADRANGLE
 10,    // ELEMENT_TYPE_TETRAHEDRON
 12,    // ELEMENT_TYPE_HEXAHEDRON
 13,    // ELEMENT_TYPE_PRISM 
 14,    // ELEMENT_TYPE_PYRAMID 
  0,
 22,    // ELEMENT_TYPE_TRIANGLE6
 28,    // ELEMENT_TYPE_QUADRANGLE9
 24,    // ELEMENT_TYPE_TETRAHEDRON10
 29,    // ELEMENT_TYPE_HEXAHEDRON27
  0,
  0,
  1,    // ELEMENT_TYPE_POINT
 23,    // ELEMENT_TYPE_QUADRANGLE8
 25,    // ELEMENT_TYPE_HEXAHEDRON20
};
// conversion de gmsh a vtk (by reading files because by following the docs it did not work).
// index  0 1 2 3 4 5 6 7 8  9 10 11 12 13 14 15 16 17 18 19
// vtk    0 1 3 2 4 6 7 5 8 10 11  9 18 17 19 14 12 15 16 13
// gmsh   0 1 3 2 4 6 7 5 8  9 12 10 15 11 16 13 18 14 17 19
int hexa20fromgmsh[20] = { 
  0 , 1  , 2  ,  3 , 4  , 5  , 6  , 7  ,
  8 , 11 , 13 ,  9 , 16 , 18 , 19 , 17 ,
 10 , 12 , 14 , 15 } ;
int hexa27fromgmsh[27] = { 
  0 , 1  , 2  ,  3 , 4  , 5  , 6  , 7  ,
  8 , 11 , 13 ,  9 , 16 , 18 , 19 , 17 ,
 10 , 12 , 14 , 15 , 22 , 23 , 21 , 24 ,
 20 , 25 , 26} ;

int mesh_vtk_write_header(FILE *file) {
  fprintf(file, "# vtk DataFile Version 2.0\n");
  fprintf(file, "wasora vtk output\n");
  fprintf(file, "ASCII\n");

  return WASORA_RUNTIME_OK;
}


int mesh_vtk_write_mesh(mesh_t *mesh, int dummy, FILE *file) {
  
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

// Here there are the cell types not supported by vtk but are shown as other cell.
//  for (i = 0; i < mesh->n_elements; i++) {
//    if (mesh->element[i].type->dim == mesh->bulk_dimensions) {
//      switch (mesh->element[i].type->id)
//        {
//        case ELEMENT_TYPE_HEXAHEDRON27:
//          size-=7;
//        break;
//        case ELEMENT_TYPE_QUADRANGLE9:
//          size-=1;
//        break;
//        }
//    }
//  }
 
  fprintf(file, "CELLS %d %d\n", volumelements, size);
  for (i = 0; i < mesh->n_elements; i++) {
    if (mesh->element[i].type->dim == mesh->bulk_dimensions) {
      switch(mesh->element[i].type->id)
        {
        case ELEMENT_TYPE_HEXAHEDRON27: 
          fprintf(file, "%d ", 27);
          for(j = 0; j < 27 ; ++j)
            {
            fprintf(file, " %d", mesh->element[i].node[hexa27fromgmsh[j]]->id-1);
            }
          fprintf(file, "\n");
        break;
        case ELEMENT_TYPE_HEXAHEDRON20:  //It is needed to get a good order.
          fprintf(file, "%d ", 20);
          for(j = 0; j < 20 ; ++j)
            {
            fprintf(file, " %d", mesh->element[i].node[hexa20fromgmsh[j]]->id-1);
            }
          fprintf(file, "\n");
        break;
        default:
          fprintf(file, "%d ", mesh->element[i].type->nodes);
          // ojo! capaz que no funcione si no estan ordenados los indices
          for (j = 0; j < mesh->element[i].type->nodes; j++) {
            // el tet10 es diferente!
            if (vtkfromgmsh_types[mesh->element[i].type->id] == 24 && (j == 8 || j == 9)) {
              if (j == 8) {
                fprintf(file, " %d", mesh->element[i].node[9]->id-1);
              } else if (j == 9) {
                fprintf(file, " %d", mesh->element[i].node[8]->id-1);
              }
            } else {
              fprintf(file, " %d", mesh->element[i].node[j]->id-1);
            }
          }
          fprintf(file, "\n");
        break;
        }
      }
    }
  fprintf(file, "\n");
  
  fprintf(file, "CELL_TYPES %d\n", volumelements);
  for (i = 0; i < mesh->n_elements; i++) {
    if (mesh->element[i].type->dim == mesh->bulk_dimensions) {
//The vtk unsupported cell types go here.
      switch(mesh->element[i].type->id)
        {
//        case ELEMENT_TYPE_HEXAHEDRON27:
//          fprintf(file, "%d\n", 25 );
//        break;
//        case ELEMENT_TYPE_QUADRANGLE9:
//          fprintf(file, "%d\n", 23 );
//        break;
        default:
          fprintf(file, "%d\n", vtkfromgmsh_types[mesh->element[i].type->id]);
        break;
        }
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
  
    if (function->type == type_pointwise_mesh_node && function->data_size == mesh_post->mesh->n_nodes) {
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

  int i, j;
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
      
    for (j = 0; j < mesh->n_nodes; j++) {
      if (function[0]->type == type_pointwise_mesh_node && function[0]->data_size == mesh_post->mesh->n_nodes) {
        fprintf(mesh_post->file->pointer, "%g ", function[0]->data_value[j]);
      } else {
        fprintf(mesh_post->file->pointer, "%g ", wasora_evaluate_function(function[0], mesh->node[j].x));
      }

      if (function[1]->type == type_pointwise_mesh_node && function[1]->data_size == mesh_post->mesh->n_nodes) {
        fprintf(mesh_post->file->pointer, "%g ", function[1]->data_value[j]);
      } else {
        fprintf(mesh_post->file->pointer, "%g ", wasora_evaluate_function(function[1], mesh->node[j].x));
      }

      if (function[2]->type == type_pointwise_mesh_node && function[2]->data_size == mesh_post->mesh->n_nodes) {
        fprintf(mesh_post->file->pointer, "%g\n", function[2]->data_value[j]);
      } else {
        fprintf(mesh_post->file->pointer, "%g\n", wasora_evaluate_function(function[2], mesh->node[j].x));
      }
    }
  }
  
     
  fflush(mesh_post->file->pointer);
  
  return WASORA_RUNTIME_OK;
  
}
