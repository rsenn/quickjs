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
  struct stat st;
  char filename[PATH_MAX], listsep[3] = {LISTSEP_CHAR, ';', 0}, pathsep = PATHSEP_CHAR;

  if((module_path = getenv("QUICKJS_MODULE_PATH")) == NULL)
    module_path = js_default_module_path;

  if(module_path[0] != PATHSEP_CHAR && module_path[1] == ':' && (module_path[2] == '/' || module_path[2] == '\\')) {
    listsep[0] = ';';
    pathsep = module_path[2];
  }

  for(p = module_path; *p; p = q) {
    size_t m, n = strchrs(p, listsep);

    if(*(q = p + n))
      ++q;

    strncpy(filename, p, n);
    filename[n] = pathsep;
    strcpy(&filename[n + 1], module_name);

    m = strlen(module_name);

    if(!(m >= 3 && !strcmp(&module_name[m - 3], ext)))
      strcpy(&filename[n + 1 + m], ext);

    if(0 == access(filename, F_OK))
      return js_strdup(ctx, filename);
  }

  return NULL;
}

char*
js_find_module(JSContext* ctx, const char* module_name) {
  char* ret = NULL;
  size_t len = strlen(module_name);

  if(!strchr(module_name, '.')) {
    if((ret = js_find_module_ext(ctx, module_name, CONFIG_SHEXT)) == NULL)
      ret = js_find_module_ext(ctx, module_name, ".js");
  } else {
    ret = js_find_module_ext(ctx, module_name, "");
  }

  return ret;
}

static JSModuleDef*
js_find_module_path(JSContext* ctx, const char* module_name, void* opaque, JSValueConst attributes) {
  JSModuleDef* ret = NULL;
  char* filename = NULL;

  if(!module_name[strchrs(module_name, "." PATHSEP_STR)])
    filename = js_find_module(ctx, module_name);
  else {
    /* explicit paths are taken literally, anything else (e.g. 'lws.so')
       is searched in the module path first and used as-is as fallback */
    BOOL explicit_path = module_name[0] == PATHSEP_CHAR || module_name[0] == '/' ||
                         (module_name[0] == '.' && (module_name[1] == '/' || module_name[1] == PATHSEP_CHAR ||
                                                    (module_name[1] == '.' && (module_name[2] == '/' || module_name[2] == PATHSEP_CHAR)))) ||
                         (module_name[0] && module_name[1] == ':');

    if(!explicit_path)
      filename = js_find_module(ctx, module_name);
    if(!filename)
      filename = js_strdup(ctx, module_name);
  }

  if(filename) {
    ret = js_module_loader(ctx, filename, opaque, attributes);
    js_free(ctx, filename);
  }

  return ret;
}

/* canonical module names: relative specifiers are joined with the importer's
   directory, bare names with a '.' or path separator are searched in the
   module path, and whatever exists is returned as realpath, so the module
   cache sees one key per file */
char*
js_module_normalize_path(JSContext* ctx, const char* base_name, const char* name, void* opaque) {
  char *tmp = NULL, *res = NULL, rp[PATH_MAX];
  size_t n;

  if(!name[strchrs(name, "." PATHSEP_STR)])
    return js_strdup(ctx, name);

  if(name[0] == '.' && (name[1] == '/' || name[1] == PATHSEP_CHAR || (name[1] == '.' && (name[2] == '/' || name[2] == PATHSEP_CHAR)))) {
    const char* p = strrchr(base_name, '/');
#ifdef PATHSEP_CHAR
    const char* q = strrchr(base_name, PATHSEP_CHAR);
    if(q > p)
      p = q;
#endif
    n = p ? (size_t)(p - base_name) + 1 : 0;
    if((tmp = js_malloc(ctx, n + strlen(name) + 1))) {
      memcpy(tmp, base_name, n);
      strcpy(tmp + n, name);
    }
  } else if(name[0] != PATHSEP_CHAR && name[0] != '/' && !(name[0] && name[1] == ':')) {
    tmp = js_find_module(ctx, name);
  }

  if(!tmp)
    return js_strdup(ctx, name);

#if !defined(_WIN32) || defined(__MSYS__)
  if(realpath(tmp, rp)) {
    res = js_strdup(ctx, rp);
    js_free(ctx, tmp);
    return res;
  }
#endif
  return tmp;
}

JSModuleDef*
js_module_loader_path(JSContext* ctx, const char* module_name, void* opaque, JSValueConst attributes) {
  return js_find_module_path(ctx, module_name, opaque, attributes);
}

static JSModuleLoaderFunc2* module_loader_path = &js_find_module_path;

void
js_std_set_module_loader_func(JSModuleLoaderFunc2* func) {
  module_loader_path = func;
}

JSModuleLoaderFunc2*
js_std_get_module_loader_func() {
  return module_loader_path;
}
