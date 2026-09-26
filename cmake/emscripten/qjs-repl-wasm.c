/*
 * Minimal driver for the qjs.html browser demo (see shell-repl.html).
 *
 * qjs.c's normal interactive REPL drives its own line-editing terminal
 * (repl.js's termInit()) through os.setReadHandler()/js_std_loop(), which
 * blocks mid-interpreter waiting for the next byte of input. Emscripten can
 * only suspend a blocking call like that via Asyncify, and Asyncify cannot
 * unwind/rewind through quickjs.c's JS_CallInternal(), which allocates its
 * argument buffer with alloca() - a construct Asyncify does not support.
 * See BUGS.
 *
 * So instead of trying to make the blocking terminal work, this exposes a
 * plain synchronous "eval one line, return its result" entry point that the
 * page calls once per Enter key press: no blocking read, no Asyncify.
 */
#include <emscripten.h>
#include <stdlib.h>
#include <string.h>

#include "quickjs-libc.h"
#include "quickjs.h"

static JSRuntime* rt;
static JSContext* ctx;

static void
qjs_repl_init(void) {
  rt = JS_NewRuntime();
  js_std_init_handlers(rt);
  ctx = JS_NewContext(rt);
  js_std_add_helpers(ctx, -1, NULL);
}

EMSCRIPTEN_KEEPALIVE
char*
qjs_repl_eval(const char* line) {
  JSValue val;
  const char* cstr;
  char* result;
  JSContext* ctx1;

  val = JS_Eval(ctx, line, strlen(line), "<repl>", JS_EVAL_TYPE_GLOBAL);
  if(JS_IsException(val)) {
    js_std_dump_error(ctx);
    result = strdup("");
  } else {
    cstr = JS_ToCString(ctx, val);
    result = strdup(cstr ? cstr : "");
    JS_FreeCString(ctx, cstr);
  }
  JS_FreeValue(ctx, val);

  /* run any promise reaction jobs the line queued up */
  while(JS_ExecutePendingJob(rt, &ctx1) > 0) {
  }

  return result;
}

EMSCRIPTEN_KEEPALIVE
void
qjs_repl_free_result(char* ptr) {
  free(ptr);
}

int
main(void) {
  qjs_repl_init();
  return 0;
}
