/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora random line
 *
 *  Copyright (C) 2009--2013 jeremy theler
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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void wasora_print_random_line(FILE *file, int length)  {

  static int first = 1;
  int steps;
  int lower, upper;
  int *l;
  int sorted;
  int tmp;
  int i, j;

  if (first) {
    srand(time(NULL));
    first = 0;
  }
  
  if (length == 0) {
    length = 31 + (int)((double)rand()/(double)RAND_MAX * 31);
  }

  steps = (int)((double)rand()/(double)RAND_MAX * 7.0) + 5;
  lower = (int)(0.5*((double)length/(double)steps));
  upper = (int)(1.5*((double)length/(double)steps));

  l = malloc(steps*sizeof(int));
  
  for (i = 0; i < steps; i++) {
    l[i] = lower +  (int)((double)rand()/(double)RAND_MAX * (upper-lower));
  }
  
  do {
    sorted = 1;
    
    for (i = 1; i < steps; i++) {
      if (l[i] > l[i-1]) {
        sorted = 0;
        tmp = l[i-1];
        l[i-1] = l[i];
        l[i] = tmp;
      }
    }
    
  } while (!sorted);
  
  for (i = 0; i < steps; i++) {
    for (j = 0; j < l[i]; j++) {
      fprintf(file, "%c", (i % 2 == 0)?'-':' ');
    }
  }
  
  fprintf(file, "\n");
  
  free(l);
 
  
}
