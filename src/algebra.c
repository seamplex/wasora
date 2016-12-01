/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora algebraic expressions parsing & evaluation routines
 *
 *  Copyright (C) 2009--2015 jeremy theler
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
#include <ctype.h>
#include <string.h>

#include "wasora.h"

const char operators[]        = "&|=!<>+-*/^()";
const char factorseparators[] = "&|=!<>+-*/^(), \t\n";

// evalua la expresion del argumento y devuelve su valor
double wasora_evaluate_expression(expr_t *algebraic_expr) {

  int i;
  int level;
  int n_tokens;
  int index_i, index_j;
  factor_t *token;
  factor_t *E,*P;

  if (algebraic_expr == NULL || (token = algebraic_expr->token) == NULL ||(n_tokens = algebraic_expr->n_tokens) == 0 ) {
    return 0;
  }
  
  for (i = 0; i < n_tokens; i++) {
    token[i].tmp_level = token[i].level;

    switch(token[i].type & EXPR_BASICTYPE_MASK) {
      case EXPR_CONSTANT:
        token[i].value = token[i].constant;
      break;
        
      case EXPR_VARIABLE:
        switch (token[i].type) {
          case EXPR_VARIABLE | EXPR_CURRENT:
            token[i].value = wasora_value(token[i].variable);
          break;
          case EXPR_VARIABLE | EXPR_INITIAL_TRANSIENT:
            token[i].value = token[i].variable->initial_transient[0];
          break;
          case EXPR_VARIABLE | EXPR_INITIAL_STATIC:
            token[i].value = token[i].variable->initial_static[0];
          break;
        }
      break;
      
      case EXPR_VECTOR:
    
        if (!token[i].vector->initialized) {
          wasora_call(wasora_vector_init(token[i].vector));
        }
        
        index_i = lrint(wasora_evaluate_expression(&token[i].arg[0]));
        if (index_i <= 0 || index_i > token[i].vector->size) {
          wasora_push_error_message("subindex %d out of range for vector %s", index_i, token[i].vector->name);
          wasora_runtime_error();
          return 0;
        }

        switch (token[i].type) {
          case EXPR_VECTOR | EXPR_CURRENT:
            token[i].value = wasora_vector_get(token[i].vector, index_i-1);
          break;
          case EXPR_VECTOR | EXPR_INITIAL_TRANSIENT:
            token[i].value = wasora_vector_get_initial_transient(token[i].vector, index_i-1);
          break;
          case EXPR_VECTOR | EXPR_INITIAL_STATIC:
            token[i].value = wasora_vector_get_initial_static(token[i].vector, index_i-1);
          break;
        }
      break;
      
      case EXPR_MATRIX:
          
        if (!token[i].matrix->initialized) {
          wasora_call(wasora_matrix_init(token[i].matrix));
        }
    
        index_i = lrint(wasora_evaluate_expression(&token[i].arg[0]));
        if (index_i <= 0 || index_i > token[i].matrix->rows) {
          wasora_push_error_message("row subindex %d out of range for matrix %s", index_i, token[i].matrix->name);
          wasora_runtime_error();
          return 0;
        }
        index_j = (int)(round(wasora_evaluate_expression(&token[i].arg[1])));
        if (index_j <= 0 || index_j > token[i].matrix->cols) {
          wasora_push_error_message("column subindex %d out of range for matrix %s", index_j, token[i].matrix->name);
          wasora_runtime_error();
        }

        switch (token[i].type) {
          case EXPR_MATRIX | EXPR_CURRENT:
            token[i].value = gsl_matrix_get(wasora_value_ptr(token[i].matrix), index_i-1, index_j-1);
          break;
          case EXPR_MATRIX | EXPR_INITIAL_TRANSIENT:
            token[i].value = gsl_matrix_get(token[i].matrix->initial_transient, index_i-1, index_j-1);
          break;
          case EXPR_MATRIX | EXPR_INITIAL_STATIC:
            token[i].value = gsl_matrix_get(token[i].matrix->initial_static, index_i-1, index_j-1);
          break;
        }
      break;

      case EXPR_BUILTIN_FUNCTION:
        token[i].value = token[i].builtin_function->routine(&token[i]);
      break;
      case EXPR_BUILTIN_VECTORFUNCTION:
        token[i].value = token[i].builtin_vectorfunction->routine(token[i].vector_arg);
      break;
      case EXPR_BUILTIN_FUNCTIONAL:
        token[i].value = token[i].builtin_functional->routine(&token[i], token[i].functional_var_arg);
      break;
      case EXPR_FUNCTION:
        token[i].value = wasora_evaluate_factor_function(&token[i]);
      break;
    }
  }

  level = 0;
  for (i = 0; i < n_tokens; i++) {
    if (token[i].level > level) {
      level = token[i].level;
    }
  }

  while (level > 0) {

    for (E = P = token; E != &token[n_tokens]; E->tmp_level != 0 && !E->oper?P=E:0,++E) {
      if (E->tmp_level == level && E->oper != 0) {
        switch(operators[(E++)->oper-1]) {
          case '&':
            P->value = (int)P->value & (int)E->value;
          break;
          case '|':
            P->value = (int)P->value | (int)E->value;
          break;
          case '=':
           if (fabs(P->value) < 1 || fabs(E->value) < 1) {
             P->value = (fabs(P->value - E->value) < wasora_var(wasora_special_var(zero)))?1:0;
           } else {
             P->value = (gsl_fcmp(P->value, E->value, wasora_var(wasora_special_var(zero))) == 0)?1:0;
           }
          break;
          case '!':
           if (fabs(P->value) < 1 || fabs(E->value) < 1) {
             P->value = (fabs(P->value - E->value) < wasora_var(wasora_special_var(zero)))?0:1;
           } else {
             P->value = (gsl_fcmp(P->value, E->value, wasora_var(wasora_special_var(zero))) == 0)?0:1;
           }
          break;
          case '<':
            P->value = P->value < E->value;
          break;
          case '>':
            P->value = P->value > E->value;
          break;
          case '+':
            P->value = P->value + E->value;
          break;
          case '-':
            P->value = P->value - E->value;
          break;
          case '*':
            P->value = P->value * E->value;
          break;
          case '/':
            if (E->value == 0) {
              wasora_nan_error();
            }
            P->value = P->value / E->value;
          break;
          case '^':
            if (P->value == 0 && E->value == 0) {
              wasora_nan_error();
            }
            P->value = pow(P->value, E->value);
          break;
        }
        E->tmp_level = 0;
      }
    }

    level--;

  }

  if (gsl_isnan(token[0].value) || gsl_isinf(token[0].value)) {
    wasora_push_error_message("in '%s'", algebraic_expr->string);
    wasora_nan_error();
  }


  return token[0].value;

}



