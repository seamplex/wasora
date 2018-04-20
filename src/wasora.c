/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora main function
 *
 *  Copyright (C) 2009--2018 jeremy theler
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
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <libgen.h>

#include <gsl/gsl_errno.h>

#include "wasora.h"

int main(int argc, char **argv) {

  int i, optc;
  int option_index = 0;
  int show_help = 0;
  int show_version = 0;
  
  const struct option longopts[] = {
    { "help",     no_argument,       NULL, 'h'},
    { "version",  no_argument,       NULL, 'v'},
    { "plugin",   required_argument, NULL, 'p'},
    { "no-debug", no_argument,       NULL, 'n'},
    { "debug",    no_argument,       NULL, 'd'},
    { "list",     no_argument,       NULL, 'l'},
    { NULL, 0, NULL, 0 }
  };  

#if HARDCODEDPLUGIN
  if (wasora_load_hardcoded_plugin() != WASORA_RUNTIME_OK) {
    wasora_runtime_error();
    exit(EXIT_FAILURE);
  }
#endif

  // antes de que getopt nos rompa cosas, nos copiamos los argumentos
  wasora.argc_orig = argc;
  wasora.argv_orig = malloc((argc+1) * sizeof(char *));
  for (i = 0; i < argc; i++) {
    wasora.argv_orig[i] = strdup(argv[i]);
  }
  wasora.argv_orig[i] = NULL;

  // que no se queje si una opcion no es valida para wasora,
  // puede ser valida para algun plugin o algo
  opterr = 0;
  while ((optc = getopt_long_only(argc, argv, "hvp:dl", longopts, &option_index)) != -1) {
    switch (optc) {
      case 'h':
        show_help = 1;
        break;
      case 'v':
        show_version = 1;
        break;
      case 'p':
        if (wasora_load_plugin(optarg) != WASORA_RUNTIME_OK) {
          wasora_push_error_message("loading plugin %s", optarg);
          wasora_pop_errors();
          exit(EXIT_FAILURE);
        }
        break;
      case 'n':
        wasora.mode = mode_ignore_debug;
        break;
      case 'd':
        wasora.mode = mode_debug;
        break;
      case 'l':
        wasora.mode = mode_list_vars;
        break;
      case '?':
        break;
      default:
        break;
    }
  }
  
  if (show_help) {
    wasora_show_help(argv[0]);
    wasora_finalize();
    exit(EXIT_SUCCESS);
  } else if (show_version && optind == argc) {
    wasora_show_version(1);
    wasora_finalize();
    exit(EXIT_SUCCESS);
  } else if (wasora.mode && optind == argc) {
    wasora_call(wasora_init_before_parser());
    wasora_list_symbols();
    wasora_finalize();
    exit(EXIT_SUCCESS);
  } else if (optind == argc) {
    wasora_show_version(0);
    wasora_finalize();
    exit(EXIT_SUCCESS);
  }
   
  // nos arcordamos de los argumentos de la linea de comandos y del indice 
  // desde donde empieza el input (y despues los argumentos a reemplazar)
  wasora.argc = argc;
  wasora.argv = argv;
  wasora.optind = optind;
  // nos acordamos del directorio del input para intentar
  // otras alternativas al abrir files
  wasora.main_input_dirname_aux = strdup(argv[optind]);
  wasora.main_input_dirname = dirname(wasora.main_input_dirname_aux);

  // sacamos el handler de la gsl y metemos el nuestro
  gsl_set_error_handler(wasora_gsl_handler);

  // instalamos signal handlers para int, kill y chld
  signal(SIGINT, wasora_signal_handler);
  signal(SIGTERM, wasora_signal_handler);
//  signal(SIGCHLD, wasora_signal_handler);

  if (wasora_parse_main_input_file(argv[optind]) != 0) {
    wasora_pop_errors();
// si hubo un error de parseo no hacemos finalize porque puede haber algo roto
//    wasora_finalize();
    exit(WASORA_PARSER_ERROR);
  }
 
  // vemos como tenemos que correr
  if (show_version) {
    wasora_show_version(1);
    exit(EXIT_SUCCESS);
  } else if (wasora.mode == mode_list_vars) {
    wasora_list_symbols();
    i = WASORA_RUNTIME_OK;
  } else if (wasora.parametric_mode) {
    i = wasora_parametric_run();
  } else if (wasora.min_mode) {
    i = wasora_min_run();
  } else if (wasora.fit_mode) {
    i = wasora_fit_run();
  } else {
    i = wasora_standard_run();
  }
  
  if (i != WASORA_RUNTIME_OK) {
//    wasora_pop_errors();
    wasora_runtime_error();
  }

  // bye bye
//  wasora_finalize();
  
  return WASORA_RUNTIME_OK;

}



