#include <quickjs.h>
#include <cutils.h>
#include <libwebsockets.h>

static JSValue lws_context_proto, lws_context_ctor;
static JSClassID lws_context_class_id;

static struct lws_protocol_vhost_options* lws_context_vh_options(JSContext* ctx, JSValueConst value);
static void lws_context_vh_options_free(JSRuntime* rt, struct lws_protocol_vhost_options* vho);

static const char*
value_to_string(JSContext* ctx, JSValueConst value) {
  if(JS_IsUndefined(value) || JS_IsNull(value))
    return 0;

  const char* s = JS_ToCString(ctx, value);
  char* x = js_strdup(ctx, s);
  JS_FreeCString(ctx, s);
  return x;
}

static const char*
atom_to_string(JSContext* ctx, JSAtom a) {
  char* x = 0;
  JSValue v = JS_AtomToValue(ctx, a);

  if(!(JS_IsUndefined(v) || JS_IsNull(v))) {
    const char* s = JS_ToCString(ctx, v);
    x = js_strdup(ctx, s);
    JS_FreeCString(ctx, s);
  }
  JS_FreeValue(ctx, v);

  return x;
}

static const int64_t
value_to_integer(JSContext* ctx, JSValueConst value) {
  int64_t i = -1;
  JS_ToInt64(ctx, &i, value);
  return i;
}

static const void*
lws_context_getarray(JSContext* ctx, JSValueConst value, void* fn(JSContext*, JSValueConst)) {
  const void** arr = 0;

  if(JS_IsArray(ctx, value)) {
    int32_t len = -1;
    JSValue vlen = JS_GetPropertyStr(ctx, value, "length");
    JS_ToInt32(ctx, &len, vlen);
    JS_FreeValue(ctx, vlen);

    if(len > 0) {
      arr = js_mallocz(ctx, (len + 1) * sizeof(void*));

      for(int32_t i = 0; i < len; i++) {
        JSValue item = JS_GetPropertyUint32(ctx, value, i);

        if(!(arr[i] = fn(ctx, item)))
          break;

        JS_FreeValue(ctx, item);
      }
    }
  }

  return arr;
}

struct protocols_closure {
  JSContext* ctx;
  JSValue callback, user;
};

static JSValue
get_cb(JSContext* ctx, int write) {
  JSValue glob = JS_GetGlobalObject(ctx);
  JSValue os = JS_GetPropertyStr(ctx, glob, "os");
  JS_FreeValue(ctx, glob);
  JSValue fn = JS_GetPropertyStr(ctx, os, write ? "setWriteHandler" : "setReadHandler");
  JS_FreeValue(ctx, os);
  return fn;
}

static void
set_handler(JSContext* ctx, int fd, JSValueConst handler, int write) {
  JSValue fn = get_cb(ctx, write);
  JSValue args[2] = {
      JS_NewInt32(ctx, fd),
      handler,
  };
  JSValue ret = JS_Call(ctx, fn, JS_NULL, 2, args);
  JS_FreeValue(ctx, ret);
  JS_FreeValue(ctx, fn);
}

static JSValue
protocol_handler(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv, int magic, JSValue* func_data) {
  BOOL write = JS_ToBool(ctx, func_data[2]);
  int64_t i64;
  int32_t fd, events;
  JS_ToInt64(ctx, &i64, func_data[3]);
  JS_ToInt32(ctx, &fd, func_data[0]);
  JS_ToInt32(ctx, &events, func_data[1]);

  struct lws_pollfd x = {.fd = fd, .events = events, .revents = write ? POLLOUT : POLLIN};

  lws_service_fd((struct lws_context*)i64, &x);
}

