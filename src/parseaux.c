/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora auxiliary parsing routines
 *
 *  Copyright (C) 2009--2020 jeremy theler
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
#include <ctype.h>

#ifndef _WASORA_H_
#include "wasora.h"
#endif


int wasora_parser_expression(expr_t *expr) {
  
  char *token;
  
  if ((token = wasora_get_next_token(NULL)) == NULL) {
    wasora_push_error_message("expected expression");
    return WASORA_PARSER_ERROR;
  }
  
  if (wasora_parse_expression(token, expr) != WASORA_PARSER_OK) {
    return WASORA_PARSER_ERROR;
  }
  
  return WASORA_PARSER_OK;
}

int wasora_parser_expressions(expr_t *expr[], size_t n) {
  
  char *token;
  int i;
  
  *expr = calloc(n, sizeof(expr_t));
  
  for (i = 0; i < n; i++) {
    if ((token = wasora_get_next_token(NULL)) == NULL) {
      wasora_push_error_message("expected expression");
      return WASORA_PARSER_ERROR;
    }
  
    if (wasora_parse_expression(token, (*expr)+i) != WASORA_PARSER_OK) {
      return WASORA_PARSER_ERROR;
    }
  }
  
  return WASORA_PARSER_OK;
}

int wasora_parser_match_keyword_expression(char *token, char *keyword[], expr_t *expr[], size_t n) {

  int i;
  int found = 0;
  
  for (i = 0; i < n; i++) {
    if (strcasecmp(token, keyword[i]) == 0) {

      found = 1;
      
      if ((token = wasora_get_next_token(NULL)) == NULL) {
        wasora_push_error_message("expected expression");
        return WASORA_PARSER_ERROR;
      }
              
      if (wasora_parse_expression(token, expr[i]) != WASORA_PARSER_OK) {
        return WASORA_PARSER_ERROR;
      }
    }
  }
          
  if (found == 0) {
    return WASORA_PARSER_UNHANDLED;
  }
  
  return WASORA_PARSER_OK;
}


int wasora_parser_expression_in_string(double *result) {
  
  char *token;
  
  if ((token = wasora_get_next_token(NULL)) == NULL) {
    wasora_push_error_message("expected expression");
    return WASORA_PARSER_ERROR;
  }
  
  *result = wasora_evaluate_expression_in_string(token);
  
  return WASORA_PARSER_OK;
}

int wasora_parser_string(char **string) {
  
  char *token;
  
  if ((token = wasora_get_next_token(NULL)) == NULL) {
    wasora_push_error_message("expected string");
    return WASORA_PARSER_ERROR;
  }
  
  *string = strdup(token);
  
  return WASORA_PARSER_OK;
}

int wasora_parser_string_format(char **string, int *n_args) {

  char *token;
  char *dummy;
  
  *n_args = 0;
  
  if ((token = wasora_get_next_token(NULL)) == NULL) {
    wasora_push_error_message("expected a string with optional printf format data");
    return WASORA_PARSER_ERROR;
  }
  *string = strdup(token);
      
  dummy = *string;
  while (*dummy != '\0') {
    if (*dummy == '\\') {
      dummy++;
    } else if (*dummy == '%' && *(dummy+1) == '%') {
      dummy++;
    } else if (*dummy == '%') {
      if (dummy[1] == 'd') {
        dummy[1] = 'g';
      }
      (*n_args)++;
    }
    dummy++;
  }
  
  return WASORA_PARSER_OK;
  
}

int wasora_parser_file(file_t **file) {
  
  char *token;
  
  if ((token = wasora_get_next_token(NULL)) == NULL) {
    wasora_push_error_message("expected file identifier");
    return WASORA_PARSER_ERROR;
  }

  if ((*file = wasora_get_file_ptr(token)) == NULL) {
    wasora_push_error_message("undefined file identifier '%s'", token);
    return WASORA_PARSER_ERROR;
  }
  
  // si alguien hace referencia a stdin entonces no nos metemos en debug
  if (*file == wasora.special_files.stdin_) {
    wasora.mode = mode_ignore_debug;
  }
  
  return WASORA_PARSER_OK;
}