int wasora_standard_run(void) {

#ifdef HAVE_IDA
  int err;
  double ida_step_dt, ida_step_t_old, ida_step_t_new, t_old;
#endif

  wasora_call(wasora_init_before_run());
  
  // calculo estatico
  wasora_value(wasora_special_var(in_static)) = 1;
  wasora_value(wasora_special_var(in_static_first)) = 1;
  while (!wasora_value(wasora_special_var(done)) && !wasora_value(wasora_special_var(done_static)) && (int)(wasora_var(wasora_special_var(step_static))) < rint(wasora_var(wasora_special_var(static_steps)))) {

    wasora_debug();

    wasora_var(wasora_special_var(step_static)) = wasora_var(wasora_special_var(step_static)) + 1;
    if ((int)wasora_var(wasora_special_var(step_static)) != 1 && (int)wasora_var(wasora_special_var(step_static)) == (int)wasora_var(wasora_special_var(static_steps))) {
      wasora_value(wasora_special_var(in_static_last)) = 1;
    }

    if (wasora_dae.daes == NULL) {
      
      wasora_call(wasora_step(STEP_ALL));
      
    } else {
      
      wasora_call(wasora_step(STEP_BEFORE_DAE));
      if (wasora_dae.system == NULL) {
        wasora_call(wasora_dae_init());
      }
      wasora_call(wasora_dae_ic());
      wasora_call(wasora_step(STEP_AFTER_DAE));
      
    }
    wasora_value(wasora_special_var(in_static_first)) = 0;
  }
  wasora_value(wasora_special_var(in_static)) = 0;
  wasora_value(wasora_special_var(in_static_last)) = 0;
  wasora_value(wasora_special_var(step_static)) = 0;

  
  
#ifdef HAVE_IDA
  ida_step_dt = INFTY;
#endif
  
  // si hay realtime, inicializamos despues del calculo estationario por si tuvimos
  // que esperar el semaforo de alguien o algo por el estilo
  if (wasora_var(wasora.special_vars.realtime_scale) != 0) {
    wasora_init_realtime();
  }
  
  // loop  transitorio (si es necesario)
  wasora_value(wasora_special_var(in_transient)) = 1;
  wasora_value(wasora_special_var(in_transient_first)) = 1;
  while (wasora_var(wasora_special_var(done)) == 0) {

    wasora_debug();
    
    wasora_value(wasora_special_var(step_transient)) = wasora_value(wasora_special_var(step_transient)) + 1;

    if (wasora_dae.dimension == 0) {
    
      wasora.next_time = wasora_var(wasora_special_var(time)) + wasora_var(wasora_special_var(dt));
      // si nos dieron un time path, vemos que no nos hayamos pasado
      if (wasora.time_path != NULL && wasora.current_time_path->n_tokens != 0 && wasora.next_time > wasora_evaluate_expression(wasora.current_time_path)) {
        // nos paramos un cachito mas adelante del next time asi ya los
        // assigns toman los nuevos valores de la posible discontinuidad
        // que hay en el punto en cuestion */
        wasora.next_time = wasora_evaluate_expression(wasora.current_time_path) + wasora_var(wasora_special_var(zero));
        wasora.current_time_path++;
      }

      // dormimos si hay que hacer realtime
      if (wasora_var(wasora.special_vars.realtime_scale) != 0) {
        wasora_wait_realtime();
      }

      // actualizamos time, si esta importada despues nos la pisan asi que no pasa nada
      wasora_var(wasora_special_var(dt)) = wasora.next_time - wasora_var(wasora_special_var(time));
      wasora_var(wasora_special_var(time)) = wasora.next_time;
      
      if (wasora_var(wasora_special_var(time)) >= wasora_var(wasora_special_var(end_time))) {
        wasora_value(wasora_special_var(in_transient_last)) = 1;
        wasora_value(wasora_special_var(done_transient)) = 1;
      }
      
      // transient step
      wasora_call(wasora_step(STEP_ALL));
      
    } else {
      
#ifdef HAVE_IDA
        
      wasora_call(wasora_step(STEP_BEFORE_DAE));
      // integration step
      // nos acordamos de cuanto valia el tiempo antes de avanzar un paso
      // para despues saber cuanto vale dt
      t_old = wasora_var(wasora_special_var(time));

      // miramos si hay max_dt
      if (wasora_var(wasora_special_var(max_dt)) != 0) {
        IDASetMaxStep(wasora_dae.system, wasora_var(wasora_special_var(max_dt)));
      }

      // miramos si el dt actual (del paso interno de ida) es mas chiquito que min_dt
      if (ida_step_dt < wasora_var(wasora_special_var(min_dt)) || (wasora_var(wasora_special_var(min_dt)) != 0 && wasora_var(wasora_special_var(time)) == 0)) {

        if (wasora.current_time_path != NULL && wasora.current_time_path->n_tokens != 0) {
          wasora_push_error_message("both min_dt and TIME_PATH given");
          wasora_runtime_error();
          return WASORA_RUNTIME_ERROR;
        }

        // ponemos stop time para que no nos pasemos
        // TODO: ver si no conviene llamar con el flag IDA_NORMAL
        IDASetStopTime(wasora_dae.system, wasora_var(wasora_special_var(time))+wasora_var(wasora_special_var(min_dt)));
        do {
          ida_step_t_old = wasora_var(wasora_special_var(time));
          ida_step_t_new = wasora_var(wasora_special_var(time))+wasora_var(wasora_special_var(min_dt));
          err = IDASolve(wasora_dae.system, wasora_var(wasora_special_var(dt)), &ida_step_t_new, wasora_dae.x, wasora_dae.dxdt, IDA_ONE_STEP);
  //        err = IDASolve(wasora_dae.system, wasora_var(wasora_special_var(dt)), &ida_step_t_new, wasora_dae.x, wasora_dae.dxdt, IDA_NORMAL);
          if (err < 0) {
            wasora_push_error_message("ida returned error code %d", err);
            wasora_runtime_error();
            return WASORA_RUNTIME_ERROR;
          }
          wasora_var(wasora_special_var(time)) = ida_step_t_new;
          ida_step_dt = ida_step_t_new - ida_step_t_old;
        } while (err != IDA_TSTOP_RETURN);

      } else {

        // miramos si hay TIME_PATH
        // nos paramos un cachito mas adelante para q los steps ya hayan actuado
        if (wasora.current_time_path != NULL && wasora.current_time_path->n_tokens != 0) {
          IDASetStopTime(wasora_dae.system, wasora_evaluate_expression(wasora.current_time_path)+wasora_var(wasora_special_var(zero)));
  //        IDASetStopTime(wasora_dae.system, evaluate_expression(wasora.current_time_path)+dt0);
        }

        ida_step_t_old = wasora_var(wasora_special_var(time));
        err = IDASolve(wasora_dae.system, wasora_var(wasora_special_var(end_time)), &wasora_var(wasora_special_var(time)), wasora_dae.x, wasora_dae.dxdt, IDA_ONE_STEP);
        ida_step_dt = wasora_var(wasora_special_var(time)) - ida_step_t_old;

  /*      
        if (wasora.in_static_step == 0 && (ida_step_dt = wasora_var(wasora_special_var(time)) - ida_step_t_old) < wasora_var(wasora_special_var(zero))) {
          wasora_push_error_message("ida returned dt = 0, usually meaning that the equations do not converge to the selected tolerance, try setting a higher error_bound or smoothing discontinuous functions of time");
          wasora_runtine_error();
          return;
        }
  */
        if (err == IDA_SUCCESS) {
          ; // ok!
        } else if (err == IDA_TSTOP_RETURN) {
          ++wasora.current_time_path;
        } else {
          wasora_push_error_message("ida returned error code %d", err);
          return WASORA_RUNTIME_ERROR;
        }
      }

      wasora_var(wasora_special_var(dt)) = wasora_var(wasora_special_var(time)) - t_old;
      if (wasora_var(wasora_special_var(time)) >= wasora_var(wasora_special_var(end_time))) {
        wasora_value(wasora_special_var(in_transient_last)) = 1;
        wasora_value(wasora_special_var(done_transient)) = 1;
      }
      
      wasora_call(wasora_step(STEP_AFTER_DAE));
      
      // dormimos si hay que hacer realtime
      if (wasora_var(wasora.special_vars.realtime_scale) != 0) {
        wasora_wait_realtime();
      }
      
#endif      
    }
    
    // listo! (bug encontrado por ramiro)
    wasora_value(wasora_special_var(in_transient_first)) = 0;

  }
  
  
  return WASORA_RUNTIME_OK;
}