static int
protocol_callback(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len) {
  struct lws_protocols const* pro = lws_get_protocol(wsi);
  struct protocols_closure* closure = pro->user;

  switch(reason) {
    case LWS_CALLBACK_LOCK_POLL:
    case LWS_CALLBACK_UNLOCK_POLL: break;

    case LWS_CALLBACK_DEL_POLL_FD: {
      struct lws_pollargs* x = in;
      set_handler(closure->ctx, x->fd, JS_NULL, 0);
      set_handler(closure->ctx, x->fd, JS_NULL, 1);
      break;
    }

    case LWS_CALLBACK_ADD_POLL_FD:
    case LWS_CALLBACK_CHANGE_MODE_POLL_FD: {
      struct lws_pollargs* x = in;
      BOOL write = !!(x->events & POLLOUT);
      JSValueConst data[] = {
          JS_NewInt32(closure->ctx, x->fd),
          JS_NewInt32(closure->ctx, x->events),
          JS_NewBool(closure->ctx, write),
          JS_NewInt64(closure->ctx, (intptr_t)lws_get_context(wsi)),
      };
      JSValue fn = JS_NewCFunctionData(closure->ctx, protocol_handler, 0, 0, countof(data), data);

      if(reason == LWS_CALLBACK_CHANGE_MODE_POLL_FD)
        set_handler(closure->ctx, x->fd, JS_NULL, !write);

      set_handler(closure->ctx, x->fd, fn, write);

      JS_FreeValue(closure->ctx, fn);
      break;
    }

    default: {
      JSValue argv[] = {
          JS_NewInt32(closure->ctx, reason),
          pro->per_session_data_size ? JS_NewArrayBufferCopy(closure->ctx, user, pro->per_session_data_size) : JS_NULL,
          in ? JS_NewArrayBufferCopy(closure->ctx, in, len) : JS_NULL,
          JS_NewInt64(closure->ctx, len),
      };
      JSValue ret = JS_Call(closure->ctx, closure->callback, JS_NULL, countof(argv), argv);
      JS_FreeValue(closure->ctx, argv[0]);
      JS_FreeValue(closure->ctx, argv[1]);
      JS_FreeValue(closure->ctx, argv[2]);
      JS_FreeValue(closure->ctx, argv[3]);

      int32_t i = -1;
      JS_ToInt32(closure->ctx, &i, ret);
      JS_FreeValue(closure->ctx, ret);

      return i;
    }
  }

  return 0;
}

static void
lws_context_protocol_free(JSRuntime* rt, struct lws_protocols* pro) {
  struct protocols_closure* closure = pro->user;

  if(closure) {
    JS_FreeValueRT(rt, closure->callback);
    JS_FreeValueRT(rt, closure->user);

    js_free_rt(rt, closure);
  }

  pro->user = 0;
  pro->callback = 0;

  js_free_rt(rt, (char*)pro->name);
}

static void
lws_context_protocols_free(JSRuntime* rt, struct lws_protocols* pro) {
  size_t i;

  for(i = 0; pro[i].name; ++i)
    lws_context_protocol_free(rt, &pro[i]);

  js_free_rt(rt, pro);
}

static struct lws_protocols
lws_context_protocol(JSContext* ctx, JSValueConst obj) {
  struct lws_protocols pro;
  struct protocols_closure* closure = 0;

  JSValue value = JS_GetPropertyStr(ctx, obj, "name");
  pro.name = value_to_string(ctx, value);
  JS_FreeValue(ctx, value);

  value = JS_GetPropertyStr(ctx, obj, "callback");
  if(JS_IsFunction(ctx, value)) {
    if((closure = js_mallocz(ctx, sizeof(struct protocols_closure)))) {
      closure->ctx = ctx;
      closure->callback = JS_DupValue(ctx, value);
      closure->user = JS_GetPropertyStr(ctx, obj, "user");

      pro.callback = protocol_callback;
    }

    pro.user = closure;
  }
  JS_FreeValue(ctx, value);

  value = JS_GetPropertyStr(ctx, obj, "per_session_data_size");
  pro.per_session_data_size = value_to_integer(ctx, value);
  JS_FreeValue(ctx, value);

  value = JS_GetPropertyStr(ctx, obj, "rx_buffer_size");
  pro.rx_buffer_size = value_to_integer(ctx, value);
  JS_FreeValue(ctx, value);

  value = JS_GetPropertyStr(ctx, obj, "id");
  pro.id = value_to_integer(ctx, value);
  JS_FreeValue(ctx, value);

  value = JS_GetPropertyStr(ctx, obj, "tx_packet_size");
  pro.tx_packet_size = value_to_integer(ctx, value);
  JS_FreeValue(ctx, value);

  return pro;
}

