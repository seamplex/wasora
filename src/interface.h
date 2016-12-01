/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora API header
 *
 *  Copyright (C) 2009-2014 jeremy theler
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
#include <gsl/gsl_matrix.h>

/* variable-related functions */
extern int wasora_exists_var(const char *);
extern double wasora_get_var_value(const char *);
extern double *wasora_get_var_value_ptr(const char *);
extern void wasora_set_var_value(const char *, double);

/* vector-related functions */
extern int wasora_exists_vector(const char *);
extern double wasora_get_vector_value(const char *, int);
extern gsl_vector *wasora_get_vector_gsl_ptr(const char *);
extern void wasora_set_vector_value(const char *, int, double);
extern double *wasora_get_crisp_pointer(const char *);
extern int wasora_get_vector_size(const char *);

/* function-related functions */
extern int wasora_exists_function(const char *);
extern double wasora_get_function_value(const char *name, double *arg);

extern int wasora_exists_var(const char *);
extern double wasoraexistsvar_(const char *, int);
extern double wasora_get_var_value(const char *);
extern double wasoragetvarvalue_(const char *, int);
extern void wasora_set_var_value(const char *, double);
extern int wasorasetvarvalue_(const char *, double *, int);

extern int wasora_exists_vector(const char *);
extern double wasoraexistsvector_(const char *, int);
extern double wasora_get_vector_value(const char *, int);
extern double wasoragetvectorvalue_(const char *, int *, int);
extern void wasora_set_vector_value(const char *, int, double);
extern int wasorasetvectorvalue_(const char *, int *, double *, int);
extern double *wasora_get_crisp_pointer(const char *);
extern int wasora_get_vector_size(const char *);
extern double *wasora_get_vector_raw_ptr(const char *);
extern int wasoragetvectorsize_(const char *, int);

extern int wasora_exists_matrix(const char *);
extern double wasoraexistsmatrix_(const char *, int);
extern double wasora_get_matrix_value(const char *, int , int);
extern gsl_matrix *wasora_get_matrix_gsl_ptr(const char *);
extern double wasoragetmatrixvalue_(const char *, int *, int *, int);
extern void wasora_set_matrix_value(const char *, int , int , double);
extern int wasorasetmatrixvalue_(const char *, int *, int *, double *, int);
extern double *wasora_get_matrix_crisp_pointer(const char *);
extern int wasora_get_matrix_rows(const char *);
extern int wasora_get_matrix_cols(const char *);
extern int wasoragetmatrixcols_(const char *, int);
extern int wasoragetmatrixrows_(const char *, int);


extern int wasora_exists_function(const char *);
extern double wasoraexistsfunction_(const char *, int);
extern double wasoragetfunctionvalue(const char *, double *, int);
extern double wasora_get_function_value(const char *name, double *arg);