int wasora_parser_file_path(file_t **file, char *mode) {
  
  char *token;
  if (wasora_parser_string(&token) != WASORA_PARSER_OK) {
    return WASORA_PARSER_ERROR;
  };
  
  if ((*file = wasora_define_file(token, token, 0, NULL, mode, 0)) == NULL) {
    return WASORA_RUNTIME_ERROR;
  }
  free(token);
  
  return WASORA_PARSER_OK;
}

int wasora_parser_function(function_t **function) {
  
  char *token;
  
  if ((token = wasora_get_next_token(NULL)) == NULL) {
    wasora_push_error_message("expected function name");
    return WASORA_PARSER_ERROR;
  }

  if ((*function = wasora_get_function_ptr(token)) == NULL) {
    wasora_push_error_message("undefined function identifier '%s' (remember that only the function name is needed, not the arguments)", token);
    return WASORA_PARSER_ERROR;
  }
  
  return WASORA_PARSER_OK;
}

int wasora_parser_vector(vector_t **vector) {
  
  char *token;
  
  if ((token = wasora_get_next_token(NULL)) == NULL) {
    wasora_push_error_message("expected vector name");
    return WASORA_PARSER_ERROR;
  }

  if ((*vector = wasora_get_vector_ptr(token)) == NULL) {
    wasora_push_error_message("undefined vector identifier '%s'", token);
    return WASORA_PARSER_ERROR;
  }
  
  return WASORA_PARSER_OK;
}

int wasora_parser_variable(var_t **var) {
  
  char *token;
  
  if ((token = wasora_get_next_token(NULL)) == NULL) {
    wasora_push_error_message("expected variable name");
    return WASORA_PARSER_ERROR;
  }

  if ((*var = wasora_get_variable_ptr(token)) == NULL) {
    wasora_push_error_message("undefined variable identifier '%s'", token);
    return WASORA_PARSER_ERROR;
  }
  
  return WASORA_PARSER_OK;
}


int wasora_parser_keywords_ints(char *keyword[], int *value, int *option) {

  char *token;
  int i = 0;
  
  if ((token = wasora_get_next_token(NULL)) == NULL) {
    while (keyword[i][0] != '\0') {
      wasora_push_error_message("%s", keyword[i++]);
    }
    wasora_push_error_message("expected one of");
    return WASORA_PARSER_ERROR;
  }
  
  while (keyword[i][0] != '\0') {
    if (strcasecmp(token, keyword[i]) == 0) {
      *option = value[i];
      return WASORA_PARSER_OK;
    }
    i++;
  }
  
  i = 0;
  while (keyword[i][0] != '\0') {
    wasora_push_error_message("%s", keyword[i++]);
  }
  wasora_push_error_message("unknown keyword '%s', expected one of", token);
  
  return WASORA_PARSER_ERROR;
  
}


int wasora_parser_read_keywords_voids(char *keyword[], void *value[], void **option) {

  char *token;
  int i = 0;
  
  if ((token = wasora_get_next_token(NULL)) == NULL) {
    while (keyword[i][0] != '\0') {
      wasora_push_error_message("%s", keyword[i++]);
    }
    wasora_push_error_message("expected one of");
    return WASORA_PARSER_ERROR;
  }
  
  while (keyword[i][0] != '\0') {
    if (strcasecmp(token, keyword[i]) == 0) {
      *option = value[i];
      return WASORA_PARSER_OK;
    }
    i++;
  }
  
  i = 0;
  while (keyword[i][0] != '\0') {
    wasora_push_error_message("%s", keyword[i++]);
  }
  wasora_push_error_message("unknown keyword '%s', expected one of", token);
  
  return WASORA_PARSER_ERROR;
  
}