static const struct lws_protocols*
lws_context_protocols(JSContext* ctx, JSValueConst value) {
  struct lws_protocols* pro = 0;

  if(JS_IsArray(ctx, value)) {
    int32_t len = -1;
    JSValue vlen = JS_GetPropertyStr(ctx, value, "length");
    JS_ToInt32(ctx, &len, vlen);
    JS_FreeValue(ctx, vlen);

    if(len > 0) {
      pro = js_mallocz(ctx, (len + 1) * sizeof(struct lws_protocols));

      for(int32_t i = 0; i < len; i++) {
        JSValue protocol = JS_GetPropertyUint32(ctx, value, i);

        pro[i] = lws_context_protocol(ctx, protocol);

        JS_FreeValue(ctx, protocol);
      }
    }
  }

  return pro;
}

static struct lws_http_mount*
lws_context_http_mount(JSContext* ctx, JSValueConst obj, const char* name) {
  struct lws_http_mount* mnt;
  JSValue value;

  if(!(mnt = js_mallocz(ctx, sizeof(struct lws_http_mount))))
    return 0;

  if(name) {
    mnt->mountpoint = js_strdup(ctx, name);
    mnt->mountpoint_len = strlen(name);
  }

  if(JS_IsArray(ctx, obj)) {
    int i = 0;

    if(!name) {
      value = JS_GetPropertyUint32(ctx, obj, i++);
      mnt->mountpoint = value_to_string(ctx, value);
      mnt->mountpoint_len = strlen(mnt->mountpoint);
      JS_FreeValue(ctx, value);
    }

    value = JS_GetPropertyUint32(ctx, obj, i++);
    mnt->origin = value_to_string(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyUint32(ctx, obj, i++);
    mnt->def = value_to_string(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyUint32(ctx, obj, i++);
    mnt->protocol = value_to_string(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyUint32(ctx, obj, i++);
    mnt->basic_auth_login_file = value_to_string(ctx, value);
  } else if(JS_IsObject(obj)) {
    value = JS_GetPropertyStr(ctx, obj, "mountpoint");
    if(JS_IsString(value)) {
      mnt->mountpoint = value_to_string(ctx, value);
      mnt->mountpoint_len = strlen(mnt->mountpoint);
    }
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, obj, "origin");
    mnt->origin = value_to_string(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, obj, "def");
    mnt->def = value_to_string(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, obj, "protocol");
    mnt->protocol = value_to_string(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, obj, "cgienv");
    mnt->cgienv = lws_context_vh_options(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, obj, "extra_mimetypes");
    mnt->extra_mimetypes = lws_context_vh_options(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, obj, "interpret");
    mnt->interpret = lws_context_vh_options(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, obj, "cgi_timeout");
    mnt->cgi_timeout = value_to_integer(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, obj, "cache_max_age");
    mnt->cache_max_age = value_to_integer(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, obj, "auth_mask");
    mnt->auth_mask = value_to_integer(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, obj, "cache_reusable");
    mnt->cache_reusable = JS_ToBool(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, obj, "cache_revalidate");
    mnt->cache_revalidate = JS_ToBool(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, obj, "cache_intermediaries");
    mnt->cache_intermediaries = JS_ToBool(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, obj, "cache_no");
    mnt->cache_no = JS_ToBool(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, obj, "origin_protocol");
    mnt->origin_protocol = value_to_integer(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, obj, "basic_auth_login_file");
    mnt->basic_auth_login_file = value_to_string(ctx, value);
    JS_FreeValue(ctx, value);
  }

  return mnt;
}

static const struct lws_http_mount*
lws_context_http_mounts(JSContext* ctx, JSValueConst value) {
  const struct lws_http_mount *mnt = 0, **ptr = &mnt, *tmp;

  if(JS_IsArray(ctx, value)) {
    int32_t len = -1;
    JSValue vlen = JS_GetPropertyStr(ctx, value, "length");
    JS_ToInt32(ctx, &len, vlen);
    JS_FreeValue(ctx, vlen);

    if(len > 0) {
      mnt = js_malloc(ctx, sizeof(struct lws_http_mount));

      for(int32_t i = 0; i < len; i++) {
        JSValue mount = JS_GetPropertyUint32(ctx, value, i);

        if((*ptr = tmp = lws_context_http_mount(ctx, mount, 0)))
          ptr = (const struct lws_http_mount**)&(*ptr)->mount_next;

        JS_FreeValue(ctx, mount);

        if(!tmp)
          break;
      }
    }
  } else if(JS_IsObject(value)) {
    JSPropertyEnum* tmp_tab = 0;
    uint32_t len;

    if(!JS_GetOwnPropertyNames(ctx, &tmp_tab, &len, value, JS_GPN_STRING_MASK | JS_GPN_SET_ENUM)) {

      for(uint32_t i = 0; i < len; i++) {
        const char* name = JS_AtomToCString(ctx, tmp_tab[i].atom);
        JSValue mount = JS_GetProperty(ctx, value, tmp_tab[i].atom);

        if((*ptr = tmp = lws_context_http_mount(ctx, mount, name)))
          ptr = (const struct lws_http_mount**)&(*ptr)->mount_next;

        JS_FreeCString(ctx, name);
        JS_FreeValue(ctx, mount);

        if(!tmp)
          break;
      }
    }
  }

  return mnt;
}

static void
lws_context_http_mounts_free(JSRuntime* rt, struct lws_http_mount* mnt) {

  for(; mnt; mnt = (struct lws_http_mount*)mnt->mount_next) {
    if(mnt->mountpoint) {
      js_free_rt(rt, (char*)mnt->mountpoint);
      mnt->mountpoint = 0;
    }
    if(mnt->origin) {
      js_free_rt(rt, (char*)mnt->origin);
      mnt->origin = 0;
    }
    if(mnt->def) {
      js_free_rt(rt, (char*)mnt->def);
      mnt->def = 0;
    }
    if(mnt->protocol) {
      js_free_rt(rt, (char*)mnt->protocol);
      mnt->protocol = 0;
    }
    if(mnt->cgienv) {
      lws_context_vh_options_free(rt, (struct lws_protocol_vhost_options*)mnt->cgienv);
      mnt->cgienv = 0;
    }
    if(mnt->extra_mimetypes) {
      lws_context_vh_options_free(rt, (struct lws_protocol_vhost_options*)mnt->extra_mimetypes);
      mnt->extra_mimetypes = 0;
    }
    if(mnt->interpret) {
      lws_context_vh_options_free(rt, (struct lws_protocol_vhost_options*)mnt->interpret);
      mnt->interpret = 0;
    }
    if(mnt->basic_auth_login_file) {
      js_free_rt(rt, (char*)mnt->basic_auth_login_file);
      mnt->basic_auth_login_file = 0;
    }
  }
}

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
    vho->name = value_to_string(ctx, name);
    vho->value = value_to_string(ctx, value);
    vho->options = lws_context_vh_options(ctx, options);
  }

  JS_FreeValue(ctx, name);
  JS_FreeValue(ctx, value);
  JS_FreeValue(ctx, options);
  return vho;
}

