/*------------ -------------- -------- --- ----- ---   --       -            -
 *  wasora routines to parse and load plugins
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

#include <stdio.h>
#include <string.h>
#ifndef __WIN32__
 #include <dlfcn.h>
#endif

#include "wasora.h"
#include "version.h"

#if HARDCODEDPLUGIN
extern char *plugin_name(void);
extern char *plugin_version(void);
extern char *plugin_longversion(void);
extern char *plugin_usage(void);
extern char *plugin_description(void);
extern char *plugin_wasorahmd5(void);
extern char *plugin_copyright(void);
extern int plugin_init_before_parser(void);
extern int plugin_parse_line(char *);
extern int plugin_init_after_parser(void);
extern int plugin_init_before_run(void);
extern int plugin_finalize(void);

int wasora_load_hardcoded_plugin(void) {

  wasora.plugin = malloc(sizeof(plugin_t));
  
  wasora.plugin[0].name = &plugin_name;
  wasora.plugin[0].version = &plugin_version;
  wasora.plugin[0].longversion = &plugin_longversion;
  wasora.plugin[0].description = &plugin_description;
  wasora.plugin[0].usage = &plugin_usage;
  wasora.plugin[0].wasorahmd5 = &plugin_wasorahmd5;
  wasora.plugin[0].copyright = &plugin_copyright;
  wasora.plugin[0].init_before_parser = &plugin_init_before_parser;
  wasora.plugin[0].parse_line = &plugin_parse_line;
  wasora.plugin[0].init_after_parser = &plugin_init_after_parser;
  wasora.plugin[0].init_before_run = &plugin_init_before_run;
  wasora.plugin[0].finalize = &plugin_finalize;

  wasora.plugin[0].name_string = wasora.plugin[wasora.i_plugin].name();
  wasora.plugin[0].version_string = wasora.plugin[wasora.i_plugin].version();
  wasora.plugin[0].longversion_string = wasora.plugin[wasora.i_plugin].longversion();
  wasora.plugin[0].description_string = wasora.plugin[wasora.i_plugin].description();
  wasora.plugin[0].usage_string = wasora.plugin[wasora.i_plugin].usage();
  wasora.plugin[0].wasorahmd5_string = wasora.plugin[wasora.i_plugin].wasorahmd5();
  wasora.plugin[0].copyright_string = wasora.plugin[wasora.i_plugin].copyright();

  if (strcmp(wasora.plugin[0].wasorahmd5_string, WASORA_HEADERMD5) != 0) {
    wasora_push_error_message("\
plugin '%s' is invalid for the host, the checksums of the wasora.h header do not match:\n\
wasora hash: %s\n\
plugin hash: %s\n\
Please recompile either wasora or the plugin.", wasora.plugin[0].library_file, WASORA_HEADERMD5, wasora.plugin[0].wasorahmd5_string);
    return WASORA_RUNTIME_ERROR;
  }
  
  wasora.i_plugin = 1;
  
  return WASORA_RUNTIME_OK;
  
}
#endif

int wasora_load_plugin(const char *library) {
 
  int i;
  
  wasora.plugin = realloc(wasora.plugin, (wasora.i_plugin+1)*sizeof(plugin_t));
  wasora.plugin[wasora.i_plugin].library_file = strdup(library);
      
  if ((wasora.plugin[wasora.i_plugin].handle = wasora_dlopen(wasora.plugin[wasora.i_plugin].library_file)) == NULL) {
    return WASORA_RUNTIME_ERROR;
  }

#ifndef __WIN32__
  if ((wasora.plugin[wasora.i_plugin].name = dlsym(wasora.plugin[wasora.i_plugin].handle, "plugin_name")) == NULL) {
    wasora_push_error_message("plugin '%s' is invalid, it does not contain plugin_name()", wasora.plugin[wasora.i_plugin].library_file);
    return WASORA_RUNTIME_ERROR;
  }
  if ((wasora.plugin[wasora.i_plugin].version = dlsym(wasora.plugin[wasora.i_plugin].handle, "plugin_version")) == NULL) {
    wasora_push_error_message("plugin '%s' is invalid, it does not contain plugin_version()", wasora.plugin[wasora.i_plugin].library_file);
    return WASORA_RUNTIME_ERROR;
  }
  if ((wasora.plugin[wasora.i_plugin].wasorahmd5 = dlsym(wasora.plugin[wasora.i_plugin].handle, "plugin_wasorahmd5")) == NULL) {
    wasora_push_error_message("plugin '%s' is invalid, it does not contain plugin_wasorahmd5()", wasora.plugin[wasora.i_plugin].library_file);
    return WASORA_RUNTIME_ERROR;
  }
  if ((wasora.plugin[wasora.i_plugin].copyright = dlsym(wasora.plugin[wasora.i_plugin].handle, "plugin_copyright")) == NULL) {
    wasora_push_error_message("plugin '%s' is invalid, it does not contain plugin_copyright()", wasora.plugin[wasora.i_plugin].library_file);
    return WASORA_RUNTIME_ERROR;
  }
  if ((wasora.plugin[wasora.i_plugin].init_before_parser = dlsym(wasora.plugin[wasora.i_plugin].handle, "plugin_init_before_parser")) == NULL) {
    wasora_push_error_message("plugin '%s' is invalid, it does not contain plugin_init_before_parser()", wasora.plugin[wasora.i_plugin].library_file);
    return WASORA_RUNTIME_ERROR;
  }
  if ((wasora.plugin[wasora.i_plugin].parse_line = dlsym(wasora.plugin[wasora.i_plugin].handle, "plugin_parse_line")) == NULL) {
    wasora_push_error_message("plugin '%s' is invalid, it does not contain plugin_parse_line()", wasora.plugin[wasora.i_plugin].library_file);
    return WASORA_RUNTIME_ERROR;
  }
  if ((wasora.plugin[wasora.i_plugin].init_after_parser = dlsym(wasora.plugin[wasora.i_plugin].handle, "plugin_init_after_parser")) == NULL) {
    wasora_push_error_message("plugin '%s' is invalid, it does not contain plugin_init_after_parser()", wasora.plugin[wasora.i_plugin].library_file);
    return WASORA_RUNTIME_ERROR;
  }
  if ((wasora.plugin[wasora.i_plugin].init_before_run = dlsym(wasora.plugin[wasora.i_plugin].handle, "plugin_init_before_run")) == NULL) {
    wasora_push_error_message("plugin '%s' is invalid, it does not contain plugin_init_before_run()", wasora.plugin[wasora.i_plugin].library_file);
    return WASORA_RUNTIME_ERROR;
  }
  if ((wasora.plugin[wasora.i_plugin].finalize = dlsym(wasora.plugin[wasora.i_plugin].handle, "plugin_finalize")) == NULL) {
    wasora_push_error_message("plugin '%s' is invalid, it does not contain plugin_finalize() ", wasora.plugin[wasora.i_plugin].library_file);
    return WASORA_RUNTIME_ERROR;
  }
  
  // estos son opcionales
  if ((wasora.plugin[wasora.i_plugin].longversion = dlsym(wasora.plugin[wasora.i_plugin].handle, "plugin_longversion")) != NULL) {
    wasora.plugin[wasora.i_plugin].longversion_string = wasora.plugin[wasora.i_plugin].longversion();
  }
  if ((wasora.plugin[wasora.i_plugin].description = dlsym(wasora.plugin[wasora.i_plugin].handle, "plugin_description")) != NULL) {
    wasora.plugin[wasora.i_plugin].description_string = wasora.plugin[wasora.i_plugin].description();
  }
  if ((wasora.plugin[wasora.i_plugin].usage = dlsym(wasora.plugin[wasora.i_plugin].handle, "plugin_usage")) != NULL) {
    wasora.plugin[wasora.i_plugin].usage_string = wasora.plugin[wasora.i_plugin].usage();
  }

#else
  
  if ((wasora.plugin[wasora.i_plugin].name = (winplugin_name_t)GetProcAddress(wasora.plugin[wasora.i_plugin].handle, "plugin_name")) == NULL) {
    wasora_push_error_message("plugin '%s' is invalid, it does not contain plugin_name()", wasora.plugin[wasora.i_plugin].library_file);
    return WASORA_RUNTIME_ERROR;
  }
  if ((wasora.plugin[wasora.i_plugin].version = (winplugin_version_t)GetProcAddress(wasora.plugin[wasora.i_plugin].handle, "plugin_version")) == NULL) {
    wasora_push_error_message("plugin '%s' is invalid, it does not contain plugin_version()", wasora.plugin[wasora.i_plugin].library_file);
    return WASORA_RUNTIME_ERROR;
  }
  if ((wasora.plugin[wasora.i_plugin].wasorahmd5 = (winplugin_wasorahmd5_t)GetProcAddress(wasora.plugin[wasora.i_plugin].handle, "plugin_wasorahmd5")) == NULL) {
    wasora_push_error_message("plugin '%s' is invalid, it does not contain plugin_wasorahmd5()", wasora.plugin[wasora.i_plugin].library_file);
    return WASORA_RUNTIME_ERROR;
  }
  if ((wasora.plugin[wasora.i_plugin].copyright = (winplugin_copyright_t)GetProcAddress(wasora.plugin[wasora.i_plugin].handle, "plugin_copyright")) == NULL) {
    wasora_push_error_message("plugin '%s' is invalid, it does not contain plugin_copyright()", wasora.plugin[wasora.i_plugin].library_file);
    return WASORA_RUNTIME_ERROR;
  }
  if ((wasora.plugin[wasora.i_plugin].init_before_parser = (winplugin_init_before_parser_t)GetProcAddress(wasora.plugin[wasora.i_plugin].handle, "plugin_init_before_parser")) == NULL) {
    wasora_push_error_message("plugin '%s' is invalid, it does not contain plugin_init_before_parser()", wasora.plugin[wasora.i_plugin].library_file);
    return WASORA_RUNTIME_ERROR;
  }
  if ((wasora.plugin[wasora.i_plugin].parse_line = (winplugin_parse_line_t)GetProcAddress(wasora.plugin[wasora.i_plugin].handle, "plugin_parse_line")) == NULL) {
    wasora_push_error_message("plugin '%s' is invalid, it does not contain plugin_parse_line()", wasora.plugin[wasora.i_plugin].library_file);
    return WASORA_RUNTIME_ERROR;
  }
  if ((wasora.plugin[wasora.i_plugin].init_after_parser = (winplugin_init_after_parser_t)GetProcAddress(wasora.plugin[wasora.i_plugin].handle, "plugin_init_after_parser")) == NULL) {
    wasora_push_error_message("plugin '%s' is invalid, it does not contain plugin_init_after_parser()", wasora.plugin[wasora.i_plugin].library_file);
    return WASORA_RUNTIME_ERROR;
  }
  if ((wasora.plugin[wasora.i_plugin].init_before_run = (winplugin_init_before_run_t)GetProcAddress(wasora.plugin[wasora.i_plugin].handle, "plugin_init_before_run")) == NULL) {
    wasora_push_error_message("plugin '%s' is invalid, it does not contain plugin_init_before_run()", wasora.plugin[wasora.i_plugin].library_file);
    return WASORA_RUNTIME_ERROR;
  }
  if ((wasora.plugin[wasora.i_plugin].finalize = (winplugin_finalize_t)GetProcAddress(wasora.plugin[wasora.i_plugin].handle, "plugin_finalize")) == NULL) {
    wasora_push_error_message("plugin '%s' is invalid, it does not contain plugin_finalize() ", wasora.plugin[wasora.i_plugin].library_file);
    return WASORA_RUNTIME_ERROR;
  }
  
  // estos son opcionales
  if ((wasora.plugin[wasora.i_plugin].longversion = (winplugin_longversion_t)GetProcAddress(wasora.plugin[wasora.i_plugin].handle, "plugin_longversion")) != NULL) {
    wasora.plugin[wasora.i_plugin].longversion_string = wasora.plugin[wasora.i_plugin].longversion();
  }
  if ((wasora.plugin[wasora.i_plugin].description = (winplugin_description_t)GetProcAddress(wasora.plugin[wasora.i_plugin].handle, "plugin_description")) != NULL) {
    wasora.plugin[wasora.i_plugin].description_string = wasora.plugin[wasora.i_plugin].description();
  }
  if ((wasora.plugin[wasora.i_plugin].usage = (winplugin_usage_t)GetProcAddress(wasora.plugin[wasora.i_plugin].handle, "plugin_usage")) != NULL) {
    wasora.plugin[wasora.i_plugin].usage_string = wasora.plugin[wasora.i_plugin].usage();
  }
#endif

  // rellenamos strings
  wasora.plugin[wasora.i_plugin].name_string = wasora.plugin[wasora.i_plugin].name();
  wasora.plugin[wasora.i_plugin].version_string = wasora.plugin[wasora.i_plugin].version();
  wasora.plugin[wasora.i_plugin].wasorahmd5_string = wasora.plugin[wasora.i_plugin].wasorahmd5();
  wasora.plugin[wasora.i_plugin].copyright_string = wasora.plugin[wasora.i_plugin].copyright();
  
  if (strcmp(wasora.plugin[wasora.i_plugin].wasorahmd5_string, WASORA_HEADERMD5) != 0) {
    wasora_push_error_message("plugin '%s' is invalid for the host, the checksums of the wasora.h header do not match:\n\
wasora hash: %s\n\
plugin hash: %s\n\
Please recompile either wasora or the plugin.", wasora.plugin[0].library_file, WASORA_HEADERMD5, wasora.plugin[wasora.i_plugin].wasorahmd5_string);
    return WASORA_RUNTIME_ERROR;
  }

  
  for (i = 0; i < wasora.i_plugin; i++) {
    if (strcmp(wasora.plugin[i].name_string, wasora.plugin[wasora.i_plugin].name_string) == 0) {
      wasora_push_error_message("plugin '%s' is already loaded", wasora.plugin[wasora.i_plugin].library_file);
      return WASORA_RUNTIME_ERROR;
    }
  }
  
  wasora.i_plugin++;
  
  return WASORA_RUNTIME_OK;
  
}


#ifndef __WIN32__
void *wasora_dlopen(const char *filepath) {
  void *handle;
#else
HMODULE wasora_dlopen(const char *filepath) {
  HMODULE handle;
#endif
  
  if (wasora_dlopen_try(filepath, NULL, NULL, &handle) != 0) {
    return handle;
  }  
    
  if (wasora_dlopen_try(filepath, "./", NULL, &handle) != 0) {
    return handle;
  }
  
#ifndef __WIN32__
  if (wasora_dlopen_try(filepath, "./", ".so", &handle) != 0) {
    return handle;
  }
  if (wasora_dlopen_try(filepath, "../", ".so", &handle) != 0) {
    return handle;
  }
  if (wasora_dlopen_try(filepath, NULL, ".so", &handle) != 0) {
    return handle;
  }
  if (wasora_dlopen_try(filepath, "lib", NULL, &handle) != 0) {
    return handle;
  }
  if (wasora_dlopen_try(filepath, "lib", ".so", &handle) != 0) {
    return handle;
  }
#else
  if (wasora_dlopen_try(filepath, "./", ".dll", &handle) != 0) {
    return handle;
  }
  if (wasora_dlopen_try(filepath, "../", ".dll", &handle) != 0) {
    return handle;
  }
  if (wasora_dlopen_try(filepath, NULL, ".dll", &handle) != 0) {
    return handle;
  }
#endif
  
  wasora_push_error_message("do not know where the shared object for plugin '%s' is", filepath);
  return NULL;
  
}

#ifndef __WIN32__
int wasora_dlopen_try(const char *basepath, const char *prefix, const char *postfix, void **handle) {
#else
int wasora_dlopen_try(const char *basepath, const char *prefix, const char *postfix, HMODULE *handle) {
#endif  
  char *fullpath;
  char *errorstring;
  int len;
  
  len = ((prefix != NULL)?strlen(prefix):0)   + strlen(basepath) +
        ((postfix != NULL)?strlen(postfix):0);
  fullpath = malloc(len+1);

  sprintf(fullpath, "%s%s%s", (prefix != NULL)?prefix:"", basepath, (postfix != NULL)?postfix:"");
  
#ifndef __WIN32__
  *handle = dlopen(fullpath, RTLD_NOW | RTLD_GLOBAL);
  free(fullpath);
  
  if (*handle == NULL) {
    errorstring = dlerror();
    if (strcmp(errorstring+len+2, "cannot open shared object file: No such file or directory") == 0) {
      return 0;
    } else {
      wasora_push_error_message("%s", errorstring);
      return -1;
    }
  }
#else
  *handle = LoadLibrary(fullpath);
  free(fullpath);
  if (*handle == NULL) {
    return 0;
  }
#endif
  
  return 1;
  
}
