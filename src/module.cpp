#include <node.h>

#include "png.h"

using namespace v8;

extern "C" void
init(v8::Handle<v8::Object> target)
{
    Isolate *isolate = Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    Png::Initialize(target);
}