static struct lws_protocol_vhost_options*
lws_context_vh_options(JSContext* ctx, JSValueConst value) {
  struct lws_protocol_vhost_options *vho = 0, **ptr = &vho, *tmp;

  if(JS_IsArray(ctx, value)) {
    int32_t len = -1;
    JSValue vlen = JS_GetPropertyStr(ctx, value, "length");
    JS_ToInt32(ctx, &len, vlen);
    JS_FreeValue(ctx, vlen);

    if(len > 0) {

      for(int32_t i = 0; i < len; i++) {
        JSValue option = JS_GetPropertyUint32(ctx, value, i);

        if((*ptr = tmp = lws_context_vh_option(ctx, option)))
          ptr = (struct lws_protocol_vhost_options**)&(*ptr)->next;

        JS_FreeValue(ctx, option);

        if(!tmp)
          break;
      }
    }
  }

  return vho;
}

static void
lws_context_vh_options_free(JSRuntime* rt, struct lws_protocol_vhost_options* vho) {

  js_free_rt(rt, (char*)vho->name);
  vho->name = 0;
  js_free_rt(rt, (char*)vho->value);
  vho->value = 0;

  lws_context_vh_options_free(rt, (struct lws_protocol_vhost_options*)vho->next);
  vho->next = 0;

  lws_context_vh_options_free(rt, (struct lws_protocol_vhost_options*)vho->options);
  vho->options = 0;
}

