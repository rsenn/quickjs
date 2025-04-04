#include <quickjs.h>
#include <cutils.h>
#include <libwebsockets.h>

static JSValue lws_context_proto, lws_context_ctor;
static JSClassID lws_context_class_id;
static const JSCFunctionListEntry lws_context_proto_funcs[] = {
    JS_PROP_STRING_DEF("[Symbol.toStringTag]", "LwsContext", JS_PROP_CONFIGURABLE),
};

static const void*
lws_context_getarray(JSContext* ctx,
                     JSValueConst value,
                     void* get_fn(JSContext*, JSValueConst)) {
  const void **arr, *pro;

  if(JS_IsArray(ctx, value)) {
    int32_t len = -1;
    JS_GetPropertyStr(ctx, value, "length");
    JS_ToInt32(ctx, &len, value);

    if(len > 0) {
      arr = js_mallocz(ctx, (len + 1) * sizeof(void*));

      for(int32_t i = 0; i < len; len++) {
        JSValue item = JS_GetPropertyUint32(ctx, value, i);

        if((pro = get_fn(ctx, item))) {
          arr[i] = pro;
        } else {
          break;
        }
      }
    }
  }

  JS_FreeValue(ctx, value);

  return arr;
}

static void*
lws_context_protocols(JSContext* ctx, JSValueConst obj) {
  struct lws_protocols* pro;
  JSValue value;

  if(!(pro = js_malloc(ctx, sizeof(struct lws_protocols))))
    return 0;

  uint32_t u;
  value = JS_GetPropertyStr(ctx, obj, "name");
  pro->name = JS_ToCString(ctx, value);
  JS_FreeValue(ctx, value);
  value = JS_GetPropertyStr(ctx, obj, "callback");
  JS_FreeValue(ctx, value);
  value = JS_GetPropertyStr(ctx, obj, "per_session_data_size");
  JS_ToUint32(ctx, &u, value);
  pro->per_session_data_size = u;
  JS_FreeValue(ctx, value);
  value = JS_GetPropertyStr(ctx, obj, "rx_buffer_size");
  JS_ToUint32(ctx, &u, value);
  pro->rx_buffer_size = u;
  JS_FreeValue(ctx, value);
  value = JS_GetPropertyStr(ctx, obj, "id");
  JS_ToUint32(ctx, &u, value);
  pro->id = u;
  JS_FreeValue(ctx, value);
  value = JS_GetPropertyStr(ctx, obj, "tx_packet_size");
  JS_ToUint32(ctx, &u, value);
  pro->tx_packet_size = u;
  JS_FreeValue(ctx, value);

  value = JS_GetPropertyStr(ctx, obj, "user");

  if(JS_IsObject(value))
    pro->user = JS_VALUE_GET_OBJ(value);

  JS_FreeValue(ctx, value);

  return pro;
}

static struct lws_http_mount*
lws_context_http_mount(JSContext* ctx, JSValueConst obj) {
  struct lws_http_mount* mnt;
  JSValue value;

  if(!(mnt = js_malloc(ctx, sizeof(struct lws_http_mount))))
    return 0;

  if(JS_IsArray(ctx, obj)) {
    value = JS_GetPropertyUint32(ctx, obj, 0);
    mnt->mountpoint = JS_ToCString(ctx, value);
    JS_FreeValue(ctx, value);
    value = JS_GetPropertyUint32(ctx, obj, 1);
    mnt->origin = JS_ToCString(ctx, value);
    JS_FreeValue(ctx, value);
    value = JS_GetPropertyUint32(ctx, obj, 2);
    mnt->def = JS_ToCString(ctx, value);
    JS_FreeValue(ctx, value);
    value = JS_GetPropertyUint32(ctx, obj, 3);
    mnt->protocol = JS_ToCString(ctx, value);
    JS_FreeValue(ctx, value);
    value = JS_GetPropertyUint32(ctx, obj, 4);
    mnt->basic_auth_login_file = JS_ToCString(ctx, value);
  } else if(JS_IsObject(obj)) {
    value = JS_GetPropertyStr(ctx, obj, "mountpoint");
    mnt->mountpoint = JS_ToCString(ctx, value);
    JS_FreeValue(ctx, value);
    value = JS_GetPropertyStr(ctx, obj, "origin");
    mnt->origin = JS_ToCString(ctx, value);
    JS_FreeValue(ctx, value);
    value = JS_GetPropertyStr(ctx, obj, "def");
    mnt->def = JS_ToCString(ctx, value);
    JS_FreeValue(ctx, value);
    value = JS_GetPropertyStr(ctx, obj, "protocol");
    mnt->protocol = JS_ToCString(ctx, value);
    JS_FreeValue(ctx, value);
    value = JS_GetPropertyStr(ctx, obj, "basic_auth_login_file");
    mnt->basic_auth_login_file = JS_ToCString(ctx, value);
    JS_FreeValue(ctx, value);
  }

  return mnt;
}

