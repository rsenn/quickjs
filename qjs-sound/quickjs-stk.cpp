#include <quickjs.h>
#include <cutils.h>

#include <vector>
#include <memory>

#include "defines.h"
#include "Stk.h"
#include "Generator.h"
#include "Filter.h"
#include "Effect.h"
#include "FM.h"
#include "Instrmnt.h"

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

/* stk::Effect */
#include "Chorus.h"
#include "Echo.h"
#include "FreeVerb.h"
#include "JCRev.h"
#include "LentPitShift.h"
#include "NRev.h"
#include "PRCRev.h"
#include "PitShift.h"

/* stk::FM */
#include "BeeThree.h"
#include "FMVoices.h"
#include "HevyMetl.h"
#include "PercFlut.h"
#include "Rhodey.h"
#include "TubeBell.h"
#include "Wurley.h"

/* stk::Instr */
#include "BandedWG.h"
#include "BlowBotl.h"
#include "BlowHole.h"
#include "Bowed.h"
#include "Brass.h"
#include "Clarinet.h"
#include "Drummer.h"
#include "FM.h"
#include "Flute.h"
#include "Mandolin.h"
#include "Mesh2D.h"
#include "Modal.h"
#include "Plucked.h"
#include "Recorder.h"
#include "Resonate.h"
#include "Sampler.h"
#include "Saxofony.h"
#include "Shakers.h"
#include "Simple.h"
#include "Sitar.h"
#include "StifKarp.h"
#include "VoicForm.h"
#include "Whistle.h"

#include "Sampler.h"
#include "Moog.h"

#include <memory>

using stk::Stk;

static JSClassID js_stkframes_class_id, js_stk_class_id, js_stkfilter_class_id, js_stkgenerator_class_id, js_stkeffect_class_id, js_stkfm_class_id,
    js_stkinstrmnt_class_id;
static JSValue stkframes_proto, stkframes_ctor, stk_proto, stk_ctor, stkfilter_proto, stkfilter_ctor, stkgenerator_proto, stkgenerator_ctor, stkeffect_proto,
    stkeffect_ctor, stkfm_proto, stkfm_ctor, stkinstrmnt_proto, stkinstrmnt_ctor;

typedef std::shared_ptr<stk::Stk> StkPtr;
typedef std::shared_ptr<stk::StkFrames> StkFramesPtr;
typedef std::shared_ptr<stk::Generator> StkGeneratorPtr;
typedef std::shared_ptr<stk::Filter> StkFilterPtr;
typedef std::shared_ptr<stk::Effect> StkEffectPtr;
typedef std::shared_ptr<stk::FM> StkFMPtr;
typedef std::shared_ptr<stk::Instrmnt> StkInstrmntPtr;

static JSAtom
js_symbol_tostringtag(JSContext* ctx) {
  JSValue g = JS_GetGlobalObject(ctx);
  JSValue sym = JS_GetPropertyStr(ctx, g, "Symbol");
  JSValue tst = JS_GetPropertyStr(ctx, sym, "toStringTag");
  JS_FreeValue(ctx, sym);
  JS_FreeValue(ctx, g);
  JSAtom ret = JS_ValueToAtom(ctx, tst);
  JS_FreeValue(ctx, tst);
  return ret;
}

static void
js_set_tostringtag(JSContext* ctx, JSValueConst obj, const char* name) {
  JSAtom tst = js_symbol_tostringtag(ctx);
  JSValue str = JS_NewString(ctx, name);
  JS_DeleteProperty(ctx, obj, tst, 0);
  JS_DefinePropertyValue(ctx, obj, tst, str, JS_PROP_CONFIGURABLE | JS_PROP_WRITABLE);
  JS_FreeAtom(ctx, tst);
}

static int64_t
array_length(JSContext* ctx, JSValueConst arr) {
  int64_t len = -1;
  JSValue lprop = JS_GetPropertyStr(ctx, arr, "length");

  if(!JS_IsException(lprop))
    JS_ToInt64(ctx, &len, lprop);

  JS_FreeValue(ctx, lprop);
  return len;
}

static void
array_to_vector(JSContext* ctx, JSValueConst arr, std::vector<double>& vec) {
  int64_t len = array_length(ctx, arr);

  for(int64_t i = 0; i < len; i++) {
    JSValue v = JS_GetPropertyUint32(ctx, arr, i);
    double f;
    JS_ToFloat64(ctx, &f, v);
    JS_FreeValue(ctx, v);

    vec.push_back(f);
  }
}