struct context_closure {
  struct lws_context* ctx;
  struct lws_context_creation_info info;
};

JSValue
lws_context_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst argv[]) {
  JSValue proto, obj;
  struct lws_context_creation_info* ci;
  struct context_closure* lc;

  if(!(lc = js_mallocz(ctx, sizeof(struct context_closure))))
    return JS_EXCEPTION;

  ci = &lc->info;

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
    ci->iface = value_to_string(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "protocols");
    ci->protocols = lws_context_protocols(ctx, value);
    JS_FreeValue(ctx, value);

#if defined(LWS_ROLE_WS)

#endif
#if defined(LWS_ROLE_H1) || defined(LWS_ROLE_H2)
    value = JS_GetPropertyStr(ctx, argv[0], "http_proxy_address");
    ci->http_proxy_address = value_to_string(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "headers");
    ci->headers = lws_context_vh_options(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "reject_service_keywords");
    ci->reject_service_keywords = lws_context_vh_options(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "pvo");
    ci->pvo = lws_context_vh_options(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "log_filepath");
    ci->log_filepath = value_to_string(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "mounts");
    ci->mounts = lws_context_http_mounts(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "server_string");
    ci->server_string = value_to_string(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "error_document_404");
    ci->error_document_404 = value_to_string(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "port");
    ci->port = value_to_integer(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "http_proxy_port");
    ci->http_proxy_port = value_to_integer(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "keepalive_timeout");
    ci->keepalive_timeout = value_to_integer(ctx, value);
    JS_FreeValue(ctx, value);
#endif

#if defined(LWS_WITH_SYS_ASYNC_DNS)
    value = JS_GetPropertyStr(ctx, argv[0], "async_dns_servers");

    if(JS_IsObject(value)) {
      JS_GetPropertyStr(ctx, value, "length");
      JS_ToInt32(ctx, &i, value);
      JS_FreeValue(ctx, value);

      if(i > 0) {
        ci->async_dns_servers = js_mallocz(ctx, (i + 1) * sizeof(const char*));

        for(int32_t j = 0; j < i; i++) {
          JSValue server = JS_GetPropertyUint32(ctx, value, j);

          ci->async_dns_servers[j] = value_to_string(ctx, server);
          JS_FreeValue(ctx, server);
        }

        ci->async_dns_servers[i] = 0;
      }
    }

    JS_FreeValue(ctx, value);
#endif

#if defined(LWS_WITH_TLS)
    value = JS_GetPropertyStr(ctx, argv[0], "ssl_private_key_password");
    ci->ssl_private_key_password = value_to_string(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "ssl_cert_filepath");
    ci->ssl_cert_filepath = value_to_string(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "ssl_private_key_filepath");
    ci->ssl_private_key_filepath = value_to_string(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "ssl_ca_filepath");
    ci->ssl_ca_filepath = value_to_string(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "ssl_cipher_list");
    ci->ssl_cipher_list = value_to_string(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "tls1_3_plus_cipher_list");
    ci->tls1_3_plus_cipher_list = value_to_string(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "client_ssl_private_key_password");
    ci->client_ssl_private_key_password = value_to_string(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "client_ssl_cert_filepath");
    ci->client_ssl_cert_filepath = value_to_string(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "client_ssl_private_key_filepath");
    ci->client_ssl_private_key_filepath = value_to_string(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "client_ssl_ca_filepath");
    ci->client_ssl_ca_filepath = value_to_string(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "client_ssl_cipher_list");
    ci->client_ssl_cipher_list = value_to_string(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "client_tls_1_3_plus_cipher_list");
    ci->client_tls_1_3_plus_cipher_list = value_to_string(ctx, value);
    JS_FreeValue(ctx, value);

#endif

#if defined(LWS_WITH_SOCKS5)
    value = JS_GetPropertyStr(ctx, argv[0], "socks_proxy_address");
    ci->socks_proxy_address = value_to_string(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "socks_proxy_port");
    ci->socks_proxy_port = value_to_integer(ctx, value);
    JS_FreeValue(ctx, value);

#endif

#if defined(LWS_WITH_SYS_ASYNC_DNS)
    value = JS_GetPropertyStr(ctx, argv[0], "async_dns_servers");

    if(JS_IsObject(value)) {
      JS_GetPropertyStr(ctx, value, "length");
      JS_ToInt32(ctx, &i, value);
      JS_FreeValue(ctx, value);

      if(i > 0) {
        ci->async_dns_servers = js_malloc(ctx, (i + 1) * sizeof(const char*));

        for(int32_t j = 0; j < i; i++) {
          JSValue server = JS_GetPropertyUint32(ctx, value, j);

          ci->async_dns_servers[j] = value_to_string(ctx, server);
          JS_FreeValue(ctx, server);
        }

        ci->async_dns_servers[i] = 0;
      }
    }

    JS_FreeValue(ctx, value);
#endif

    value = JS_GetPropertyStr(ctx, argv[0], "default_loglevel");
    ci->default_loglevel = value_to_integer(ctx, value);
    JS_FreeValue(ctx, value);

    value = JS_GetPropertyStr(ctx, argv[0], "vh_listen_sockfd");
    ci->vh_listen_sockfd = value_to_integer(ctx, value);
    JS_FreeValue(ctx, value);
  }

  ci->user = JS_VALUE_GET_OBJ(obj);

  lc->ctx = lws_create_context(ci);

  JS_SetOpaque(obj, lc);

  return obj;

