/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related post-processing generation routines
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

#include <stdio.h>

int wasora_instruction_mesh_post(void *arg) {

  mesh_post_t *mesh_post = (mesh_post_t *)arg;
  mesh_post_dist_t *mesh_post_dist;

  if (mesh_post->file->pointer == NULL) {
    if (wasora_instruction_open_file(mesh_post->file) != WASORA_RUNTIME_OK) {
      return WASORA_RUNTIME_ERROR;
    }
  }


  // si primero llamo a gmsh write header no puedo chequear si el archivo
  // es nuevito o no
  if (ftell(mesh_post->file->pointer) == 0) {
    wasora_call(mesh_post->write_header(mesh_post->file->pointer));
    if (mesh_post->no_mesh == 0) {
      wasora_call(mesh_post->write_mesh(mesh_post->mesh, mesh_post->no_physical_names, mesh_post->file->pointer));
    }
    mesh_post->point_init = 0;      // TODO: generalizar el nombre
  }

  LL_FOREACH(mesh_post->mesh_post_dists, mesh_post_dist) {
    if (mesh_post_dist->centering == centering_cells && mesh_post->mesh->n_cells == 0) {
      wasora_call(mesh_element2cell(mesh_post->mesh));
    }
    
    if (mesh_post_dist->scalar != NULL) {
      if (mesh_post_dist->scalar->initialized == 0) {
        wasora_call(wasora_function_init(mesh_post_dist->scalar));
      }
      wasora_call(mesh_post->write_scalar(mesh_post, mesh_post_dist->scalar, mesh_post_dist->centering));
    } else {
      wasora_call(mesh_post->write_vector(mesh_post, mesh_post_dist->vector, mesh_post_dist->centering));
    }
    // TODO: tensores
  }

  return WASORA_RUNTIME_OK;
}