int wasora_step(int whence) {
  instruction_t *first, *last;

  // ponemos done en true si nos pasamos para que este disponible durante el step
  if ((int)(wasora_var(wasora_special_var(in_static)))) {
    if ((int)(wasora_var(wasora_special_var(step_static))) != 1 && (int)(wasora_var(wasora_special_var(step_static))) >= (int)(wasora_var(wasora_special_var(static_steps)))) {
      wasora_var(wasora_special_var(done_static)) = 1;
    }
  }
  if (wasora_var(wasora_special_var(end_time)) == 0) {
    wasora_var(wasora_special_var(done)) = wasora_var(wasora_special_var(done_static));
  } else if (wasora_var(wasora_special_var(time)) >= (wasora_var(wasora_special_var(end_time)) - 1e-1*wasora_var(wasora_special_var(dt)))) {
    wasora_var(wasora_special_var(done)) = 1;
  }  
  
  switch (whence) {
    case STEP_BEFORE_DAE:
      first = wasora.instructions;
      last = wasora_dae.instruction;
    break;
    case STEP_AFTER_DAE:
      first = wasora_dae.instruction;
      last = NULL;
    break;
    default:
      first = wasora.instructions;
      last = NULL;
    break;
  }

  // barremos la lista en los extremos que nos dijeron
  // pero siguiendo la logica de condicionales
  wasora.ip = first;
  while (wasora.ip != last) {
    wasora_call(wasora.ip->routine(wasora.ip->argument));

    if (wasora.next_flow_instruction != NULL) {
      wasora.ip = wasora.next_flow_instruction;
      wasora.next_flow_instruction = NULL;
    } else {
      wasora.ip = wasora.ip->next;
    }
  }


  // volvemos a poner done en true por si algun salame puso done = 0 por alguna otra razon
  if ((int)(wasora_var(wasora_special_var(in_static)))) {
    if ((int)(wasora_var(wasora_special_var(step_static))) >= (int)(wasora_var(wasora_special_var(static_steps)))) {
      wasora_var(wasora_special_var(done_static)) = 1;
    }
  }
  if (wasora_var(wasora_special_var(end_time)) == 0) {
    wasora_var(wasora_special_var(done)) = wasora_var(wasora_special_var(done_static));
  } else if (wasora_var(wasora_special_var(time)) >= (wasora_var(wasora_special_var(end_time)) - 1e-1*wasora_var(wasora_special_var(dt)))) {
    wasora_var(wasora_special_var(done)) = 1;
  }  

  return WASORA_RUNTIME_OK;

}


int wasora_instruction_abort(void *arg) {
  exit(1);
}