// strtok modificado que mira si el proximo token arranca con comilla o no
// y se aguanta leer cosas entre mismas
char *wasora_get_next_token(char *line) {

  int i;
  int n;
  char *token;

  // MAMA! lo que renegue para sacar la lectura invalida de este rutina que
  // acusaba el valgrind! el chiste es asi: cuando se termina de parsear una
  // linea vieja, el internal apunta a cualquier fruta (diferente de NULL)
  // entonces no vale mirar que tiene. La solucion que se me ocurrio es que si
  // line es diferente de NULL estamos empezando una linea nueva, eso quiere
  // decir que internal apunta a fruta entonces lo hacemos igual a NULL y usamos
  // los delimitadores sin comillas, porque total es una keyword
  if (line != NULL) {
    wasora.strtok_internal = NULL;
  }


  if (wasora.strtok_internal == NULL) {
    token = strtok_r(line, UNQUOTED_DELIM, &(wasora.strtok_internal));
  } else {
    if (*wasora.strtok_internal == '"') {
      token = strtok_r(line, QUOTED_DELIM, &(wasora.strtok_internal));

      // si es quoted, barremos token y reemplazamos el caracter 255 por un quote
      n = strlen(token);
      for (i = 0; i < n; i++) {
        if (token[i] == -1 || token[i] == 255) {
          token[i] = '"';
        }
      }
    } else {
      token = strtok_r(line, UNQUOTED_DELIM, &(wasora.strtok_internal));
    }
  }

  // avanzamos el apuntador strtok_internal hasta el siguiente caracter no-blanco
  // asi si la cadena q sigue empieza con comillas, la ve bien el if de arriba
  // sino si hay muchos espacios en blanco el tipo piensa que el token esta unquoted
  // el if de que no sea null saltaba solamente en windoze, ye ne se pa
  if (wasora.strtok_internal != NULL) {
    while (*wasora.strtok_internal == ' ' || *wasora.strtok_internal == '\t') {
      wasora.strtok_internal++;
    }
  }

  // esto casi nunca pasa, pero si nos quedamos sin tokens entonces otra vez
  // internal apunta a cualquier lado 
  // OJO! a partir de algun update de la libc si dejamos esto tenemos segfault cuando
  // algun primary keyword consume todas las keywords de la linea como PHYSICAL_ENTITY BC strings
/*  
  if (token == NULL) {
    wasora.strtok_internal = NULL;
  }
*/
  return token;

}







// parsea el rango de indices 
int wasora_parse_range(char *string, const char left_delim, const char middle_delim, const char right_delim, expr_t *a, expr_t *b) {
  char *first_bracket;
  char *second_bracket;
  char *colon;

  if ((first_bracket = strchr(string, left_delim)) == NULL) {
    wasora_push_error_message("range '%s' does not start with '%c'", string, left_delim);
    return WASORA_PARSER_ERROR;
  }

  if ((second_bracket = strrchr(string, right_delim)) == NULL) {
    wasora_push_error_message("unmatched '%c' for range in '%s'", left_delim, string);
    return WASORA_PARSER_ERROR;
  }

  *second_bracket = '\0';

  if ((colon = strchr(string, middle_delim)) == NULL) {
    wasora_push_error_message("delimiter '%c' not found when giving range", middle_delim);
    return WASORA_PARSER_ERROR;
  }
  *colon = '\0';

  if (wasora_parse_expression(first_bracket+1, a) != 0) {
    wasora_push_error_message("in min range expression");
    return WASORA_PARSER_ERROR;
  }
  if (wasora_parse_expression(colon+1, b) != 0) {
    wasora_push_error_message("in max range expression");
    return WASORA_PARSER_ERROR;
  }
  
  *second_bracket = right_delim;;
  *colon = middle_delim;
          
  return WASORA_PARSER_OK;
}


// saca los comentarios y los espacios en blanco iniciales de una linea
void wasora_strip_comments(char *line) {
  int i = 0;
  int j = 0;
  char *buff = strdup(line);

  // saca los espacios y tabs iniciales
  while (isspace((int)buff[i])) {
    i++;
  }

//  while ((buff[i] != '#' && buff[i] != '\r' && buff[i] != '\n' && buff[i] != '\0') || (i > 0 && buff[i] == '#' && buff[i-1] == '\\')) {
  while ((buff[i] != '#' && buff[i] != '\0') || (i > 0 && buff[i] == '#' && buff[i-1] == '\\')) {  
    if (i > 0 && buff[i] == '#' && buff[i-1] == '\\') {
      j--;
    }
    line[j++] = buff[i++];
  }

  line[j] = '\0';

  free(buff);

  return;
}


