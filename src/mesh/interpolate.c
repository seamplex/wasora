/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related function interpolation routines
 *
 *  Copyright (C) 2014--2019 jeremy theler
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
#include <thirdparty/kdtree.h>

#include <string.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multiroots.h>

struct mesh_interp_params {
  element_t *element;
  const double *x;
};


double mesh_interpolate_function_node(struct function_t *function, const double *x) {
  
  double x_nearest[3] = {0, 0, 0};
  double r[3] = {0, 0, 0};    // vector with the local coordinates within the element
  double y, dist2;
  static element_t *chosen_element;
  element_list_item_t *associated_element;
  void *res_item;  
  node_t *nearest_node;
  mesh_t *m = function->mesh;  
  int i, j;

  
  if (function->data_value == NULL) {
    return 0;
  }
  
  // try the last chosen element
  chosen_element = function->mesh->last_chosen_element;
  
  if (m->kd_nodes != NULL) {

    // find the nearest node
    res_item = kd_nearest(m->kd_nodes, x);
    nearest_node = (node_t *)(kd_res_item(res_item, x_nearest));
    kd_res_free(res_item);    

    switch (m->spatial_dimensions) {
      case 1:
        if ((dist2 = gsl_pow_2(fabs(x[0]-x_nearest[0]))) < gsl_pow_2(function->multidim_threshold)) {
          return function->data_value[nearest_node->index_mesh];
        }
      break;
      case 2:
        if ((dist2 = (mesh_subtract_squared_module2d(x, x_nearest))) < gsl_pow_2(function->multidim_threshold)) {
          return function->data_value[nearest_node->index_mesh];
        }
      break;
      case 3:
        if ((dist2 = (mesh_subtract_squared_module(x, x_nearest))) < gsl_pow_2(function->multidim_threshold)) {
          return function->data_value[nearest_node->index_mesh];
        }
      break;
    }

    // test if the last (cached) chosen_element is the one
    if (chosen_element == NULL || chosen_element->type->point_in_element(chosen_element, x) == 0) {
      chosen_element = NULL;
      LL_FOREACH(nearest_node->associated_elements, associated_element) {
        if (associated_element->element->type->dim == m->bulk_dimensions && associated_element->element->type->point_in_element(associated_element->element, x)) {
          chosen_element = associated_element->element;
          break;
        }  
      }
    }

    
    // if we do not find any then the mesh might be deformed or the point might be outside the domain
    // so we see if we can find another one
    if (chosen_element == NULL && wasora_var(wasora_mesh.vars.mesh_failed_interpolation_factor) > 0) {
      
      // this is a hash of elements we already saw so we do not need to check for them many times
      struct cached_element {
        int id;
        UT_hash_handle hh;
      };
      
      struct cached_element *cache = NULL;
      struct cached_element *tmp = NULL;
      struct cached_element *cached_element = NULL;

      // we ask for the nodes which are within a radius mesh_failed_interpolation_factor times the last one
      struct kdres *presults = kd_nearest_range(m->kd_nodes, x, wasora_var(wasora_mesh.vars.mesh_failed_interpolation_factor)*sqrt(dist2));
      
      while(chosen_element == NULL && kd_res_end(presults) == 0) {
        nearest_node = (node_t *)(kd_res_item(presults, x_nearest));
        LL_FOREACH(nearest_node->associated_elements, associated_element) {
          
          cached_element = NULL;
          HASH_FIND_INT(cache, &associated_element->element->tag, cached_element);
          if (cached_element == NULL) {
            struct cached_element *cached_element = malloc(sizeof(struct cached_element));
            cached_element->id = associated_element->element->tag;
            HASH_ADD_INT(cache, id, cached_element);
          
            if (associated_element->element->type->dim == m->bulk_dimensions && associated_element->element->type->point_in_element(associated_element->element, x)) {
              chosen_element = associated_element->element;
              break;
            }
          }  
        }
        kd_res_next(presults);
      }
      kd_res_free(presults);

      HASH_ITER(hh, cache, cached_element, tmp) {
        HASH_DEL(cache, cached_element);
        free(cached_element);
      }  
    }
    
    // fallback: if we still did not find it, choose the value at the nearest node
    if (chosen_element == NULL && nearest_node != NULL) {
      return function->data_value[nearest_node->index_mesh];
    }
    
  } else {

    // this logic applies only if for some reason there is no kd-tree
    for (i = 0; i < m->n_nodes; i++) {
      switch (m->spatial_dimensions) {
        case 1:
          if (fabs(x[0]-m->node[i].x[0]) < function->multidim_threshold) {
            return function->data_value[m->node[i].index_mesh];
          }
        break;
        case 2:
          if (mesh_subtract_squared_module2d(x, m->node[i].x) < gsl_pow_2(function->multidim_threshold)) {
            return function->data_value[m->node[i].index_mesh];
          }
        break;
        case 3:
          if (mesh_subtract_squared_module(x, m->node[i].x) < gsl_pow_2(function->multidim_threshold)) {
            return function->data_value[m->node[i].index_mesh];
          }
        break;
      }
    }

    chosen_element = NULL;
    for (i = 0; i < m->n_elements; i++) {
      if (m->element[i].type->dim == m->bulk_dimensions) {
        if (m->element[i].type->point_in_element(&m->element[i], x)) {
          chosen_element = &m->element[i];
          break;
        }
      }
    }
  }

  // update cache
  function->mesh->last_chosen_element = chosen_element;  
  
  if (chosen_element != NULL) {
    if (mesh_interp_solve_for_r(chosen_element, x, r) != WASORA_RUNTIME_OK) {
      return 0;
    }
  } else {
    return 0;
  }
  
  // compute the interpolation
  y = 0;
  if (function->spatial_derivative_of == NULL) {
    
    for (j = 0; j < chosen_element->type->nodes; j++) {
      y += chosen_element->type->h(j, r) * function->data_value[chosen_element->node[j]->index_mesh];    
    }
    
  } else {
    
    gsl_matrix *dhdx = gsl_matrix_alloc(chosen_element->type->nodes, chosen_element->type->dim);
    
    mesh_compute_dhdx(chosen_element, r, NULL, dhdx);
      
    for (j = 0; j < chosen_element->type->nodes; j++) {
      y += gsl_matrix_get(dhdx, j, function->spatial_derivative_with_respect_to)
            * function->spatial_derivative_of->data_value[chosen_element->node[j]->index_mesh];
    }
    
    gsl_matrix_free(dhdx);
    
  }  
  
  return y;
 
}


