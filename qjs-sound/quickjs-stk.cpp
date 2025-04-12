#include <quickjs.h>
#include <cutils.h>
#include "defines.h"
#include "Stk.h"
#include "Generator.h"
#include "Filter.h"

/* stk::Filter */
#include "BiQuad.h"
#include "DelayA.h"
#include "DelayL.h"
#include "Delay.h"
#include "Fir.h"
#include "FormSwep.h"
#include "Iir.h"
#include "OnePole.h"
#include "OneZero.h"
#include "PoleZero.h"
#include "TapDelay.h"
#include "TwoPole.h"
#include "TwoZero.h"

/* stk::Generator */
#include "ADSR.h"
#include "Asymp.h"
#include "BlitSaw.h"
#include "BlitSquare.h"
#include "Blit.h"
#include "Envelope.h"
#include "Granulate.h"
#include "Modulate.h"
#include "Noise.h"
#include "SineWave.h"
#include "SingWave.h"

#include <memory>

using stk::Stk;

/*VISIBLE*/ JSClassID js_stkframes_class_id = 0, js_stk_class_id = 0, js_stkfilter_class_id = 0, js_stkgenerator_class_id = 0;
/*VISIBLE*/ JSValue stkframes_proto = {{0}, JS_TAG_UNDEFINED}, stkframes_ctor = {{0}, JS_TAG_UNDEFINED}, stk_proto = {{0}, JS_TAG_UNDEFINED},
                    stk_ctor = {{0}, JS_TAG_UNDEFINED}, stkfilter_proto = {{0}, JS_TAG_UNDEFINED}, stkfilter_ctor = {{0}, JS_TAG_UNDEFINED},
                    stkgenerator_proto = {{0}, JS_TAG_UNDEFINED}, stkgenerator_ctor = {{0}, JS_TAG_UNDEFINED};

typedef std::shared_ptr<stk::Stk> StkPtr;
typedef std::shared_ptr<stk::StkFrames> StkFramesPtr;
typedef std::shared_ptr<stk::Generator> StkGeneratorPtr;
typedef std::shared_ptr<stk::Filter> StkFilterPtr;

static void
array_to_vector(JSContext* ctx, JSValueConst arr, std::vector<double>& vec) {
  uint64_t i, len;
  JSValue lprop = JS_GetPropertyStr(ctx, arr, "length");
  JS_ToIndex(ctx, &len, lprop);
  JS_FreeValue(ctx, lprop);

  for(i = 0; i < len; i++) {
    JSValue v = JS_GetPropertyUint32(ctx, arr, i);
    double f;
    JS_ToFloat64(ctx, &f, v);
    JS_FreeValue(ctx, v);

    vec.push_back(f);
  }
}

static void
array_to_vector(JSContext* ctx, JSValueConst arr, std::vector<unsigned long>& vec) {
  uint64_t i, len;
  JSValue lprop = JS_GetPropertyStr(ctx, arr, "length");
  JS_ToIndex(ctx, &len, lprop);
  JS_FreeValue(ctx, lprop);

  for(i = 0; i < len; i++) {
    JSValue v = JS_GetPropertyUint32(ctx, arr, i);
    uint32_t u;
    JS_ToUint32(ctx, &u, v);
    JS_FreeValue(ctx, v);

    vec.push_back(u);
  }
}

static JSValue
js_stk_wrap(JSContext* ctx, Stk* s) {
  /* using new_target to get the prototype is necessary when the class is extended. */
  JSValue obj = JS_NewObjectProtoClass(ctx, stk_proto, js_stk_class_id);

  if(JS_IsException(obj))
    goto fail;

  JS_SetOpaque(obj, s);
  return obj;

fail:
  JS_FreeValue(ctx, obj);
  return JS_EXCEPTION;
}

enum {
  PROP_SAMPLERATE,
};