// lee una linea del input y la devuelve redonda
int wasora_read_line(FILE *file_ptr) {

  int i;
  int c;
  int in_comment = 0;
  int in_brackets = 0;
  int lines = 0;

  // ignoramos los espacios iniciales
  do {
    c = fgetc(file_ptr);
    // pero si encontramos un enter volvemos
    if (c == '\n') {
      wasora.line[0] = '\0';
      return 1;
    }

  } while (isspace(c));

  i = 0;
  while ( !((c == EOF) || (in_brackets == 0 && c == '\n')) ) {
    if (in_comment == 0) {
      if (c == '#' || c == ';') {
        in_comment = 1;
      } else if (c == '{') {
        in_brackets = 1;
      } else if (c == '}') {
        in_brackets = 0;
      } else if (c == '$') {
        // commandline arguments
        int j, n;
        if (fscanf(file_ptr, "%d", &n) != 1) {
          wasora_push_error_message("failed to match $n");
          return (lines==0 && i!=0)?-1:-lines;
        }
        if (wasora.optind+n >= wasora.argc) {
          // nos llamamos recursivamente asi terminamos de leer la linea
          wasora_read_line(file_ptr);
          // nos aseguramos de que line[0] no sea '\0'
          if (wasora.line[0] == '\0') {
            wasora.line[0] = ' ';
            wasora.line[1] = '\0';
           }
          return (lines==0 && i!=0)?-1:-lines;
        }

        j = 0;
        if (wasora.argv[wasora.optind+n] == NULL) {
          return WASORA_PARSER_ERROR;
        }
        while (wasora.argv[wasora.optind+n][j] != 0) {
          // ojo! no se puede meter en una linea porque me puede llegar a meter
          // el \0 en line[] antes de evaluar la condicion para salir del while
          wasora.line[i++] = wasora.argv[wasora.optind+n][j++];
        }

      } else if (c == '\\') {

        switch (c = fgetc(file_ptr)) {
          case '"':
            // si hay una comilla escapeada, le sacamos el escape y ponemos
            // un marcador magico igual a 255, despues en get_next_token volvemos
            // a reemplazar 255 por la comilla sin escapear
            wasora.line[i++] = 0xff;
          break;
            // escape sequences
          case 'a':
            wasora.line[i++] = '\a';
          break;
          case 'b':
            wasora.line[i++] = '\b';
          break;
          case 'n':
            wasora.line[i++] = '\n';
          break;
          case 'r':
            wasora.line[i++] = '\r';
          break;
          case 't':
            wasora.line[i++] = '\t';
          break;
          case 'v':
            wasora.line[i++] = '\v';
          break;
          case '\n':
            // las newline escapeadas las tomamos como continuaciones
            // asi que no hay que hacer nada, excepto sumar una linea
            lines++; 
          break;
          case '\\':
            // en verdad este esta tenido en cuenta en el default,
            // pero lo ponemos explicitamente por si acaso
            wasora.line[i++] = '\\';
          break;
          // TODO: hex representation
          default:
            wasora.line[i++] = c;
          break;
        }
//      } else if (!in_comment && !escaped_quote && c != '{' && c != '}' && c != '\n' && c != '\r' && c != EOF) {
      } else if (!in_comment && c != '\n' && c != '\r' && c != EOF) {
        wasora.line[i++] = c;
      } else if (in_brackets && c == '\n') {
        // mantenemos los newlines para python
        wasora.line[i++] = '\n';
      }
    }

    // salimos del estado de comentario si c es newline
    if (c == '\n') {
      in_comment = 0;
    }

    // vemos si tenemos que reallocar el buffer
    if (i == wasora.actual_buffer_size-1) {
      wasora.actual_buffer_size += wasora.page_size;
      wasora.line = realloc(wasora.line, wasora.actual_buffer_size);
    }

    // pedimos lo que viene en futbol de primera
    // contamos las lineas aca porque si nos toca \n nos vamos sin comerla ni beberla

    if ((c = fgetc(file_ptr)) == '\n') {
      lines++;
    }

  }
  
  wasora.line[i] = '\0';
  
//  wasora_strip_comments(wasora.line);

  // si lines es 0 pero leimos algo devolvemos uno, porque sino nos comemos 
  // las ultimas lineas de los archivos que no terminan en \n

  return (lines==0 && i!=0)?1:lines;

}


