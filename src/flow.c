/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora flow control routines
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

#include "wasora.h"

int wasora_instruction_if(void *arg) {
  conditional_block_t *conditional_block = (conditional_block_t *)arg;
  int condition;

  condition = (int)wasora_evaluate_expression(conditional_block->condition);

  if (condition) {
    conditional_block->evaluated_to_true = 1;
    conditional_block->evaluated_to_false = 0;
    wasora.next_flow_instruction = conditional_block->first_true_instruction;
  } else {
    conditional_block->evaluated_to_true = 0;
    conditional_block->evaluated_to_false = 1;
    wasora.next_flow_instruction = conditional_block->first_false_instruction;
  }

  return WASORA_RUNTIME_OK;
}

int wasora_instruction_else(void *arg) {
  conditional_block_t *conditional_block = (conditional_block_t *)arg;

  if (conditional_block->else_of == NULL) {
    wasora_push_error_message("ooops");
    return WASORA_RUNTIME_ERROR;
  }

  if (conditional_block->else_of->evaluated_to_true) {
    wasora.next_flow_instruction = conditional_block->first_true_instruction;
  } else if (conditional_block->else_of->evaluated_to_false) {
    wasora.next_flow_instruction = conditional_block->first_false_instruction;
  } else {
    wasora_push_error_message("ooooooops");
    return WASORA_RUNTIME_ERROR;
  }

  return WASORA_RUNTIME_OK;
}

int wasora_instruction_endif(void *arg) {
  return WASORA_RUNTIME_OK;
}