static JSValue
js_stk_get(JSContext* ctx, JSValueConst this_val, int magic) {
  StkPtr* s;
  JSValue ret = JS_UNDEFINED;

  if(!(s = static_cast<StkPtr*>(JS_GetOpaque2(ctx, this_val, js_stk_class_id))))
    return JS_EXCEPTION;

  switch(magic) {
    case PROP_SAMPLERATE: {
      ret = JS_NewFloat64(ctx, (*s)->sampleRate());
      break;
    }
  }

  return ret;
}

static JSValue
js_stk_set(JSContext* ctx, JSValueConst this_val, JSValueConst value, int magic) {
  StkPtr* s;
  JSValue ret = JS_UNDEFINED;

  if(!(s = static_cast<StkPtr*>(JS_GetOpaque2(ctx, this_val, js_stk_class_id))))
    return JS_EXCEPTION;

  switch(magic) {
    case PROP_SAMPLERATE: {
      double rate;
      JS_ToFloat64(ctx, &rate, value);
      (*s)->setSampleRate(rate);
      break;
    }
  }

  return ret;
}

static void
js_stk_finalizer(JSRuntime* rt, JSValue val) {
  StkPtr* s;

  if((s = static_cast<StkPtr*>(JS_GetOpaque(val, js_stk_class_id)))) {
    s->~StkPtr();
    js_free_rt(rt, s);
  }
}

static JSClassDef js_stk_class = {
    .class_name = "Stk",
    .finalizer = js_stk_finalizer,
};

static const JSCFunctionListEntry js_stk_funcs[] = {
    JS_CGETSET_MAGIC_DEF("sampleRate", js_stk_get, js_stk_set, PROP_SAMPLERATE),
    JS_PROP_STRING_DEF("[Symbol.toStringTag]", "Stk", JS_PROP_CONFIGURABLE),
};

static JSValue
js_stkframes_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst argv[]) {
  JSValue proto, obj = JS_UNDEFINED;
  double value;
  uint32_t nframes = 0, nchannels = 0;
  int argi = 0;

  if(argc > 2)
    JS_ToFloat64(ctx, &value, argv[argi++]);

  if(argc > argi)
    JS_ToUint32(ctx, &nframes, argv[argi]);
  ++argi;
  if(argc > argi)
    JS_ToUint32(ctx, &nchannels, argv[argi]);

  StkFramesPtr* f = static_cast<StkFramesPtr*>(js_mallocz(ctx, sizeof(StkFramesPtr)));

  if(argc > 2)
    new(f) StkFramesPtr(std::make_shared<stk::StkFrames>(value, nframes, nchannels));
  else
    new(f) StkFramesPtr(std::make_shared<stk::StkFrames>(nframes, nchannels));

  /* using new_target to get the prototype is necessary when the class is extended. */
  proto = JS_GetPropertyStr(ctx, new_target, "prototype");
  if(JS_IsException(proto))
    goto fail;

  if(!JS_IsObject(proto))
    proto = stkframes_proto;

  /* using new_target to get the prototype is necessary when the class is extended. */
  obj = JS_NewObjectProtoClass(ctx, proto, js_stkframes_class_id);
  JS_FreeValue(ctx, proto);

  if(JS_IsException(obj))
    goto fail;

  JS_SetOpaque(obj, f);
  return obj;

fail:
  JS_FreeValue(ctx, obj);
  return JS_EXCEPTION;
}

enum {
  METHOD_RESIZE,
  METHOD_INTERPOLATE,
};

static JSValue
js_stkframes_method(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst argv[], int magic) {
  StkFramesPtr* f;
  JSValue ret = JS_UNDEFINED;

  if(!(f = static_cast<StkFramesPtr*>(JS_GetOpaque2(ctx, this_val, js_stkframes_class_id))))
    return JS_EXCEPTION;

  switch(magic) {
    case METHOD_RESIZE: {
      uint32_t nframes, nchannels = 1;

      JS_ToUint32(ctx, &nframes, argv[0]);
      if(argc > 1)
        JS_ToUint32(ctx, &nchannels, argv[1]);

      if(argc > 2) {
        double value;
        JS_ToFloat64(ctx, &value, argv[2]);
        (*f)->resize(nframes, nchannels, value);
      } else {
        (*f)->resize(nframes, nchannels);
      }

      break;
    }
    case METHOD_INTERPOLATE: {
      double frame;
      uint32_t channel = 0;
      JS_ToFloat64(ctx, &frame, argv[0]);
      if(argc > 1)
        JS_ToUint32(ctx, &channel, argv[1]);

      ret = JS_NewFloat64(ctx, (*f)->interpolate(frame, channel));
      break;
    }
  }

  return ret;
}

