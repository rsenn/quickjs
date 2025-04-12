#include <quickjs.h>
#include <cutils.h>
#include "defines.h"
#include "LabSound/LabSound.h"
#include "LabSound/backends/AudioDevice_RtAudio.h"

extern int js_stk_init(JSContext* ctx, JSModuleDef* m);

extern "C" void js_init_module_stk(JSContext* ctx, JSModuleDef*);

/*VISIBLE*/ JSClassID js_audiocontext_class_id = 0, js_audiodestinationnode_class_id = 0, js_audiolistener_class_id = 0, js_audiodevice_class_id = 0;
/*VISIBLE*/ JSValue audiocontext_proto = {{0}, JS_TAG_UNDEFINED}, audiocontext_ctor = {{0}, JS_TAG_UNDEFINED}, audiodestinationnode_proto = {{0}, JS_TAG_UNDEFINED},
                    audiodestinationnode_ctor = {{0}, JS_TAG_UNDEFINED}, audiolistener_proto = {{0}, JS_TAG_UNDEFINED}, audiolistener_ctor = {{0}, JS_TAG_UNDEFINED},
                    audiodevice_proto = {{0}, JS_TAG_UNDEFINED}, audiodevice_ctor = {{0}, JS_TAG_UNDEFINED};

typedef std::shared_ptr<lab::AudioContext> AudioContextPtr;
typedef std::shared_ptr<lab::AudioDestinationNode> AudioDestinationNodePtr;
typedef std::shared_ptr<lab::AudioListener> AudioListenerPtr;
typedef std::shared_ptr<lab::AudioDevice> AudioDevicePtr;

static JSValue
js_audiocontext_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst argv[]) {
  JSValue proto, obj = JS_UNDEFINED;
  bool isOffline = false, autoDispatchEvents = true;

  if(argc > 0)
    isOffline = JS_ToBool(ctx, argv[0]);
  if(argc > 1)
    autoDispatchEvents = JS_ToBool(ctx, argv[1]);

  AudioContextPtr* sac = static_cast<AudioContextPtr*>(js_mallocz(ctx, sizeof(AudioContextPtr)));

  new(sac) AudioContextPtr(std::make_shared<lab::AudioContext>(isOffline, autoDispatchEvents));

  /* using new_target to get the prototype is necessary when the class is extended. */
  proto = JS_GetPropertyStr(ctx, new_target, "prototype");
  if(JS_IsException(proto))
    goto fail;

  if(!JS_IsObject(proto))
    proto = audiocontext_proto;

  /* using new_target to get the prototype is necessary when the class is extended. */
  obj = JS_NewObjectProtoClass(ctx, proto, js_audiocontext_class_id);
  JS_FreeValue(ctx, proto);

  if(JS_IsException(obj))
    goto fail;

  JS_SetOpaque(obj, sac);
  return obj;

fail:
  JS_FreeValue(ctx, obj);
  return JS_EXCEPTION;
}

enum {
  PROP_SAMPLERATE,
  PROP_DESTINATION_NODE,
  PROP_LISTENER,
  PROP_CURRENTTIME,
  PROP_CURRENTSAMPLEFRAME,
  PROP_PREDICTED_CURRENTTIME,
};

static JSValue
js_audiocontext_get(JSContext* ctx, JSValueConst this_val, int magic) {
  AudioContextPtr* sac;
  JSValue ret = JS_UNDEFINED;

  if(!(sac = static_cast<AudioContextPtr*>(JS_GetOpaque2(ctx, this_val, js_audiocontext_class_id))))
    return JS_EXCEPTION;

  switch(magic) {
    case PROP_SAMPLERATE: {
      ret = JS_NewFloat64(ctx, (*sac)->sampleRate());
      break;
    }

    case PROP_DESTINATION_NODE: {
      AudioDestinationNodePtr sadn = (*sac)->destinationNode();

      ret = JS_NewObjectProtoClass(ctx, audiodestinationnode_proto, js_audiodestinationnode_class_id);

      AudioDestinationNodePtr* ptr = static_cast<AudioDestinationNodePtr*>(js_mallocz(ctx, sizeof(AudioDestinationNodePtr)));

      new(ptr) AudioDestinationNodePtr(sadn);

      JS_SetOpaque(ret, ptr);
      break;
    }

    case PROP_LISTENER: {
      AudioListenerPtr sal = (*sac)->listener();

      ret = JS_NewObjectProtoClass(ctx, audiolistener_proto, js_audiolistener_class_id);

      AudioListenerPtr* ptr = static_cast<AudioListenerPtr*>(js_mallocz(ctx, sizeof(AudioListenerPtr)));

      new(ptr) AudioListenerPtr(sal);

      JS_SetOpaque(ret, ptr);
      break;
    }

    case PROP_CURRENTTIME: {
      ret = JS_NewFloat64(ctx, (*sac)->currentTime());
      break;
    }
    case PROP_CURRENTSAMPLEFRAME: {
      ret = JS_NewInt64(ctx, (*sac)->currentSampleFrame());
      break;
    }

    case PROP_PREDICTED_CURRENTTIME: {
      ret = JS_NewFloat64(ctx, (*sac)->predictedCurrentTime());
      break;
    }
  }

  return ret;
}