// evalua la expresion que hay en la cadena
// para eso genera una expresion temporal, la rellena y la evalua
double wasora_evaluate_expression_in_string(const char *string) {
  
  expr_t expr;
  double val;
  
  if (wasora_parse_expression(string, &expr) != 0) {
    return 0;
  }
  val = wasora_evaluate_expression(&expr);
  wasora_destroy_expression(&expr);
  
  return val;
}




//  parsea una cadena conteniendo una expresion algebraica y rellena la estructura algebraic_expr
int wasora_parse_expression(const char *string, expr_t *expr) {

  // conviene pasarle al wasora_parser.de expresiones una copia de string asi la puede romper como quiera
  char *string_local_copy = strdup(string);

  // y ademas aca la dejamos linda para que pueda ser parseada bien
  wasora_strip_blanks(string_local_copy);
  // le pasamos la direccion de string_local_copy porque tal vez haya que
  // hacer un realloc y el apuntador vaya a parar donde judas perdio el poncho
  wasora_add_leading_zeros(&string_local_copy);

  // nos copiamos el string por si acaso
  expr->string = strdup(string_local_copy);

  // primero evaluamos cuantos tokens hay que allocar (NULL en el segundo argumento)
  if ((expr->n_tokens = wasora_parse_madeup_expression(string_local_copy, NULL)) < 0) {
    free(string_local_copy);
    return WASORA_PARSER_ERROR;
  }

  // alocamos los tokens
  expr->token = calloc(expr->n_tokens, sizeof(factor_t));

  // y los rellenamos
  if ((wasora_parse_madeup_expression(string_local_copy, expr->token)) < 0) {
    free(expr->token);
    free(string_local_copy);
    return WASORA_PARSER_ERROR;
  }

  free(string_local_copy);

  return WASORA_PARSER_OK;
}