static void
array_to_vector(JSContext* ctx, JSValueConst arr, std::vector<unsigned long>& vec) {
  int64_t len = array_length(ctx, arr);

  for(int64_t i = 0; i < len; i++) {
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
  PROP_SAMPLERATE = 0,
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
  int i = 0;

  if(argc > 2)
    JS_ToFloat64(ctx, &value, argv[i++]);

  if(argc > i)
    JS_ToUint32(ctx, &nframes, argv[i]);

  ++i;
  if(argc > i)
    JS_ToUint32(ctx, &nchannels, argv[i]);

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
  METHOD_RESIZE = 0,
  METHOD_INTERPOLATE,
  METHOD_GETCHANNEL,
  METHOD_SETCHANNEL,
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
    case METHOD_GETCHANNEL: {
      uint32_t channel, dstChannel;
      StkFramesPtr* a;

      JS_ToUint32(ctx, &channel, argv[0]);

      if(!(a = static_cast<StkFramesPtr*>(JS_GetOpaque(argv[1], js_stkframes_class_id))))
        return JS_ThrowTypeError(ctx, "argument 2 must be StkFrames");

      JS_ToUint32(ctx, &dstChannel, argv[2]);

      (*f)->getChannel(channel, *a->get(), dstChannel);
      ret = JS_DupValue(ctx, argv[1]);
      break;
    }
    case METHOD_SETCHANNEL: {
      uint32_t channel, srcChannel;
      StkFramesPtr* a;

      JS_ToUint32(ctx, &channel, argv[0]);

      if(!(a = static_cast<StkFramesPtr*>(JS_GetOpaque(argv[1], js_stkframes_class_id))))
        return JS_ThrowTypeError(ctx, "argument 2 must be StkFrames");

      JS_ToUint32(ctx, &srcChannel, argv[2]);

      (*f)->setChannel(channel, *a->get(), srcChannel);
      break;
    }
  }

  return ret;
}

enum {
  PROP_SIZE = 0,
  PROP_EMPTY,
  PROP_CHANNELS,
  PROP_FRAMES,
  PROP_DATA_RATE,
  PROP_BUFFER,
};

static void
js_stkframes_free_buf(JSRuntime* rt, void* opaque, void* ptr) {
  StkFramesPtr* f = static_cast<StkFramesPtr*>(opaque);
  f->~StkFramesPtr();
  js_free_rt(rt, f);
}

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
    case PROP_BUFFER: {
      stk::StkFloat* ptr = &((*(*f))[0]);
      size_t len = (*f)->size();

      StkFramesPtr* opaque = static_cast<StkFramesPtr*>(js_mallocz(ctx, sizeof(StkFramesPtr)));

      new(opaque) StkFramesPtr(*f);

      ret = JS_NewArrayBuffer(ctx, reinterpret_cast<uint8_t*>(ptr), sizeof(stk::StkFloat) * len, js_stkframes_free_buf, opaque, FALSE);
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
    JS_CFUNC_MAGIC_DEF("getChannel", 3, js_stkframes_method, METHOD_GETCHANNEL),
    JS_CFUNC_MAGIC_DEF("setChannel", 3, js_stkframes_method, METHOD_SETCHANNEL),
    JS_CGETSET_MAGIC_DEF("size", js_stkframes_get, 0, PROP_SIZE),
    JS_CGETSET_MAGIC_DEF("empty", js_stkframes_get, 0, PROP_EMPTY),
    JS_CGETSET_MAGIC_DEF("channels", js_stkframes_get, 0, PROP_CHANNELS),
    JS_CGETSET_MAGIC_DEF("frames", js_stkframes_get, 0, PROP_FRAMES),
    JS_CGETSET_MAGIC_DEF("dataRate", js_stkframes_get, js_stkframes_set, PROP_DATA_RATE),
    JS_CGETSET_MAGIC_DEF("buffer", js_stkframes_get, 0, PROP_BUFFER),
    JS_PROP_STRING_DEF("[Symbol.toStringTag]", "StkFrames", JS_PROP_CONFIGURABLE),
};

enum {
  INSTANCE_ADSR = 0,
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
    case INSTANCE_ADSR: {
      *g = std::make_shared<stk::ADSR>();
      break;
    }
    case INSTANCE_ASYMP: {
      *g = std::make_shared<stk::Asymp>();
      break;
    }
    case INSTANCE_BLIT: {
      *g = std::make_shared<stk::Blit>(argc > 0 ? arg : 220.0);
      break;
    }
    case INSTANCE_BLIT_SAW: {
      *g = std::make_shared<stk::BlitSaw>(argc > 0 ? arg : 220.0);
      break;
    }
    case INSTANCE_BLIT_SQUARE: {
      *g = std::make_shared<stk::BlitSquare>(argc > 0 ? arg : 220.0);
      break;
    }
    case INSTANCE_ENVELOPE: {
      *g = std::make_shared<stk::Envelope>();
      break;
    }
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
    case INSTANCE_MODULATE: {
      *g = std::make_shared<stk::Modulate>();
      break;
    }
    case INSTANCE_NOISE: {
      *g = std::make_shared<stk::Noise>(argc > 0 ? arg : 0);
      break;
    }
    case INSTANCE_SINE_WAVE: {
      *g = std::make_shared<stk::SineWave>();
      break;
    }
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

