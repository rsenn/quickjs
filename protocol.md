# Connection

The default debugger transport is a TCP/IP connection from the qjs binary to the QuickJS VS Code extension. This can be changed by implementing a different [transport](https://github.com/koush/quickjs/blob/master/quickjs-debugger-transport-unix.c).

# Wire Protocol and Framing

Once a connection has been initiated, debugger protocol messages begin being sent. The wire protocol messages are framed similarly to chunked encoding and is human readable:

```
<8 character hex length>\n<message of hex length bytes>\n
```

For example, sending hello world:

```
0000000B\nhello world\n
```

# JSON Messages

All debugger protocol messages are JSON payloads.

```
00000019\n{"message": "hello world"}\n
```

For on the wire readability, the JSON messages are ended with a new line (but this is not required, as the JSON is parseable with or without a new line):

```
0000001A\n{"message": "hello world"}\n
```

So when viewing it in sniffer, the message would look as follows in the console (new lines are printed here):

```
0000001A
{"message": "hello world"}
```

# QuickJS Debug Protocol

### Message Flow
```
[qjs] <-----> [QuickJS Debug Extension] <-----> [VS Code]
```

## Terminology
* Debug Server: the qjs runtime and debugger that handles debugging requests.
* Debug Adapter: the QuickJS Debug VS Code extension that handles communication with the debug server on the behalf of VSCode.

VS Code [debug protocol messages](https://github.com/microsoft/vscode/blob/master/src/vs/workbench/contrib/debug/common/debugProtocol.d.ts) delivered to the extension are basically sent as-is across the wire to the QuickJS debugger runtime.

The debug protocol messages will not be documented here. Refer to the [official documentation](https://code.visualstudio.com/blogs/2018/08/07/debug-adapter-protocol-website).

The debug adapter handles the following requests:

 * launch
 * terminate
 * continue
 * pause
 * next
 * stepIn
 * stepOut
 * setBreakpoints
 * setExceptionBreakpoints
 * threads
 * stackTrace
 * scopes
 * variables
 * evaluate
 * completions

## QuickJS Debug Session

The default transport works as following:

1. The debug adapter starts and waits for incoming connections on the specified port.
2. The qjs binary or embedded binary with the debug server starts and connects with one of the following mechanisms:
	* programatically via the `js_debugger_connect(JSContext *ctx, const char *address)` method.
	* automatically via the `QUICKJS_DEBUG_ADDRESS` environment variable, which is in `address:port` notation.

The debug adapter simply serves as a proxy for debug protocol messages. The debug server and debug adapter send messages back and forth, until the session ends.

## Debugging Multiple JSContexts

The QuickJS Debug extension supports debugging multiple JSContexts.
 * The QuickJS Debug Server will initiate one connection per JSContext. This keeps the implementation simple on the runtime side. The Debug Server is implemented as if it is a single instance.
 * The QuickJS Debug Adapter will accept connections from multiple Debug Servers.

Each JSContext is represented as a "thread" by the Debug Adapter (see threads request). All identifiers sent
from the debug server are namespaced to the originating thread by the Debug Adapter. The debug adapter
will intercept the following requests and translate the references of variables before sending them along to VS Code:
 * stackTrace
 * scopes
 * variables
 * evaluate
