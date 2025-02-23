#include <quickjs.h>
#include <cutils.h>
#include "defines.h"

VISIBLE JSClassID js_labsound_class_id = 0;
VISIBLE JSValue labsound_proto = {{0}, JS_TAG_UNDEFINED}, labsound_ctor = {{0}, JS_TAG_UNDEFINED};

static JSValue
js_labsound_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst argv[]) {
  JSValue proto, obj = JS_UNDEFINED;

  /* using new_target to get the prototype is necessary when the class is extended. */
  proto = JS_GetPropertyStr(ctx, new_target, "prototype");
  if(JS_IsException(proto))
    goto fail;

  if(!JS_IsObject(proto))
    proto = labsound_proto;

  /* using new_target to get the prototype is necessary when the class is extended. */
  obj = JS_NewObjectProtoClass(ctx, proto, js_labsound_class_id);
  JS_FreeValue(ctx, proto);

  if(JS_IsException(obj))
    goto fail;

  JS_SetOpaque(obj, 0);
  return obj;

fail:
  JS_FreeValue(ctx, obj);
  return JS_EXCEPTION;
}

static void
js_labsound_finalizer(JSRuntime* rt, JSValue val) {
  void* ptr;

  if((ptr = JS_GetOpaque(val, js_labsound_class_id))) {
  }
}

static JSClassDef js_labsound_class = {
    .class_name = "LabSound",
    .finalizer = js_labsound_finalizer,
};

static const JSCFunctionListEntry js_labsound_funcs[] = {
    JS_PROP_STRING_DEF("[Symbol.toStringTag]", "LabSound", JS_PROP_CONFIGURABLE),
};

int
js_labsound_init(JSContext* ctx, JSModuleDef* m) {
  JS_NewClassID(&js_labsound_class_id);
  JS_NewClass(JS_GetRuntime(ctx), js_labsound_class_id, &js_labsound_class);

  labsound_ctor = JS_NewCFunction2(ctx, js_labsound_constructor, "LabSound", 1, JS_CFUNC_constructor, 0);
  labsound_proto = JS_NewObject(ctx);

  JS_SetPropertyFunctionList(ctx, labsound_proto, js_labsound_funcs, countof(js_labsound_funcs));
  // JS_SetPropertyFunctionList(ctx, labsound_ctor, js_labsound_static,
  // countof(js_labsound_static));

  JS_SetClassProto(ctx, js_labsound_class_id, labsound_proto);

  if(m)
    JS_SetModuleExport(ctx, m, "LabSound", labsound_ctor);

  return 0;
}

extern "C" VISIBLE JSModuleDef*
js_init_module(JSContext* ctx, const char* module_name) {
  JSModuleDef* m;

  if((m = JS_NewCModule(ctx, module_name, js_labsound_init)))
    JS_AddModuleExport(ctx, m, "LabSound");

  return m;
}