  js_set_tostringtag(ctx,
                     obj,
                     ((const char*[]){
                         "StkADSR",
                         "StkAsymp",
                         "StkBlit",
                         "StkBlitSaw",
                         "StkBlitSquare",
                         "StkEnvelope",
                         "StkGranulate",
                         "StkModulate",
                         "StkNoise",
                         "StkSineWave",
                         "StkSingWave",
                     })[magic]);

  return obj;

fail:
  JS_FreeValue(ctx, obj);
  return JS_EXCEPTION;
}

enum {
  METHOD_TICK = 0,
};

static JSValue
js_stkgenerator_method(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst argv[], int magic) {
  StkGeneratorPtr* f;
  JSValue ret = JS_UNDEFINED;

  if(!(f = static_cast<StkGeneratorPtr*>(JS_GetOpaque2(ctx, this_val, js_stkgenerator_class_id))))
    return JS_EXCEPTION;

  switch(magic) {
    case METHOD_TICK: {
      StkFramesPtr* a;
      uint32_t channel = 0;

      if(!(a = static_cast<StkFramesPtr*>(JS_GetOpaque(argv[0], js_stkframes_class_id))))
        return JS_ThrowTypeError(ctx, "argument 1 must be StkFrames");

      if(argc > 1)
        JS_ToUint32(ctx, &channel, argv[1]);

      (*f)->tick(*a->get(), channel);

      ret = JS_DupValue(ctx, argv[0]);
      break;
    }
  }

  return ret;
}

enum {
  PROP_CHANNELS_OUT = 0,
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
    JS_CFUNC_MAGIC_DEF("tick", 1, js_stkgenerator_method, METHOD_TICK),
    JS_CGETSET_MAGIC_DEF("channelsOut", js_stkgenerator_get, js_stkgenerator_set, PROP_SAMPLERATE),
    JS_PROP_STRING_DEF("[Symbol.toStringTag]", "StkGenerator", JS_PROP_CONFIGURABLE),
};

enum {
  INSTANCE_BIQUAD = 0,
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
        case INSTANCE_DELAY: {
          *f = std::make_shared<stk::Delay>();
          break;
        }
        case INSTANCE_DELAY_A: {
          *f = std::make_shared<stk::DelayA>(delay, maxDelay);
          break;
        }
        case INSTANCE_DELAY_L: {
          *f = std::make_shared<stk::DelayL>(delay, maxDelay);
          break;
        }
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
        case INSTANCE_BIQUAD: {
          *f = std::make_shared<stk::BiQuad>();
          break;
        }
        case INSTANCE_FORM_SWEP: {
          *f = std::make_shared<stk::FormSwep>();
          break;
        }
        case INSTANCE_ONE_POLE: {
          *f = std::make_shared<stk::OnePole>(arg);
          break;
        }
        case INSTANCE_ONE_ZERO: {
          *f = std::make_shared<stk::OneZero>(arg);
          break;
        }
        case INSTANCE_POLE_ZERO: {
          *f = std::make_shared<stk::PoleZero>();
          break;
        }
        case INSTANCE_TWO_POLE: {
          *f = std::make_shared<stk::TwoPole>();
          break;
        }
        case INSTANCE_TWO_ZERO: {
          *f = std::make_shared<stk::TwoZero>();
          break;
        }
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