int mesh_interp_solve_for_r(element_t *element, const double *x, double *r) {
  
  int gsl_status;
  int m;
  size_t iter = 0;  
  struct mesh_interp_params p;
  gsl_vector *test;
  gsl_multiroot_fdfsolver *s;  
  const gsl_multiroot_fdfsolver_type *T;
  gsl_multiroot_function_fdf fun = {&mesh_interp_residual,
                                    &mesh_interp_jacob,
                                    &mesh_interp_residual_jacob,
                                    element->type->dim, &p};
  
  if (element->type->id == ELEMENT_TYPE_TETRAHEDRON4 || element->type->id == ELEMENT_TYPE_TETRAHEDRON10) {
    
    // the tetrahedron is an easy one
    mesh_compute_r_tetrahedron(element, x, r);

  } else {
  
    p.element = element;
    p.x = x;
  
    test = gsl_vector_calloc(element->type->dim);    // guess inicial cero

//    T = gsl_multiroot_fsolver_hybrids;
//    T = gsl_multiroot_fsolver_hybrid;      
//    T = gsl_multiroot_fsolver_dnewton;
//    T = gsl_multiroot_fsolver_broyden;
    T = gsl_multiroot_fdfsolver_gnewton;
      
    s = gsl_multiroot_fdfsolver_alloc (T, element->type->dim);
    gsl_multiroot_fdfsolver_set (s, &fun, test);

    do {
      iter++;
      if ((gsl_status = gsl_multiroot_fdfsolver_iterate(s)) != GSL_SUCCESS) {
        return WASORA_RUNTIME_ERROR;
      }
      gsl_status = gsl_multiroot_test_residual(s->f, wasora_var(wasora_mesh.vars.eps));
    } while (gsl_status == GSL_CONTINUE && iter < 10);

    for (m = 0; m < element->type->dim; m++) {
      r[m] = gsl_vector_get(gsl_multiroot_fdfsolver_root(s), m);
    }  

    gsl_vector_free(test);
    gsl_multiroot_fdfsolver_free(s);
  
  }
  
  return WASORA_RUNTIME_OK;
    
  
}




// vemos que r hace que las x se interpolen exactamente (isoparametricos)
int mesh_interp_residual(const gsl_vector *test, void *params, gsl_vector *residual) {

  int i, j;
  double xi;
  
  element_t *element = ((struct mesh_interp_params *)params)->element;
  const double *x = ((struct mesh_interp_params *)params)->x;

  for (i = 0; i < element->type->dim; i++) {
    xi = x[i];
    for (j = 0; j< element->type->nodes; j++) {
      xi -= element->type->h(j, (double *)gsl_vector_const_ptr(test, 0)) * element->node[j]->x[i];
    }
    gsl_vector_set(residual, i, xi);
  }
  
  return GSL_SUCCESS;
  
}


