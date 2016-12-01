/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora simulated annealing functions
 *
 *  Copyright (C) 2013 jeremy theler
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

#include <gsl/gsl_vector.h>
#include <gsl/gsl_siman.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_blas.h>

#include "wasora.h"

/*
static double wasora_boltzmann(double, double, double, gsl_siman_params_t *);

static inline double wasora_boltzmann(double E, double new_E, double T, gsl_siman_params_t *params)
{
  double x = -(new_E - E) / (params->k * T);
  return (x < GSL_LOG_DBL_MIN) ? 0.0 : exp(x);
}


void wasora_min_siman(gsl_vector *real_x) {
  

  void *x, *x0_p, *new_x, *best_x;
  double E, new_E, best_E;
  int i;
  double T, T_factor;
  int n_evals = 1, n_iter = 0, n_accepts, n_rejects, n_eless;  
  
  double (*Efunc) (void *);
  void *(*initial) (void);
  void (*step) (const gsl_rng *, void *, double);
  void (*copy) (void *, void *);
  void *(*copy_construct) (void *);
  void (*print) (void *);
  void (*best) (void *);
  void (*destroy) (void *);
  
  
  gsl_siman_params_t params;
  gsl_rng *r;

  params.n_tries = (wasora.min.n_tries.n_tokens != 0) ? wasora_evaluate_expression(&wasora.min.n_tries) : 200;
  params.iters_fixed_T = (wasora.min.max_iter.n_tokens != 0) ? wasora_evaluate_expression(&wasora.min.max_iter) : 1000;
  params.step_size = (wasora.min.step != NULL) ? wasora_evaluate_expression(&wasora.min.step[0]) : 1;
  params.k = (wasora.min.k.n_tokens != 0) ? wasora_evaluate_expression(&wasora.min.k) : 1;
  params.t_initial = (wasora.min.t_init.n_tokens != 0) ? wasora_evaluate_expression(&wasora.min.t_init) : 0.008;
  params.mu_t = (wasora.min.mu.n_tokens != 0) ? wasora_evaluate_expression(&wasora.min.mu) : 1.003;
  params.t_min = (wasora.min.t_min.n_tokens != 0) ? wasora_evaluate_expression(&wasora.min.t_min) : 2e-6;

  r = gsl_rng_alloc(wasora.min.siman_type);

  if (wasora.min.siman_Efunc == NULL) {
    Efunc = &wasora_siman_Efunc_real;
    initial = NULL;
    step = &wasora_siman_step_real;
    copy = &wasora_siman_copy_real;
    copy_construct = &wasora_siman_copy_construct_real;
    print = &wasora_siman_print_real;
    best = NULL;
    destroy = &wasora_siman_destroy_real;
  } else {
    Efunc = wasora.min.siman_Efunc;
    initial = wasora.min.siman_init;
    step = wasora.min.siman_step;
    copy = wasora.min.siman_copy;
    copy_construct = wasora.min.siman_copy_construct;
    print = wasora.min.siman_print;
    best = wasora.min.siman_best;
    destroy = wasora.min.siman_destroy;
  }
  
/*
  if (wasora.min.verbose) {
    gsl_siman_solve(r, x, siman_compute_f, siman_step, siman_metric, siman_print, siman_copy, siman_copy_construct, siman_destroy, 0, params);
  } else {
    gsl_siman_solve(r, x, siman_compute_f, siman_step, siman_metric, NULL,        siman_copy, siman_copy_construct, siman_destroy, 0, params);
  }
*/
  
//  distance = 0 ; /* This parameter is not currently used */
/*
  if (wasora.min.siman_Efunc == NULL) {
    x = (void *)real_x;
    x0_p = gsl_vector_alloc(wasora.min.function->n_arguments);
    gsl_vector_memcpy(x0_p, x);
  } else {
    x = initial();
    x0_p = copy_construct(x);
  }
  E = Efunc(x0_p);  
  new_x = copy_construct(x0_p);
  best_x = copy_construct(x0_p);  

  best_E = E;

  T = params.t_initial;
  T_factor = 1.0 / params.mu_t;

  if (wasora.min.verbose) {
    printf ("#-iter  #-evals   temperature     position   energy\n");
  }
  
  while (1) {

    n_accepts = 0;
    n_rejects = 0;
    n_eless = 0;

    for (i = 0; i < params.iters_fixed_T; ++i) {

      copy(x, new_x);
      step(r, new_x, params.step_size);
      new_E = Efunc(new_x);

      if (new_E <= best_E) {
        copy(new_x, best_x);
        best_E=new_E;
      }

      ++n_evals;                /* keep track of Ef() evaluations */
      /* now take the crucial step: see if the new point is accepted
         or not, as determined by the boltzmann probability */
