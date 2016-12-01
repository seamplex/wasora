/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora's mesh-related geometry routines
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
#ifdef WASORA_MESH
#include <gsl/gsl_math.h>
#include <wasora.h>

// c = b - a
void mesh_subtract(const double *a, const double *b, double *c) {
  c[0] = b[0] - a[0];
  c[1] = b[1] - a[1];
  c[2] = b[2] - a[2];
  return;
}

// c = a \times b
void mesh_cross(const double *a, const double *b, double *c) {
  c[0] = a[1]*b[2] - a[2]*b[1];
  c[1] = a[2]*b[0] - a[0]*b[2];
  c[2] = a[0]*b[1] - a[1]*b[0];
  return;
}

// c = a \times b / | a \times b |
void mesh_normalized_cross(const double *a, const double *b, double *c) {
  double norm;
  c[0] = a[1]*b[2] - a[2]*b[1];
  c[1] = a[2]*b[0] - a[0]*b[2];
  c[2] = a[0]*b[1] - a[1]*b[0];
  norm = gsl_hypot3(c[0], c[1], c[2]);
  if (norm != 0) {
    c[0] /= norm;
    c[1] /= norm;
    c[2] /= norm;
  }

  return;
}

// devuelve ( (a \times b) \cdot c ) en tres dimensiones
double mesh_cross_dot(const double *a, const double *b, const double *c) {
  return c[0]*(a[1]*b[2] - a[2]*b[1]) + c[1]*(a[2]*b[0] - a[0]*b[2]) + c[2]*(a[0]*b[1] - a[1]*b[0]);
}


// devuelve ( (b-a) \times (c-a)) ) en dos dimensiones
double mesh_subtract_cross_2d(const double *a, const double *b, const double *c) {
  return a[0]*(b[1]-c[1]) + b[0]*(c[1]-a[1]) + c[0]*(a[1]-b[1]);
}

// devuelve el producto escalar entre a y b
double mesh_dot(const double *a, const double *b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

// devuelve el producto escalar entre (b-a) y c
double mesh_subtract_dot(const double *b, const double *a, const double *c) {
  return (b[0]-a[0])*c[0] + (b[1]-a[1])*c[1] + (b[2]-a[2])*c[2];
}

// devuelve el modulo de (b-a)
double mesh_subtract_module(const double *b, const double *a) {
  return gsl_hypot3(b[0]-a[0], b[1]-a[1], b[2]-a[2]);
}

// devuelve el modulo al cuadrado de (b-a)
double mesh_subtract_squared_module(const  double *b, const  double *a) {
  return (b[0]-a[0])*(b[0]-a[0]) + (b[1]-a[1])*(b[1]-a[1]) + (b[2]-a[2])*(b[2]-a[2]);
}

// devuelve el modulo al cuadrado de (b-a)
double mesh_subtract_squared_module2d(const  double *b, const  double *a) {
  return (b[0]-a[0])*(b[0]-a[0]) + (b[1]-a[1])*(b[1]-a[1]);
}
#endif