// lee una linea y eventualmente procesa los {}
int wasora_read_data_line(FILE *file_ptr, char *buffer) {

  int l;
  int lines = 1;

  if (fgets(buffer, BUFFER_SIZE*BUFFER_SIZE, file_ptr) == NULL) {
    return 0;
  }
  
  wasora_strip_comments(buffer); // limpiamos la linea

  // TODO: explicar esto!
  // si no tiene "{" entonces se curte
  if (strchr(buffer, '{') == NULL) {
    return 1;
  }

  do {
    l = strlen(buffer);
    if (fgets(buffer + l + 1, (BUFFER_SIZE*BUFFER_SIZE)-1, file_ptr) == 0) {
      return WASORA_PARSER_ERROR;
    }
    buffer[l] = ' ';
    wasora_strip_comments(buffer);  // limpiamos la linea
    lines++;
  } while (strchr(buffer, '}') == NULL);

//  wasora_strip_brackets(buffer);

  return lines;

}

// saca los blancos de una cadena (inline)
void wasora_strip_blanks(char *string) {
  int i = 0;
  int j = 0;
  // capaz se pueda hacer sin duplicar string, pero por
  // si acaso vamos a lo seguro
  char *buff;

  buff = strdup(string);

  for (i = 0; i < strlen(string); i++) {
    if (!isspace((int)buff[i])) {
      string[j++] = buff[i];
    }
  }

  string[j] = '\0';

  free(buff);

  return;

}


// cambia construcciones del tipo (-variable por (0-variable 
// el argumento es **string porque eventualmente necesitamos hacer un realloc
// del puntero original
void wasora_add_leading_zeros(char **string) {
  int i = 0;
  int j = 0;
  int chars_to_add = 0;
  char *buff;

  if ((*string)[0] == '-' || (*string)[0] == '+') {
    chars_to_add = 1;
  }
  if (strlen(*string) > 2) {
    for (i = 0; i < strlen(*string)-2; i++) {
      if ((*string)[i] == '(' && ((*string)[i+1] == '-' || (*string)[i+1] == '+') && !isdigit((int)(*string)[i+2])) {
        chars_to_add++;
      }
    }
  }

  if (chars_to_add == 0) {
    return;
  }

  buff = malloc(strlen(*string)+chars_to_add+2);


  if ((*string)[0] == '-' || (*string)[0] == '+') {
    buff[j++] = '0';
  }
  if (strlen(*string) > 2) {
    for (i = 0; i < strlen(*string)-2; i++) {
      if ((*string)[i] == '(' && ((*string)[i+1] == '-' || (*string)[i+1] == '+') && !isdigit((int)(*string)[i+2])) {
        buff[j++] = (*string)[i];
        buff[j++] = '0';
      } else {
        buff[j++] = (*string)[i];
      }
    }
  }
  buff[j++] = (*string)[i++];
  buff[j++] = (*string)[i++];
  buff[j++] = '\0';


  *string = realloc(*string, j+1);
  strcpy(*string, buff);

  free(buff);

  return;

}


char *wasora_ends_in_zero(char *name) {

  char *dummy;

  if (((dummy = strstr(name, "_0")) != 0) && (*(dummy+2) == 0)) {
    return dummy;
  } else {
    return NULL;
  }
  
}

char *wasora_ends_in_init(char *name) {

  char *dummy;

  if (((dummy = strstr(name, "_init")) != 0) && (dummy[5] == '\0' || dummy[5] == '(')) {
    return dummy;
  } else {
    return NULL;
  }
}

