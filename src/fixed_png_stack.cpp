#include <cstring>
#include <cstdlib>
#include "common.h"
#include "png_encoder.h"
#include "fixed_png_stack.h"
#include <node_buffer.h>

using namespace v8; 
using namespace node;

void
FixedPngStack::Initialize(Handle<Object> target)
{
    Isolate *isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    Local<FunctionTemplate> t = FunctionTemplate::New(isolate, New);
    t->InstanceTemplate()->SetInternalFieldCount(1);
    NODE_SET_PROTOTYPE_METHOD(t, "push", Push);
    NODE_SET_PROTOTYPE_METHOD(t, "encode", PngEncodeAsync);
    NODE_SET_PROTOTYPE_METHOD(t, "encodeSync", PngEncodeSync);
    target->Set(String::NewFromUtf8(isolate, "FixedPngStack"), t->GetFunction());
}

FixedPngStack::FixedPngStack(int wwidth, int hheight, buffer_type bbuf_type, int fill, int alpha) :
    width(wwidth), height(hheight), buf_type(bbuf_type) {
        data = (unsigned char *)malloc(sizeof(*data) * width * height * 4);
        int i;
        int size = width*height*4;
        if (!data) throw "malloc failed in node-png (FixedPngStack ctor)";
        memset(data, fill, width*height*4);

        for (i = 3; i < size-3; i+=4){
            data[i]=alpha;
        }
    }

FixedPngStack::~FixedPngStack(){
    free(data);
}

void FixedPngStack::Push(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    Isolate *isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    if (!Buffer::HasInstance(args[0])){
        isolate->ThrowException(VException("First argument must be Buffer."));
        return;
    }
    if (!args[1]->IsInt32()){
        isolate->ThrowException(VException("Second argument must be integer x."));
        return;
    }
    if (!args[2]->IsInt32()){
        isolate->ThrowException(VException("Third argument must be integer y."));
        return;
    }
    if (!args[3]->IsInt32()){
        isolate->ThrowException(VException("Fourth argument must be integer w."));
        return;
    }
    if (!args[4]->IsInt32()){
        isolate->ThrowException(VException("Fifth argument must be integer h."));
        return;
    }

    FixedPngStack *png = ObjectWrap::Unwrap<FixedPngStack>(args.This());
    int x = args[1]->Int32Value();
    int y = args[2]->Int32Value();
    int w = args[3]->Int32Value();
    int h = args[4]->Int32Value();

    if (x < 0){
        isolate->ThrowException(VException("Coordinate x smaller than 0."));
        return;
    }
    if (y < 0){
        isolate->ThrowException(VException("Coordinate y smaller than 0."));
        return;
    }
    if (w < 0){
        isolate->ThrowException(VException("Width smaller than 0."));
        return;
    }
    if (h < 0){
        isolate->ThrowException(VException("Height smaller than 0."));
        return;
    }
    if (x >= png->width){
        isolate->ThrowException(VException("Coordinate x exceeds FixedPngStack's dimensions."));
        return;
    }
    if (y >= png->height) {
        isolate->ThrowException(VException("Coordinate y exceeds FixedPngStack's dimensions."));
        return;
    }
    if (x+w > png->width) {
        isolate->ThrowException(VException("Pushed PNG exceeds FixedPngStack's width."));
        return;
    }
    if (y+h > png->height) {
        isolate->ThrowException(VException("Pushed PNG exceeds FixedPngStack's height."));
        return;
    }

    char *buf_data = node::Buffer::Data(args[0]->ToObject());
    png->Push((unsigned char*)buf_data, x, y, w, h);
    return;
}


Handle<Value>
FixedPngStack::PngEncodeSync()
{
    Isolate *isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    // Local<Value> buf_val = GetHiddenValue(isolate, Local<Object>::New(isolate, persistent()),String::NewFromUtf8(isolate, "buffer"));
    //char *buf_data = node::Buffer::Data(buf_val->ToObject());

    try {
        PngEncoder encoder(data, width, height, buf_type, 8);
        encoder.encode();
        int png_len = encoder.get_png_len();
        Local<Object> retbuf = Buffer::New(isolate, png_len).ToLocalChecked();
        memcpy(node::Buffer::Data(retbuf), encoder.get_png(), png_len);
        return retbuf;
    }
    catch (const char *err) {
        return VException(err);
    }
}