fail:
  js_free(ctx, lc);
  JS_FreeValue(ctx, obj);
  return JS_EXCEPTION;
}

static void
lws_context_creation_info_free(JSRuntime* rt, struct lws_context_creation_info* ci) {
  if(ci->iface)
    js_free_rt(rt, (char*)ci->iface);
  if(ci->protocols)
    lws_context_protocols_free(rt, (struct lws_protocols*)ci->protocols);
  if(ci->http_proxy_address)
    js_free_rt(rt, (char*)ci->http_proxy_address);
  if(ci->headers)
    lws_context_vh_options_free(rt, (struct lws_protocol_vhost_options*)ci->headers);
  if(ci->reject_service_keywords)
    lws_context_vh_options_free(rt, (struct lws_protocol_vhost_options*)ci->reject_service_keywords);
  if(ci->pvo)
    lws_context_vh_options_free(rt, (struct lws_protocol_vhost_options*)ci->pvo);
  if(ci->log_filepath)
    js_free_rt(rt, (char*)ci->log_filepath);
  if(ci->mounts)
    lws_context_http_mounts_free(rt, (struct lws_http_mount*)ci->mounts);
  if(ci->server_string)
    js_free_rt(rt, (char*)ci->server_string);
  if(ci->error_document_404)
    js_free_rt(rt, (char*)ci->error_document_404);
#if defined(LWS_WITH_SYS_ASYNC_DNS)
  if(ci->async_dns_servers) {
    for(size_t i = 0; ci->async_dns_servers[i]; ++i)
      js_free_rt(rt, (char*)ci->async_dns_servers[i]);
    js_free_rt(rt, ci->async_dns_servers);
  }
#endif
#if defined(LWS_WITH_TLS)
  if(ci->ssl_private_key_password)
    js_free_rt(rt, (char*)ci->ssl_private_key_password);
  if(ci->ssl_cert_filepath)
    js_free_rt(rt, (char*)ci->ssl_cert_filepath);
  if(ci->ssl_private_key_filepath)
    js_free_rt(rt, (char*)ci->ssl_private_key_filepath);
  if(ci->ssl_ca_filepath)
    js_free_rt(rt, (char*)ci->ssl_ca_filepath);
  if(ci->ssl_cipher_list)
    js_free_rt(rt, (char*)ci->ssl_cipher_list);
  if(ci->tls1_3_plus_cipher_list)
    js_free_rt(rt, (char*)ci->tls1_3_plus_cipher_list);
  if(ci->client_ssl_private_key_password)
    js_free_rt(rt, (char*)ci->client_ssl_private_key_password);
  if(ci->client_ssl_cert_filepath)
    js_free_rt(rt, (char*)ci->client_ssl_cert_filepath);
  if(ci->client_ssl_private_key_filepath)
    js_free_rt(rt, (char*)ci->client_ssl_private_key_filepath);
  if(ci->client_ssl_ca_filepath)
    js_free_rt(rt, (char*)ci->client_ssl_ca_filepath);
  if(ci->client_ssl_cipher_list)
    js_free_rt(rt, (char*)ci->client_ssl_cipher_list);
  if(ci->client_tls_1_3_plus_cipher_list)
    js_free_rt(rt, (char*)ci->client_tls_1_3_plus_cipher_list);
#endif
#if defined(LWS_WITH_SOCKS5)
  if(ci->socks_proxy_address)
    js_free_rt(rt, (char*)ci->socks_proxy_address);
#endif
}

