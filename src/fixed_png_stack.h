#ifndef NODE_PNG_FIXE_H
#define NODE_PNG_FIXE_H

#include <node.h>
#include <node_buffer.h>
#include <node_object_wrap.h>
#include <uv.h>

#include "common.h"

class FixedPngStack : public node::ObjectWrap {
    int width;
    int height;
    unsigned char *data;
    buffer_type buf_type;

    static void UV_PngEncode(uv_work_t *req);
    static void UV_PngEncodeAfter(uv_work_t *req);
    static void UV_PngEncodeSaveAfter(uv_work_t *req);
public:
    static void Initialize(v8::Handle<v8::Object> target);
    FixedPngStack(int wwidth, int hheight, buffer_type bbuf_type, int fill, int alpha);
    ~FixedPngStack();

    void Push(unsigned char *buf_data, int x, int y, int w, int h);
    v8::Handle<v8::Value> PngEncodeSync();

    static void New(const v8::FunctionCallbackInfo<v8::Value> &args);
    static void Push(const v8::FunctionCallbackInfo<v8::Value> &args);
    static void PngEncodeAndSaveAsync(const v8::FunctionCallbackInfo<v8::Value> &args);
    static void PngEncodeSync(const v8::FunctionCallbackInfo<v8::Value> &args);
    static void PngEncodeAsync(const v8::FunctionCallbackInfo<v8::Value> &args);
};

#endif