void
FixedPngStack::New(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    Isolate *isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    if (args.Length() < 3)
    {
      isolate->ThrowException(VException("Requires three arguments"));
      return;
    }
    
    if (!args[0]->IsInt32())
    {
      isolate->ThrowException(VException("Second argument must be integer height."));
      return;
    }
    if (!args[1]->IsInt32())
    {
      isolate->ThrowException(VException("Third argument must be integer height."));
      return;
    }

    buffer_type buf_type = BUF_RGB;
    if (args.Length() >= 3) {
        if (!args[2]->IsString())
        {
          isolate->ThrowException(VException("Fourth argument must be 'gray', 'rgb', 'bgr', 'rgba' or 'bgra'."));
          return;
        }

        String::Utf8Value bts(args[2]->ToString());
        if (!(str_eq(*bts, "rgb") || str_eq(*bts, "bgr") ||
            str_eq(*bts, "rgba") || str_eq(*bts, "bgra") ||
            str_eq(*bts, "gray")))
        {
            isolate->ThrowException(VException("Fourth argument must be 'gray', 'rgb', 'bgr', 'rgba' or 'bgra'."));
            return;
        }
        
        if (str_eq(*bts, "rgb"))
            buf_type = BUF_RGB;
        else if (str_eq(*bts, "bgr"))
            buf_type = BUF_BGR;
        else if (str_eq(*bts, "rgba"))
            buf_type = BUF_RGBA;
        else if (str_eq(*bts, "bgra"))
            buf_type = BUF_BGRA;
        else if (str_eq(*bts, "gray"))
            buf_type = BUF_GRAY;
        else
        {
          isolate->ThrowException(VException("Fourth argument wasn't 'gray', 'rgb', 'bgr', 'rgba' or 'bgra'."));
          return;
        }
    }

    int fill = 0xFF;
    if (args.Length() >= 4) {
        if (!args[3]->IsInt32()){
            isolate->ThrowException(VException("Third argument must be integer height."));
            return;
        }
        fill = args[3]->Int32Value();
    }

    int alpha = 0x99;
    if (args.Length() >= 5) {
        if (!args[4]->IsInt32()){
            isolate->ThrowException(VException("Third argument must be integer height."));
            return;
        }
        alpha = args[4]->Int32Value();
    }


    int w = args[0]->Int32Value();
    int h = args[1]->Int32Value();

    if (w < 0)
    {
      isolate->ThrowException(VException("Width smaller than 0."));
      return;
    }
    if (h < 0)
    {
      isolate->ThrowException(VException("Height smaller than 0."));
      return;
    }

    FixedPngStack *png = new FixedPngStack(w, h, buf_type, fill, alpha);
    png->Wrap(args.This());

    // Save buffer.
    // SetHiddenValue(isolate,Local<Object>::New(isolate, png->persistent()),String::NewFromUtf8(isolate, "buffer"), args[0]);

    args.GetReturnValue().Set(args.This());
}

void FixedPngStack::Push(unsigned char *buf_data, int x, int y, int w, int h)
{
    int start = y*width*4 + x*4;
    for (int i = 0; i < h; i++) {
        unsigned char *datap = &data[start + i*width*4];
        for (int j = 0; j < w; j++) {
            *datap++ = *buf_data++;
            *datap++ = *buf_data++;
            *datap++ = *buf_data++;
            *datap++ = (buf_type == BUF_RGB || buf_type == BUF_BGR) ? 0x00 : *buf_data++;
        }
    }

    return;
}

void FixedPngStack::PngEncodeSync(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    Isolate *isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    FixedPngStack *png = ObjectWrap::Unwrap<FixedPngStack>(args.This());
    args.GetReturnValue().Set(png->PngEncodeSync());
}

void
FixedPngStack::UV_PngEncode(uv_work_t* req)
{
    encode_request *enc_req = (encode_request *)req->data;
    FixedPngStack *png = (FixedPngStack *)enc_req->png_obj;

    try {
        PngEncoder encoder(png->data, png->width, png->height, png->buf_type, 8);
        encoder.encode();
        enc_req->png_len = encoder.get_png_len();
        enc_req->png = (char *)malloc(sizeof(*enc_req->png)*enc_req->png_len);
        if (!enc_req->png) {
            enc_req->error = strdup("malloc in FixedPngStack::UV_PngEncode failed.");
            return;
        }
        else {
            memcpy(enc_req->png, encoder.get_png(), enc_req->png_len);
        }
    }
    catch (const char *err) {
        enc_req->error = strdup(err);
    }
}

void 
FixedPngStack::UV_PngEncodeAfter(uv_work_t *req)
{
    Isolate *isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    encode_request *enc_req = (encode_request *)req->data;
    delete req;

    Handle<Value> argv[2];

    if (enc_req->error) {
        argv[0] = Undefined(isolate);
        argv[1] = ErrorException(enc_req->error);
    }
    else {
        Local<Object> buf = Buffer::New(isolate, enc_req->png_len).ToLocalChecked();
        memcpy(node::Buffer::Data(buf), enc_req->png, enc_req->png_len);
        argv[0] = buf;
        argv[1] = Undefined(isolate);
    }

    TryCatch try_catch; // don't quite see the necessity of this

    Local<Function>::New(isolate, enc_req->callback)->Call(isolate->GetCurrentContext()->Global(), 2, argv);

    if (try_catch.HasCaught())
        FatalException(try_catch);

    enc_req->callback.Reset();
    free(enc_req->png);
    free(enc_req->error);

    ((FixedPngStack *)enc_req->png_obj)->Unref();
    delete enc_req;
}

void
FixedPngStack::PngEncodeAsync(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    Isolate *isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    if (args.Length() != 1)
    {
      isolate->ThrowException(VException("One argument required - callback function."));
      return;
    }

    if (!args[0]->IsFunction())
    {
      isolate->ThrowException(VException("First argument must be a function."));
      return;
    }

    FixedPngStack *png = ObjectWrap::Unwrap<FixedPngStack>(args.This());

    encode_request *enc_req = new encode_request();
    if (!enc_req)
    {
      isolate->ThrowException(VException("malloc in FixedPngStack::PngEncodeAsync failed."));
      return;
    }

    enc_req->callback.Reset(isolate, args[0].As<Function>());
    enc_req->png_obj = png;
    enc_req->png = NULL;
    enc_req->png_len = 0;
    enc_req->error = NULL;

    // We need to pull out the buffer data before
    // we go to the thread pool.
    // Local<Value> buf_val = GetHiddenValue(isolate,Local<Object>::New(isolate, png->persistent()),String::NewFromUtf8(isolate, "buffer"));

    // enc_req->buf_data = node::Buffer::Data(buf_val->ToObject());


    uv_work_t* req = new uv_work_t;
    req->data = enc_req;
    uv_queue_work(uv_default_loop(), req, UV_PngEncode, (uv_after_work_cb)UV_PngEncodeAfter);

    png->Ref();
}

NODE_MODULE(png, FixedPngStack::Initialize)