enum {
  PROP_SIZE,
  PROP_EMPTY,
  PROP_CHANNELS,
  PROP_FRAMES,
  PROP_DATA_RATE,
};

static JSValue
js_stkframes_get(JSContext* ctx, JSValueConst this_val, int magic) {
  StkFramesPtr* f;
  JSValue ret = JS_UNDEFINED;

  if(!(f = static_cast<StkFramesPtr*>(JS_GetOpaque2(ctx, this_val, js_stkframes_class_id))))
    return JS_EXCEPTION;

  switch(magic) {
    case PROP_SIZE: {
      ret = JS_NewUint32(ctx, (*f)->size());
      break;
    }
    case PROP_EMPTY: {
      ret = JS_NewBool(ctx, (*f)->empty());
      break;
    }
    case PROP_CHANNELS: {
      ret = JS_NewUint32(ctx, (*f)->channels());
      break;
    }
    case PROP_FRAMES: {
      ret = JS_NewUint32(ctx, (*f)->frames());
      break;
    }
    case PROP_DATA_RATE: {
      ret = JS_NewFloat64(ctx, (*f)->dataRate());
      break;
    }
  }

  return ret;
}

static JSValue
js_stkframes_set(JSContext* ctx, JSValueConst this_val, JSValueConst value, int magic) {
  StkFramesPtr* f;
  JSValue ret = JS_UNDEFINED;

  if(!(f = static_cast<StkFramesPtr*>(JS_GetOpaque2(ctx, this_val, js_stkframes_class_id))))
    return JS_EXCEPTION;

  switch(magic) {
    case PROP_DATA_RATE: {
      double rate;
      JS_ToFloat64(ctx, &rate, value);
      (*f)->setDataRate(rate);
      break;
    }
  }

  return ret;
}

static void
js_stkframes_finalizer(JSRuntime* rt, JSValue val) {
  StkFramesPtr* f;

  if((f = static_cast<StkFramesPtr*>(JS_GetOpaque(val, js_stkframes_class_id)))) {
    f->~StkFramesPtr();
    js_free_rt(rt, f);
  }
}

static JSClassDef js_stkframes_class = {
    .class_name = "StkFrames",
    .finalizer = js_stkframes_finalizer,
};

static const JSCFunctionListEntry js_stkframes_funcs[] = {
    JS_CFUNC_MAGIC_DEF("resize", 1, js_stkframes_method, METHOD_RESIZE),
    JS_CFUNC_MAGIC_DEF("interpolate", 1, js_stkframes_method, METHOD_INTERPOLATE),
    JS_CGETSET_MAGIC_DEF("size", js_stkframes_get, 0, PROP_SIZE),
    JS_CGETSET_MAGIC_DEF("empty", js_stkframes_get, 0, PROP_EMPTY),
    JS_CGETSET_MAGIC_DEF("channels", js_stkframes_get, 0, PROP_CHANNELS),
    JS_CGETSET_MAGIC_DEF("frames", js_stkframes_get, 0, PROP_FRAMES),
    JS_CGETSET_MAGIC_DEF("dataRate", js_stkframes_get, js_stkframes_set, PROP_DATA_RATE),
    JS_PROP_STRING_DEF("[Symbol.toStringTag]", "StkFrames", JS_PROP_CONFIGURABLE),
};
enum {
  INSTANCE_ADSR,
  INSTANCE_ASYMP,
  INSTANCE_BLIT_SAW,
  INSTANCE_BLIT_SQUARE,
  INSTANCE_BLIT,
  INSTANCE_ENVELOPE,
  INSTANCE_GRANULATE,
  INSTANCE_MODULATE,
  INSTANCE_NOISE,
  INSTANCE_SINE_WAVE,
  INSTANCE_SING_WAVE,
};