// parsea una expresion ya maquillada
// si expr == null entones solamente contamos cuantos tokens hays
// si expr != null rellenamos los tokens
int wasora_parse_madeup_expression(char *string, factor_t *expr) {

  char *current_op;
  int level = 1;
  int count = 0;
  int last_one_operator = 0;
  int n;

  while (*string != '\0') {

    if ((current_op = strchr(operators, *string)) != NULL) {
      if (*string == '(') {
        level += sizeof(operators);
        string++;
      } else if (*string == ')') {
        level -= sizeof(operators);
        string++;
      } else {       // +-*/^.
        // si hay un signo y es el primer termino o el anterior ya es un
        // operador, entonces el signo forma parte del factor
        // esto no deberia pasar, para eso rellenamos con ceros al principio
        if ((count == 0) || last_one_operator != 0) {
          // una constante, variable o funcion
          if ((n = wasora_parse_factor(string, expr)) == -1) {
            return WASORA_PARSER_ERROR;
          }
          if (expr != NULL) {
            expr->level = level;
            expr++;
          }
          string += n;
          last_one_operator = 0;
          count++;
        } else {
          last_one_operator = 1;
          count++;
          if (expr != NULL) {
            expr->type = EXPR_OPERATOR;
            expr->oper = current_op-operators+1;
            // precedencia de a dos, izquierda a derecha
            expr->level = level+(current_op-operators)/2*2; 
            expr++;
          }
          string++;
        }
      }
    } else {
      // una constante, variable o funcion
      if ((n = wasora_parse_factor(string, expr)) == -1) {
        return WASORA_PARSER_ERROR;
      }
      if (expr != NULL) {
        expr->level = level;
        expr++;
      }
      string += n;
      last_one_operator = 0;
      count++;
    }
  }

  if (level != 1) {
    wasora_push_error_message("unmatched bracket");
    return WASORA_PARSER_ERROR;
  } else if (current_op != NULL && *current_op != ')') {
    wasora_push_error_message("missing argument for operator '%c'", *current_op);
    return WASORA_PARSER_ERROR;
  }

  return count;

}