static JSValue
js_audiocontext_set(JSContext* ctx, JSValueConst this_val, JSValueConst value, int magic) {
  AudioContextPtr* sac;
  JSValue ret = JS_UNDEFINED;

  if(!(sac = static_cast<AudioContextPtr*>(JS_GetOpaque2(ctx, this_val, js_audiocontext_class_id))))
    return JS_EXCEPTION;

  switch(magic) {
    case PROP_DESTINATION_NODE: {
      AudioDestinationNodePtr* sadn;

      if(!(sadn = static_cast<AudioDestinationNodePtr*>(JS_GetOpaque2(ctx, value, js_audiodestinationnode_class_id))))
        return JS_ThrowInternalError(ctx, "value must be AudioDestinationNode");

      (*sac)->setDestinationNode(*sadn);

      break;
    }
  }

  return ret;
}

static void
js_audiocontext_finalizer(JSRuntime* rt, JSValue val) {
  AudioContextPtr* sac;

  if((sac = static_cast<AudioContextPtr*>(JS_GetOpaque(val, js_audiocontext_class_id)))) {
    sac->~AudioContextPtr();
    js_free_rt(rt, sac);
  }
}

static JSClassDef js_audiocontext_class = {
    .class_name = "AudioContext",
    .finalizer = js_audiocontext_finalizer,
};

static const JSCFunctionListEntry js_audiocontext_funcs[] = {
    JS_CGETSET_MAGIC_DEF("sampleRate", js_audiocontext_get, 0, PROP_SAMPLERATE),
    JS_CGETSET_MAGIC_DEF("destinationNode", js_audiocontext_get, js_audiocontext_set, PROP_DESTINATION_NODE),
    JS_CGETSET_MAGIC_DEF("listener", js_audiocontext_get, 0, PROP_LISTENER),
    JS_CGETSET_MAGIC_DEF("currentTime", js_audiocontext_get, 0, PROP_CURRENTTIME),
    JS_CGETSET_MAGIC_DEF("currentSampleFrame", js_audiocontext_get, 0, PROP_CURRENTSAMPLEFRAME),
    JS_CGETSET_MAGIC_DEF("predictedCurrentTime", js_audiocontext_get, 0, PROP_PREDICTED_CURRENTTIME),
    JS_PROP_STRING_DEF("[Symbol.toStringTag]", "AudioContext", JS_PROP_CONFIGURABLE),
};

static JSValue
js_audiodestinationnode_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst argv[]) {
  JSValue proto, obj = JS_UNDEFINED;
  lab::AudioContext* ac = nullptr;
  std::shared_ptr<lab::AudioDevice> device;

  if(argc > 0) {
    AudioContextPtr* acptr;

    if(!(acptr = static_cast<AudioContextPtr*>(JS_GetOpaque2(ctx, argv[0], js_audiocontext_class_id))))
      return JS_EXCEPTION;

    ac = acptr->get();
  }

  if(!ac) {
    return JS_ThrowInternalError(ctx, "argument 1 must be AudioContext");
  }

  if(argc > 1) {
    AudioDevicePtr* adptr;

    if(!(adptr = static_cast<AudioDevicePtr*>(JS_GetOpaque2(ctx, argv[1], js_audiodevice_class_id))))
      return JS_EXCEPTION;

    device = *adptr;
  }

  if(!device.get()) {
    return JS_ThrowInternalError(ctx, "argument 2 must be AudioDevice");
  }

  AudioDestinationNodePtr* sadn = static_cast<AudioDestinationNodePtr*>(js_mallocz(ctx, sizeof(AudioDestinationNodePtr)));

  new(sadn) AudioDestinationNodePtr(std::make_shared<lab::AudioDestinationNode>(*ac, device));

  /* using new_target to get the prototype is necessary when the class is extended. */
  proto = JS_GetPropertyStr(ctx, new_target, "prototype");
  if(JS_IsException(proto))
    goto fail;

  if(!JS_IsObject(proto))
    proto = audiodestinationnode_proto;

  /* using new_target to get the prototype is necessary when the class is extended. */
  obj = JS_NewObjectProtoClass(ctx, proto, js_audiodestinationnode_class_id);
  JS_FreeValue(ctx, proto);

  if(JS_IsException(obj))
    goto fail;

  JS_SetOpaque(obj, sadn);
  return obj;

fail:
  JS_FreeValue(ctx, obj);
  return JS_EXCEPTION;
}