static JSValue
js_stkgenerator_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst argv[], int magic) {
  StkGeneratorPtr* g = static_cast<StkGeneratorPtr*>(js_mallocz(ctx, sizeof(StkGeneratorPtr)));
  double arg = 0;
  if(argc > 0)
    JS_ToFloat64(ctx, &arg, argv[0]);

  switch(magic) {
    case INSTANCE_ASYMP: *g = std::make_shared<stk::Asymp>(); break;
    case INSTANCE_ADSR: *g = std::make_shared<stk::ADSR>(); break;
    case INSTANCE_BLIT: *g = std::make_shared<stk::Blit>(argc > 0 ? arg : 220.0); break;
    case INSTANCE_BLIT_SAW: *g = std::make_shared<stk::BlitSaw>(argc > 0 ? arg : 220.0); break;
    case INSTANCE_BLIT_SQUARE: *g = std::make_shared<stk::BlitSquare>(argc > 0 ? arg : 220.0); break;
    case INSTANCE_ENVELOPE: *g = std::make_shared<stk::Envelope>(); break;
    case INSTANCE_GRANULATE: {
      if(argc > 1) {
        uint32_t nvoices;
        const char* filename = JS_ToCString(ctx, argv[1]);
        BOOL raw = FALSE;
        JS_ToUint32(ctx, &nvoices, argv[0]);
        if(argc > 2)
          raw = JS_ToBool(ctx, argv[2]);
        *g = std::make_shared<stk::Granulate>(nvoices, filename, raw);
        JS_FreeCString(ctx, filename);
      } else {
        *g = std::make_shared<stk::Granulate>();
      }
      break;
    }
    case INSTANCE_MODULATE: *g = std::make_shared<stk::Modulate>(); break;
    case INSTANCE_NOISE: *g = std::make_shared<stk::Noise>(argc > 0 ? arg : 0); break;
    case INSTANCE_SINE_WAVE: *g = std::make_shared<stk::SineWave>(); break;
    case INSTANCE_SING_WAVE: {
      const char* filename = JS_ToCString(ctx, argv[0]);
      BOOL raw = FALSE;
      if(argc > 1)
        raw = JS_ToBool(ctx, argv[1]);
      *g = std::make_shared<stk::SingWave>(filename, raw);
      JS_FreeCString(ctx, filename);

      break;
    }
  }

  /* using new_target to get the prototype is necessary when the class is extended. */
  JSValue obj = JS_UNDEFINED, proto = JS_GetPropertyStr(ctx, new_target, "prototype");
  if(JS_IsException(proto))
    goto fail;

  if(!JS_IsObject(proto))
    proto = stkgenerator_proto;

  /* using new_target to get the prototype is necessary when the class is extended. */
  obj = JS_NewObjectProtoClass(ctx, proto, js_stkgenerator_class_id);
  JS_FreeValue(ctx, proto);

  if(JS_IsException(obj))
    goto fail;

  JS_SetOpaque(obj, g);
  return obj;

fail:
  JS_FreeValue(ctx, obj);
  return JS_EXCEPTION;
}

enum {
  PROP_CHANNELS_OUT,
};

static JSValue
js_stkgenerator_get(JSContext* ctx, JSValueConst this_val, int magic) {
  StkGeneratorPtr* g;
  JSValue ret = JS_UNDEFINED;

  if(!(g = static_cast<StkGeneratorPtr*>(JS_GetOpaque2(ctx, this_val, js_stkgenerator_class_id))))
    return JS_EXCEPTION;

  switch(magic) {
    case PROP_CHANNELS_OUT: {
      ret = JS_NewUint32(ctx, (*g)->channelsOut());
      break;
    }
  }

  return ret;
}

static JSValue
js_stkgenerator_set(JSContext* ctx, JSValueConst this_val, JSValueConst value, int magic) {
  StkGeneratorPtr* g;
  JSValue ret = JS_UNDEFINED;

  if(!(g = static_cast<StkGeneratorPtr*>(JS_GetOpaque2(ctx, this_val, js_stkgenerator_class_id))))
    return JS_EXCEPTION;

  switch(magic) {}

  return ret;
}