  js_set_tostringtag(ctx,
                     obj,
                     ((const char*[]){
                         "StkBiQuad",
                         "StkDelay",
                         "StkDelayA",
                         "StkDelayL",
                         "StkFir",
                         "StkFormSwep",
                         "StkIir",
                         "StkOnePole",
                         "StkOneZero",
                         "StkPoleZero",
                         "StkTapDelay",
                         "StkTwoPole",
                         "StkTwoZero",
                     })[magic]);

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

enum {
  INSTANCE_CHORUS = 0,
  INSTANCE_ECHO,
  INSTANCE_FREEVERB,
  INSTANCE_JCREV,
  INSTANCE_LENTPITSHIFT,
  INSTANCE_NREV,
  INSTANCE_PITSHIFT,
  INSTANCE_PRCREV,
};

static JSValue
js_stkeffect_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst argv[], int magic) {
  StkEffectPtr* e = static_cast<StkEffectPtr*>(js_mallocz(ctx, sizeof(StkEffectPtr)));
  double arg = 0;
  if(argc > 0)
    JS_ToFloat64(ctx, &arg, argv[0]);

  switch(magic) {
    case INSTANCE_CHORUS: {
      *e = std::make_shared<stk::Chorus>(argc > 0 ? arg : 6000);
      break;
    }
    case INSTANCE_ECHO: {
      *e = std::make_shared<stk::Echo>(argc > 0 ? arg : stk::Stk::sampleRate());
      break;
    }
    case INSTANCE_FREEVERB: {
      *e = std::make_shared<stk::FreeVerb>();
      break;
    }
    case INSTANCE_JCREV: {
      *e = std::make_shared<stk::JCRev>(argc > 0 ? arg : 1.0);
      break;
    }
    case INSTANCE_LENTPITSHIFT: {
      int32_t tmax = stk::RT_BUFFER_SIZE;
      if(argc > 1)
        JS_ToInt32(ctx, &tmax, argv[1]);
      *e = std::make_shared<stk::LentPitShift>(argc > 0 ? arg : 1.0, tmax);
      break;
    }
    case INSTANCE_NREV: {
      *e = std::make_shared<stk::NRev>(argc > 0 ? arg : 1.0);
      break;
    }
    case INSTANCE_PITSHIFT: {
      *e = std::make_shared<stk::PitShift>();
      break;
    }
    case INSTANCE_PRCREV: {
      *e = std::make_shared<stk::PRCRev>(argc > 0 ? arg : 1.0);
      break;
    }
  }

  /* using new_target to get the prototype is necessary when the class is extended. */
  JSValue obj = JS_UNDEFINED, proto = JS_GetPropertyStr(ctx, new_target, "prototype");
  if(JS_IsException(proto))
    goto fail;

  if(!JS_IsObject(proto))
    proto = stkeffect_proto;

  /* using new_target to get the prototype is necessary when the class is extended. */
  obj = JS_NewObjectProtoClass(ctx, proto, js_stkeffect_class_id);
  JS_FreeValue(ctx, proto);

  if(JS_IsException(obj))
    goto fail;

  JS_SetOpaque(obj, e);

  js_set_tostringtag(ctx,
                     obj,
                     ((const char*[]){
                         "StkChorus",
                         "StkEcho",
                         "StkFreeVerb",
                         "StkJCRev",
                         "StkLentPitShift",
                         "StkNRev",
                         "StkPRCRev",
                         "StkPitShift",
                     })[magic]);

  return obj;

fail:
  JS_FreeValue(ctx, obj);
  return JS_EXCEPTION;
}

static void
js_stkeffect_finalizer(JSRuntime* rt, JSValue val) {
  StkEffectPtr* e;

  if((e = static_cast<StkEffectPtr*>(JS_GetOpaque(val, js_stkeffect_class_id)))) {
    e->~StkEffectPtr();
    js_free_rt(rt, e);
  }
}

static JSClassDef js_stkeffect_class = {
    .class_name = "StkEffect",
    .finalizer = js_stkeffect_finalizer,
};

static const JSCFunctionListEntry js_stkeffect_funcs[] = {
    JS_PROP_STRING_DEF("[Symbol.toStringTag]", "StkEffect", JS_PROP_CONFIGURABLE),
};

enum {
  INSTANCE_BEETHREE = 0,
  INSTANCE_FMVOICES,
  INSTANCE_HEVYMETL,
  INSTANCE_PERCFLUT,
  INSTANCE_RHODEY,
  INSTANCE_TUBEBELL,
  INSTANCE_WURLEY,
};

static JSValue
js_stkfm_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst argv[], int magic) {
  StkFMPtr* fm = static_cast<StkFMPtr*>(js_mallocz(ctx, sizeof(StkFMPtr)));
  double arg = 0;
  if(argc > 0)
    JS_ToFloat64(ctx, &arg, argv[0]);

  switch(magic) {}

  /* using new_target to get the prototype is necessary when the class is extended. */
  JSValue obj = JS_UNDEFINED, proto = JS_GetPropertyStr(ctx, new_target, "prototype");
  if(JS_IsException(proto))
    goto fail;

  if(!JS_IsObject(proto))
    proto = stkfm_proto;

  /* using new_target to get the prototype is necessary when the class is extended. */
  obj = JS_NewObjectProtoClass(ctx, proto, js_stkfm_class_id);
  JS_FreeValue(ctx, proto);

  if(JS_IsException(obj))
    goto fail;

  JS_SetOpaque(obj, fm);
  return obj;

fail:
  JS_FreeValue(ctx, obj);
  return JS_EXCEPTION;
}

static void
js_stkfm_finalizer(JSRuntime* rt, JSValue val) {
  StkFMPtr* fm;

  if((fm = static_cast<StkFMPtr*>(JS_GetOpaque(val, js_stkfm_class_id)))) {
    fm->~StkFMPtr();
    js_free_rt(rt, fm);
  }
}