enum {
  PROP_NAME,
  PROP_DEVICE,
};

static JSValue
js_audiodestinationnode_get(JSContext* ctx, JSValueConst this_val, int magic) {
  AudioDestinationNodePtr* sadn;
  JSValue ret = JS_UNDEFINED;

  if(!(sadn = static_cast<AudioDestinationNodePtr*>(JS_GetOpaque2(ctx, this_val, js_audiodestinationnode_class_id))))
    return JS_EXCEPTION;

  if(sadn->get() == nullptr)
    return JS_UNDEFINED;

  switch(magic) {
    case PROP_NAME: {
      ret = JS_NewString(ctx, (*sadn)->name());
      break;
    }

      /* case PROP_DEVICE: {
          lab::AudioDevice* ad = (*sadn)->device();

         ret = JS_NewObjectProtoClass(ctx, audiodevice_proto, js_audiodevice_class_id);

         AudioDevicePtr* ptr = static_cast<AudioDevicePtr*>(js_mallocz(ctx, sizeof(AudioDevicePtr)));

         new(ptr) AudioDevicePtr(*ad);

         JS_SetOpaque(ret, ptr);
         break;
       }*/
  }

  return ret;
}

enum {
  AUDIODESTINATION_RESET,
};

static JSValue
js_audiodestinationnode_method(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst argv[], int magic) {
  AudioDestinationNodePtr* sadn;
  JSValue ret = JS_UNDEFINED;

  if(!(sadn = static_cast<AudioDestinationNodePtr*>(JS_GetOpaque2(ctx, this_val, js_audiodestinationnode_class_id))))
    return JS_EXCEPTION;

  switch(magic) {
    case AUDIODESTINATION_RESET: {
      sadn->reset();
      break;
    }
  }

  return ret;
}

static void
js_audiodestinationnode_finalizer(JSRuntime* rt, JSValue val) {
  AudioDestinationNodePtr* sadn;

  if((sadn = static_cast<AudioDestinationNodePtr*>(JS_GetOpaque(val, js_audiodestinationnode_class_id)))) {
    sadn->~AudioDestinationNodePtr();
    js_free_rt(rt, sadn);
  }
}

static JSClassDef js_audiodestinationnode_class = {
    .class_name = "AudioDestinationNode",
    .finalizer = js_audiodestinationnode_finalizer,
};

static const JSCFunctionListEntry js_audiodestinationnode_funcs[] = {
    JS_CGETSET_MAGIC_DEF("name", js_audiodestinationnode_get, 0, PROP_NAME),
    JS_CFUNC_MAGIC_DEF("reset", 0, js_audiodestinationnode_method, AUDIODESTINATION_RESET),
    // JS_CGETSET_MAGIC_DEF("device", js_audiodestinationnode_get, 0, PROP_DEVICE),
    JS_PROP_STRING_DEF("[Symbol.toStringTag]", "AudioDestinationNode", JS_PROP_CONFIGURABLE),
};

static JSValue
js_audiolistener_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst argv[]) {
  JSValue proto, obj = JS_UNDEFINED;

  AudioListenerPtr* sal = static_cast<AudioListenerPtr*>(js_mallocz(ctx, sizeof(AudioListenerPtr)));

  new(sal) AudioListenerPtr(std::make_shared<lab::AudioListener>());

  /* using new_target to get the prototype is necessary when the class is extended. */
  proto = JS_GetPropertyStr(ctx, new_target, "prototype");
  if(JS_IsException(proto))
    goto fail;

  if(!JS_IsObject(proto))
    proto = audiolistener_proto;

  /* using new_target to get the prototype is necessary when the class is extended. */
  obj = JS_NewObjectProtoClass(ctx, proto, js_audiolistener_class_id);
  JS_FreeValue(ctx, proto);

  if(JS_IsException(obj))
    goto fail;

  JS_SetOpaque(obj, sal);
  return obj;

fail:
  JS_FreeValue(ctx, obj);
  return JS_EXCEPTION;
}