static void
js_stkgenerator_finalizer(JSRuntime* rt, JSValue val) {
  StkGeneratorPtr* s;

  if((s = static_cast<StkGeneratorPtr*>(JS_GetOpaque(val, js_stkgenerator_class_id)))) {
    s->~StkGeneratorPtr();
    js_free_rt(rt, s);
  }
}

static JSClassDef js_stkgenerator_class = {
    .class_name = "StkGenerator",
    .finalizer = js_stkgenerator_finalizer,
};

static const JSCFunctionListEntry js_stkgenerator_funcs[] = {
    JS_CGETSET_MAGIC_DEF("channelsOut", js_stkgenerator_get, js_stkgenerator_set, PROP_SAMPLERATE),
    JS_PROP_STRING_DEF("[Symbol.toStringTag]", "StkGenerator", JS_PROP_CONFIGURABLE),
};

enum {
  INSTANCE_BI_QUAD,
  INSTANCE_DELAY,
  INSTANCE_DELAY_A,
  INSTANCE_DELAY_L,
  INSTANCE_FIR,
  INSTANCE_FORM_SWEP,
  INSTANCE_IIR,
  INSTANCE_ONE_POLE,
  INSTANCE_ONE_ZERO,
  INSTANCE_POLE_ZERO,
  INSTANCE_TAP_DELAY,
  INSTANCE_TWO_POLE,
  INSTANCE_TWO_ZERO,
};

static JSValue
js_stkfilter_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst argv[], int magic) {
  StkFilterPtr* f = static_cast<StkFilterPtr*>(js_mallocz(ctx, sizeof(StkFilterPtr)));

  switch(magic) {
    case INSTANCE_DELAY:
    case INSTANCE_DELAY_A:
    case INSTANCE_DELAY_L: {
      double delay = 0, maxDelay = 4095;
      if(argc > 0)
        JS_ToFloat64(ctx, &delay, argv[0]);
      if(argc > 1)
        JS_ToFloat64(ctx, &maxDelay, argv[1]);

      switch(magic) {
        case INSTANCE_DELAY: *f = std::make_shared<stk::Delay>(); break;
        case INSTANCE_DELAY_A: *f = std::make_shared<stk::DelayA>(delay, maxDelay); break;
        case INSTANCE_DELAY_L: *f = std::make_shared<stk::DelayL>(delay, maxDelay); break;
      }

      break;
    }

    case INSTANCE_FIR: {
      if(argc > 0) {
        std::vector<double> coeff;
        array_to_vector(ctx, argv[0], coeff);
        *f = std::make_shared<stk::Fir>(coeff);
      } else {
        *f = std::make_shared<stk::Fir>();
      }

      break;
    }
    case INSTANCE_IIR: {
      std::vector<double> acoeff, bcoeff;

      if(argc > 0) {
        array_to_vector(ctx, argv[0], bcoeff);
        if(argc > 1)
          array_to_vector(ctx, argv[1], bcoeff);

        *f = std::make_shared<stk::Iir>(bcoeff, acoeff);
      } else {
        *f = std::make_shared<stk::Iir>();
      }

      break;
    }
    case INSTANCE_TAP_DELAY: {
      std::vector<unsigned long> taps;

      if(argc > 0) {
        uint32_t maxDelay = 4095;

        array_to_vector(ctx, argv[0], taps);
        if(argc > 1)
          JS_ToUint32(ctx, &maxDelay, argv[1]);

        *f = std::make_shared<stk::TapDelay>(taps, maxDelay);
      } else {
        *f = std::make_shared<stk::TapDelay>();
      }

      break;
    }

    default: {
      double arg = 0;
      if(argc > 0)
        JS_ToFloat64(ctx, &arg, argv[0]);

      switch(magic) {
        case INSTANCE_BI_QUAD: *f = std::make_shared<stk::BiQuad>(); break;
        case INSTANCE_FORM_SWEP: *f = std::make_shared<stk::FormSwep>(); break;
        case INSTANCE_ONE_POLE: *f = std::make_shared<stk::OnePole>(arg); break;
        case INSTANCE_ONE_ZERO: *f = std::make_shared<stk::OneZero>(arg); break;
        case INSTANCE_POLE_ZERO: *f = std::make_shared<stk::PoleZero>(); break;
        case INSTANCE_TWO_POLE: *f = std::make_shared<stk::TwoPole>(); break;
        case INSTANCE_TWO_ZERO: *f = std::make_shared<stk::TwoZero>(); break;
      }

      break;
    }
  }

  /* using new_target to get the prototype is necessary when the class is extended. */
  JSValue obj = JS_UNDEFINED, proto = JS_GetPropertyStr(ctx, new_target, "prototype");
  if(JS_IsException(proto))
    goto fail;

  if(!JS_IsObject(proto))
    proto = stkfilter_proto;

  /* using new_target to get the prototype is necessary when the class is extended. */
  obj = JS_NewObjectProtoClass(ctx, proto, js_stkfilter_class_id);
  JS_FreeValue(ctx, proto);

  if(JS_IsException(obj))
    goto fail;

  JS_SetOpaque(obj, f);
  return obj;

fail:
  JS_FreeValue(ctx, obj);
  return JS_EXCEPTION;
}

