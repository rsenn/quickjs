#include "quickjs.h"
#include "quickjs-libc.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static inline size_t
str_chrs(const char* in, const char needles[], size_t nn) {
  const char* t;
  for(t = in; *t; ++t)
    if(memchr(needles, *t, nn))
      break;
  return (size_t)(t - in);
}

const char* js_default_module_path = "."
#ifdef QUICKJS_MODULE_PATH
                                     ";" QUICKJS_MODULE_PATH
#elif defined(CONFIG_PREFIX)
                                     ";" CONFIG_PREFIX "/lib/quickjs"
#endif
    ;

static char*
js_find_module_ext_path(JSContext* ctx, const char* name, const char* ext, const char* path) {
  const char* s;
  char* file = NULL;
  size_t i, j;
  struct stat st;

  for(s = path; *s; s += i) {
    if((i = str_chrs(s, ";:\n", 3)) == 0)
      break;

    file = js_malloc(ctx, i + 1 + strlen(name) + strlen(ext) + 1);

    strncpy(file, s, i);
    file[i] = '/';
    strcpy(&file[i + 1], name);

    j = strlen(name);

    if(!(j >= 3 && !strcmp(&name[j - 3], ext)))
      strcpy(&file[i + 1 + j], ext);

    if(!stat(file, &st))
      break;

    js_free(ctx, file);
    file = NULL;

    if(s[i])
      ++i;
  }

  return file;
}

char*
js_find_module_ext(JSContext* ctx, const char* name, const char* ext) {
  const char* module_path;

  if((module_path = getenv("QUICKJS_MODULE_PATH"))) {
    char* file;
    if((file = js_find_module_ext_path(ctx, name, ext, module_path)))
      return file;
  }
  return js_find_module_ext_path(ctx, name, ext, js_default_module_path);
}

char*
js_add_module_ext(JSContext* ctx, const char* module_name, const char* ext) {
  struct stat st;
  char* file = NULL;
  size_t i = strlen(module_name);

  file = js_malloc(ctx, i + strlen(ext) + 1);

  strcpy(file, module_name);
  strcpy(&file[i], ext);

  if(stat(file, &st) == -1) {
    js_free(ctx, file);
    file = 0;
  }

  return file;
}

char*
js_find_module(JSContext* ctx, const char* module_name) {
  char* ret = NULL;
  size_t len;

  while(!strncmp(module_name, "./", 2)) module_name += 2;
  len = strlen(module_name);

  if(strchr(module_name, '/') == NULL || has_suffix(module_name, ".so"))
    ret = js_find_module_ext(ctx, module_name, ".so");
  else if(!has_suffix(module_name, ".js")) {
    if(!(ret = js_add_module_ext(ctx, module_name, "/index.js")))
      ret = js_add_module_ext(ctx, module_name, ".js");
  }

  if(ret == NULL)
    ret = js_find_module_ext(ctx, module_name, ".js");
  return ret;
}

static JSModuleDef*
js_find_module_path(JSContext* ctx, const char* module_name, void* opaque) {
  char* filename;
  JSModuleDef* ret = NULL;
  if(module_name[0] == '/' /*|| (module_name[0] == '.' && module_name[1] == '/')*/)
    filename = js_strdup(ctx, module_name);
  else
    filename = js_find_module(ctx, module_name);

  if(filename) {
    ret = js_module_loader(ctx, filename, opaque);
    js_free(ctx, filename);
  }
  return ret;
}

JSModuleLoaderFunc* js_module_loader_path = &js_find_module_path;

void
js_std_set_module_loader_func(JSModuleLoaderFunc* func) {
  js_module_loader_path = func;
}

JSModuleLoaderFunc*
js_std_get_module_loader_func() {
  return js_module_loader_path;
}