enum {
  PROP_POSITION_X,
  PROP_POSITION_Y,
  PROP_POSITION_Z,
};

static JSValue
js_audiolistener_get(JSContext* ctx, JSValueConst this_val, int magic) {
  AudioListenerPtr* sal;
  JSValue ret = JS_UNDEFINED;

  if(!(sal = static_cast<AudioListenerPtr*>(JS_GetOpaque2(ctx, this_val, js_audiolistener_class_id))))
    return JS_EXCEPTION;

  switch(magic) {
    case PROP_POSITION_X: {
      break;
    }

    case PROP_POSITION_Y: {
      break;
    }

    case PROP_POSITION_Z: {
      break;
    }
  }

  return ret;
}

static void
js_audiolistener_finalizer(JSRuntime* rt, JSValue val) {
  AudioListenerPtr* sal;

  if((sal = static_cast<AudioListenerPtr*>(JS_GetOpaque(val, js_audiolistener_class_id)))) {
    sal->~AudioListenerPtr();
    js_free_rt(rt, sal);
  }
}

static JSClassDef js_audiolistener_class = {
    .class_name = "AudioListener",
    .finalizer = js_audiolistener_finalizer,
};

static const JSCFunctionListEntry js_audiolistener_funcs[] = {
    JS_CGETSET_MAGIC_DEF("positionX", js_audiolistener_get, 0, PROP_POSITION_X),
    JS_CGETSET_MAGIC_DEF("positionY", js_audiolistener_get, 0, PROP_POSITION_Y),
    JS_CGETSET_MAGIC_DEF("positionZ", js_audiolistener_get, 0, PROP_POSITION_Z),
    JS_PROP_STRING_DEF("[Symbol.toStringTag]", "AudioListener", JS_PROP_CONFIGURABLE),
};

static JSValue
js_audiodevice_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst argv[]) {
  JSValue proto, obj = JS_UNDEFINED;

  lab::AudioStreamConfig in_config, out_config;
  AudioDevicePtr* sad = static_cast<AudioDevicePtr*>(js_mallocz(ctx, sizeof(AudioDevicePtr)));

  in_config.device_index = 0;
  in_config.desired_channels = 2;
  in_config.desired_samplerate = 44100;

  out_config.device_index = 0;
  out_config.desired_channels = 2;
  out_config.desired_samplerate = 44100;

  new(sad) AudioDevicePtr(std::make_shared<lab::AudioDevice_RtAudio>(in_config, out_config));

  /* using new_target to get the prototype is necessary when the class is extended. */
  proto = JS_GetPropertyStr(ctx, new_target, "prototype");
  if(JS_IsException(proto))
    goto fail;

  if(!JS_IsObject(proto))
    proto = audiodevice_proto;

  /* using new_target to get the prototype is necessary when the class is extended. */
  obj = JS_NewObjectProtoClass(ctx, proto, js_audiodevice_class_id);
  JS_FreeValue(ctx, proto);

  if(JS_IsException(obj))
    goto fail;

  JS_SetOpaque(obj, sad);
  return obj;

fail:
  JS_FreeValue(ctx, obj);
  return JS_EXCEPTION;
}

enum { DEVICE_DESTINATION_NODE };

static JSValue
js_audiodevice_get(JSContext* ctx, JSValueConst this_val, int magic) {
  AudioDevicePtr* sad;
  JSValue ret = JS_UNDEFINED;

  if(!(sad = static_cast<AudioDevicePtr*>(JS_GetOpaque2(ctx, this_val, js_audiodevice_class_id))))
    return JS_EXCEPTION;

  switch(magic) {}

  return ret;
}

static JSValue
js_audiodevice_set(JSContext* ctx, JSValueConst this_val, JSValueConst value, int magic) {
  AudioDevicePtr* sad;
  JSValue ret = JS_UNDEFINED;

  if(!(sad = static_cast<AudioDevicePtr*>(JS_GetOpaque2(ctx, this_val, js_audiodevice_class_id))))
    return JS_EXCEPTION;

  switch(magic) {
    case DEVICE_DESTINATION_NODE: {
      AudioDestinationNodePtr* sadn;

      if(!(sadn = static_cast<AudioDestinationNodePtr*>(JS_GetOpaque2(ctx, value, js_audiodestinationnode_class_id))))
        return JS_ThrowInternalError(ctx, "value must be AudioDestinationNode");

      (*sad)->setDestinationNode(*sadn);
      break;
    }
  }

  return ret;
}