int mesh_interp_jacob(const gsl_vector *test, void *params, gsl_matrix *J) {

  int i, j, k;
  double xi;
  
  element_t *element = ((struct mesh_interp_params *)params)->element;

  for (i = 0; i < element->type->dim; i++) {
    for (j = 0; j < element->type->dim; j++) {
      xi = 0;
      for (k = 0; k < element->type->nodes; k++) {
        // es negativo por como definimos el residuo
        // el cast explicito es para sacarnos de encima el const
        xi -= element->type->dhdr(k, j, (double *)gsl_vector_const_ptr(test, 0)) * element->node[k]->x[i];
      }
      gsl_matrix_set(J, i, j, xi);
    }
  }
     
  return GSL_SUCCESS;

}


int mesh_interp_residual_jacob(const gsl_vector *test, void *params, gsl_vector *residual, gsl_matrix * J) {
  mesh_interp_residual(test, params, residual);
  mesh_interp_jacob(test, params, J);
  
  return GSL_SUCCESS;
}





int mesh_compute_r_tetrahedron(element_t *element, const double *x, double *r) {

  int j, j_prime;
  
  
  if (element->type->id == ELEMENT_TYPE_TETRAHEDRON4 || element->type->id == ELEMENT_TYPE_TETRAHEDRON10) {
//    double xi0, one;
    double sixV;
//    double sixV01;
    double sixV02, sixV03, sixV04;

    // porque ya teniamos todo desde 1    
    double dx[5][5];
    double dy[5][5];
    double dz[5][5];
    for (j = 0; j < 4; j++) {
      for (j_prime = 0; j_prime < 4; j_prime++) {
        dx[j+1][j_prime+1] = element->node[j]->x[0] - element->node[j_prime]->x[0];
        dy[j+1][j_prime+1] = element->node[j]->x[1] - element->node[j_prime]->x[1];
        dz[j+1][j_prime+1] = element->node[j]->x[2] - element->node[j_prime]->x[2];
      }
    }
  
    // arrancan en uno
    sixV = dx[2][1] * (dy[2][3] * dz[3][4] - dy[3][4] * dz[2][3] ) + dx[3][2] * (dy[3][4] * dz[1][2] - dy[1][2] * dz[3][4] ) + dx[4][3] * (dy[1][2] * dz[2][3] - dy[2][3] * dz[1][2]);
 
    // estos si arrancan en cero
//    sixV01 = element->node[1]->x[0] * (element->node[2]->x[1]*element->node[3]->x[2] - element->node[3]->x[1]*element->node[2]->x[2]) + element->node[2]->x[0] * (element->node[3]->x[1]*element->node[1]->x[2] - element->node[1]->x[1]*element->node[3]->x[2]) + element->node[3]->x[0] * (element->node[1]->x[1]*element->node[2]->x[2] - element->node[2]->x[1]*element->node[1]->x[2]);
    sixV02 = element->node[0]->x[0] * (element->node[3]->x[1]*element->node[2]->x[2] - element->node[2]->x[1]*element->node[3]->x[2]) + element->node[2]->x[0] * (element->node[0]->x[1]*element->node[3]->x[2] - element->node[3]->x[1]*element->node[0]->x[2]) + element->node[3]->x[0] * (element->node[2]->x[1]*element->node[0]->x[2] - element->node[0]->x[1]*element->node[2]->x[2]);
    sixV03 = element->node[0]->x[0] * (element->node[1]->x[1]*element->node[3]->x[2] - element->node[3]->x[1]*element->node[1]->x[2]) + element->node[1]->x[0] * (element->node[3]->x[1]*element->node[0]->x[2] - element->node[0]->x[1]*element->node[3]->x[2]) + element->node[3]->x[0] * (element->node[0]->x[1]*element->node[1]->x[2] - element->node[1]->x[1]*element->node[0]->x[2]);
    sixV04 = element->node[0]->x[0] * (element->node[2]->x[1]*element->node[1]->x[2] - element->node[1]->x[1]*element->node[2]->x[2]) + element->node[1]->x[0] * (element->node[0]->x[1]*element->node[2]->x[2] - element->node[2]->x[1]*element->node[0]->x[2]) + element->node[2]->x[0] * (element->node[1]->x[1]*element->node[0]->x[2] - element->node[0]->x[1]*element->node[1]->x[2]);
    
    // otra vez en uno
//    xi0 =                1.0/sixV * (sixV01 * 1 + (dy[4][2]*dz[3][2] - dy[3][2]*dz[4][2])*gsl_vector_get(x, 0) + (dx[3][2]*dz[4][2] - dx[4][2]*dz[3][2])*gsl_vector_get(x, 1) + (dx[4][2]*dy[3][2] - dx[3][2]*dy[4][2])*gsl_vector_get(x, 2));
    r[0] = 1.0/sixV * (sixV02 * 1 + (dy[3][1]*dz[4][3] - dy[3][4]*dz[1][3])*x[0] + (dx[4][3]*dz[3][1] - dx[1][3]*dz[3][4])*x[1] + (dx[3][1]*dy[4][3] - dx[3][4]*dy[1][3])*x[2]);
    r[1] = 1.0/sixV * (sixV03 * 1 + (dy[2][4]*dz[1][4] - dy[1][4]*dz[2][4])*x[0] + (dx[1][4]*dz[2][4] - dx[2][4]*dz[1][4])*x[1] + (dx[2][4]*dy[1][4] - dx[1][4]*dy[2][4])*x[2]);
    r[2] = 1.0/sixV * (sixV04 * 1 + (dy[1][3]*dz[2][1] - dy[1][2]*dz[3][1])*x[0] + (dx[2][1]*dz[1][3] - dx[3][1]*dz[1][2])*x[1] + (dx[1][3]*dy[2][1] - dx[1][2]*dy[3][1])*x[2]);
    
/*    
    one = xi0 + gsl_vector_get(r,0) + gsl_vector_get(r,1) + gsl_vector_get(r,2);
    if (gsl_fcmp(one, 1.0, 1e-3) != 0) {
      printf("internal mismatch when computing canonical coordinates in element %d\n", element->id);
      return WASORA_RUNTIME_ERROR;
    }
 */
    
  } else {
    wasora_push_error_message("not for element type %d", element->type->id) ;
    return WASORA_RUNTIME_ERROR;
  }

  return WASORA_RUNTIME_OK;
}




