#include "quickjs.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static const char js_module_path[] = ".:" CONFIG_PREFIX "/lib/quickjs";

char*
js_find_module(JSContext* ctx, const char* module_name) {
  const char *module_path, *p, *q;
  char* filename = NULL;
  size_t n, m;
  struct stat st;

  if((module_path = getenv("QUICKJS_MODULE_PATH")) == NULL)
    module_path = js_module_path;

  for(p = module_path; *p; p = q) {
    q = strchr(p, ':');
    if(q == NULL)
      q = p + strlen(p);

    n = q - p;

    filename = js_malloc(ctx, n + 1 + strlen(module_name) + 3 + 1);

    strncpy(filename, p, n);
    filename[n] = '/';
    strcpy(&filename[n + 1], module_name);

    m = strlen(module_name);

    if(!(m >= 3 && !strcmp(&module_name[m - 3], ".so")))
      strcpy(&filename[n + 1 + m], ".so");

    if(!stat(filename, &st))
      return filename;

    if(!(m >= 3 && !strcmp(&module_name[m - 3], ".js")))
      strcpy(&filename[n + 1 + m], ".js");

    if(!stat(filename, &st))
      return filename;

    js_free(ctx, filename);

    if(*q == ':')
      ++q;
  }
  return NULL;
}