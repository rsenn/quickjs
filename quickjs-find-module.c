#include "quickjs.h"
#include "quickjs-libc.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

const char js_default_module_path[] = "."
#ifdef CONFIG_PREFIX
                                      ":" CONFIG_PREFIX "/lib/quickjs"
#endif
    ;

char*
js_find_module_ext(JSContext* ctx, const char* module_name, const char* ext) {
  const char *module_path, *p, *q;
  char* filename = NULL;
  size_t n, m;
  struct stat st;

  if((module_path = getenv("QUICKJS_MODULE_PATH")) == NULL)
    module_path = js_default_module_path;

  for(p = module_path; *p; p = q) {

    if((q = strchr(p, ':')) == NULL)
      q = p + strlen(p);

    n = q - p;

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

    if(*q == ':')
      ++q;
  }
  return NULL;
}

char*
js_find_module(JSContext* ctx, const char* module_name) {
  char* ret = NULL;
  size_t len;

  while(!strncmp(module_name, "./", 2)) module_name += 2;
  len = strlen(module_name);

  if(strchr(module_name, '/') == NULL || (len >= 3 && !strcmp(&module_name[len - 3], ".so")))
    ret = js_find_module_ext(ctx, module_name, ".so");

  if(ret == NULL)
    ret = js_find_module_ext(ctx, module_name, ".js");
  return ret;
}

static JSModuleDef*
js_find_module_path(JSContext* ctx, const char* module_name, void* opaque) {
  char* filename;
  JSModuleDef* ret = NULL;
  filename = module_name[0] == '/' ? js_strdup(ctx, module_name) : js_find_module(ctx, module_name);
  if(filename) {
    ret = js_module_loader(ctx, filename, opaque);
    js_free(ctx, filename);
  }
  return ret;
}

static JSModuleLoaderFunc* module_loader_path  = &js_find_module_path;

void js_std_set_module_loader_func(JSModuleLoaderFunc*func) {
module_loader_path = func;
}

JSModuleLoaderFunc* js_std_get_module_loader_func() {
 return module_loader_path;
}