// parsea un factor (una constante, una variable o una funcion)
// y rellena la estructura expr
int wasora_parse_factor(char *string, factor_t *factor) {

  char *backup;
  char *argument;
  double constant;
  int i, n;
  int n_arguments;
  int n_allocs;
  char *token;
  char *dummy_argument;

  var_t *dummy_var = NULL;
  vector_t *dummy_vector = NULL;
  matrix_t *dummy_matrix = NULL;
  builtin_function_t *dummy_builtin_function = NULL;
  builtin_vectorfunction_t *dummy_builtin_vectorfunction = NULL;
  builtin_functional_t *dummy_builtin_functional = NULL;
  function_t *dummy_function = NULL;
  
  char *dummy = NULL;
  char char_backup = ' ';
  
  // estas flags se podrian agrupar en una unica variable y trabajar con mascaras
  int wants_initial_static = 0;
  int wants_initial_transient = 0;
  int got_it = 0;

  backup = strdup(string);
  
  if (isdigit((int)(*string)) || *string == '-' || *string == '+') {
    // un numerito
    sscanf(string, "%lf%n", &constant, &n);
    if (factor != NULL) {
      factor->type = EXPR_CONSTANT;
      factor->constant = constant;
    }
  } else {
    // letritas
    if ((token = strtok(backup, factorseparators)) == NULL || strlen(token) == 0) {
      return WASORA_PARSER_ERROR;
    }

    // miramos si termina en _0 indicando que es el valor inicial de algo
    // y le sacamos el pedazo
    if (wasora_ends_in_zero(token) != NULL) {
      dummy = wasora_ends_in_zero(token);
      wants_initial_transient = 1;
      *dummy = '\0';
    }

    // miramos si termina en _init indicando que es el valor inicial de algo
    // y le sacamos el pedazo
    // si pusiesemos if (dummy = wasora_ends_in_init) y el chabon devuelve null, rompemos
    //   el dummy que nos devolvio el wasora_ends_in_zero de arribenio!! muy sutil...
    if (wasora_ends_in_init(token) != NULL) {
      dummy = wasora_ends_in_init(token);
      wants_initial_static = 1;
      *dummy = '\0';
    }

    // MOMENTO! no vale pedir combinaciones!
    if (wants_initial_transient && wants_initial_static) {
      wasora_push_error_message("cannot ask for both _0 and _init");
      return WASORA_PARSER_ERROR;
    }
    
    // matriz, vector o variable
    if ((dummy_matrix = wasora_get_matrix_ptr(token)) != NULL) {
      got_it = 1;
      if (factor != NULL) {
        factor->matrix = dummy_matrix;
        factor->type = EXPR_MATRIX;
      }
    } else if ((dummy_vector = wasora_get_vector_ptr(token)) != NULL) {
      got_it = 1;
      if (factor != NULL) {
        factor->vector = dummy_vector;
        factor->type = EXPR_VECTOR;
      }
    } else if ((dummy_var = wasora_get_variable_ptr(token)) != NULL) {
      // verificamos que a las variables no quieran pasarles argumentos
      if (string[strlen(token)] == '(') {
        wasora_push_error_message("variable '%s' does not take arguments (it is a variable)", token);
        free(backup);
        return WASORA_PARSER_ERROR;
      }
      got_it = 1;
      if (factor != NULL) {
        factor->variable = dummy_var;
        factor->type = EXPR_VARIABLE;
      }
    }

    if (got_it) {
      n = strlen(token);
      if (wants_initial_transient) {
        n += 2;
        if (factor != NULL) {
          factor->type |= EXPR_INITIAL_TRANSIENT;
        }
      } else if (wants_initial_static) {
        n += 5;
        if (factor != NULL) {
          factor->type |= EXPR_INITIAL_STATIC;
        }
      }
      // le volvemos a poner el underscore
      if (wants_initial_transient || wants_initial_static)  {
        *dummy = '_';
      }
    }
    
    if (dummy_matrix != NULL || dummy_vector != NULL ||
        (dummy_builtin_function = wasora_get_builtin_function_ptr(token)) != NULL ||
        (dummy_builtin_vectorfunction = wasora_get_builtin_vectorfunction_ptr(token)) != NULL ||
        (dummy_builtin_functional = wasora_get_builtin_functional_ptr(token)) != NULL || 
        (dummy_function = wasora_get_function_ptr(token)) != NULL) {

      int level;
      
      got_it = 1;

      // copiamos en argument lo que esta inmediatamente despues del nombre
      argument = strdup(string+strlen(token));

      // los argumentos tienen que estar entre parentesis
      if (argument[0] != '(') {
        wasora_push_error_message("expected parenthesis after '%s'", token);
        free(argument);
        free(backup);
        return WASORA_PARSER_ERROR;
      }

      // contamos cuantos argumentos hay, teniendo en cuenta posibles
      // parentesis dentro de los argumentos
      if ((n_arguments = wasora_count_arguments(argument)) <= 0) {
        free(argument);
        free(backup);
        return WASORA_PARSER_ERROR;
      }

      // n es la cantidad de caracteres que vamos a parsear
      n = strlen(token)+strlen(argument);

      if (factor != NULL) {

        if (dummy_vector != NULL) {
          if (n_arguments < 1 || n_arguments > 1) {
            wasora_push_error_message("vector '%s' takes exactly one subindex expression", dummy_vector->name);
            free(argument);
            free(backup);
            return WASORA_PARSER_ERROR;
          }
          n_allocs = 1;
        } else if (dummy_matrix != NULL) {
          if (n_arguments < 2 || n_arguments > 2) {
            wasora_push_error_message("matrix '%s' takes exactly two subindex expressions", dummy_matrix->name);
            free(argument);
            free(backup);
            return WASORA_PARSER_ERROR;
          }
          n_allocs = 2;
        } else if (dummy_builtin_function != NULL) {

          // tenemos una funcion interna
          factor->type = EXPR_BUILTIN_FUNCTION;
          factor->builtin_function = dummy_builtin_function;

          if (n_arguments < factor->builtin_function->min_arguments) {
            wasora_push_error_message("function '%s' takes at least %d argument%s instead of %d", token, factor->builtin_function->min_arguments, (factor->builtin_function->min_arguments==1)?"":"s", n_arguments);
            free(argument);
            free(backup);
            return WASORA_PARSER_ERROR;
          }
          if (n_arguments > factor->builtin_function->max_arguments) {
            wasora_push_error_message("function '%s' takes at most %d argument%s instead of %d", token, factor->builtin_function->max_arguments, (factor->builtin_function->max_arguments==1)?"":"s", n_arguments);
            free(argument);
            free(backup);
            return WASORA_PARSER_ERROR;
          }

          n_allocs = factor->builtin_function->max_arguments;

        } else if (dummy_builtin_vectorfunction != NULL) {
          
          // tenemos una funcion sobre vectores interna
          factor->type = EXPR_BUILTIN_VECTORFUNCTION;
          factor->builtin_vectorfunction = dummy_builtin_vectorfunction;
          
          if (n_arguments < factor->builtin_vectorfunction->min_arguments) {
            wasora_push_error_message("function '%s' takes at least %d argument%s instead of %d", token, factor->builtin_vectorfunction->min_arguments, (factor->builtin_vectorfunction->min_arguments==1)?"":"s", n_arguments);
            free(argument);
            free(backup);
            return WASORA_PARSER_ERROR;
          }
          if (n_arguments > factor->builtin_vectorfunction->max_arguments) {
            wasora_push_error_message("function '%s' takes at most %d argument%s instead of %d", token, factor->builtin_vectorfunction->max_arguments, (factor->builtin_vectorfunction->max_arguments==1)?"":"s", n_arguments);
            free(argument);
            free(backup);
            return WASORA_PARSER_ERROR;
          }

          n_allocs = factor->builtin_vectorfunction->max_arguments;
          
          
        } else if (dummy_builtin_functional != NULL) {

          // tenemos un funcional interno
          factor->type = EXPR_BUILTIN_FUNCTIONAL;
          factor->builtin_functional = dummy_builtin_functional;

          if (n_arguments < factor->builtin_functional->min_arguments) {
            wasora_push_error_message("functional '%s' takes at least %d argument%s instead of %d", token, factor->builtin_functional->min_arguments, (factor->builtin_functional->min_arguments==1)?"":"s", n_arguments);
            free(argument);
            free(backup);
            return WASORA_PARSER_ERROR;
          }
          if (n_arguments > factor->builtin_functional->max_arguments) {
            wasora_push_error_message("functional '%s' takes at most %d argument%s instead of %d", token, factor->builtin_functional->max_arguments, (factor->builtin_functional->max_arguments==1)?"":"s", n_arguments);
            free(argument);
            free(backup);
            return WASORA_PARSER_ERROR;
          }

          n_allocs = factor->builtin_functional->max_arguments;

        } else if (dummy_function != NULL) {

          // tenemos una funcion del input
          factor->type = EXPR_FUNCTION;
          factor->function = dummy_function;

          if (n_arguments != factor->function->n_arguments) {
            wasora_push_error_message("function '%s' takes exactly %d argument%s instead of %d", token, factor->function->n_arguments, (factor->function->n_arguments==1)?"":"s", n_arguments);
            free(argument);
            free(backup);
            return WASORA_PARSER_ERROR;
          }

          n_allocs = factor->function->n_arguments;

        } else {
          free(backup);
          free(argument);
          return WASORA_PARSER_ERROR;
        }

        if (factor->type != EXPR_BUILTIN_VECTORFUNCTION) {
          factor->arg = calloc(n_allocs, sizeof(expr_t));
        } else if (factor->type == EXPR_BUILTIN_VECTORFUNCTION) {
          factor->vector_arg = calloc(n_allocs, sizeof(vector_t *));
        }


        if (n_arguments == 1) {

          // si la funcion toma un solo argumento es facil
          if (factor->type != EXPR_BUILTIN_VECTORFUNCTION) {
            if (wasora_parse_expression(argument, &factor->arg[0]) != 0) {
              free(argument);
              free(backup);
              return WASORA_PARSER_ERROR;
            }
          } else if (factor->type == EXPR_BUILTIN_VECTORFUNCTION) {
            // le sacamos los parentesis
            argument[strlen(argument)-1] = '\0';
            if ((factor->vector_arg[0] = wasora_get_vector_ptr(&argument[1])) == NULL) {
              wasora_push_error_message("undefined vector '%s'", &argument[1]);
              free(argument);
              free(backup);
              return WASORA_PARSER_ERROR;
            }
          }

        } else {

          // si la funcion toma mas argumentos tenemos que buscarlos de a uno

          // si incrementamos argument despues no podemos hacer free, asi
          // que tenemos un apuntador dummy para argument 
          dummy_argument = argument;
          for (i = 0; i < n_arguments; i++) {
            if (i != n_arguments-1) {

              // si no estamos en el ultimo argumento, tenemos que
              // poner un ")\0" donde termina el argumento
              level = 1;
              dummy = dummy_argument+1;
              while (1) {
                if (*dummy == ',' && level == 1) {
                  break;
                }
                if (*dummy == '(') {
                  level++;
                } else if (*dummy == ')') {
                  level--;
                } else if (*dummy == '\0') {
                  free(argument);
                  free(backup);
                  return WASORA_PARSER_ERROR;
                }
                dummy++;
              }

              *dummy = ')';
              // ponemos un \0 despues de dummy, per necesitamos despues
              // restaurar el caracter que habia ahi
              char_backup = *(dummy+1);
              *(dummy+1) = '\0';
            }


            // en dummy_argument tenemos el argumento i-esimo
            // entre parentesis

            if (factor->type == EXPR_BUILTIN_VECTORFUNCTION) {
              
              // le sacamos los parentesis
              dummy_argument[strlen(dummy_argument)-1] = '\0';
              if ((factor->vector_arg[i] = wasora_get_vector_ptr(&dummy_argument[1])) == NULL) {
                wasora_push_error_message("undefined vector '%s'", &dummy_argument[1]);
                free(argument);
                free(backup);
                return WASORA_PARSER_ERROR;
              }
              
            } else if (factor->type == EXPR_BUILTIN_FUNCTIONAL && i == 1) {
              // si es un funcional, el segundo argumento es una variable
              // pero tenemos que sacarle los parentesis
              *(dummy) = '\0';
              if ((factor->functional_var_arg = wasora_get_variable_ptr(dummy_argument+1)) == NULL) {
                free(argument);
                free(backup);
                return WASORA_PARSER_ERROR;
              }
            } else {
              if (wasora_parse_expression(dummy_argument, &factor->arg[i]) != 0) {
                free(argument);
                free(backup);
                return WASORA_PARSER_ERROR;
              }
            }

            if (i != n_arguments-1) {
              *dummy = '(';
              *(dummy+1) = char_backup;
              dummy_argument = dummy;
            }
          }
        }
      }
      
      free(argument);
      
    } 
    
    if (got_it == 0) {
      wasora_push_error_message("unknown symbol '%s'", token);
      free(backup);
      return WASORA_PARSER_ERROR;
    }
  }

  free(backup);

  return n;

}


int wasora_count_divisions(expr_t *expr) {
  
  int i, j;
  int n = 0;
  int m = 0;
  
  for (i = 0; i < expr->n_tokens; i++) {
    
    if (expr->token[i].type == EXPR_OPERATOR && operators[expr->token[i].oper-1] == '/') {
      m++;
    }
    
    if (expr->token[i].arg != NULL) {
      switch (expr->token[i].type) {
        case EXPR_BUILTIN_FUNCTION:
          n = expr->token[i].builtin_function->max_arguments;
        break;
        case EXPR_BUILTIN_FUNCTIONAL: 
          n = expr->token[i].builtin_functional->max_arguments;
        break;
        case EXPR_FUNCTION:
          n = expr->token[i].function->n_arguments;
          if (expr->token[i].function->type == type_algebraic) {
            m += wasora_count_divisions(&expr->token[i].function->algebraic_expression);
          }
        break;
        case EXPR_BUILTIN_VECTORFUNCTION:
          n = expr->token[i].builtin_vectorfunction->max_arguments;
        break;
      }
      for (j = 0; j < n; j++) {
        m += wasora_count_divisions(&expr->token[i].arg[j]);
      }
    }
    
    
  }
  
  return m;
  
}