static void
js_stkfilter_finalizer(JSRuntime* rt, JSValue val) {
  StkFilterPtr* f;

  if((f = static_cast<StkFilterPtr*>(JS_GetOpaque(val, js_stkfilter_class_id)))) {
    f->~StkFilterPtr();
    js_free_rt(rt, f);
  }
}

static JSClassDef js_stkfilter_class = {
    .class_name = "StkFilter",
    .finalizer = js_stkfilter_finalizer,
};

static const JSCFunctionListEntry js_stkfilter_funcs[] = {
    JS_PROP_STRING_DEF("[Symbol.toStringTag]", "StkFilter", JS_PROP_CONFIGURABLE),
};

int
js_stk_init(JSContext* ctx, JSModuleDef* m) {
  JS_NewClassID(&js_stk_class_id);
  JS_NewClass(JS_GetRuntime(ctx), js_stk_class_id, &js_stk_class);

  stk_ctor = JS_NewObject(ctx); // JS_NewCFunction2(ctx, js_stk_constructor, "Stk", 1, JS_CFUNC_constructor, 0);
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

  stkfilter_ctor = JS_NewObject(ctx); // JS_NewCFunction2(ctx, js_stkfilter_constructor, "StkGenerator", 1, JS_CFUNC_constructor, 0);
  stkfilter_proto = JS_NewObject(ctx);

  JS_SetPropertyFunctionList(ctx, stkfilter_proto, js_stkfilter_funcs, countof(js_stkfilter_funcs));

  JS_SetClassProto(ctx, js_stkfilter_class_id, stkfilter_proto);

  JSValue ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkfilter_constructor, "BiQuad", 0, JS_CFUNC_constructor, INSTANCE_BI_QUAD);
  JS_SetModuleExport(ctx, m, "StkBiQuad", ctor);
  JS_FreeValue(ctx, ctor);
  ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkfilter_constructor, "DelayA", 0, JS_CFUNC_constructor, INSTANCE_DELAY_A);
  JS_SetModuleExport(ctx, m, "StkDelayA", ctor);
  JS_FreeValue(ctx, ctor);
  ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkfilter_constructor, "DelayL", 0, JS_CFUNC_constructor, INSTANCE_DELAY_L);
  JS_SetModuleExport(ctx, m, "StkDelayL", ctor);
  JS_FreeValue(ctx, ctor);
  ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkfilter_constructor, "Delay", 0, JS_CFUNC_constructor, INSTANCE_DELAY);
  JS_SetModuleExport(ctx, m, "StkDelay", ctor);
  JS_FreeValue(ctx, ctor);
  ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkfilter_constructor, "Fir", 1, JS_CFUNC_constructor, INSTANCE_FIR);
  JS_SetModuleExport(ctx, m, "StkFir", ctor);
  JS_FreeValue(ctx, ctor);
  ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkfilter_constructor, "FormSwep", 0, JS_CFUNC_constructor, INSTANCE_FORM_SWEP);
  JS_SetModuleExport(ctx, m, "StkFormSwep", ctor);
  JS_FreeValue(ctx, ctor);
  ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkfilter_constructor, "Iir", 0, JS_CFUNC_constructor, INSTANCE_IIR);
  JS_SetModuleExport(ctx, m, "StkIir", ctor);
  JS_FreeValue(ctx, ctor);
  ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkfilter_constructor, "OnePole", 0, JS_CFUNC_constructor, INSTANCE_ONE_POLE);
  JS_SetModuleExport(ctx, m, "StkOnePole", ctor);
  JS_FreeValue(ctx, ctor);
  ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkfilter_constructor, "OneZero", 0, JS_CFUNC_constructor, INSTANCE_ONE_ZERO);
  JS_SetModuleExport(ctx, m, "StkOneZero", ctor);
  JS_FreeValue(ctx, ctor);
  ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkfilter_constructor, "PoleZero", 0, JS_CFUNC_constructor, INSTANCE_POLE_ZERO);
  JS_SetModuleExport(ctx, m, "StkPoleZero", ctor);
  JS_FreeValue(ctx, ctor);
  ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkfilter_constructor, "TapDelay", 0, JS_CFUNC_constructor, INSTANCE_TAP_DELAY);
  JS_SetModuleExport(ctx, m, "StkTapDelay", ctor);
  JS_FreeValue(ctx, ctor);
  ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkfilter_constructor, "TwoPole", 0, JS_CFUNC_constructor, INSTANCE_TWO_POLE);
  JS_SetModuleExport(ctx, m, "StkTwoPole", ctor);
  JS_FreeValue(ctx, ctor);
  ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkfilter_constructor, "TwoZero", 0, JS_CFUNC_constructor, INSTANCE_TWO_ZERO);
  JS_SetModuleExport(ctx, m, "StkTwoZero", ctor);
  JS_FreeValue(ctx, ctor);

  JS_NewClassID(&js_stkgenerator_class_id);
  JS_NewClass(JS_GetRuntime(ctx), js_stkgenerator_class_id, &js_stkgenerator_class);

  stkgenerator_ctor = JS_NewObject(ctx); // JS_NewCFunction2(ctx, js_stkgenerator_constructor, "StkFilter", 1, JS_CFUNC_constructor, 0);
  stkgenerator_proto = JS_NewObject(ctx);

  JS_SetPropertyFunctionList(ctx, stkgenerator_proto, js_stkgenerator_funcs, countof(js_stkgenerator_funcs));

  JS_SetClassProto(ctx, js_stkgenerator_class_id, stkgenerator_proto);

  ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkgenerator_constructor, "ADSR", 0, JS_CFUNC_constructor, INSTANCE_ADSR);
  JS_SetModuleExport(ctx, m, "StkADSR", ctor);
  JS_FreeValue(ctx, ctor);
  ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkgenerator_constructor, "Asymp", 0, JS_CFUNC_constructor, INSTANCE_ASYMP);
  JS_SetModuleExport(ctx, m, "StkAsymp", ctor);
  JS_FreeValue(ctx, ctor);
  ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkgenerator_constructor, "BlitSaw", 0, JS_CFUNC_constructor, INSTANCE_BLIT_SAW);
  JS_SetModuleExport(ctx, m, "StkBlitSaw", ctor);
  JS_FreeValue(ctx, ctor);
  ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkgenerator_constructor, "BlitSquare", 0, JS_CFUNC_constructor, INSTANCE_BLIT_SQUARE);
  JS_SetModuleExport(ctx, m, "StkBlitSquare", ctor);
  JS_FreeValue(ctx, ctor);
  ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkgenerator_constructor, "Blit", 0, JS_CFUNC_constructor, INSTANCE_BLIT);
  JS_SetModuleExport(ctx, m, "StkBlit", ctor);
  JS_FreeValue(ctx, ctor);
  ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkgenerator_constructor, "Envelope", 0, JS_CFUNC_constructor, INSTANCE_ENVELOPE);
  JS_SetModuleExport(ctx, m, "StkEnvelope", ctor);
  JS_FreeValue(ctx, ctor);
  ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkgenerator_constructor, "Granulate", 0, JS_CFUNC_constructor, INSTANCE_GRANULATE);
  JS_SetModuleExport(ctx, m, "StkGranulate", ctor);
  JS_FreeValue(ctx, ctor);
  ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkgenerator_constructor, "Modulate", 0, JS_CFUNC_constructor, INSTANCE_MODULATE);
  JS_SetModuleExport(ctx, m, "StkModulate", ctor);
  JS_FreeValue(ctx, ctor);
  ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkgenerator_constructor, "Noise", 0, JS_CFUNC_constructor, INSTANCE_NOISE);
  JS_SetModuleExport(ctx, m, "StkNoise", ctor);
  JS_FreeValue(ctx, ctor);
  ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkgenerator_constructor, "SineWave", 0, JS_CFUNC_constructor, INSTANCE_SINE_WAVE);
  JS_SetModuleExport(ctx, m, "StkSineWave", ctor);
  JS_FreeValue(ctx, ctor);
  ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkgenerator_constructor, "SingWave", 0, JS_CFUNC_constructor, INSTANCE_SING_WAVE);
  JS_SetModuleExport(ctx, m, "StkStkSingWave", ctor);
  JS_FreeValue(ctx, ctor);

  if(m) {
    JS_SetModuleExport(ctx, m, "Stk", stk_ctor);
    JS_SetModuleExport(ctx, m, "StkFrames", stkframes_ctor);
    JS_SetModuleExport(ctx, m, "StkGenerator", stkfilter_ctor);
    JS_SetModuleExport(ctx, m, "StkFilter", stkgenerator_ctor);
  }

  return 0;
}

