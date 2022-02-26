#include "quickjs.h"
#include "quickjs-libc.h"
#include "cutils.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef char* find_module_function(JSContext*, const char*);

#define HL(str, codes) "\x1b[" codes "m" str "\x1b[0m"
#define PRN "36;1"
#define FUN "33;1"
#define ARG "35;1"
#define VAR "1;34"
#define PRT_BOOL(b) ((b) ? HL("TRUE", "0;32") : HL("FALSE", "0;31"))

const char* js_default_module_path = "."
#ifdef QUICKJS_MODULE_PATH
                                     ";" QUICKJS_MODULE_PATH
#elif defined(CONFIG_PREFIX)
                                     ";" CONFIG_PREFIX "/lib/quickjs"
#endif
    ;

static const char* js_optional_module_extensions[] = {
    ".so",
    ".js",
    "/index.js",
    "/package.json",
};

static inline size_t
str_chrs(const char* in, const char needles[], size_t nn) {
  const char* t;
  for(t = in; *t; ++t)
    if(memchr(needles, *t, nn))
      break;
  return (size_t)(t - in);
}

static BOOL
is_file(const char* module_name) {
  struct stat st;

  if(stat(module_name, &st) == 0)
    return S_ISREG(st.st_mode);

  return FALSE;
}

/* clang-format off */
static inline BOOL is_absolute(const char* path) { return path [0] == '/'; }
static inline BOOL is_dotslash(const char* path) { return !strncmp(path, "./", 2); }
static inline BOOL is_dotdotslash(const char* path) { return !strncmp(path, "../", 3); }
/* clang-format on */

static BOOL
is_searchable(const char* path) {
  return !is_absolute(path) && !is_dotslash(path) && !is_dotdotslash(path);
}

static BOOL
is_relative(const char* path) {
  return is_dotslash(path) || is_dotdotslash(path) || !is_absolute(path);
}

static char*
is_module(JSContext* ctx, const char* module_name) {
  BOOL yes = is_file(module_name);

  /* clang-format off */ printf(HL("%16s", FUN) HL("(", PRN) HL("module_name", ARG) "=\"%s\"" HL(")", PRN) "=%s\n", __func__, module_name, PRT_BOOL(yes)); /* clang-format on */

  return yes ? js_strdup(ctx, module_name) : 0;
}

static BOOL
module_has_suffix(JSContext* ctx, const char* module_name) {
  size_t i, n;

  n = countof(js_optional_module_extensions);
  for(i = 0; i < n; i++)
    if(has_suffix(module_name, js_optional_module_extensions[i]))
      return TRUE;

  return FALSE;
}

static char*
search_list(JSContext* ctx, const char* module_name, const char* path) {
  const char* s;
  char *ret = 0, *file = 0;
  size_t i, j = strlen(module_name);

  /* clang-format off */ printf(HL("%16s", FUN) HL("(", PRN) HL("module_name", ARG) "=\"%s\" " HL("path", ARG) "=\"%s\"" HL(")", PRN) "\n", __func__, module_name, path); /* clang-format on */

  if(!(file = js_malloc(ctx, strlen(path) + 1 + strlen(module_name) + 1)))
    return 0;
  for(s = path; *s; s += i) {
    if((i = str_chrs(s, ";:\n", 3)) == 0)
      break;
    strncpy(file, s, i);
    file[i] = '/';
    strcpy(&file[i + 1], module_name);
    if((ret = is_module(ctx, file)))
      break;

    if(s[i])
      ++i;
  }
  //  if(ret != file)
  js_free(ctx, file);
  return ret;
}

static char*
search_module(JSContext* ctx, const char* module_name) {
  const char* module_path;
  char* file;

  /* clang-format off */ printf(HL("%16s", FUN) HL("(", PRN) HL("module_name", ARG) "=\"%s\"" HL(")", PRN) "\n", __func__, module_name); /* clang-format on */

  assert(is_searchable(module_name));

  if(!(module_path = getenv("QUICKJS_MODULE_PATH")))
    module_path = js_default_module_path;

  return search_list(ctx, module_name, module_path);
}

static char*
find_suffix(JSContext* ctx, const char* module_name, find_module_function* fn) {
  size_t i, n, len = strlen(module_name);
  char *file = 0, *p;

  /* clang-format off */ printf(HL("%16s", FUN) HL("(", PRN) HL("module_name", ARG) "=\"%s\" " HL("fn", ARG) "=%s" HL(")", PRN) "\n", __func__, module_name, fn == &is_module ? "is_module": fn == &search_module ? "search_module": "<unknown>"); /* clang-format on */

  if(!(p = js_mallocz(ctx, (len + 31) & (~0xf))))
    return 0;

  strcpy(p, module_name);
  n = countof(js_optional_module_extensions);
  for(i = 0; i < n; i++) {
    p[len] = '\0';
    if(has_suffix(p, js_optional_module_extensions[i]))
      continue;
    strcat(p, js_optional_module_extensions[i]);

    if((file = fn(ctx, p)))
      break;
  }
  if(p != file)
    js_free(ctx, p);
  return file;
}

static char*
locate_module(JSContext* ctx, const char* module_name) {
  char* path = 0;
  BOOL searchable = is_searchable(module_name);
  BOOL suffixed = module_has_suffix(ctx, module_name);
  find_module_function* fn = searchable ? &search_module : &is_module;

  if(!suffixed)
    path = find_suffix(ctx, module_name, fn);
  else
    path = fn(ctx, module_name);

  /* clang-format off */ printf(HL("%16s", FUN) HL("(", PRN) HL("module_name", ARG) "=\"%s\"" HL(")", PRN) " " HL("searchable", VAR) "=%s " HL("suffixed", VAR) "=%s " HL("fn", VAR) "=%s " HL("result", VAR) "=%s\n", __func__, module_name,  PRT_BOOL(searchable),  PRT_BOOL(suffixed), searchable ? "search_module" : "is_module", path); /* clang-format on */

  return path;
}

static JSModuleDef*
js_search_module(JSContext* ctx, const char* module_name, void* opaque) {
  char* file;
  JSModuleDef* ret = 0;

  if(!(file = is_module(ctx, module_name)))
    file = locate_module(ctx, module_name);

  if(file) {
    ret = js_module_loader(ctx, file, opaque);
    js_free(ctx, file);
  }
  /* clang-format off */ printf(HL("%16s", FUN) HL("(", PRN) HL("module_name", ARG) "=\"%s\" " HL("opaque", ARG) "=%p" HL(")", PRN) " " HL("file", VAR) "=%s " HL("result", VAR) "=%p\n", __func__, module_name, opaque, file,ret); /* clang-format on */
  return ret;
}

JSModuleLoaderFunc* js_module_loader_path = &js_search_module;

void
js_std_set_module_loader_func(JSModuleLoaderFunc* func) {
  js_module_loader_path = func;
}

JSModuleLoaderFunc*
js_std_get_module_loader_func() {
  return js_module_loader_path;
}