static JSClassDef js_stkfm_class = {
    .class_name = "StkFM",
    .finalizer = js_stkfm_finalizer,
};

static const JSCFunctionListEntry js_stkfm_funcs[] = {
    JS_PROP_STRING_DEF("[Symbol.toStringTag]", "StkFM", JS_PROP_CONFIGURABLE),
};

enum {
  INSTANCE_BANDEDWG = 0,
  INSTANCE_BLOWBOTL,
  INSTANCE_BLOWHOLE,
  INSTANCE_BOWED,
  INSTANCE_BRASS,
  INSTANCE_CLARINET,
  INSTANCE_DRUMMER,
  INSTANCE_FLUTE,
  INSTANCE_MANDOLIN,
  INSTANCE_MESH2D,
  INSTANCE_PLUCKED,
  INSTANCE_RECORDER,
  INSTANCE_RESONATE,
  INSTANCE_SAXOFONY,
  INSTANCE_SHAKERS,
  INSTANCE_SIMPLE,
  INSTANCE_SITAR,
  INSTANCE_STIFKARP,
  INSTANCE_VOICFORM,
  INSTANCE_WHISTLE,
};

static JSValue
js_stkinstrmnt_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst argv[], int magic) {
  StkInstrmntPtr* i = static_cast<StkInstrmntPtr*>(js_mallocz(ctx, sizeof(StkInstrmntPtr)));
  double arg = 0;
  if(argc > 0)
    JS_ToFloat64(ctx, &arg, argv[0]);

  switch(magic) {
    case INSTANCE_BANDEDWG: {
      *i = std::make_shared<stk::BandedWG>();
      break;
    }
    case INSTANCE_BLOWBOTL: {
      *i = std::make_shared<stk::BlowBotl>();
      break;
    }
    case INSTANCE_BLOWHOLE: {
      *i = std::make_shared<stk::BlowHole>(arg);
      break;
    }
    case INSTANCE_BOWED: {
      *i = std::make_shared<stk::Bowed>(argc > 0 ? arg : 8.0);
      break;
    }
    case INSTANCE_BRASS: {
      *i = std::make_shared<stk::Brass>(argc > 0 ? arg : 8.0);
      break;
    }
    case INSTANCE_CLARINET: {
      *i = std::make_shared<stk::Clarinet>(argc > 0 ? arg : 8.0);
      break;
    }
    case INSTANCE_DRUMMER: {
      *i = std::make_shared<stk::Drummer>();
      break;
    }
    case INSTANCE_FLUTE: {
      *i = std::make_shared<stk::Flute>(arg);
      break;
    }
    case INSTANCE_MANDOLIN: {
      *i = std::make_shared<stk::Mandolin>(arg);
      break;
    }
    case INSTANCE_MESH2D: {
      uint32_t nx, ny;
      JS_ToUint32(ctx, &nx, argv[0]);
      JS_ToUint32(ctx, &ny, argv[1]);
      *i = std::make_shared<stk::Mesh2D>(nx, ny);
      break;
    }
    // case INSTANCE_MODAL: { *i = std::make_shared<stk::Modal>(argc > 0 ? arg : 4); break; }
    case INSTANCE_PLUCKED: {
      *i = std::make_shared<stk::Plucked>(argc > 0 ? arg : 10.0);
      break;
    }
    case INSTANCE_RECORDER: {
      *i = std::make_shared<stk::Recorder>();
      break;
    }
    case INSTANCE_RESONATE: {
      *i = std::make_shared<stk::Resonate>();
      break;
    }
    // case INSTANCE_SAMPLER: { *i = std::make_shared<stk::Sampler>(); break; }
    case INSTANCE_SAXOFONY: {
      *i = std::make_shared<stk::Saxofony>(arg);
      break;
    }
    case INSTANCE_SHAKERS: {
      *i = std::make_shared<stk::Shakers>(argc > 0 ? arg : 0);
      break;
    }
    case INSTANCE_SIMPLE: {
      *i = std::make_shared<stk::Simple>();
      break;
    }
    case INSTANCE_SITAR: {
      *i = std::make_shared<stk::Sitar>(argc > 0 ? arg : 8.0);
      break;
    }
    case INSTANCE_STIFKARP: {
      *i = std::make_shared<stk::StifKarp>(argc > 0 ? arg : 10.0);
      break;
    }
    case INSTANCE_VOICFORM: {
      *i = std::make_shared<stk::VoicForm>();
      break;
    }
    case INSTANCE_WHISTLE: {
      *i = std::make_shared<stk::Whistle>();
      break;
    }
  }

  /* using new_target to get the prototype is necessary when the class is extended. */
  JSValue obj = JS_UNDEFINED, proto = JS_GetPropertyStr(ctx, new_target, "prototype");
  if(JS_IsException(proto))
    goto fail;

  if(!JS_IsObject(proto))
    proto = stkinstrmnt_proto;

  /* using new_target to get the prototype is necessary when the class is extended. */
  obj = JS_NewObjectProtoClass(ctx, proto, js_stkinstrmnt_class_id);
  JS_FreeValue(ctx, proto);

  if(JS_IsException(obj))
    goto fail;

  JS_SetOpaque(obj, i);
  return obj;

fail:
  JS_FreeValue(ctx, obj);
  return JS_EXCEPTION;
}