static const struct lws_http_mount*
lws_context_http_mounts(JSContext* ctx, JSValueConst value) {
  const struct lws_http_mount *mnt = 0, **ptr = &mnt;

  if(JS_IsArray(ctx, value)) {
    int32_t len = -1;
    JS_GetPropertyStr(ctx, value, "length");
    JS_ToInt32(ctx, &len, value);

    if(len > 0) {
      mnt = js_malloc(ctx, sizeof(struct lws_http_mount));

      for(int32_t i = 0; i < len; len++) {
        JSValue mount = JS_GetPropertyUint32(ctx, value, i);

        if((*ptr = lws_context_http_mount(ctx, mount))) {
          ptr = (const struct lws_http_mount**)&(*ptr)->mount_next;
        } else {
          break;
        }
      }
    }
  }

  JS_FreeValue(ctx, value);

  return mnt;
}

static struct lws_protocol_vhost_options* lws_context_vh_options(JSContext*, JSValueConst);

static struct lws_protocol_vhost_options*
lws_context_vh_option(JSContext* ctx, JSValueConst obj) {
  struct lws_protocol_vhost_options* vho;
  JSValue name, value, options;

  if(JS_IsArray(ctx, obj)) {
    name = JS_GetPropertyUint32(ctx, obj, 0);
    value = JS_GetPropertyUint32(ctx, obj, 1);
    options = JS_GetPropertyUint32(ctx, obj, 2);
  } else if(JS_IsObject(obj)) {
    name = JS_GetPropertyStr(ctx, obj, "name");
    value = JS_GetPropertyStr(ctx, obj, "value");
    options = JS_GetPropertyStr(ctx, obj, "options");
  }

  if((vho = js_malloc(ctx, sizeof(struct lws_protocol_vhost_options)))) {
    vho->name = JS_ToCString(ctx, name);
    vho->value = JS_ToCString(ctx, value);
    vho->options = lws_context_vh_options(ctx, options);
  }

  JS_FreeValue(ctx, name);
  JS_FreeValue(ctx, value);
  return vho;
}

static struct lws_protocol_vhost_options*
lws_context_vh_options(JSContext* ctx, JSValueConst value) {
  struct lws_protocol_vhost_options *vho = 0, **ptr = &vho;

  if(JS_IsArray(ctx, value)) {
    int32_t len = -1;
    JS_GetPropertyStr(ctx, value, "length");
    JS_ToInt32(ctx, &len, value);

    if(len > 0) {
      vho = js_malloc(ctx, sizeof(struct lws_protocol_vhost_options));

      for(int32_t i = 0; i < len; len++) {
        JSValue option = JS_GetPropertyUint32(ctx, value, i);

        if((*ptr = lws_context_vh_option(ctx, option))) {
          ptr = (struct lws_protocol_vhost_options**)&(*ptr)->next;
        } else {
          break;
        }
      }
    }
  }

  JS_FreeValue(ctx, value);

  return vho;
}

