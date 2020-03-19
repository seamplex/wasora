/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related definitions routines
 *
 *  Copyright (C) 2014--2016, 2018 jeremy theler
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

mesh_t *wasora_define_mesh(char *name, file_t *file, int bulk_dimensions, int spatial_dimensions, int degrees, int ordering, int structured, expr_t *scale_factor, expr_t *offsets, expr_t *ncells, expr_t *lengths, expr_t *deltas) {

  mesh_t *mesh;
  char default_name[] = "mesh";

  if (name == NULL) {
    name = default_name;
  }

  if (wasora_get_mesh_ptr(name) != NULL) {
    wasora_push_error_message("there already exists a mesh named '%s'", name);
    return NULL;
  }

  mesh = calloc(1, sizeof(mesh_t));
  mesh->name = strdup(name);
  mesh->file = file;
  mesh->bulk_dimensions = bulk_dimensions;
  mesh->spatial_dimensions = spatial_dimensions;
  mesh->degrees_of_freedom = degrees;
  mesh->ordering = ordering;
  mesh->structured = structured;

  mesh->scale_factor = scale_factor;
  mesh->offset_x = &offsets[0];
  mesh->offset_y = &offsets[1];
  mesh->offset_z = &offsets[2];
    
  if (ncells != NULL) {
    mesh->expr_ncells_x = &ncells[0];
    mesh->expr_ncells_y = &ncells[1];
    mesh->expr_ncells_z = &ncells[2];
  }
  if (lengths != NULL) {
    mesh->expr_length_x = &lengths[0];
    mesh->expr_length_y = &lengths[1];
    mesh->expr_length_z = &lengths[2];
  }
  if (deltas != NULL) {
    mesh->expr_uniform_delta_x = &deltas[0];
    mesh->expr_uniform_delta_y = &deltas[1];
    mesh->expr_uniform_delta_z = &deltas[2];
  }
 
  // seteamos la principal como la primera
  if (wasora_mesh.meshes == NULL) {
    wasora_mesh.main_mesh = mesh;
  }

  // y la agregamos al hash
  HASH_ADD_KEYPTR(hh, wasora_mesh.meshes, mesh->name, strlen(mesh->name), mesh);

  return mesh;
}

physical_entity_t *wasora_define_physical_entity(char *name, mesh_t *new_mesh, int dimension) {
  char *dummy_aux = NULL;
  physical_entity_t *physical_entity;
  mesh_t *mesh; 
  int already_exists = 0;
  
  if (name == NULL) {
    wasora_push_error_message("mandatory name needed for physical entity");
    return NULL;
  }
  if ((mesh = new_mesh) == NULL) {
    if ((mesh = wasora_mesh.main_mesh) == NULL) {
      wasora_push_error_message("no mesh for defining physical entity '%s'", name);
      return NULL;  
    }
  }
  
  if ((physical_entity = wasora_get_physical_entity_ptr(name, mesh)) == NULL) {
    already_exists = 0; // esto lo usamos para definir las variables especiales abajo
    physical_entity = calloc(1, sizeof(physical_entity_t));
    physical_entity->name = strdup(name);
    HASH_ADD_KEYPTR(hh, mesh->physical_entities, physical_entity->name, strlen(name), physical_entity);
  } else {
    already_exists = 1;
  }
  
/*  
  if (physical_entity->id != 0 && id != 0 && physical_entity->id != id) {
    wasora_push_error_message("physical entity '%s' has been previously defined using id '%d' and now id '%d' is required", name, physical_entity->id, id);
    return NULL;
  } else if (physical_entity->id == 0 && id != 0) {
    physical_entity->id = id;
  }
 */
  if (physical_entity->dimension != 0 && dimension != 0 && physical_entity->dimension != dimension) {
    wasora_push_error_message("physical entity '%s' has been previously defined as dimension '%d' and now dimension '%d' is required", name, physical_entity->dimension, dimension);
    return NULL;
  } else if (physical_entity->dimension == 0 && dimension != 0) {
    physical_entity->dimension = dimension;
  }
  
  // -----------------------------  
  if (already_exists == 0 && wasora_check_name(name) == WASORA_PARSER_OK) {
    // volumen (o area o longitud)
    if (asprintf(&dummy_aux, "%s_vol", physical_entity->name) == -1) {
      wasora_push_error_message("memory allocation error");
      return NULL;
    }
    // volvemos a chequear por si acaso hay duplicados en mallas 
    if (wasora_check_name(dummy_aux) == WASORA_PARSER_OK) {
      if ((physical_entity->var_vol = wasora_define_variable(dummy_aux)) == NULL) {
        return NULL;
      }
    } else {
      wasora_pop_error_message();
    }
    free(dummy_aux);

    // centro de gravedad
    if (asprintf(&dummy_aux, "%s_cog", physical_entity->name) == -1) {
      wasora_push_error_message("memory allocation error");
      return NULL;
    }
    if (wasora_check_name(dummy_aux) == WASORA_PARSER_OK) {
      if ((physical_entity->vector_cog = wasora_define_vector(dummy_aux, 3, NULL, NULL)) == NULL) {
        return NULL;
      }
    } else {
      wasora_pop_error_message();
    }  
    free(dummy_aux);

  } 
  return physical_entity;
}