/*
char *wasora_ends_in_dot(char *name) {

  char *dummy;

  if (((dummy = strstr(name, "_dot")) != 0) && (dummy[4] == '\0' || dummy[4] == '(')) {
    return dummy;
  } else {
    return NULL;
  }
}
*/
int wasora_count_arguments(char *string) {

  int n_arguments;
  int level;
  char *dummy;

  // los argumentos tienen que estar entre parentesis
  if (string[0] != '(') {
    wasora_push_error_message("expected arguments for function");
    return WASORA_PARSER_ERROR;
  }

  // contamos cuantos argumentos hay, teniendo en cuenta posibles
  // parentesis dentro de los argumentos 
  dummy = string+1;
  level = 1;
  n_arguments = 1;
  while (level != 0) {
    if (*dummy == '(') {
      level++;
    } else if (*dummy == ')') {
      level--;
    } else if (*dummy == '\0') {
      wasora_push_error_message("argument list needs to be closed with ')'");
      return WASORA_PARSER_ERROR;
    }
    if (*dummy == ',' && level == 1) {
      n_arguments++;
    }
    dummy++;
  }
  *dummy = '\0';
  
  return n_arguments;
}

int wasora_parse_assignment(char *line, assignment_t *assignment) {
  
  char *dummy;
  char *left_hand;
  char *right_hand;
  vector_t *vector = NULL;
  matrix_t *matrix = NULL;
  char *dummy_at;
  char *at_expression;
  int at_offset;
  char *dummy_time_range;
  char *dummy_index_range;
  char *dummy_par;
  char *dummy_init;
  char *dummy_0;
  
  // comenzamos suponiendo que es plain
  assignment->plain = 1;
  
  // left hand
  left_hand = strdup(line);
  dummy = strchr(left_hand, '=');
  *dummy = '\0';
  
  // primero hacemos que el right hand sea dummy+1
  right_hand = strdup(dummy+1);
  wasora_strip_blanks(right_hand);
  if (strlen(right_hand) == 0) {
    wasora_push_error_message("empty right-hand side");
    return WASORA_PARSER_ERROR;
  }
  
  // y despues sacamos blancos y otras porquerias que nos molestan
  wasora_strip_blanks(left_hand);
  
  // este es el orden inverso en el que se tienen que dar los subindices
  // por ejemplo un construcciones validas serian
  // matrix_0(i,j)<1:3;2:4> = algo
  // matrix(i,j)<1:3;2:4>[0.99:1.01] = algo
  
  // como ultima cosa, la arroba para hacer steps
  if ((dummy_at = strchr(left_hand, '@')) != NULL) {
    at_offset = dummy_at - left_hand;
    at_expression = strdup(dummy_at+1);
    left_hand = realloc(left_hand, strlen(left_hand) + 2*strlen(at_expression) + 32);
    sprintf(left_hand+at_offset, "[(%s)-0.1*dt:%s+(0.1)*dt]", at_expression, at_expression);
    free(at_expression);
  }
  
  // como anteultima cosa, el rango en tiempo
  if ((dummy_time_range = strchr(left_hand, '[')) != NULL) {
    *dummy_time_range = '\0';
  }
  
  // el rango en indices
  if ((dummy_index_range = strchr(left_hand, '<')) != NULL) {
    *dummy_index_range = '\0';
  }
  
  // despues los subindices entre parentesis si es un vector o matriz
  if ((dummy_par = strchr(left_hand, '(')) != NULL) {
    *dummy_par = '\0';
  }
  
  // despues _init o _0
  if ((dummy_init = wasora_ends_in_init(left_hand)) != NULL) {
    *dummy_init = '\0';
    assignment->initial_static = 1;
  }
  
  if ((dummy_0 = wasora_ends_in_zero(left_hand)) != NULL) {
    *dummy_0 = '\0';
    assignment->initial_transient = 1;
  }
  
  if ((matrix = wasora_get_matrix_ptr(left_hand)) != NULL) {
    assignment->matrix = matrix;
  } else if ((vector = wasora_get_vector_ptr(left_hand)) != NULL) {
    assignment->vector = vector;
  } else {
    // buscamos una variable, y si no existe vemos si tenemos que declararla implicitamente
    if ((assignment->variable = wasora_get_variable_ptr(left_hand)) == NULL) {
      if (wasora.implicit_none) {
        wasora_push_error_message("undefined symbol '%s' and disabled implicit definition", left_hand);
        return WASORA_PARSER_ERROR;
      } else {
        if ((assignment->variable = wasora_define_variable(left_hand)) == NULL) {
          return WASORA_PARSER_ERROR;
        } else {
          // las variables siempre son escalares
          assignment->plain = 0;
          assignment->scalar = 1;
        }
      }
    } else {
      // las variables siempre son escalares
      assignment->plain = 0;
      assignment->scalar = 1;
    }
  }
  
  // ahora vamos de izquierda a derecha
  // reconstruimos el string
  if (dummy_0 != NULL) {
    *dummy_0 = '_';
  }
  if (dummy_init != NULL) {
    *dummy_init = '_';
  }
  
  // si hay parentesis son subindices
  if (dummy_par != NULL) {
    if (matrix == NULL && vector == NULL) {
      if (wasora_get_function_ptr(left_hand) != NULL) {
        wasora_push_error_message("functions are defined using ':=' instead of '='", left_hand);  
      } else {
        wasora_push_error_message("'%s' is neither a vector nor a matrix", left_hand);
      }
      return WASORA_PARSER_ERROR;
    }
    *dummy_par = '(';
    
    if (matrix != NULL) {
      // si aparecen o la letra "i" o la letra "j" entonces no es escalar
      assignment->scalar = (strchr(dummy_par, 'i') == NULL) && (strchr(dummy_par, 'j') == NULL);
      // si aparece la letra "i" y no la letra "j" entonces se varia solo las filas
      assignment->expression_only_of_i = (strchr(dummy_par, 'j') == NULL) && (strchr(dummy_par, 'i') != NULL);
      // si no aparece la letra "i" pero si la letra "j" entonces se varia solo las columnas
      assignment->expression_only_of_j = (strchr(dummy_par, 'i') == NULL) && (strchr(dummy_par, 'j') != NULL);
      // si no es es "i,j" entonces no es plain
      if (strcmp("(i,j)", dummy_par) != 0) {
        assignment->plain = 0;
      }
      wasora_call(wasora_parse_range(dummy_par, '(', ',', ')', &assignment->row, &assignment->col));
    } else if (vector != NULL) {
      if ((dummy = strrchr(dummy_par, ')')) == NULL) {
        wasora_push_error_message("unmatched parenthesis for '%s'", left_hand);
        return WASORA_PARSER_ERROR;
      }
      *dummy = '\0';
      
      // si aparece la letra "i" entonces no es escalar
      assignment->scalar = strchr(dummy_par+1, 'i') == NULL;
      
      // si no es "i" entonces no es plain
      if (strcmp("i", dummy_par+1) != 0) {
        assignment->plain = 0;
      }
      wasora_call(wasora_parse_expression(dummy_par+1, &assignment->row));
      *dummy = ')';
    }
  }
  
  // si hay rango de indices
  if (dummy_index_range != NULL) {
    if (matrix == NULL && vector == NULL) {
      if (wasora_get_function_ptr(left_hand) != NULL) {
        wasora_push_error_message("functions are defined using ':=' instead of '='", left_hand);  
      } else {
        wasora_push_error_message("'%s' is neither a vector nor a matrix", left_hand);
      }
      return WASORA_PARSER_ERROR;
    }
    *dummy_index_range = '<';
    
    if (matrix != NULL) {
      wasora_call(wasora_parse_range(dummy_index_range, '<', ':', ';', &assignment->i_min, &assignment->i_max));
      wasora_call(wasora_parse_range(strchr(dummy_index_range, ';'), ';', ':', '>', &assignment->j_min, &assignment->j_max));
    } else if (vector != NULL) {
      wasora_call(wasora_parse_range(dummy_index_range, '<', ':', '>', &assignment->i_min, &assignment->i_max));
    }
  }
  
  // rango de tiempo
  if (dummy_time_range != NULL)  {
    *dummy_time_range = '[';
    wasora_call(wasora_parse_range(dummy_time_range, '[', ':', ']', &assignment->t_min, &assignment->t_max));
  }
  
  wasora_call(wasora_parse_expression(right_hand, &assignment->rhs));
  
  free(left_hand);
  free(right_hand);
  
  return WASORA_RUNTIME_OK;
}
