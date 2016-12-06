/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related function interpolation routines
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
#include <thirdparty/kdtree.h>

#include <string.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multiroots.h>

struct mesh_interp_params {
  element_t *element;
  const double *x;
};


double mesh_interpolate_function_node(struct function_t *f, const double *x) {
  
  int i, j;
  static element_t *chosen_element;
  element_list_item_t *associated_element;
  void *res_item;  
  node_t *nearest_node;
  double y, dist2;
  double x_nearest[3] = {0, 0, 0};
  mesh_t *m = f->mesh;  

  gsl_vector *r = NULL;    // vector con r y s (lo que queremos encontrar)

  if (m->kd_nodes != NULL) {

    // TODO: ver si esto es lo mejor (probablemente no)
    res_item = kd_nearest(m->kd_nodes, x);
    nearest_node = (node_t *)(kd_res_item(res_item, x_nearest));
    kd_res_free(res_item);    

    switch (m->spatial_dimensions) {
      case 1:
        if ((dist2 = gsl_pow_2(fabs(x[0]-x_nearest[0]))) < gsl_pow_2(f->multidim_threshold)) {
          return f->data_value[nearest_node->id - 1];
        }
      break;
      case 2:
        if ((dist2 = (mesh_subtract_squared_module2d(x, x_nearest))) < gsl_pow_2(f->multidim_threshold)) {
          return f->data_value[nearest_node->id - 1];
        }
      break;
      case 3:
        if ((dist2 = (mesh_subtract_squared_module(x, x_nearest))) < gsl_pow_2(f->multidim_threshold)) {
          return f->data_value[nearest_node->id - 1];
        }
      break;
    }

    chosen_element = NULL;
    LL_FOREACH(nearest_node->associated_elements, associated_element) {
      if (associated_element->element->type->dim == m->bulk_dimensions && associated_element->element->type->point_in_element(associated_element->element, x)) {
        chosen_element = associated_element->element;
        break;
      }
    }

    
    // si no encontramos ninguno entonces capaz que la malla este deformada, probamos con otros nodos
    if (chosen_element == NULL) {
      struct kdres *presults;
      // pedimos los que estén en un radio de una vez y media veces el anterior
      presults = kd_nearest_range(m->kd_nodes, x, 1.5*sqrt(dist2));
      while(chosen_element == NULL && kd_res_end(presults) == 0) {
        nearest_node = (node_t *)(kd_res_item(presults, x_nearest));
        LL_FOREACH(nearest_node->associated_elements, associated_element) {
          if (associated_element->element->type->dim == m->bulk_dimensions && associated_element->element->type->point_in_element(associated_element->element, x)) {
            chosen_element = associated_element->element;
            break;
          }
        }
        kd_res_next(presults);
      }
      kd_res_free(presults);
    }

  } else {

    for (i = 0; i < m->n_nodes; i++) {
      switch (m->spatial_dimensions) {
        case 1:
          if (fabs(x[0]-m->node[i].x[0]) < f->multidim_threshold) {
            return f->data_value[m->node[i].id - 1];
          }
        break;
        case 2:
          if (mesh_subtract_squared_module2d(x, m->node[i].x) < gsl_pow_2(f->multidim_threshold)) {
            return f->data_value[m->node[i].id - 1];
          }
        break;
        case 3:
          if (mesh_subtract_squared_module(x, m->node[i].x) < gsl_pow_2(f->multidim_threshold)) {
            return f->data_value[m->node[i].id - 1];
          }
        break;
      }
    }

    // interpolamos porque no es ningun nodo exacto
    // barremos hasta que lo encontramos
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
    
  if (chosen_element != NULL) {
    r = gsl_vector_alloc(chosen_element->type->dim);
    if (mesh_interp_solve_for_r(chosen_element, x, r) != WASORA_RUNTIME_OK) {
      gsl_vector_free(r);
      return 0;
    }
  } else {
    return 0;
  }
  
  // calculamos el valor de y
  y = 0;
  for (j = 0; j < chosen_element->type->nodes; j++) {
    y += chosen_element->type->h(j, r) * f->data_value[chosen_element->node[j]->id - 1];
  }
  
  gsl_vector_free(r);
  
  return y;
 
}


int mesh_interp_solve_for_r(element_t *element, const double *x, gsl_vector *r) {
  
  int gsl_status;
  size_t iter = 0;  
  struct mesh_interp_params p;
  gsl_vector *test;
  gsl_multiroot_fdfsolver *s;  
  const gsl_multiroot_fdfsolver_type *T;
  gsl_multiroot_function_fdf fun = {&mesh_interp_residual,
                                    &mesh_interp_jacob,
                                    &mesh_interp_residual_jacob,
                                    element->type->dim, &p};
  
  p.element = element;
  p.x = x;
  
  test = gsl_vector_calloc(element->type->dim);    // guess inicial cero
  
//  T = gsl_multiroot_fsolver_hybrids;
//  T = gsl_multiroot_fsolver_hybrid;      
//  T = gsl_multiroot_fsolver_dnewton;
//  T = gsl_multiroot_fsolver_broyden;
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

  gsl_vector_memcpy(r, gsl_multiroot_fdfsolver_root(s));

  gsl_vector_free(test);
  gsl_multiroot_fdfsolver_free(s);
  
  return WASORA_RUNTIME_OK;
  
}



int mesh_interp_residual(const gsl_vector *test, void *params, gsl_vector *residual) {

  int i, j;
  double xi;
  
  element_t *element = ((struct mesh_interp_params *)params)->element;
  const double *x = ((struct mesh_interp_params *)params)->x;

  for (i = 0; i < element->type->dim; i++) {
    xi = x[i];
    for (j = 0; j< element->type->nodes; j++) {
      xi -= element->type->h(j, (gsl_vector *)test) * element->node[j]->x[i];
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
        xi -= element->type->dhdr(k, j, (gsl_vector *)test) * element->node[k]->x[i];
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





double mesh_interpolate_function_cell(struct function_t *f, const double *x) {
  
  int i;
  static cell_t *chosen_cell;
  node_t *nearest_node;
  element_list_item_t *associated_element;
  mesh_t *m = (mesh_t *)f->mesh;


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

  return f->data_value[chosen_cell->id - 1];
 
}

double mesh_interpolate_function_property(struct function_t *function, const double *x) {
  double y = 0;
  property_data_t *property_data = NULL;
  physical_property_t *property = (physical_property_t *)function->property;
  mesh_t *mesh= (mesh_t *)function->mesh;
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