static void
js_audiodevice_finalizer(JSRuntime* rt, JSValue val) {
  AudioDevicePtr* sad;

  if((sad = static_cast<AudioDevicePtr*>(JS_GetOpaque(val, js_audiodevice_class_id)))) {
    sad->~AudioDevicePtr();
    js_free_rt(rt, sad);
  }
}

static JSClassDef js_audiodevice_class = {
    .class_name = "AudioDevice",
    .finalizer = js_audiodevice_finalizer,
};

static const JSCFunctionListEntry js_audiodevice_funcs[] = {
    JS_CGETSET_MAGIC_DEF("destinationNode", 0, js_audiodevice_set, DEVICE_DESTINATION_NODE),

    JS_PROP_STRING_DEF("[Symbol.toStringTag]", "AudioDevice", JS_PROP_CONFIGURABLE),
};

int
js_labsound_init(JSContext* ctx, JSModuleDef* m) {
  JS_NewClassID(&js_audiocontext_class_id);
  JS_NewClass(JS_GetRuntime(ctx), js_audiocontext_class_id, &js_audiocontext_class);

  audiocontext_ctor = JS_NewCFunction2(ctx, js_audiocontext_constructor, "AudioContext", 1, JS_CFUNC_constructor, 0);
  audiocontext_proto = JS_NewObject(ctx);

  JS_SetPropertyFunctionList(ctx, audiocontext_proto, js_audiocontext_funcs, countof(js_audiocontext_funcs));

  JS_SetClassProto(ctx, js_audiocontext_class_id, audiocontext_proto);

  JS_NewClassID(&js_audiodestinationnode_class_id);
  JS_NewClass(JS_GetRuntime(ctx), js_audiodestinationnode_class_id, &js_audiodestinationnode_class);

  audiodestinationnode_ctor = JS_NewCFunction2(ctx, js_audiodestinationnode_constructor, "AudioDestinationNode", 1, JS_CFUNC_constructor, 0);
  audiodestinationnode_proto = JS_NewObject(ctx);

  JS_SetPropertyFunctionList(ctx, audiodestinationnode_proto, js_audiodestinationnode_funcs, countof(js_audiodestinationnode_funcs));

  JS_SetClassProto(ctx, js_audiodestinationnode_class_id, audiodestinationnode_proto);

  JS_NewClassID(&js_audiolistener_class_id);
  JS_NewClass(JS_GetRuntime(ctx), js_audiolistener_class_id, &js_audiolistener_class);

  audiolistener_ctor = JS_NewCFunction2(ctx, js_audiolistener_constructor, "AudioListener", 1, JS_CFUNC_constructor, 0);
  audiolistener_proto = JS_NewObject(ctx);

  JS_SetPropertyFunctionList(ctx, audiolistener_proto, js_audiolistener_funcs, countof(js_audiolistener_funcs));

  JS_SetClassProto(ctx, js_audiolistener_class_id, audiolistener_proto);

  JS_NewClassID(&js_audiodevice_class_id);
  JS_NewClass(JS_GetRuntime(ctx), js_audiodevice_class_id, &js_audiodevice_class);

  audiodevice_ctor = JS_NewCFunction2(ctx, js_audiodevice_constructor, "AudioDevice", 1, JS_CFUNC_constructor, 0);
  audiodevice_proto = JS_NewObject(ctx);

  JS_SetPropertyFunctionList(ctx, audiodevice_proto, js_audiodevice_funcs, countof(js_audiodevice_funcs));

  JS_SetClassProto(ctx, js_audiodevice_class_id, audiodevice_proto);

  if(m) {
    JS_SetModuleExport(ctx, m, "AudioContext", audiocontext_ctor);
    JS_SetModuleExport(ctx, m, "AudioDestinationNode", audiodestinationnode_ctor);
    JS_SetModuleExport(ctx, m, "AudioListener", audiolistener_ctor);
    JS_SetModuleExport(ctx, m, "AudioDevice", audiodevice_ctor);
  }

  js_stk_init(ctx, m);

  return 0;
}

extern "C" VISIBLE JSModuleDef*
js_init_module(JSContext* ctx, const char* module_name) {
  JSModuleDef* m;

  if((m = JS_NewCModule(ctx, module_name, js_labsound_init))) {
    JS_AddModuleExport(ctx, m, "AudioContext");
    JS_AddModuleExport(ctx, m, "AudioDestinationNode");
    JS_AddModuleExport(ctx, m, "AudioListener");
    JS_AddModuleExport(ctx, m, "AudioDevice");
    js_init_module_stk(ctx, m);
  }

  return m;
}
