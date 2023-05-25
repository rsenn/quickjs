#include "quickjs-config.h"
#include "quickjs.h"
#include "quickjs-libc.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define PATHSEP_CHAR ';'
#define PATHSEP_CHARS ":;"
#define PATHSEP_STR ";"

#ifdef _WIN32
#define CONFIG_SHEXT ".dll"
#else
#define CONFIG_SHEXT ".so"
#endif

const char js_default_module_path[] =
#ifdef QUICKJS_MODULE_PATH
    QUICKJS_MODULE_PATH
#elif defined(QUICKJS_C_MODULE_DIR) && defined(QUICKJS_JS_MODULE_DIR)
    QUICKJS_C_MODULE_DIR PATHSEP_STR QUICKJS_JS_MODULE_DIR
#elif defined(CONFIG_PREFIX)
#ifdef HOST_SYSTEM_NAME
    CONFIG_PREFIX "/lib/" HOST_SYSTEM_NAME "/quickjs" PATHSEP_STR
#endif
        CONFIG_PREFIX "/lib/quickjs"
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
  char* filename = NULL;
  size_t n, m;
  struct stat st;

  if((module_path = getenv("QUICKJS_MODULE_PATH")) == NULL)
    module_path = js_default_module_path;

  for(p = module_path; *p; p = q) {

    n = strchrs(p, PATHSEP_CHARS);
    /*    if(!p[(n = strchrs(p, PATHSEP_CHARS))])
          n = strlen(p);*/

    q = p + n + 1;

    filename = js_malloc(ctx, n + 1 + strlen(module_name) + 3 + 1);

    strncpy(filename, p, n);
    filename[n] = '/';
    strcpy(&filename[n + 1], module_name);

    m = strlen(module_name);

    if(!(m >= 3 && !strcmp(&module_name[m - 3], ext)))
      strcpy(&filename[n + 1 + m], ext);

    if(!stat(filename, &st))
      return filename;

    js_free(ctx, filename);

    // while(strchrs(q, PATHSEP_CHARS) == 0) ++q;
  }

  return NULL;
}

char*
js_find_module(JSContext* ctx, const char* module_name) {
  char* ret = NULL;
  size_t len;

  // while(!strncmp(module_name, "./", 2)) module_name += 2;
  len = strlen(module_name);

  if(!strchr(module_name, ".")) {
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
  filename = module_name[strchrs(module_name, "./")] ? js_strdup(ctx, module_name) : js_find_module(ctx, module_name);
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
