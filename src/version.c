/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora version and description routines
 *
 *  Copyright (C) 2009--2016 jeremy theler
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
#include <gsl/gsl_version.h>
#if HAVE_READLINE
#include <readline/readline.h>
#endif
#if HAVE_IDA
#include <sundials/sundials_config.h>
#endif

#include "wasora.h"
#include "version.h"

void wasora_show_help(char *progname) {
  int i;
  
  printf("usage: %s [options] inputfile [replacement arguments]\n", progname);
  
  printf("\n\
  -p library            load wasora plugin named library\n\
 --plugin library       before reading the input file\n\n\
  -d, --debug           start in debug mode\n\
      --no-debug        ignore standard input, avoid debug mode\n\
  -l, --list            list defined symbols and exit\n\
  -h, --help            display this help and exit\n\
  -v, --version         display version information and exit\n\n\
 instructions are read from standard input if \"-\" is passed as inputfile\n");

  for (i = 0; i < wasora.i_plugin; i++) {
    if (wasora.plugin[i].usage_string != NULL && wasora.plugin[i].usage_string[0] != '\0') {
      printf("\n%s usage:\n", wasora.plugin[i].name_string);
      printf("%s\n", wasora.plugin[i].usage_string);
    }
  }
  
  fflush(stdout);

  return;
}


void wasora_show_version(int long_version) {
  
  int i;
  char numbers[][8] = {"one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten"};
  
  if (wasora.i_plugin == 0) {
    
    wasora_shortversion();
    if (long_version) {
      wasora_longversion();
    }
    
  } if (wasora.i_plugin == 1) {
    
    printf("%s %s\n", wasora.plugin[0].name_string, wasora.plugin[0].version_string);
    if (wasora.plugin[0].description_string != NULL) {
      printf("%s\n", wasora.plugin[0].description_string);
    }
    
    if (long_version) {
      if (wasora.plugin[0].longversion_string != NULL) {
        printf("%s\n\n", wasora.plugin[0].longversion_string);
      }
      if (wasora.plugin[0].copyright_string != NULL) {
        printf("%s\n\n", wasora.plugin[0].copyright_string);
      }
      
      printf("\n");
      wasora_print_random_line(stdout, 72);
      
      wasora_shortversion();
      wasora_longversion();
    }
    
  } else if (wasora.i_plugin != 0) {

    wasora_shortversion();
    if (long_version) {
      wasora_longversion();
    }

    if (wasora.i_plugin < 10) {
      printf("\n\n%s plugin%s loaded:\n\n", numbers[wasora.i_plugin-1], (wasora.i_plugin==1)?"":"s");
    } else {
      printf("\n\n%d plugins loaded:\n\n", wasora.i_plugin-1);
     }    
  
    for (i = 0; i < wasora.i_plugin; i++) {
      if (wasora.i_plugin != 1) {
        wasora_print_random_line(stdout, 72);
        printf("plugin #%d: ", i+1);
      }
      printf("%s %s\n", wasora.plugin[i].name_string, wasora.plugin[i].version_string);
      if (wasora.plugin[i].description_string != NULL) {
        printf("%s\n", wasora.plugin[i].description_string);
      }
      
      if (long_version) {
        if (wasora.plugin[i].longversion_string != NULL) {
          printf("%s\n\n", wasora.plugin[i].longversion_string);
        }
        if (wasora.plugin[i].copyright_string != NULL) {
          printf("%s\n\n", wasora.plugin[0].copyright_string);
        }
      }
    }
  }

  fflush(stdout);

  return;
}

void wasora_shortversion(void) {

#ifdef WASORA_VCS_BRANCH
  printf("wasora %s.%s %s (%s%s %s)\n", WASORA_VCS_MAJOR, WASORA_VCS_MINOR,
                                      strcmp(WASORA_VCS_BRANCH, "default")?WASORA_VCS_BRANCH:"",
                                      WASORA_VCS_SHORTID,
                                      (WASORA_VCS_CLEAN==0)?"":"+Δ",
                                      WASORA_VCS_DATE);
#else
  printf("wasora %s\n", PACKAGE_VERSION);
#endif

  printf("wasora's an advanced suite for optimization & reactor analysis\n");

  return;
}

void wasora_longversion(void) {

#ifdef WASORA_VCS_BRANCH
  printf("\n");
  printf(" rev hash %s\n", WASORA_VCS_REVID);
  printf(" last commit on %s (rev %d)\n",
   WASORA_VCS_DATE,
   WASORA_VCS_REVNO);
#endif

  printf("\n");
  printf(" compiled on %s by %s@%s (%s)\n",
   COMPILATION_DATE,
   COMPILATION_USERNAME,
   COMPILATION_HOSTNAME,
   COMPILATION_ARCH);
  printf(" with %s using %s and linked against\n",
   COMPILER_VERSION,
   COMPILER_CFLAGS);
  printf("  GNU Scientific Library version %s\n", gsl_version);
#if HAVE_IDA
  printf("  SUNDIALs Library version %s\n", SUNDIALS_PACKAGE_VERSION);
#endif
#if HAVE_READLINE
  printf("  GNU Readline version %s\n", rl_library_version);
#endif
  
  /* It is important to separate the year from the rest of the message,
     as done here, to avoid having to retranslate the message when a new
     year comes around.  */  
  printf("\n\
 wasora is copyright (C) %d-%d jeremy theler\n\
 licensed under GNU GPL version 3 or later.\n\
 wasora is free software: you are free to change and redistribute it.\n\
 There is NO WARRANTY, to the extent permitted by law.\n", 2009, 2016);
  
  return;
}

