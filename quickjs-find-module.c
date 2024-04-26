#include "quickjs-config.h"
#include "quickjs.h"
#include "cutils.h"
#include "quickjs-libc.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>

#ifndef CONFIG_SHEXT
#ifdef _WIN32
#define CONFIG_SHEXT ".dll"
#else
#define CONFIG_SHEXT ".so"
#endif
#endif

#if defined(_WIN32) && !defined(__MSYS__)
#include <io.h>
#define PATHSEP_CHAR '\\'
#define PATHSEP_STR "\\"
#define LISTSEP_CHAR ';'
#define LISTSEP_STR ";"
#else
#include <unistd.h>
#define PATHSEP_CHAR '/'
#define PATHSEP_STR "/"
#define LISTSEP_CHAR ':'
#define LISTSEP_STR ":"
#endif

const char js_default_module_path[] =
#ifdef QUICKJS_MODULE_PATH
    QUICKJS_MODULE_PATH
#elif defined(QUICKJS_C_MODULE_DIR) && defined(QUICKJS_JS_MODULE_DIR)
    QUICKJS_C_MODULE_DIR LISTSEP_STR QUICKJS_JS_MODULE_DIR
#elif defined(QUICKJS_PREFIX)
#ifdef HOST_SYSTEM_NAME
    QUICKJS_PREFIX "/lib/" HOST_SYSTEM_NAME "/quickjs" LISTSEP_STR
#endif
        QUICKJS_PREFIX "/lib/quickjs"
#endif
    ;

static inline size_t
strchrs(const char* in, const char needles[]) {
  const char* t;

  for(t = in; *t; ++t)
    for(size_t i = 0; needles[i]; i++)
      if(*t == needles[i])
        return (size_t)(t - in);

  return (size_t)(t - in);
}

char*
js_find_module_ext(JSContext* ctx, const char* module_name, const char* ext) {
  const char *module_path, *p, *q;
  char  filename[PATH_MAX];
  size_t n, m;
  struct stat st;
  char listsep[3] = { LISTSEP_CHAR, ';', 0}, pathsep = PATHSEP_CHAR;

  if((module_path = getenv("QUICKJS_MODULE_PATH")) == NULL)
    module_path = js_default_module_path;

  if(module_path[0] != PATHSEP_CHAR && module_path[1] == ':' && (module_path[2] == '/' || module_path[2] == '\\')) {
    listsep[0] = ';';
    pathsep = module_path[2];
  }

  for(p = module_path; *p; p = q) {
    n =  strchrs(p, listsep);
    //n = (q = strchr(p, listsep)) ? q - p : strlen(p);

    if(*(q = p + n))
      ++q;

    //filename = js_malloc(ctx, n + 1 + strlen(module_name) + 3 + 1);

    strncpy(filename, p, n);
    filename[n] = pathsep;
    strcpy(&filename[n + 1], module_name);

    m = strlen(module_name);

    if(!(m >= 3 && !strcmp(&module_name[m - 3], ext)))
      strcpy(&filename[n + 1 + m], ext);

    if(0 == access(filename, F_OK))
      return js_strdup(ctx, filename);

    // while(strchrs(q, LISTSEP_CHARS) == 0) ++q;
  }

  return NULL;
}

char*
js_find_module(JSContext* ctx, const char* module_name) {
  char* ret = NULL;
  size_t len;

  // while(!strncmp(module_name, "." PATHSEP_STR, 2)) module_name += 2;
  len = strlen(module_name);

  if(!strchr(module_name, '.')) {
    ret = js_find_module_ext(ctx, module_name, CONFIG_SHEXT);
    if(ret == NULL)
      ret = js_find_module_ext(ctx, module_name, ".js");
  } else {
    ret = js_find_module_ext(ctx, module_name, "");
  }

  return ret;
}

static JSModuleDef*
js_find_module_path(JSContext* ctx, const char* module_name, void* opaque) {
  char* filename;
  JSModuleDef* ret = NULL;
  filename =
      module_name[strchrs(module_name, "." PATHSEP_STR)] ? js_strdup(ctx, module_name) : js_find_module(ctx, module_name);
  if(filename) {
    ret = js_module_loader(ctx, filename, opaque);
    js_free(ctx, filename);
  }
  return ret;
}

JSModuleDef*
js_module_loader_path(JSContext* ctx, const char* module_name, void* opaque) {
  return js_find_module_path(ctx, module_name, opaque);
}

static JSModuleLoaderFunc* module_loader_path = &js_find_module_path;

void
js_std_set_module_loader_func(JSModuleLoaderFunc* func) {
  module_loader_path = func;
}

JSModuleLoaderFunc*
js_std_get_module_loader_func() {
  return module_loader_path;
}