static void
js_stkinstrmnt_finalizer(JSRuntime* rt, JSValue val) {
  StkInstrmntPtr* i;

  if((i = static_cast<StkInstrmntPtr*>(JS_GetOpaque(val, js_stkinstrmnt_class_id)))) {
    i->~StkInstrmntPtr();
    js_free_rt(rt, i);
  }
}

static JSClassDef js_stkinstrmnt_class = {
    .class_name = "StkInstrmnt",
    .finalizer = js_stkinstrmnt_finalizer,
};

static const JSCFunctionListEntry js_stkinstrmnt_funcs[] = {
    JS_PROP_STRING_DEF("[Symbol.toStringTag]", "StkInstrmnt", JS_PROP_CONFIGURABLE),
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

  JSValue ctor;

  if(m) {
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkfilter_constructor, "BiQuad", 0, JS_CFUNC_constructor_magic, INSTANCE_BIQUAD);
    JS_SetModuleExport(ctx, m, "StkBiQuad", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkfilter_constructor, "DelayA", 0, JS_CFUNC_constructor_magic, INSTANCE_DELAY_A);
    JS_SetModuleExport(ctx, m, "StkDelayA", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkfilter_constructor, "DelayL", 0, JS_CFUNC_constructor_magic, INSTANCE_DELAY_L);
    JS_SetModuleExport(ctx, m, "StkDelayL", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkfilter_constructor, "Delay", 0, JS_CFUNC_constructor_magic, INSTANCE_DELAY);
    JS_SetModuleExport(ctx, m, "StkDelay", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkfilter_constructor, "Fir", 1, JS_CFUNC_constructor_magic, INSTANCE_FIR);
    JS_SetModuleExport(ctx, m, "StkFir", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkfilter_constructor, "FormSwep", 0, JS_CFUNC_constructor_magic, INSTANCE_FORM_SWEP);
    JS_SetModuleExport(ctx, m, "StkFormSwep", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkfilter_constructor, "Iir", 0, JS_CFUNC_constructor_magic, INSTANCE_IIR);
    JS_SetModuleExport(ctx, m, "StkIir", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkfilter_constructor, "OnePole", 0, JS_CFUNC_constructor_magic, INSTANCE_ONE_POLE);
    JS_SetModuleExport(ctx, m, "StkOnePole", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkfilter_constructor, "OneZero", 0, JS_CFUNC_constructor_magic, INSTANCE_ONE_ZERO);
    JS_SetModuleExport(ctx, m, "StkOneZero", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkfilter_constructor, "PoleZero", 0, JS_CFUNC_constructor_magic, INSTANCE_POLE_ZERO);
    JS_SetModuleExport(ctx, m, "StkPoleZero", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkfilter_constructor, "TapDelay", 0, JS_CFUNC_constructor_magic, INSTANCE_TAP_DELAY);
    JS_SetModuleExport(ctx, m, "StkTapDelay", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkfilter_constructor, "TwoPole", 0, JS_CFUNC_constructor_magic, INSTANCE_TWO_POLE);
    JS_SetModuleExport(ctx, m, "StkTwoPole", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkfilter_constructor, "TwoZero", 0, JS_CFUNC_constructor_magic, INSTANCE_TWO_ZERO);
    JS_SetModuleExport(ctx, m, "StkTwoZero", ctor);
  }
  JS_NewClassID(&js_stkgenerator_class_id);
  JS_NewClass(JS_GetRuntime(ctx), js_stkgenerator_class_id, &js_stkgenerator_class);

  stkgenerator_ctor = JS_NewObject(ctx); // JS_NewCFunction2(ctx, js_stkgenerator_constructor, "StkFilter", 1, JS_CFUNC_constructor, 0);
  stkgenerator_proto = JS_NewObject(ctx);

  JS_SetPropertyFunctionList(ctx, stkgenerator_proto, js_stkgenerator_funcs, countof(js_stkgenerator_funcs));

  JS_SetClassProto(ctx, js_stkgenerator_class_id, stkgenerator_proto);

  if(m) {
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkgenerator_constructor, "ADSR", 0, JS_CFUNC_constructor_magic, INSTANCE_ADSR);
    JS_SetModuleExport(ctx, m, "StkADSR", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkgenerator_constructor, "Asymp", 0, JS_CFUNC_constructor_magic, INSTANCE_ASYMP);
    JS_SetModuleExport(ctx, m, "StkAsymp", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkgenerator_constructor, "BlitSaw", 0, JS_CFUNC_constructor_magic, INSTANCE_BLIT_SAW);
    JS_SetModuleExport(ctx, m, "StkBlitSaw", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkgenerator_constructor, "BlitSquare", 0, JS_CFUNC_constructor_magic, INSTANCE_BLIT_SQUARE);
    JS_SetModuleExport(ctx, m, "StkBlitSquare", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkgenerator_constructor, "Blit", 0, JS_CFUNC_constructor_magic, INSTANCE_BLIT);
    JS_SetModuleExport(ctx, m, "StkBlit", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkgenerator_constructor, "Envelope", 0, JS_CFUNC_constructor_magic, INSTANCE_ENVELOPE);
    JS_SetModuleExport(ctx, m, "StkEnvelope", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkgenerator_constructor, "Granulate", 0, JS_CFUNC_constructor_magic, INSTANCE_GRANULATE);
    JS_SetModuleExport(ctx, m, "StkGranulate", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkgenerator_constructor, "Modulate", 0, JS_CFUNC_constructor_magic, INSTANCE_MODULATE);
    JS_SetModuleExport(ctx, m, "StkModulate", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkgenerator_constructor, "Noise", 0, JS_CFUNC_constructor_magic, INSTANCE_NOISE);
    JS_SetModuleExport(ctx, m, "StkNoise", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkgenerator_constructor, "SineWave", 0, JS_CFUNC_constructor_magic, INSTANCE_SINE_WAVE);
    JS_SetModuleExport(ctx, m, "StkSineWave", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkgenerator_constructor, "SingWave", 0, JS_CFUNC_constructor_magic, INSTANCE_SING_WAVE);
    JS_SetModuleExport(ctx, m, "StkStkSingWave", ctor);
  }

  JS_NewClassID(&js_stkeffect_class_id);
  JS_NewClass(JS_GetRuntime(ctx), js_stkeffect_class_id, &js_stkeffect_class);

  stkeffect_ctor = JS_NewObject(ctx); // JS_NewCFunction2(ctx, js_stkeffect_constructor, "StkGenerator", 1, JS_CFUNC_constructor, 0);
  stkeffect_proto = JS_NewObject(ctx);

  JS_SetPropertyFunctionList(ctx, stkeffect_proto, js_stkeffect_funcs, countof(js_stkeffect_funcs));

  JS_SetClassProto(ctx, js_stkeffect_class_id, stkeffect_proto);

  stkinstrmnt_ctor = JS_NewObject(ctx); // JS_NewCFunction2(ctx, js_stkinstrmnt_constructor, "StkGenerator", 1, JS_CFUNC_constructor, 0);
  stkinstrmnt_proto = JS_NewObject(ctx);

  JS_SetPropertyFunctionList(ctx, stkinstrmnt_proto, js_stkinstrmnt_funcs, countof(js_stkinstrmnt_funcs));

  JS_SetClassProto(ctx, js_stkinstrmnt_class_id, stkinstrmnt_proto);

  if(m) {
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkinstrmnt_constructor, "BandedWG", 0, JS_CFUNC_constructor_magic, INSTANCE_BANDEDWG);
    JS_SetModuleExport(ctx, m, "StkBandedWG", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkinstrmnt_constructor, "BlowBotl", 0, JS_CFUNC_constructor_magic, INSTANCE_BLOWBOTL);
    JS_SetModuleExport(ctx, m, "StkBlowBotl", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkinstrmnt_constructor, "BlowHole", 0, JS_CFUNC_constructor_magic, INSTANCE_BLOWHOLE);
    JS_SetModuleExport(ctx, m, "StkBlowHole", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkinstrmnt_constructor, "Bowed", 0, JS_CFUNC_constructor_magic, INSTANCE_BOWED);
    JS_SetModuleExport(ctx, m, "StkBowed", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkinstrmnt_constructor, "Brass", 0, JS_CFUNC_constructor_magic, INSTANCE_BRASS);
    JS_SetModuleExport(ctx, m, "StkBrass", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkinstrmnt_constructor, "Clarinet", 0, JS_CFUNC_constructor_magic, INSTANCE_CLARINET);
    JS_SetModuleExport(ctx, m, "StkClarinet", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkinstrmnt_constructor, "Drummer", 0, JS_CFUNC_constructor_magic, INSTANCE_DRUMMER);
    JS_SetModuleExport(ctx, m, "StkDrummer", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkinstrmnt_constructor, "Flute", 0, JS_CFUNC_constructor_magic, INSTANCE_FLUTE);
    JS_SetModuleExport(ctx, m, "StkFlute", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkinstrmnt_constructor, "Mandolin", 0, JS_CFUNC_constructor_magic, INSTANCE_MANDOLIN);
    JS_SetModuleExport(ctx, m, "StkMandolin", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkinstrmnt_constructor, "Mesh2D", 0, JS_CFUNC_constructor_magic, INSTANCE_MESH2D);
    JS_SetModuleExport(ctx, m, "StkMesh2D", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkinstrmnt_constructor, "Plucked", 0, JS_CFUNC_constructor_magic, INSTANCE_PLUCKED);
    JS_SetModuleExport(ctx, m, "StkPlucked", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkinstrmnt_constructor, "Recorder", 0, JS_CFUNC_constructor_magic, INSTANCE_RECORDER);
    JS_SetModuleExport(ctx, m, "StkRecorder", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkinstrmnt_constructor, "Resonate", 0, JS_CFUNC_constructor_magic, INSTANCE_RESONATE);
    JS_SetModuleExport(ctx, m, "StkResonate", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkinstrmnt_constructor, "Saxofony", 0, JS_CFUNC_constructor_magic, INSTANCE_SAXOFONY);
    JS_SetModuleExport(ctx, m, "StkSaxofony", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkinstrmnt_constructor, "Shakers", 0, JS_CFUNC_constructor_magic, INSTANCE_SHAKERS);
    JS_SetModuleExport(ctx, m, "StkShakers", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkinstrmnt_constructor, "Simple", 0, JS_CFUNC_constructor_magic, INSTANCE_SIMPLE);
    JS_SetModuleExport(ctx, m, "StkSimple", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkinstrmnt_constructor, "Sitar", 0, JS_CFUNC_constructor_magic, INSTANCE_SITAR);
    JS_SetModuleExport(ctx, m, "StkSitar", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkinstrmnt_constructor, "StifKarp", 0, JS_CFUNC_constructor_magic, INSTANCE_STIFKARP);
    JS_SetModuleExport(ctx, m, "StkStifKarp", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkinstrmnt_constructor, "VoicForm", 0, JS_CFUNC_constructor_magic, INSTANCE_VOICFORM);
    JS_SetModuleExport(ctx, m, "StkVoicForm", ctor);
    ctor = JS_NewCFunction2(ctx, (JSCFunction*)js_stkinstrmnt_constructor, "Whistle", 0, JS_CFUNC_constructor_magic, INSTANCE_WHISTLE);
    JS_SetModuleExport(ctx, m, "StkWhistle", ctor);

    JS_SetModuleExport(ctx, m, "Stk", stk_ctor);
    JS_SetModuleExport(ctx, m, "StkFrames", stkframes_ctor);
    JS_SetModuleExport(ctx, m, "StkGenerator", stkfilter_ctor);
    JS_SetModuleExport(ctx, m, "StkFilter", stkgenerator_ctor);
  }

  return 0;
}

