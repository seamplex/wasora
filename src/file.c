/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora file access routines
 *
 *  Copyright (C) 2013-2014 jeremy theler
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
#include <errno.h>
#include <string.h>
#include <libgen.h>


#include "wasora.h"


char *wasora_evaluate_string(char *format, int nargs, expr_t *arg) {

  char *string = malloc(strlen(format) + 1 + nargs*32);
/*  double *evaluated_args = malloc(nargs * sizeof(double));
  
  for (i = 0; i < nargs; i++) {
    evaluated_args[i] = wasora_evaluate_expression(&arg[i]);
  }
*/  
  
  switch (nargs) {
    case 0:
      sprintf(string, "%s", format);
    break;
    case 1:
      sprintf(string, format,
              wasora_evaluate_expression(&arg[0]));
    break;
    case 2:
      sprintf(string, format,
              wasora_evaluate_expression(&arg[0]),
              wasora_evaluate_expression(&arg[1]));
    break;
    case 3:
      sprintf(string, format,
              wasora_evaluate_expression(&arg[0]),
              wasora_evaluate_expression(&arg[1]),
              wasora_evaluate_expression(&arg[2]));
    break;
    case 4:
      sprintf(string, format,
              wasora_evaluate_expression(&arg[0]),
              wasora_evaluate_expression(&arg[1]),
              wasora_evaluate_expression(&arg[2]),
              wasora_evaluate_expression(&arg[3]));
    break;
    case 5:
      sprintf(string, format,
              wasora_evaluate_expression(&arg[0]),
              wasora_evaluate_expression(&arg[1]),
              wasora_evaluate_expression(&arg[2]),
              wasora_evaluate_expression(&arg[3]),
              wasora_evaluate_expression(&arg[4]));
    break;
    case 6:
      sprintf(string, format,
              wasora_evaluate_expression(&arg[0]),
              wasora_evaluate_expression(&arg[1]),
              wasora_evaluate_expression(&arg[2]),
              wasora_evaluate_expression(&arg[3]),
              wasora_evaluate_expression(&arg[4]),
              wasora_evaluate_expression(&arg[5]));
    break;
    case 7:
      sprintf(string, format,
              wasora_evaluate_expression(&arg[0]),
              wasora_evaluate_expression(&arg[1]),
              wasora_evaluate_expression(&arg[2]),
              wasora_evaluate_expression(&arg[3]),
              wasora_evaluate_expression(&arg[4]),
              wasora_evaluate_expression(&arg[5]),
              wasora_evaluate_expression(&arg[6]));
    break;
    case 8:
      sprintf(string, format,
              wasora_evaluate_expression(&arg[0]),
              wasora_evaluate_expression(&arg[1]),
              wasora_evaluate_expression(&arg[2]),
              wasora_evaluate_expression(&arg[3]),
              wasora_evaluate_expression(&arg[4]),
              wasora_evaluate_expression(&arg[5]),
              wasora_evaluate_expression(&arg[6]),
              wasora_evaluate_expression(&arg[7]));
    break;
    default:
      wasora_push_error_message("more than eight arguments for string");
      return NULL;
    break;
  }

  return string;
  
}


int wasora_instruction_open_file(void *arg) {
  
  file_t *file = (file_t *)arg;
  char *newfilepath;
  
  if ((newfilepath = wasora_evaluate_string(file->format, file->n_args, file->arg)) == NULL) {
    return WASORA_RUNTIME_ERROR;
  }

  if (file->pointer == NULL || file->path == NULL || strcmp(newfilepath, file->path) != 0) {
    file->path = realloc(file->path, strlen(newfilepath)+1);
    strcpy(file->path, newfilepath);
    if (file->pointer != NULL) {
      fclose(file->pointer);
      file->pointer = NULL;
    }
    if (file->mode == NULL) {
      wasora_push_error_message("unknown open mode for file '%s' ('%s')", file->name, file->path);
      return WASORA_RUNTIME_ERROR;
    }
    if ((file->pointer = wasora_fopen(file->path, file->mode)) == NULL) {
      wasora_push_error_message("'%s' when opening file '%s' with mode '%s'", strerror(errno), file->path, file->mode);
      return WASORA_RUNTIME_ERROR;
    }
  }
  
  free(newfilepath);
  return WASORA_RUNTIME_OK;
  
}


int wasora_instruction_close_file(void *arg) {
  
  file_t *file = (file_t *)arg;
  if (file->pointer != NULL) {
    fclose(file->pointer);
    file->pointer = NULL;
  }
  return WASORA_RUNTIME_OK;
  
}

int wasora_instruction_file(void *arg) {
  
  file_t *file = (file_t *)arg;
  
  if (file->do_not_open == 0) {
    wasora_call(wasora_instruction_open_file(arg));
  }
  
  return WASORA_RUNTIME_OK;
}

FILE *wasora_fopen(const char *filepath, const char *mode) {
  
  FILE *handle;
  char *newtry;
  
  if ((handle = fopen(filepath, mode)) != NULL) {
    return handle;
  }
  
  newtry = malloc(strlen(wasora.main_input_dirname) + 1 + strlen(filepath) + 1);
  sprintf(newtry, "%s/%s", wasora.main_input_dirname, filepath);
  
  handle = fopen(newtry, mode);
  free(newtry);
  return handle;
  
}