material_t *wasora_define_material(const char *name) {

  material_t *material;

  HASH_FIND_STR(wasora_mesh.materials, name, material);
  if (material != NULL) {
    wasora_push_error_message("there already exists a material named '%s'", name);
    return NULL;
  }

  material = calloc(1, sizeof(material_t));
  material->name = strdup(name);

  HASH_ADD_KEYPTR(hh, wasora_mesh.materials, material->name, strlen(material->name), material);

  return material;
}

physical_property_t *wasora_define_physical_property(const char *name, mesh_t *mesh) {

  physical_property_t *property;
  function_t *function;

  if (mesh == NULL) {
    mesh = wasora_mesh.main_mesh;
  }
  
  if (mesh == NULL) {
    wasora_push_error_message("physical properties can be given only after at least giving one mesh");
    return NULL;;
  } else if (mesh->bulk_dimensions == 0) {
    wasora_push_error_message("mesh '%s' has zero dimensions when defining property '%s', keyword DIMENSIONS is needed for MESH definition", mesh->name, name);
    return NULL;
  }
  
  HASH_FIND_STR(wasora_mesh.physical_properties, name, property);
  if (property != NULL) {
    wasora_push_error_message("there already exists a property named '%s'", name);
    return NULL;
  }

  property = calloc(1, sizeof(physical_property_t));
  property->name = strdup(name);
  HASH_ADD_KEYPTR(hh, wasora_mesh.physical_properties, property->name, strlen(property->name), property);

  // ademas de la propiedad, definimos una function con el nombre de la propiedad
  // que se va a resolver en funcion de x,y,z
  if ((function = wasora_define_function(name, mesh->bulk_dimensions)) == NULL) {
    return NULL;
  }

  function->mesh = mesh;
  function->type = type_pointwise_mesh_property;
  function->property = property;
  

  return property;
}


property_data_t *wasora_define_property_data(const char *materialname, const char *propertyname, const char *expr_string) {

  property_data_t *property_data;
  material_t *material;
  physical_property_t *property;
  function_t *function;
  char *name;

  HASH_FIND_STR(wasora_mesh.materials, materialname, material);
  if (material == NULL) {
    material = wasora_define_material(materialname);
  }
  HASH_FIND_STR(wasora_mesh.physical_properties, propertyname, property);
  if (property == NULL) {
    if ((property = wasora_define_physical_property(propertyname, material->mesh)) == NULL) {
      return NULL;
    }
  }

  property_data = calloc(1, sizeof(property_data_t));
  property_data->material = material;
  property_data->property = property;
  if (wasora_parse_expression(expr_string, &property_data->expr) != WASORA_RUNTIME_OK) {
    return NULL;
  }

  // function llamada material_property que se evalua en x,y,z pero directamente
  // el usuario elige la propiedad sin tener que depender de x,y,z
  if (material->mesh->bulk_dimensions == 0) {
    wasora_push_error_message("mesh '%s' has zero dimensions when defining property '%s', keyword DIMENSIONS is needed for MESH definition", wasora_mesh.main_mesh->name, property->name);
    return NULL;
  }
  name = malloc(strlen(material->name)+strlen(property->name)+8);
  sprintf(name, "%s_%s", material->name, property->name);
  if ((function = wasora_define_function(name, wasora_mesh.main_mesh->bulk_dimensions)) == NULL) {
    return NULL;
  }
  function->type = type_algebraic;
  function->var_argument = wasora_mesh.vars.arr_x;
  function->algebraic_expression = property_data->expr;
  free(name);
  
  HASH_ADD_KEYPTR(hh, material->property_datums, property->name, strlen(property->name), property_data);
//  HASH_ADD_KEYPTR(hh, property->property_datums, material->name, strlen(material->name), property_data);

  return property_data;
}
