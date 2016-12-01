/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora M4 routines
 *
 *  Copyright (C) 2015 jeremy theler
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
#include <string.h>

#include "wasora.h"


int wasora_instruction_m4(void *arg) {
  m4_t *m4 = (m4_t *)arg;
  m4_macro_t *macro;

  char *input_file_path;
  char *output_file_path;
  char *commandline = malloc(BUFFER_SIZE);
  char *delta = malloc(BUFFER_SIZE);
  char *templ = malloc(BUFFER_SIZE);
  int totalchars = 0;
  int deltachars = 0;
  
  
  if ((input_file_path = wasora_evaluate_string(m4->input_file->format, m4->input_file->n_args, m4->input_file->arg)) == NULL) {
    return WASORA_RUNTIME_ERROR;
  }
  if ((output_file_path = wasora_evaluate_string(m4->output_file->format, m4->output_file->n_args, m4->output_file->arg)) == NULL) {
    return WASORA_RUNTIME_ERROR;
  }
  
  totalchars = sprintf(commandline, "m4");
  
  LL_FOREACH(m4->macros, macro) {
    if (macro->print_token.text != NULL) {
      if ((deltachars = snprintf(delta, BUFFER_SIZE, " -D%s=\"%s\"", macro->name, macro->print_token.text)) >= BUFFER_SIZE) {
        wasora_push_error_message("buffer not big enough");
        return WASORA_RUNTIME_ERROR;
      }
    } else {
      // TODO: ver como hacer esto de una
      if (snprintf(templ, BUFFER_SIZE, " -D%%s=\"%s\"", macro->print_token.format) >= BUFFER_SIZE) {
        wasora_push_error_message("buffer not big enough");
        return WASORA_RUNTIME_ERROR;
      }
      if ((deltachars = snprintf(delta, BUFFER_SIZE, templ, macro->name, wasora_evaluate_expression(&macro->print_token.expression))) >= BUFFER_SIZE) {
        wasora_push_error_message("buffer not big enough");
        return WASORA_RUNTIME_ERROR;
      }
    }
    if ((totalchars += deltachars) > BUFFER_SIZE) {
      commandline = realloc(commandline, totalchars + BUFFER_SIZE);
    }
    strcat(commandline, delta);
  }
  
  sprintf(delta, " %s > %s", input_file_path, output_file_path);
  strcat(commandline, delta);
  
  if (system(commandline) != 0) {
    return WASORA_RUNTIME_ERROR;
  };
  
  free(templ);
  free(delta);
  free(commandline);
  
  return WASORA_RUNTIME_OK;
  
}