extern "C" VISIBLE void
js_init_module_stk(JSContext* ctx, JSModuleDef* m) {
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
  JS_AddModuleExport(ctx, m, "StkBandedWG");
  JS_AddModuleExport(ctx, m, "StkBlowBotl");
  JS_AddModuleExport(ctx, m, "StkBlowHole");
  JS_AddModuleExport(ctx, m, "StkBowed");
  JS_AddModuleExport(ctx, m, "StkBrass");
  JS_AddModuleExport(ctx, m, "StkClarinet");
  JS_AddModuleExport(ctx, m, "StkDrummer");
  JS_AddModuleExport(ctx, m, "StkFlute");
  JS_AddModuleExport(ctx, m, "StkMandolin");
  JS_AddModuleExport(ctx, m, "StkMesh2D");
  JS_AddModuleExport(ctx, m, "StkPlucked");
  JS_AddModuleExport(ctx, m, "StkRecorder");
  JS_AddModuleExport(ctx, m, "StkResonate");
  JS_AddModuleExport(ctx, m, "StkSaxofony");
  JS_AddModuleExport(ctx, m, "StkShakers");
  JS_AddModuleExport(ctx, m, "StkSimple");
  JS_AddModuleExport(ctx, m, "StkSitar");
  JS_AddModuleExport(ctx, m, "StkStifKarp");
  JS_AddModuleExport(ctx, m, "StkVoicForm");
  JS_AddModuleExport(ctx, m, "StkWhistle");
  JS_AddModuleExport(ctx, m, "Stk");
  JS_AddModuleExport(ctx, m, "StkFrames");
  JS_AddModuleExport(ctx, m, "StkGenerator");
  JS_AddModuleExport(ctx, m, "StkFilter");
}
