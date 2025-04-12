#include <quickjs.h>
#include <cutils.h>
#include "defines.h"
#include "Stk.h"
#include "Generator.h"

/*VISIBLE*/ JSClassID js_stkframes_class_id = 0, js_stk_class_id = 0, js_stkfilter_class_id = 0, js_stkgenerator_class_id = 0;
/*VISIBLE*/ JSValue stkframes_proto = {{0}, JS_TAG_UNDEFINED}, stkframes_ctor = {{0}, JS_TAG_UNDEFINED}, stk_proto = {{0}, JS_TAG_UNDEFINED},
                    stk_ctor = {{0}, JS_TAG_UNDEFINED}, stkfilter_proto = {{0}, JS_TAG_UNDEFINED}, stkfilter_ctor = {{0}, JS_TAG_UNDEFINED},
                    stkgenerator_proto = {{0}, JS_TAG_UNDEFINED}, stkgenerator_ctor = {{0}, JS_TAG_UNDEFINED};

static JSValue
js_stk_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst argv[]) {
  return JS_UNDEFINED;
}
static JSClassDef js_stk_class = {
    .class_name = "Stk",
    //.finalizer = js_stk_finalizer,
};

static const JSCFunctionListEntry js_stk_funcs[] = {
    JS_PROP_STRING_DEF("[Symbol.toStringTag]", "Stk", JS_PROP_CONFIGURABLE),
};

static JSValue
js_stkframes_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst argv[]) {
  return JS_UNDEFINED;
}
static JSClassDef js_stkframes_class = {
    .class_name = "StkFrames",
    //.finalizer = js_stkframes_finalizer,
};

static const JSCFunctionListEntry js_stkframes_funcs[] = {
    JS_PROP_STRING_DEF("[Symbol.toStringTag]", "StkFrames", JS_PROP_CONFIGURABLE),
};

static JSValue
js_stkgenerator_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst argv[]) {
  return JS_UNDEFINED;
}

static JSClassDef js_stkgenerator_class = {
    .class_name = "StkGenerator",
    //.finalizer = js_stkgenerator_finalizer,
};

static const JSCFunctionListEntry js_stkgenerator_funcs[] = {
    JS_PROP_STRING_DEF("[Symbol.toStringTag]", "StkGenerator", JS_PROP_CONFIGURABLE),
};

static JSValue
js_stkfilter_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst argv[]) {
  return JS_UNDEFINED;
}

static JSClassDef js_stkfilter_class = {
    .class_name = "StkFilter",
    //.finalizer = js_stkfilter_finalizer,
};

static const JSCFunctionListEntry js_stkfilter_funcs[] = {
    JS_PROP_STRING_DEF("[Symbol.toStringTag]", "StkFilter", JS_PROP_CONFIGURABLE),
};

int
js_stk_init(JSContext* ctx, JSModuleDef* m) {
  JS_NewClassID(&js_stk_class_id);
  JS_NewClass(JS_GetRuntime(ctx), js_stk_class_id, &js_stk_class);

  stk_ctor = JS_NewCFunction2(ctx, js_stk_constructor, "Stk", 1, JS_CFUNC_constructor, 0);
  stk_proto = JS_NewObject(ctx);

  JS_SetPropertyFunctionList(ctx, stk_proto, js_stk_funcs, countof(js_stk_funcs));

  JS_SetClassProto(ctx, js_stk_class_id, stk_proto);

  JS_NewClassID(&js_stkframes_class_id);
  JS_NewClass(JS_GetRuntime(ctx), js_stkframes_class_id, &js_stkframes_class);

  stkframes_ctor = JS_NewCFunction2(ctx, js_stkframes_constructor, "StkFrames", 1, JS_CFUNC_constructor, 0);
  stkframes_proto = JS_NewObject(ctx);

  JS_SetPropertyFunctionList(ctx, stkframes_proto, js_stkframes_funcs, countof(js_stkframes_funcs));

  JS_SetClassProto(ctx, js_stkframes_class_id, stkframes_proto);

  JS_NewClassID(&js_stkfilter_class_id);
  JS_NewClass(JS_GetRuntime(ctx), js_stkfilter_class_id, &js_stkfilter_class);

  stkfilter_ctor = JS_NewCFunction2(ctx, js_stkfilter_constructor, "StkGenerator", 1, JS_CFUNC_constructor, 0);
  stkfilter_proto = JS_NewObject(ctx);

  JS_SetPropertyFunctionList(ctx, stkfilter_proto, js_stkfilter_funcs, countof(js_stkfilter_funcs));

  JS_SetClassProto(ctx, js_stkfilter_class_id, stkfilter_proto);

  JS_NewClassID(&js_stkgenerator_class_id);
  JS_NewClass(JS_GetRuntime(ctx), js_stkgenerator_class_id, &js_stkgenerator_class);

  stkgenerator_ctor = JS_NewCFunction2(ctx, js_stkgenerator_constructor, "StkFilter", 1, JS_CFUNC_constructor, 0);
  stkgenerator_proto = JS_NewObject(ctx);

  JS_SetPropertyFunctionList(ctx, stkgenerator_proto, js_stkgenerator_funcs, countof(js_stkgenerator_funcs));

  JS_SetClassProto(ctx, js_stkgenerator_class_id, stkgenerator_proto);

  if(m) {
    JS_SetModuleExport(ctx, m, "Stk", stk_ctor);
    JS_SetModuleExport(ctx, m, "StkFrames", stkframes_ctor);
    JS_SetModuleExport(ctx, m, "StkGenerator", stkfilter_ctor);
    JS_SetModuleExport(ctx, m, "StkFilter", stkgenerator_ctor);
  }

  return 0;
}