JSValue
lws_context_constructor(JSContext* ctx,
                        JSValueConst new_target,
                        int argc,
                        JSValueConst argv[]) {
  JSValue proto, obj;
  struct lws_context_creation_info ci = {}, *pci = &ci, **lci = &pci;
  struct lws_context **lc, *other;
  BOOL got_url = FALSE;
  int32_t i;

  if(!(lc = js_malloc(ctx, sizeof(struct lws_context*))))
    return JS_EXCEPTION;

  *lc = 0;

  /* using new_target to get the prototype is necessary when the class is extended. */
  proto = JS_GetPropertyStr(ctx, new_target, "prototype");
  if(JS_IsException(proto))
    proto = JS_DupValue(ctx, lws_context_proto);

  obj = JS_NewObjectProtoClass(ctx, proto, lws_context_class_id);
  JS_FreeValue(ctx, proto);
  if(JS_IsException(obj))
    goto fail;

  if(JS_IsObject(argv[0])) {
    JSValue value;

    value = JS_GetPropertyStr(ctx, argv[0], "iface");
    (*lci)->iface = JS_ToCString(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "protocols");
    (*lci)->protocols = lws_context_getarray(ctx, value, lws_context_protocols);
    JS_FreeValue(ctx, value);

#if defined(LWS_ROLE_WS)

#endif
    value = JS_GetPropertyStr(ctx, argv[0], "http_proxy_address");
    (*lci)->http_proxy_address = JS_ToCString(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "headers");
    (*lci)->headers = lws_context_vh_options(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "reject_service_keywords");
    (*lci)->reject_service_keywords = lws_context_vh_options(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "pvo");
    (*lci)->pvo = lws_context_vh_options(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "log_filepath");
    (*lci)->log_filepath = JS_ToCString(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "mounts");
    (*lci)->mounts = lws_context_http_mounts(ctx, value);
    JS_FreeValue(ctx, value);

#if defined(LWS_WITH_SYS_ASYNC_DNS)
    value = JS_GetPropertyStr(ctx, argv[0], "async_dns_servers");

    if(JS_IsObject(value)) {
      JS_GetPropertyStr(ctx, value, "length");
      JS_ToInt32(ctx, &i, value);

      if(i > 0) {
        (*lci)->async_dns_servers = js_malloc(ctx, (i + 1) * sizeof(const char*));

        for(int32_t j = 0; j < i; i++) {
          JSValue server = JS_GetPropertyUint32(ctx, value, j);

          (*lci)->async_dns_servers[j] = JS_ToCString(ctx, server);
          JS_FreeValue(ctx, server);
        }

        (*lci)->async_dns_servers[i] = 0;
      }
    }

    JS_FreeValue(ctx, value);
#endif

    JSValue protocols = JS_GetPropertyStr(ctx, argv[0], "protocols");

#if defined(LWS_ROLE_H1) || defined(LWS_ROLE_H2)
    JSValue vport = JS_GetPropertyStr(ctx, argv[0], "port");
    int32_t port;
    JS_ToInt32(ctx, &port, vport);
    JS_FreeValue(ctx, vport);

    (*lci)->port = port;
#endif

#if defined(LWS_WITH_TLS)
    value = JS_GetPropertyStr(ctx, argv[0], "ssl_private_key_password");
    (*lci)->ssl_private_key_password = JS_ToCString(ctx, value);
    JS_FreeValue(ctx, value);
    value = JS_GetPropertyStr(ctx, argv[0], "ssl_cert_filepath");
    (*lci)->ssl_cert_filepath = JS_ToCString(ctx, value);
    JS_FreeValue(ctx, value);
    value = JS_GetPropertyStr(ctx, argv[0], "ssl_private_key_filepath");
    (*lci)->ssl_private_key_filepath = JS_ToCString(ctx, value);
    JS_FreeValue(ctx, value);
    value = JS_GetPropertyStr(ctx, argv[0], "ssl_ca_filepath");
    (*lci)->ssl_ca_filepath = JS_ToCString(ctx, value);
    JS_FreeValue(ctx, value);
    value = JS_GetPropertyStr(ctx, argv[0], "ssl_cipher_list");
    (*lci)->ssl_cipher_list = JS_ToCString(ctx, value);
    JS_FreeValue(ctx, value);
    value = JS_GetPropertyStr(ctx, argv[0], "tls1_3_plus_cipher_list");
    (*lci)->tls1_3_plus_cipher_list = JS_ToCString(ctx, value);
    JS_FreeValue(ctx, value);
    value = JS_GetPropertyStr(ctx, argv[0], "client_ssl_private_key_password");
    (*lci)->client_ssl_private_key_password = JS_ToCString(ctx, value);
    JS_FreeValue(ctx, value);
    value = JS_GetPropertyStr(ctx, argv[0], "client_ssl_cert_filepath");
    (*lci)->client_ssl_cert_filepath = JS_ToCString(ctx, value);
    JS_FreeValue(ctx, value);
    value = JS_GetPropertyStr(ctx, argv[0], "client_ssl_private_key_filepath");
    (*lci)->client_ssl_private_key_filepath = JS_ToCString(ctx, value);
    JS_FreeValue(ctx, value);
    value = JS_GetPropertyStr(ctx, argv[0], "client_ssl_ca_filepath");
    (*lci)->client_ssl_ca_filepath = JS_ToCString(ctx, value);
    JS_FreeValue(ctx, value);
    value = JS_GetPropertyStr(ctx, argv[0], "client_ssl_cipher_list");
    (*lci)->client_ssl_cipher_list = JS_ToCString(ctx, value);
    JS_FreeValue(ctx, value);
    value = JS_GetPropertyStr(ctx, argv[0], "client_tls_1_3_plus_cipher_list");
    (*lci)->client_tls_1_3_plus_cipher_list = JS_ToCString(ctx, value);
    JS_FreeValue(ctx, value);

#endif

#if defined(LWS_WITH_SOCKS5)
    value = JS_GetPropertyStr(ctx, argv[0], "socks_proxy_address");
    (*lci)->socks_proxy_address = JS_ToCString(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "socks_proxy_port");
    JS_ToInt32(ctx, &i, value);
    (*lci)->socks_proxy_port = i;
    JS_FreeValue(ctx, value);

#endif

#if defined(LWS_WITH_SYS_ASYNC_DNS)
    value = JS_GetPropertyStr(ctx, argv[0], "async_dns_servers");

    if(JS_IsObject(value)) {
      JS_GetPropertyStr(ctx, value, "length");
      JS_ToInt32(ctx, &i, value);

      if(i > 0) {
        (*lci)->async_dns_servers = js_malloc(ctx, (i + 1) * sizeof(const char*));

        for(int32_t j = 0; j < i; i++) {
          JSValue server = JS_GetPropertyUint32(ctx, value, j);

          (*lci)->async_dns_servers[j] = JS_ToCString(ctx, server);
          JS_FreeValue(ctx, server);
        }

        (*lci)->async_dns_servers[i] = 0;
      }
    }

    JS_FreeValue(ctx, value);
#endif

    value = JS_GetPropertyStr(ctx, argv[0], "default_loglevel");
    JS_ToInt32(ctx, &i, value);
    (*lci)->default_loglevel = i;
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "vh_listen_sockfd");
    JS_ToInt32(ctx, &i, value);
    (*lci)->vh_listen_sockfd = i;
    JS_FreeValue(ctx, value);
  }

  (*lci)->user = JS_VALUE_GET_OBJ(obj);

  JS_SetOpaque(obj, lc);

  return obj;