static void
lws_context_finalizer(JSRuntime* rt, JSValue val) {
  struct context_closure* lc;

  if((lc = JS_GetOpaque(val, lws_context_class_id))) {
    lws_context_destroy(lc->ctx);
    lc->ctx = 0;

    lws_context_creation_info_free(rt, &lc->info);

    js_free_rt(rt, lc);
  }
}

static const JSClassDef lws_context_class = {
    "MinnetWebsocket",
    .finalizer = lws_context_finalizer,
};
static const JSCFunctionListEntry lws_context_proto_funcs[] = {
    JS_PROP_STRING_DEF("[Symbol.toStringTag]", "LwsContext", JS_PROP_CONFIGURABLE),
};

static const JSCFunctionListEntry lws_funcs[] = {
    JS_PROP_INT32_DEF("LWSMPRO_HTTP", LWSMPRO_HTTP, 0),
    JS_PROP_INT32_DEF("LWSMPRO_HTTPS", LWSMPRO_HTTPS, 0),
    JS_PROP_INT32_DEF("LWSMPRO_FILE", LWSMPRO_FILE, 0),
};

int
lws_context_init(JSContext* ctx, JSModuleDef* m) {
  JS_NewClassID(&lws_context_class_id);
  JS_NewClass(JS_GetRuntime(ctx), lws_context_class_id, &lws_context_class);
  lws_context_proto = JS_NewObject(ctx);
  JS_SetPropertyFunctionList(ctx, lws_context_proto, lws_context_proto_funcs, countof(lws_context_proto_funcs));

  lws_context_ctor = JS_NewCFunction2(ctx, lws_context_constructor, "MinnetRequest", 1, JS_CFUNC_constructor, 0);
  JS_SetConstructor(ctx, lws_context_ctor, lws_context_proto);

  if(m) {
    JS_SetModuleExport(ctx, m, "LwsContext", lws_context_ctor);
    JS_SetModuleExportList(ctx, m, lws_funcs, countof(lws_funcs));
  }

  return 0;
}

__attribute__((visibility("default"))) JSModuleDef*
js_init_module(JSContext* ctx, const char* module_name) {
  JSModuleDef* m;

  if((m = JS_NewCModule(ctx, module_name, lws_context_init))) {
    JS_AddModuleExport(ctx, m, "LwsContext");
    JS_AddModuleExportList(ctx, m, lws_funcs, countof(lws_funcs));
  }

  return m;
}
