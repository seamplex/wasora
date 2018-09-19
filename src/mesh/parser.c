/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related parser routines
 *
 *  Copyright (C) 2014--2018 jeremy theler
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
#define _GNU_SOURCE         
#include <wasora.h>
#include <string.h>

// wasora_parser
int wasora_mesh_parse_line(char *line) {

  char *token;

  if ((token = wasora_get_next_token(line)) != NULL) {

// ---- MESH ----------------------------------------------------
///kw+MESH+usage MESH
    if (strcasecmp(token, "MESH") == 0) {

      mesh_t *mesh;
      char *name = NULL;
      file_t *file = NULL;
      double xi;
      expr_t *ncells = calloc(3, sizeof(expr_t));
      expr_t *lengths = calloc(3, sizeof(expr_t));
      expr_t *deltas = calloc(3, sizeof(expr_t));
      expr_t *scale_factor = calloc(1, sizeof(expr_t));
      expr_t *offset = calloc(3, sizeof(expr_t));
      int ordering = 0;
      int dimensions = 0;
      int degrees = 0;
      int structured = 0;
      node_data_t *node_datas = NULL;

      while ((token = wasora_get_next_token(NULL)) != NULL) {
        
///kw+MESH+usage [ NAME <name> ]
        if (strcasecmp(token, "NAME") == 0) {
          wasora_call(wasora_parser_string(&name));
          
///kw+MESH+usage [ FILE <file_id> |
        } else if (strcasecmp(token, "FILE") == 0) {
          wasora_call(wasora_parser_file(&file));
          structured = 0;
          
///kw+MESH+usage FILE_PATH <file_path> ]
        } else if (strcasecmp(token, "FILE_PATH") == 0) {
          wasora_call(wasora_parser_file_path(&file, "r"));
          structured = 0;
          
///kw+MESH+usage [ STRUCTURED ]
        } else if (strcasecmp(token, "STRUCTURED") == 0) {
          structured = 1;
          
///kw+MESH+usage [ DIMENSIONS <num_expr> ]
        } else if (strcasecmp(token, "DIMENSIONS") == 0) {
          wasora_call(wasora_parser_expression_in_string(&xi));
          dimensions = (int)(xi);
          if (dimensions < 1 || dimensions > 3) {
            wasora_push_error_message("mesh dimensions have to be either 1, 2 or 3, not '%d'", dimensions);
            return WASORA_PARSER_ERROR;
          }
          
///kw+MESH+usage [ ORDERING { unknown | node } ]
        } else if (strcasecmp(token, "ORDERING") == 0) {
          char *keywords[] = {"node", "unknown", ""};
          int values[] = {ordering_node_based, ordering_unknown_based, 0};
          wasora_call(wasora_parser_keywords_ints(keywords, values, &ordering));
          
///kw+MESH+usage [ SCALE <expr> ]
        } else if (strcasecmp(token, "SCALE") == 0) {
          wasora_call(wasora_parser_expression(scale_factor));

///kw+MESH+usage [ OFFSET <expr_x> <expr_y> <expr_z>]
        } else if (strcasecmp(token, "OFFSET") == 0) {
          wasora_call(wasora_parser_expression(&offset[0]));
          wasora_call(wasora_parser_expression(&offset[1]));
          wasora_call(wasora_parser_expression(&offset[2]));
          
///kw+MESH+usage [ DEGREES <num_expr> ]
        } else if (strcasecmp(token, "DEGREES") == 0) {
          wasora_call(wasora_parser_expression_in_string(&xi));
          degrees = (int)(xi);

///kw+MESH+usage [ NCELLS_X <expr> ]
        } else if (strcasecmp(token, "NCELLS_X") == 0) {
          wasora_call(wasora_parser_expression(&ncells[0]));
          structured = 1;

///kw+MESH+usage [ NCELLS_Y <expr> ]
        } else if (strcasecmp(token, "NCELLS_Y") == 0) {
          wasora_call(wasora_parser_expression(&ncells[1]));
          structured = 1;

///kw+MESH+usage [ NCELLS_Z <expr> ]
        } else if (strcasecmp(token, "NCELLS_Z") == 0) {
          wasora_call(wasora_parser_expression(&ncells[2]));
          structured = 1;
          
///kw+MESH+usage [ LENGTH_X <expr> ]
        } else if (strcasecmp(token, "LENGTH_X") == 0) {
          wasora_call(wasora_parser_expression(&lengths[0]));
          structured = 1;

///kw+MESH+usage [ LENGTH_Y <expr> ]
        } else if (strcasecmp(token, "LENGTH_Y") == 0) {
          wasora_call(wasora_parser_expression(&lengths[1]));
          structured = 1;

///kw+MESH+usage [ LENGTH_Z <expr> ]
        } else if (strcasecmp(token, "LENGTH_Z") == 0) {
          wasora_call(wasora_parser_expression(&lengths[2]));
          structured = 1;

///kw+MESH+usage [ DELTA_X <expr> ]
        } else if (strcasecmp(token, "DELTA_X") == 0) {
          wasora_call(wasora_parser_expression(&deltas[0]));
          structured = 1;

///kw+MESH+usage [ DELTA_Y <expr> ]
        } else if (strcasecmp(token, "DELTA_Y") == 0) {
          wasora_call(wasora_parser_expression(&deltas[1]));
          structured = 1;

///kw+MESH+usage [ DELTA_Z <expr> ]
        } else if (strcasecmp(token, "DELTA_Z") == 0) {
          wasora_call(wasora_parser_expression(&deltas[2]));
          structured = 1;

 ///kw+MESH+usage [ READ_FUNCTION <function_name> ] [...]
        } else if (strcasecmp(token, "READ_FUNCTION") == 0 ) {
          // TODO: que funcione con cell-centered
          char *function_name;
          node_data_t *node_data;

          if (dimensions == 0) {
            wasora_push_error_message("MESH READ_FUNCTION needs DIMENSIONS to be set", token);
            return WASORA_PARSER_ERROR;
          }
          
          wasora_call(wasora_parser_string(&function_name));
          
          node_data = calloc(1, sizeof(node_data_t));
          node_data->name_in_mesh = strdup(function_name);
          node_data->function = wasora_define_function(function_name, dimensions);
          LL_APPEND(node_datas, node_data);
          free(function_name);
          
///kw+MESH+usage [ READ_SCALAR <name_in_mesh> AS <function_name> ] [...]
        } else if (strcasecmp(token, "READ_DATA") == 0 || strcasecmp(token, "READ_SCALAR") == 0) {
          char *name_in_mesh;
          char *function_name;
          node_data_t *node_data;

          if (dimensions == 0) {
            wasora_push_error_message("MESH READ_DATA needs DIMENSIONS to be set", token);
            return WASORA_PARSER_ERROR;
          }
          
          wasora_call(wasora_parser_string(&name_in_mesh));
          // el "AS"
          wasora_call(wasora_parser_string(&token));
          if (strcasecmp(token, "AS") != 0) {
            wasora_push_error_message("expected AS instead of '%s'", token);
            return WASORA_PARSER_ERROR;
          }
          wasora_call(wasora_parser_string(&function_name));
          
          node_data = calloc(1, sizeof(node_data_t));
          node_data->name_in_mesh = strdup(name_in_mesh);
          node_data->function = wasora_define_function(function_name, dimensions);
          LL_APPEND(node_datas, node_data);
          free(name_in_mesh);
          free(function_name);

        } else {
          wasora_push_error_message("unknown keyword '%s'", token);
          return WASORA_PARSER_ERROR;
        }
      }

      // si no tenemos nombre 
      if (name == NULL) {
        if (wasora_mesh.meshes == NULL) {
          // y es la primera malla, la llamamos main
          name = strdup("first");
        } else {
          // y es otra malla, nos quejamos
          wasora_push_error_message("when defining multiples MESHes, a NAME is mandatory");
          return WASORA_PARSER_ERROR;
        }
      }

      if (file == NULL && structured == 0) {
        wasora_push_error_message("either FILE, FILE_PATH or STRUCTURED should be given to MESH");
        return WASORA_PARSER_ERROR;
      }
      
      if ((mesh = wasora_define_mesh(name, file, dimensions, dimensions, degrees, ordering, structured, scale_factor, offset, ncells, lengths, deltas)) == NULL) {
        return WASORA_PARSER_ERROR;
      }
      
      if (node_datas != NULL) {
        mesh->node_datas = node_datas;
      }
      
      if (mesh->format == mesh_format_fromextension && mesh->file != NULL) {
        char *ext = mesh->file->format + strlen(mesh->file->format) - 4;
        
               if (strcasecmp(ext, ".msh") == 0) {
          mesh->format = mesh_format_gmsh;
        } else if (strcasecmp(ext, ".vtk") == 0) {
          mesh->format = mesh_format_vtk;
        } else if (strcasecmp(ext, ".frd") == 0) {
          mesh->format = mesh_format_frd;
        } else {
          wasora_push_error_message("unknown extension '%s' and no FORMAT given", ext);
          return WASORA_PARSER_ERROR;
        }
      }
      
      
      if (wasora_define_instruction(wasora_instruction_mesh, mesh) == NULL) {
        return WASORA_PARSER_ERROR;
      }
      free(name);

      return WASORA_PARSER_OK;

// --- MESH_MAIN ------------------------------------------------------
    } else if (strcasecmp(token, "MESH_MAIN") == 0) {
      char *mesh_name;
      
///kw+MESH_MAIN+usage MESH_MAIN
///kw+MESH_MAIN+usage [ <name> ]
      wasora_call(wasora_parser_string(&mesh_name));
      if ((wasora_mesh.main_mesh = wasora_get_mesh_ptr(mesh_name)) == NULL) {
        wasora_push_error_message("unknown mesh '%s'", mesh_name);
        free(mesh_name);
        return WASORA_PARSER_ERROR;
      }
      free(mesh_name);
      
      return WASORA_PARSER_OK;
      

// --- MESH_POST ------------------------------------------------------
    } else if (strcasecmp(token, "MESH_POST") == 0) {

///kw+MESH_POST+usage MESH_POST
      mesh_post_t *mesh_post = calloc(1, sizeof(mesh_post_t));
      
      // con esto le pegamos la mayor parte de las veces
//      mesh_post->cell_centered = wasora_mesh.default_cell_centered;

      while ((token = wasora_get_next_token(NULL)) != NULL) {
///kw+MESH_POST+usage [ MESH <mesh_identifier> ]
        if (strcasecmp(token, "MESH") == 0) {
          char *mesh_name;
          wasora_call(wasora_parser_string(&mesh_name));
          if ((mesh_post->mesh = wasora_get_mesh_ptr(mesh_name)) == NULL) {
            wasora_push_error_message("unknown mesh '%s'", mesh_name);
            free(mesh_name);
            return WASORA_PARSER_ERROR;
          }
          free(mesh_name);
          
///kw+MESH_POST+usage { FILE <name> |
        } else if (strcasecmp(token, "FILE") == 0) {
          wasora_call(wasora_parser_file(&mesh_post->file));
///kw+MESH_POST+usage FILE_PATH <file_path> }
        } else if (strcasecmp(token, "FILE_PATH") == 0) {
          char *file_path;
          wasora_call(wasora_parser_string(&file_path));
          if ((mesh_post->file = wasora_define_file(file_path, file_path, 0, NULL, "w", 0)) == NULL) {
            return WASORA_RUNTIME_ERROR;
          }
          free(file_path);

///kw+MESH_POST+usage [ NO_MESH ]
        } else if (strcasecmp(token, "NOMESH") == 0 || strcasecmp(token, "NO_MESH") == 0) {
          mesh_post->no_mesh = 1;

///kw+MESH_POST+usage [ FORMAT { gmsh | vtk } ]
        } else if (strcasecmp(token, "FORMAT") == 0) {
          char *keywords[] = {"gmsh", "vtk", ""};
          int values[] = {post_format_gmsh, post_format_vtk, 0};
          wasora_call(wasora_parser_keywords_ints(keywords, values, (int *)&mesh_post->format));

///kw+MESH_POST+usage [ CELLS | ]
        } else if (strcasecmp(token, "CELLS") == 0) {
          mesh_post->centering = centering_cells;

///kw+MESH_POST+usage  NODES ]
        } else if (strcasecmp(token, "NODES") == 0) {
          mesh_post->centering = centering_nodes;

///kw+MESH_POST+usage [ NO_PHYSICAL_NAMES ]
        } else if (strcasecmp(token, "NO_PHYSICAL_NAMES") == 0) {
          mesh_post->no_physical_names = 1;
          
///kw+MESH_POST+usage [ VECTOR <function1_x> <function1_y> <function1_z> ] [...]
        } else if (strcasecmp(token, "VECTOR") == 0) {
          int i;
          mesh_post_dist_t *mesh_post_dist = calloc(1, sizeof(mesh_post_dist_t));
          mesh_post_dist->vector = calloc(3, sizeof(function_t *));
          
          for (i = 0; i < 3; i++) {
            
            if ((token = wasora_get_next_token(NULL)) == NULL) {
              wasora_push_error_message("expected function name");
              return WASORA_PARSER_ERROR;
            }
            
            if ((mesh_post_dist->vector[i] = wasora_get_function_ptr(token)) == NULL) {
              mesh_post_dist->vector[i] = calloc(1, sizeof(function_t));
              mesh_post_dist->vector[i]->name = strdup(token);
              mesh_post_dist->vector[i]->type = type_algebraic;
              mesh_post_dist->vector[i]->n_arguments = 3;
              mesh_post_dist->vector[i]->var_argument = wasora_mesh.vars.arr_x;
              wasora_call(wasora_parse_expression(token, &mesh_post_dist->vector[i]->algebraic_expression)); 
            }
            // TODO: como tenemos una funcion podemos ver si es node o cell
            mesh_post_dist->centering = mesh_post->centering;
          }
          
          LL_APPEND(mesh_post->mesh_post_dists, mesh_post_dist);
          
          
        } else {
          
///kw+MESH_POST+usage [ <scalar_function_1> ] [ <scalar_function_2> ] ...
          mesh_post_dist_t *mesh_post_dist = calloc(1, sizeof(mesh_post_dist_t));
          
          if ((mesh_post_dist->scalar = wasora_get_function_ptr(token)) == NULL) {
            mesh_post_dist->scalar = calloc(1, sizeof(function_t));
            mesh_post_dist->scalar->name = strdup(token);
            mesh_post_dist->scalar->type = type_algebraic;
            mesh_post_dist->scalar->n_arguments = 3;   // por generalidad
            mesh_post_dist->scalar->var_argument = wasora_mesh.vars.arr_x;
            wasora_call(wasora_parse_expression(token, &mesh_post_dist->scalar->algebraic_expression)); 
          }
          mesh_post_dist->centering = mesh_post->centering;
          LL_APPEND(mesh_post->mesh_post_dists, mesh_post_dist);
        }
      }

      // si hay una sola malla usamos esa, si hay mas hay que pedir cual
      if (mesh_post->mesh == NULL) {
        if (wasora_mesh.main_mesh == wasora_mesh.meshes) {
          mesh_post->mesh = wasora_mesh.main_mesh;
        } else {
          wasora_push_error_message("do not know what mesh should the post-processing be applied to");
          return WASORA_PARSER_ERROR;
        }
      }
      
      if (mesh_post->file == NULL) {
        wasora_push_error_message("neither FILE not FILE_PATH given (use explicitly 'stdout' if you intend to)");
        return WASORA_PARSER_ERROR;
      }
      
      if (mesh_post->format == post_format_fromextension) {
        char *ext = mesh_post->file->format + strlen(mesh_post->file->format) - 4;
        
               if (strcasecmp(ext, ".pos") == 0 || strcasecmp(ext, ".msh") == 0) {
          mesh_post->format = post_format_gmsh;
        } else if (strcasecmp(ext, ".vtk") == 0) {
          mesh_post->format = post_format_vtk;
        } else {
          wasora_push_error_message("unknown extension '%s' and no FORMAT given", ext);
          return WASORA_PARSER_ERROR;
        }
      }
        
      switch (mesh_post->format) {
        case post_format_gmsh:
          mesh_post->write_header = mesh_gmsh_write_header;
          mesh_post->write_mesh = mesh_gmsh_write_mesh;
          mesh_post->write_scalar = mesh_gmsh_write_scalar;
          mesh_post->write_vector = mesh_gmsh_write_vector;
        break;
        case post_format_vtk:
          mesh_post->write_header = mesh_vtk_write_header;
          mesh_post->write_mesh = mesh_vtk_write_mesh;
          mesh_post->write_scalar = mesh_vtk_write_scalar;
          mesh_post->write_vector = mesh_vtk_write_vector;
        break;
        default:
          return WASORA_PARSER_ERROR;
        break;
      }
      
      LL_APPEND(wasora_mesh.posts, mesh_post);
      wasora_define_instruction(wasora_instruction_mesh_post, mesh_post);
      return WASORA_PARSER_OK;

// --- MESH_INTEGRATE --------------------------------------------------------
    } else if (strcasecmp(token, "MESH_INTEGRATE") == 0) {
///kw+MESH_INTEGRATE+usage MESH_INTEGRATE
      
      mesh_integrate_t *mesh_integrate = calloc(1, sizeof(mesh_integrate_t));
      // con esto le pegamos la mayor parte de las veces
//      mesh_integrate->cell_centered = wasora_mesh.default_cell_centered;
      
      while ((token = wasora_get_next_token(NULL)) != NULL) {
// TODO: integrand y que wasora vea si es una funcion o una expresion        
///kw+MESH_INTEGRATE+usage { FUNCTION <function>
        if (strcasecmp(token, "FUNCTION") == 0) {
          wasora_call(wasora_parser_function(&mesh_integrate->function));

///kw+MESH_INTEGRATE+usage | EXPRESSION <expr> }
        } else if (strcasecmp(token, "EXPRESSION") == 0 || strcasecmp(token, "EXPR") == 0) {
          wasora_call(wasora_parser_expression(&mesh_integrate->expr));
          
///kw+MESH_INTEGRATE+usage [ MESH <mesh_identifier> ]
        } else if (strcasecmp(token, "MESH") == 0) {
          char *mesh_name;
          wasora_call(wasora_parser_string(&mesh_name));
          if ((mesh_integrate->mesh = wasora_get_mesh_ptr(mesh_name)) == NULL) {
            wasora_push_error_message("unknown mesh '%s'", mesh_name);
            free(mesh_name);
            return WASORA_PARSER_ERROR;
          }
          free(mesh_name);        
///kw+MESH_INTEGRATE+usage [ OVER <physical_entity_name> ]
        } else if (strcasecmp(token, "OVER") == 0) {
          char *name;
          wasora_call(wasora_parser_string(&name));
          if ((mesh_integrate->physical_entity = wasora_get_physical_entity_ptr(name, mesh_integrate->mesh)) == NULL) {
            if ((mesh_integrate->physical_entity = wasora_define_physical_entity(name, mesh_integrate->mesh, 0)) == NULL) {
              free(name);
              return WASORA_PARSER_ERROR;
            }
          }
          
///kw+MESH_INTEGRATE+usage RESULT <variable>
        } else if (strcasecmp(token, "RESULT") == 0) {
          char *variable;
          wasora_call(wasora_parser_string(&variable));
          if ((mesh_integrate->result = wasora_get_or_define_variable_ptr(variable)) == NULL) {
            return WASORA_PARSER_ERROR;
          }
          
///kw+MESH_INTEGRATE+usage [ NODES
        } else if (strcasecmp(token, "NODES") == 0) {
            mesh_integrate->centering = centering_nodes;
///kw+MESH_INTEGRATE+usage | CELLS ]
          } else if (strcasecmp(token, "CELLS") == 0) {
            mesh_integrate->centering = centering_cells;

///kw+MESH_INTEGRATE+usage [ GAUSS_POINTS <num_expr> ]
        } else if (strcasecmp(token, "GAUSS_POINTS") == 0) {
          double xi;
          wasora_call(wasora_parser_expression_in_string(&xi));
          mesh_integrate->gauss_points = (int)(xi);
          if (mesh_integrate->gauss_points < 1 || mesh_integrate->gauss_points > 2) {
            wasora_push_error_message("GAUSS_POINTS have to be either 1 or 2, not '%d'", mesh_integrate->gauss_points);
            return WASORA_PARSER_ERROR;
          }
            
        } else {
          wasora_push_error_message("unknown keyword '%s'", token);
          return WASORA_PARSER_ERROR;
        }
      }
      
      if (mesh_integrate->expr.n_tokens == 0 && mesh_integrate->function == NULL) {
        wasora_push_error_message("either EXPR of FUNCTION needed");
        return WASORA_PARSER_ERROR;
      }
/*      
      if (mesh_integrate->physical_entity == NULL) {
        wasora_push_error_message("MESH_INTEGRATE needs a OVER physical_entity");
        return WASORA_PARSER_ERROR;
      }
*/      
      if (mesh_integrate->mesh == NULL) {
        if ((mesh_integrate->mesh = wasora_mesh.main_mesh) == NULL) {
          wasora_push_error_message("no MESH defined for MESH_INTEGRATE");
          return WASORA_PARSER_ERROR;
        }
      }
         
      
      
      LL_APPEND(wasora_mesh.integrates, mesh_integrate);
      wasora_define_instruction(wasora_instruction_mesh_integrate, mesh_integrate);
      return WASORA_PARSER_OK;

// --- MESH_FILL_VECTOR ------------------------------------------------------
    } else if (strcasecmp(token, "MESH_FILL_VECTOR") == 0) {
///kw+MESH_FILL_VECTOR+usage MESH_FILL_VECTOR
      mesh_fill_vector_t *mesh_fill_vector = calloc(1, sizeof(mesh_fill_vector_t));
      
      // con esto le pegamos la mayor parte de las veces
//      mesh_fill_vector->cell_centered = wasora_mesh.default_cell_centered;

      while ((token = wasora_get_next_token(NULL)) != NULL) {
///kw+MESH_FILL_VECTOR+usage [ MESH <name> ]
        if (strcasecmp(token, "MESH") == 0) {
          char *mesh_name;
          wasora_call(wasora_parser_string(&mesh_name));
          if ((mesh_fill_vector->mesh = wasora_get_mesh_ptr(mesh_name)) == NULL) {
            wasora_push_error_message("unknown mesh '%s'", mesh_name);
            free(mesh_name);
            return WASORA_PARSER_ERROR;
          }
          free(mesh_name);
          
///kw+MESH_FILL_VECTOR+usage [ NODES
        } else if (strcasecmp(token, "NODES") == 0) {
            mesh_fill_vector->centering = centering_nodes;
///kw+MESH_FILL_VECTOR+usage | CELLS ]
          } else if (strcasecmp(token, "CELLS") == 0) {
            mesh_fill_vector->centering = centering_cells;

///kw+MESH_FILL_VECTOR+usage VECTOR <vector>
        } else if (strcasecmp(token, "VECTOR") == 0) {
          wasora_call(wasora_parser_vector(&mesh_fill_vector->vector));

///kw+MESH_FILL_VECTOR+usage { FUNCTION <function>
        } else if (strcasecmp(token, "FUNCTION") == 0) {
          wasora_call(wasora_parser_function(&mesh_fill_vector->function));

///kw+MESH_FILL_VECTOR+usage | EXPRESSION <expr> }
        } else if (strcasecmp(token, "EXPRESSION") == 0 || strcasecmp(token, "EXPR") == 0) {
          wasora_call(wasora_parser_expression(&mesh_fill_vector->expr));
          
        } else {
          wasora_push_error_message("unknown keyword '%s'", token);
          return WASORA_PARSER_ERROR;
        }
      }
                
      // si hay una sola malla usamos esa, si hay mas hay que pedir cual
      if (mesh_fill_vector->mesh == NULL) {
        if (wasora_mesh.main_mesh == wasora_mesh.meshes) {
          mesh_fill_vector->mesh = wasora_mesh.main_mesh;
        } else {
          wasora_push_error_message("do not know what mesh should the post-processing be applied to");
          return WASORA_PARSER_ERROR;
        }
      }
      
      if (mesh_fill_vector->vector == NULL) {
        wasora_push_error_message("no VECTOR given to MESH_FILL_VECTOR");
        return WASORA_PARSER_ERROR;
      }
      
      LL_APPEND(wasora_mesh.fill_vectors, mesh_fill_vector);
      wasora_define_instruction(wasora_instruction_mesh_fill_vector, mesh_fill_vector);
      return WASORA_PARSER_OK;
      
// --- MESH_FIND_MAX ------------------------------------------------------
    } else if (strcasecmp(token, "MESH_FIND_MAX") == 0) {

///kw+MESH_FIND_MAX+usage MESH_FIND_MAX
      mesh_find_max_t *mesh_find_max = calloc(1, sizeof(mesh_find_max_t));
      
      // con esto le pegamos la mayor parte de las veces
//      mesh_find_max->cell_centered = wasora_mesh.default_cell_centered;

      while ((token = wasora_get_next_token(NULL)) != NULL) {
///kw+MESH_FIND_MAX+usage { FUNCTION <function>
        if (strcasecmp(token, "FUNCTION") == 0) {
          wasora_call(wasora_parser_function(&mesh_find_max->function));

///kw+MESH_FIND_MAX+usage | EXPRESSION <expr> }
        } else if (strcasecmp(token, "EXPRESSION") == 0 || strcasecmp(token, "EXPR") == 0) {
          wasora_call(wasora_parser_expression(&mesh_find_max->expr));
        
///kw+MESH_FIND_MAX+usage [ MESH <name> ]
        } else if (strcasecmp(token, "MESH") == 0) {
          char *mesh_name;
          wasora_call(wasora_parser_string(&mesh_name));
          if ((mesh_find_max->mesh = wasora_get_mesh_ptr(mesh_name)) == NULL) {
            wasora_push_error_message("unknown mesh '%s'", mesh_name);
            free(mesh_name);
            return WASORA_PARSER_ERROR;
          }
          free(mesh_name);

///kw+MESH_FIND_MAX+usage [ PHYSICAL_ENTITY <physical_entity_name> ]
        } else if (strcasecmp(token, "PHYSICAL_ENTITY") == 0) {
          char *name;
          wasora_call(wasora_parser_string(&name));
          if ((mesh_find_max->physical_entity = wasora_get_physical_entity_ptr(name, mesh_find_max->mesh)) == NULL) {
            if ((mesh_find_max->physical_entity = wasora_define_physical_entity(name, mesh_find_max->mesh, 0)) == NULL) {
              free(name);
              return WASORA_PARSER_ERROR;
            }
          }
          free(name);
          
///kw+MESH_FIND_MAX+usage [ NODES
        } else if (strcasecmp(token, "NODES") == 0) {
          mesh_find_max->centering = centering_nodes;
///kw+MESH_FIND_MAX+usage | CELLS ]
        } else if (strcasecmp(token, "CELLS") == 0) {
          mesh_find_max->centering = centering_cells;
          
///kw+MESH_FIND_MAX+usage [ MAX <variable> ]
        } else if (strcasecmp(token, "MAX") == 0) {
          wasora_call(wasora_parser_variable(&mesh_find_max->max));

///kw+MESH_FIND_MAX+usage [ I_MAX <variable> ]
        } else if (strcasecmp(token, "I_MAX") == 0 || strcasecmp(token, "INDEX") == 0) {
          wasora_call(wasora_parser_variable(&mesh_find_max->i_max));

///kw+MESH_FIND_MAX+usage [ X_MAX <variable> ]
        } else if (strcasecmp(token, "X_MAX") == 0) {
          wasora_call(wasora_parser_variable(&mesh_find_max->x_max));
          
///kw+MESH_FIND_MAX+usage [ Y_MAX <variable> ]
        } else if (strcasecmp(token, "Y_MAX") == 0) {
          wasora_call(wasora_parser_variable(&mesh_find_max->y_max));

///kw+MESH_FIND_MAX+usage [Z_MAX <variable> ]
        } else if (strcasecmp(token, "Z_MAX") == 0) {
          wasora_call(wasora_parser_variable(&mesh_find_max->z_max));

        } else {
          wasora_push_error_message("unknown keyword '%s'", token);
          return WASORA_PARSER_ERROR;
        }
      }
                
      // si hay una sola malla usamos esa, si hay mas hay que pedir cual
      if (mesh_find_max->mesh == NULL) {
        if (wasora_mesh.main_mesh == wasora_mesh.meshes) {
          mesh_find_max->mesh = wasora_mesh.main_mesh;
        } else {
          wasora_push_error_message("do not know what mesh should the post-processing be applied to");
          return WASORA_PARSER_ERROR;
        }
      }
      
      if (mesh_find_max->function == NULL && mesh_find_max->expr.n_tokens == 0) {
        wasora_push_error_message("neither FUNCTION nor EXPRESSION given");
        return WASORA_PARSER_ERROR;
      }
      
      LL_APPEND(wasora_mesh.find_maxs, mesh_find_max);
      wasora_define_instruction(wasora_instruction_mesh_find_max, mesh_find_max);
      return WASORA_PARSER_OK;      

// ---- PHYSICAL_ENTITY ----------------------------------------------------
    } else if (strcasecmp(token, "PHYSICAL_ENTITY") == 0) {

///kw+PHYSICAL_ENTITY+usage PHYSICAL_ENTITY
      char *name = NULL;
      double xi = 0;
      mesh_t *mesh = NULL;
      int dimension = 0;
      material_t *material = NULL;
      bc_t *bcs = NULL;
      bc_t *bc = NULL;
      expr_t *pos = NULL;
      physical_entity_t *physical_entity = NULL;

      while ((token = wasora_get_next_token(NULL)) != NULL) {

        // TODO: habria que sacarlo?
///kw+PHYSICAL_ENTITY+usage NAME <name>
        if (strcasecmp(token, "NAME") == 0) {
          if (wasora_parser_string(&name) != WASORA_PARSER_OK) {
            return WASORA_PARSER_ERROR;
          }
          
///kw+PHYSICAL_ENTITY+usage [ DIMENSION <expr> ]
        } else if (strcasecmp(token, "DIMENSION") == 0 || strcasecmp(token, "DIM") == 0) {
          if (wasora_parser_expression_in_string(&xi) != WASORA_PARSER_OK) {
            return WASORA_PARSER_ERROR;
          }
          dimension = (int)(xi);          

///kw+PHYSICAL_ENTITY+usage [ MESH <name> ]
        } else if (strcasecmp(token, "MESH") == 0) {
          char *mesh_name;
          wasora_call(wasora_parser_string(&mesh_name));
          if ((mesh = wasora_get_mesh_ptr(mesh_name)) == NULL) {
            wasora_push_error_message("unknown mesh '%s'", mesh_name);
            free(mesh_name);
            return WASORA_PARSER_ERROR;
          }
          free(mesh_name);

///kw+PHYSICAL_ENTITY+usage [ MATERIAL <name> ]
        } else if (strcasecmp(token, "MATERIAL") == 0) {
          char *material_name;
          wasora_call(wasora_parser_string(&material_name));
          if ((material = wasora_get_material_ptr(material_name)) == NULL) {
            wasora_push_error_message("unknown material '%s'", material_name);
            free(material_name);
            return WASORA_PARSER_ERROR;
          }
          free(material_name);
          
///kw+PHYSICAL_ENTITY+usage [ X_MIN <expr> ] [ X_MAX <expr> ] [ Y_MIN <expr> ] [ Y_MAX <expr> ] [ Z_MIN <expr> ] [ Z_MAX <expr> ]
        } else if (strcasecmp(token+1, "_MIN") == 0 || strcasecmp(token+1, "_MAX") == 0) {

          int ndim = 0;
          int ndir = 0;
                    
          if (token[0] == 'X' || token[0] == 'x') {
            ndim = 0;
          } else if (token[0] == 'Y' || token[0] == 'y') {
            ndim = 1;
          } else if (token[0] == 'Z' || token[0] == 'z') {
            ndim = 2;
          } else {
            wasora_push_error_message("unknown keyword '%s'", token);
            return WASORA_PARSER_ERROR;
          }
          
          if (token[3] == 'I' || token[3] == 'i') {
            ndir = 0;
          } else if (token[3] == 'A' || token[3] == 'a') {
            ndir = 1;
          } else {
            wasora_push_error_message("unknown keyword '%s'", token);
            return WASORA_PARSER_ERROR;
          }
          
          if (pos == NULL) {
            pos = calloc(6, sizeof(expr_t));
          }
          
          if (wasora_parser_expression(&pos[2*ndim + ndir]) != WASORA_PARSER_OK) {
            return WASORA_PARSER_ERROR;
          }
          if ((ndim+1) > dimension) {
            dimension = (ndim+1);
          }
          
///kw+PHYSICAL_ENTITY+usage [ BC <bc_1> <bcg_2> ... ]
        } else if (strcasecmp(token, "BOUNDARY") == 0 || strcasecmp(token, "BC") == 0) {

          // los argumentos como una linked list de strings
          while ((token = wasora_get_next_token(NULL)) != NULL) {
            bc = calloc(1, sizeof(bc_t));
            bc->string = strdup(token);
            LL_APPEND(bcs, bc);
          }

        } else {
          wasora_push_error_message("undefined keyword '%s'", token);
          return WASORA_PARSER_ERROR;
        }
      }

      if (mesh == NULL && (mesh = wasora_mesh.main_mesh) == NULL) {
        wasora_push_error_message("unknown mesh for physical entity '%s'", name);
        return WASORA_PARSER_ERROR;
      }
      
      if (dimension == 0) {
        if (material != NULL) {
          dimension = mesh->bulk_dimensions;
        } else if (bcs != NULL) {
          dimension = mesh->bulk_dimensions-1;
        } 
      }
      
      if (name == NULL) {
        wasora_push_error_message("NAME is mandatory for PHYSICAL_ENTIY");
        return WASORA_PARSER_ERROR;
      }
      if ((physical_entity = wasora_get_physical_entity_ptr(name, mesh)) == NULL) {
        if ((physical_entity = wasora_define_physical_entity(name, mesh, dimension)) == NULL) {
          return WASORA_PARSER_ERROR;
        }
      }

      if (material != NULL) {
        physical_entity->material = material;
      }
      
      if (bcs != NULL) {
        physical_entity->bcs = bcs;
      }
      
      if (pos != NULL) {
        memcpy(&physical_entity->pos, pos, 6*sizeof(expr_t));
        free(pos);
      }
      
      free(name);
      return WASORA_PARSER_OK;

// ---- MATERIAL ----------------------------------------------------
    } else if (strcasecmp(token, "MATERIAL") == 0) {

///kw+MATERIAL+usage MATERIAL
      material_t *material;
      physical_entity_t *physical_entity = NULL;
      char *name;
      char *material_name;
      
///kw+MATERIAL+usage <name>
      wasora_call(wasora_parser_string(&material_name));

      // si ya hay un material le agregamos propiedades a ese
      HASH_FIND_STR(wasora_mesh.materials, material_name, material);
      if (material == NULL) {
        material = wasora_define_material(material_name);
      }
      
      // por default es la main mesh
      if ((material->mesh = wasora_mesh.main_mesh) == NULL) {
        wasora_push_error_message("MATERIAL before MESH");
        return WASORA_PARSER_ERROR;
      }      
      
      while ((token = wasora_get_next_token(NULL)) != NULL) {

        char *expr_string;

///kw+MATERIAL+usage [ MESH <name> ]
        if (strcasecmp(token, "MESH") == 0) {
          wasora_call(wasora_parser_string(&name));         
          if ((material->mesh = wasora_get_mesh_ptr(name)) == NULL) {
            wasora_push_error_message("undefined mesh '%s'" , name);
            return WASORA_PARSER_ERROR;
          }
          free(name);
          
///kw+MATERIAL+usage [ PHYSICAL_ENTITY <name_1>  [ PHYSICAL_ENTITY <name_2> [ ... ] ] ]
        } else if (strcasecmp(token, "PHYSICAL_ENTITY") == 0) {
          wasora_call(wasora_parser_string(&name));  

          if ((physical_entity = wasora_get_physical_entity_ptr(name, material->mesh)) == NULL) {
            if ((physical_entity = wasora_define_physical_entity(name, material->mesh, material->mesh->bulk_dimensions)) == NULL) {
              return WASORA_PARSER_ERROR;
            }
          }
          
          physical_entity->material = material;
          free(name);
          
        } else {
          name = strdup(token);
///kw+MATERIAL+usage [ <property_name_1> <expr_1> [ <property_name_2> <expr_2> [ ... ] ] ]
          wasora_call(wasora_parser_string(&expr_string));
          if (wasora_define_property_data(material_name, name, expr_string) == NULL) {
            return WASORA_PARSER_ERROR;
          }
          free(expr_string);
          free(name);
        }
      }
      free(material_name);
      
      return WASORA_PARSER_OK;
      
// ---- PHYSICAL_PROPERTY ----------------------------------------------------
    } else if (strcasecmp(token, "PHYSICAL_PROPERTY") == 0) {

///kw+PHYSICAL_PROPERTY+usage PHYSICAL_PROPERTY
      physical_property_t *physical_property;
      char *property_name;
      char *material_name;
      char *expr_string;
      
///kw+PHYSICAL_PROPERTY+usage <name>
      if (wasora_parser_string(&property_name) != WASORA_PARSER_OK) {
        return WASORA_PARSER_ERROR;
      }

      // si ya hay una material le agregamos cosa a esa
      HASH_FIND_STR(wasora_mesh.physical_properties, property_name, physical_property);
      if (physical_property == NULL) {
        physical_property = wasora_define_physical_property(property_name, NULL);
      }

      while ((token = wasora_get_next_token(NULL)) != NULL) {

///kw+PHYSICAL_PROPERTY+usage [ <material_name1> <expr1> [ <material_name2> <expr2> ] ... ]
        material_name = strdup(token);
        if (wasora_parser_string(&expr_string) != WASORA_PARSER_OK) {
          return WASORA_PARSER_ERROR;
        }

        if (wasora_define_property_data(material_name, property_name, expr_string) == NULL) {
          return WASORA_PARSER_ERROR;
        }
        free(expr_string);
        free(material_name);
      }

      free(property_name);

      return WASORA_PARSER_OK;
      
    }    
  }

  return WASORA_PARSER_UNHANDLED;

}


// devuelve la direccion del material que se llama name
material_t *wasora_get_material_ptr(const char *name) {
  material_t *material;
  HASH_FIND_STR(wasora_mesh.materials, name, material);
  return material;
}

// devuelve la direccion de la  physical entity que se llama name
physical_entity_t *wasora_get_physical_entity_ptr(const char *name, mesh_t *mesh) {
  physical_entity_t *physical_entity;
  mesh_t *dummy;
  mesh_t *tmp;
  if (mesh != NULL) {
    HASH_FIND_STR(mesh->physical_entities, name, physical_entity);
  } else {
    // barremos todas las mallas
    HASH_ITER(hh, wasora_mesh.meshes, dummy, tmp) {
      HASH_FIND_STR(dummy->physical_entities, name, physical_entity);
      if (physical_entity != NULL) {
        return physical_entity;
      }
    } 
  }
  return physical_entity;
}

