/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora error handling functions
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>

#include <gsl/gsl_errno.h>

#ifndef _WASORA_H_
#include "wasora.h"
#endif

void wasora_push_error_message(const char *fmt, ...) {
  
  va_list ap;
  
  assert(wasora.error_level<100);
  
  wasora.error = realloc(wasora.error, (++wasora.error_level)*sizeof(char *));
  wasora.error[wasora.error_level-1] = malloc(BUFFER_SIZE);
  
  va_start(ap, fmt);
  vsnprintf(wasora.error[wasora.error_level-1], BUFFER_SIZE, fmt, ap);
  va_end(ap);
  // TODO: si nos quedamos sin lugar hacer realloc
  
  return;
}

void wasora_pop_error_message(void) {
  
  if (wasora.error_level > 0) {
    free(wasora.error[wasora.error_level-1]);
    wasora.error_level--;
  }
  
  return;
}


void wasora_pop_errors(void) {

  fprintf(stderr, "error: ");
  if (wasora.error_level == 0) {
    fprintf(stderr, "unspecified error\n");
  } else {
    while (wasora.error_level > 0) {
      fprintf(stderr, "%s%s", wasora.error[wasora.error_level-1], (wasora.error_level != 1)?" ":"\n");
      wasora_pop_error_message();
    }
  }

  return;
}


void wasora_runtime_error(void) {

  wasora_pop_errors();
  wasora_polite_exit(WASORA_RUNTIME_ERROR);

  return;
  
}

void wasora_nan_error(void) {

  int on_nan = (int)(wasora_value(wasora_special_var(on_nan)));

  if (!(on_nan & ON_ERROR_NO_REPORT)) {
    fprintf(stderr, "error: NaN found\n");
  }

  if (!(on_nan & ON_ERROR_NO_QUIT)) {
    wasora_runtime_error();
  }

  return;
  
}


void wasora_gsl_handler (const char *reason, const char *file_ptr, int line, int gsl_errno) {

  
  int on_gsl_error = (int)(wasora_value(wasora_special_var(on_gsl_error)));

  if (!(on_gsl_error & ON_ERROR_NO_REPORT)) {
    wasora_push_error_message("gsl error #%d '%s' in %s", gsl_errno, reason, file_ptr);
    wasora_pop_errors();
  }

  if (!(on_gsl_error & ON_ERROR_NO_QUIT)) {
    wasora_polite_exit(WASORA_RUNTIME_ERROR);
  }

  return;
}


void wasora_signal_handler(int sig_num) {

  fprintf(stderr, "\npid %d: signal #%d caught, finnishing... ", getpid(), sig_num);
  fflush(stderr);

  wasora_var(wasora_special_var(done)) = (double)1;

/*
  if (wasora.fork_sem != NULL) {
    wasora_free_semaphore(wasora.fork_sem, wasora.fork_sem_name);
  }
*/

  fprintf(stderr, "ok\n");
  wasora_polite_exit(EXIT_SUCCESS);
  exit(1);

}