/*
      if (new_E < E) {

        if (new_E < best_E) {
          copy(new_x, best_x);
          best_E = new_E;
        }

        // yay! take a step
        copy(new_x, x);
        E = new_E;
        ++n_eless;

      } else if (gsl_rng_uniform(r) < wasora_boltzmann(E, new_E, T, &params)) {
        // yay! take a step
        copy(new_x, x);
        E = new_E;
        ++n_accepts;

      } else {
        ++n_rejects;
      }
    }

    if (wasora.min.verbose) {
      /* see if we need to print stuff as we go */
      /*       printf("%5d %12g %5d %3d %3d %3d", n_iter, T, n_evals, */
      /*           100*n_eless/n_steps, 100*n_accepts/n_steps, */
      /*           100*n_rejects/n_steps); */
/*
      printf ("%5d   %7d  %12g", n_iter, n_evals, T);
      if (print != NULL) {
        print(x);
      }
      printf ("  %12g  %12g\n", E, best_E);
    }

    /* apply the cooling schedule to the temperature */
    /* FIXME: I should also introduce a cooling schedule for the iters */
/*
    T *= T_factor;
    ++n_iter;
    if (T < params.t_min) {
      break;
    }
  }

  /* at the end, copy the result onto the initial point, so we pass it
     back to the caller */
/*
  if (wasora.min.siman_Efunc == NULL) {
    gsl_vector_memcpy(real_x, best_x);
  } else if (wasora.min.siman_best != NULL) {
    best(best_x);
  }


//  destroy(x);
  destroy(new_x);
  destroy(best_x);
  
  gsl_rng_free(r);
  
  return;  
}


double wasora_siman_Efunc_real(void *xp) {

  wasora_min_read_params_from_solver((gsl_vector *)xp);  
  return wasora_min_compute_f(gsl_vector_const_ptr((gsl_vector *)xp, 0));
  
}

void *wasora_siman_copy_construct_real(void *source) {
  
  gsl_vector *vector_dest = gsl_vector_alloc(wasora.min.n); 
  gsl_vector_memcpy(vector_dest, (gsl_vector *)source);
  return (void *)vector_dest;
  
}

void wasora_siman_copy_real(void *source, void *dest) {
  
  gsl_vector_memcpy((gsl_vector *)dest, (gsl_vector *)source);
  return;
  
}

void wasora_siman_destroy_real(void *xp) {
  
  gsl_vector_free((gsl_vector *)xp);
  return;
  
}



double wasora_siman_metric_real(void *xp1, void *xp2) {
  double r;

  if (wasora.min.n == 1) {
    r = fabs(gsl_vector_get((gsl_vector *)xp1, 0) - gsl_vector_get((gsl_vector *)xp2, 0));
  } else {
    gsl_vector *delta_x = gsl_vector_alloc(wasora.min.n);
    gsl_vector_memcpy(delta_x, (gsl_vector *)xp1);
    gsl_vector_sub(delta_x, (gsl_vector *)xp2);
    r = gsl_blas_dnrm2(delta_x);
    gsl_vector_free(delta_x);
  }
  
  return r;
  
}

void wasora_siman_step_real(const gsl_rng *r, void *xp, double step_size) {

  double xi;
  double dx, dy, dz;
  double *x = gsl_vector_ptr((gsl_vector *)xp, 0);
  double *deltax;

  switch (wasora.min.n) {
    case 1:
      xi = step_size * (2*gsl_rng_uniform(r) - 1);
      x[0] += xi;
    break;
    case 2:
      gsl_ran_dir_2d(r, &dx, &dy);
      xi = step_size * gsl_rng_uniform(r);
      x[0] += xi*dx;
      x[1] += xi*dy;
    break;
    case 3:
      gsl_ran_dir_3d(r, &dx, &dy, &dz);
      xi = step_size * gsl_rng_uniform(r);
      x[0] += xi*dx;
      x[1] += xi*dy;
      x[2] += xi*dz;
    break;
    default:
      deltax = malloc(wasora.min.n * sizeof(double));
      int i;
      
      gsl_ran_dir_nd(r, wasora.min.n, x);
      xi = step_size * gsl_rng_uniform(r);
      for (i = 0; i < wasora.min.n; i++) {
        x[i] += xi*deltax[i];
      }
      free(deltax);
    break;
  }
      
  return;  
  
}

void wasora_siman_print_real(void *xp) {
  
  int i;
  for (i = 0; i < wasora.min.n; i++) {
    printf("\t%g\t", gsl_vector_get((gsl_vector *)xp, i));
  }
  
  return;
  
}
*/