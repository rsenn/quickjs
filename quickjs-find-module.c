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

#ifdef DEBUG_OUTPUT
#define debug(x...) printf(x)
#else
#define debug(x...)
#endif

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

  /* clang-format off */ debug(HL("%16s", FUN) HL("(", PRN) HL("module_name", ARG) "=\"%s\"" HL(")", PRN) "=%s\n", __func__, module_name, PRT_BOOL(yes)); /* clang-format on */

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
search_list(JSContext* ctx, const char* module_name, const char* list) {
  const char* s;
  char *u = 0, *t = 0;
  size_t i, j = strlen(module_name);

  /* clang-format off */ debug(HL("%16s", FUN) HL("(", PRN) HL("module_name", ARG) "=\"%s\" " HL("list", ARG) "=\"%s\"" HL(")", PRN) "\n", __func__, module_name, list); /* clang-format on */

  if(!(t = js_malloc(ctx, strlen(list) + 1 + strlen(module_name) + 1)))
    return 0;

  for(s = list; *s; s += i) {
    if((i = str_chrs(s, ";:\n", 3)) == 0)
      break;
    strncpy(t, s, i);
    t[i] = '/';
    strcpy(&t[i + 1], module_name);
    if((u = is_module(ctx, t)))
      break;
    if(s[i])
      ++i;
  }
  js_free(ctx, t);
  return u;
}

static char*
search_module(JSContext* ctx, const char* module_name) {
  const char* list;

  /* clang-format off */ debug(HL("%16s", FUN) HL("(", PRN) HL("module_name", ARG) "=\"%s\"" HL(")", PRN) "\i", __func__, module_name); /* clang-format on */

  assert(is_searchable(module_name));

  if(!(list = getenv("QUICKJS_MODULE_PATH")))
    list = js_default_module_path;

  return search_list(ctx, module_name, list);
}

static char*
find_suffix(JSContext* ctx, const char* module_name, find_module_function* fn) {
  size_t i, n, len = strlen(module_name);
  char *s, *t = 0;

  /* clang-format off */ debug(HL("%16s", FUN) HL("(", PRN) HL("module_name", ARG) "=\"%s\" " HL("fn", ARG) "=%s" HL(")", PRN) "\n", __func__, module_name, fn == &is_module ? "is_module": fn == &search_module ? "search_module": "<unknown>"); /* clang-format on */

  if(!(s = js_mallocz(ctx, (len + 31) & (~0xf))))
    return 0;

  strcpy(s, module_name);
  n = countof(js_optional_module_extensions);
  for(i = 0; i < n; i++) {
    s[len] = '\0';
    if(has_suffix(s, js_optional_module_extensions[i]))
      continue;
    strcat(s, js_optional_module_extensions[i]);

    if((t = fn(ctx, s)))
      break;
  }
  js_free(ctx, s);
  return t;
}

static char*
locate_module(JSContext* ctx, const char* module_name) {
  char* s = 0;
  BOOL search = is_searchable(module_name);
  BOOL suffix = module_has_suffix(ctx, module_name);
  find_module_function* fn = search ? &search_module : &is_module;

  s = suffix ? fn(ctx, module_name) : find_suffix(ctx, module_name, fn);

  /* clang-format off */ debug(HL("%16s", FUN) HL("(", PRN) HL("module_name", ARG) "=\"%s\"" HL(")", PRN) " " HL("search", VAR) "=%s " HL("suffix", VAR) "=%s " HL("fn", VAR) "=%s " HL("result", VAR) "=%s\n", __func__, module_name,  PRT_BOOL(search),  PRT_BOOL(suffix), search ? "search_module" : "is_module", s); /* clang-format on */

  return s;
}

static JSModuleDef*
js_search_module(JSContext* ctx, const char* module_name, void* opaque) {
  char* s;
  JSModuleDef* m = 0;

  if(!(s = is_module(ctx, module_name)))
    s = locate_module(ctx, module_name);

  if(s) {
    m = js_module_loader(ctx, s, opaque);
    js_free(ctx, s);
  }
  /* clang-format off */ debug(HL("%16s", FUN) HL("(", PRN) HL("module_name", ARG) "=\"%s\" " HL("opaque", ARG) "=%p" HL(")", PRN) " " HL("s", VAR) "=%s " HL("result", VAR) "=%p\n", __func__, module_name, opaque, s,m); /* clang-format on */
  return m;
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