extern "C" VISIBLE JSModuleDef*
js_init_module_stk(JSContext* ctx, const char* module_name) {
  JSModuleDef* m;

  if((m = JS_NewCModule(ctx, module_name, js_stk_init))) {
    JS_AddModuleExport(ctx, m, "StkBiQuad");
    JS_AddModuleExport(ctx, m, "StkDelayA");
    JS_AddModuleExport(ctx, m, "StkDelayL");
    JS_AddModuleExport(ctx, m, "StkDelay");
    JS_AddModuleExport(ctx, m, "StkFir");
    JS_AddModuleExport(ctx, m, "StkFormSwep");
    JS_AddModuleExport(ctx, m, "StkIir");
    JS_AddModuleExport(ctx, m, "StkOnePole");
    JS_AddModuleExport(ctx, m, "StkOneZero");
    JS_AddModuleExport(ctx, m, "StkPoleZero");
    JS_AddModuleExport(ctx, m, "StkTapDelay");
    JS_AddModuleExport(ctx, m, "StkTwoPole");
    JS_AddModuleExport(ctx, m, "StkTwoZero");
    JS_AddModuleExport(ctx, m, "StkADSR");
    JS_AddModuleExport(ctx, m, "StkAsymp");
    JS_AddModuleExport(ctx, m, "StkBlitSaw");
    JS_AddModuleExport(ctx, m, "StkBlitSquare");
    JS_AddModuleExport(ctx, m, "StkBlit");
    JS_AddModuleExport(ctx, m, "StkEnvelope");
    JS_AddModuleExport(ctx, m, "StkGranulate");
    JS_AddModuleExport(ctx, m, "StkModulate");
    JS_AddModuleExport(ctx, m, "StkNoise");
    JS_AddModuleExport(ctx, m, "StkSineWave");
    JS_AddModuleExport(ctx, m, "StkStkSingWave");
    JS_AddModuleExport(ctx, m, "Stk");
    JS_AddModuleExport(ctx, m, "StkFrames");
    JS_AddModuleExport(ctx, m, "StkGenerator");
    JS_AddModuleExport(ctx, m, "StkFilter");
  }

  return m;
}