double mesh_interpolate_function_cell(struct function_t *function, const double *x) {
  
  int i;
  static cell_t *chosen_cell;
  node_t *nearest_node;
  element_list_item_t *associated_element;
  mesh_t *m = function->mesh;


  if (m->kd_nodes != NULL) {
    nearest_node = (node_t *)(kd_res_item_data(kd_nearest(m->kd_nodes, x)));
    chosen_cell = NULL;
    LL_FOREACH(nearest_node->associated_elements, associated_element) {
      if (associated_element->element->type->dim == m->bulk_dimensions && associated_element->element->type->point_in_element(associated_element->element, x)) {
        chosen_cell = associated_element->element->cell;
        break;
      }
    }
  }

  // primero probamos la ultima celda;
  if (chosen_cell == NULL || chosen_cell->element->type->point_in_element(chosen_cell->element, x) == 0) {
    chosen_cell = NULL;
    // y si no barremos hasta que lo encontramos
    for (i = 0; i < m->n_cells; i++) {
      if (m->cell[i].element->type->point_in_element(m->cell[i].element, x)) {
        chosen_cell = &m->cell[i];
        break;
      }
    }
  }
  
  if (chosen_cell == NULL) {
    return 0;
  }

  return function->data_value[chosen_cell->id - 1];
 
}

double mesh_interpolate_function_property(struct function_t *function, const double *x) {
  double y = 0;
  property_data_t *property_data = NULL;
  physical_property_t *property = (physical_property_t *)function->property;
  mesh_t *mesh= function->mesh;
  element_t *element; 

  // si no hay malla puede ser que todavia no se inicializo, no es un error  
  if (mesh == NULL || mesh->n_nodes == 0) {
    return 1.0;
  }
  
  element = mesh_find_element(function->mesh, x);

  if (element != NULL && element->physical_entity != NULL && element->physical_entity->material != NULL) {
    HASH_FIND_STR(element->physical_entity->material->property_datums, property->name, property_data);
    if (property_data != NULL) {
      wasora_var(wasora_mesh.vars.x) = x[0];
      if (function->n_arguments > 1) {
        wasora_var(wasora_mesh.vars.y) = x[1];
      }
      if (function->n_arguments > 2) {
        wasora_var(wasora_mesh.vars.z) = x[2];
      }

      // evaluamos la expresion del material
      y = wasora_evaluate_expression(&property_data->expr);
    }
  }
  return y;
}