fail:
  js_free(ctx, lc);
  JS_FreeValue(ctx, obj);
  return JS_EXCEPTION;
}

static void
lws_context_finalizer(JSRuntime* rt, JSValue val) {
  struct lws_context** lc;

  if((lc = JS_GetOpaque(val, lws_context_class_id))) {
    js_free_rt(rt, lc);
  }
}
static const JSClassDef lws_context_class = {
    "MinnetWebsocket",
    .finalizer = lws_context_finalizer,
};

int
lws_context_init(JSContext* ctx, JSModuleDef* m) {
  JS_NewClassID(&lws_context_class_id);
  JS_NewClass(JS_GetRuntime(ctx), lws_context_class_id, &lws_context_class);
  lws_context_proto = JS_NewObject(ctx);
  JS_SetPropertyFunctionList(ctx,
                             lws_context_proto,
                             lws_context_proto_funcs,
                             countof(lws_context_proto_funcs));

  lws_context_ctor = JS_NewCFunction2(
      ctx, lws_context_constructor, "MinnetRequest", 1, JS_CFUNC_constructor, 0);
  JS_SetConstructor(ctx, lws_context_ctor, lws_context_proto);

  if(m)
    JS_SetModuleExport(ctx, m, "LwsContext", lws_context_ctor);

  return 0;
}
