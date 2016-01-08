#include <cstdlib>
#include <cassert>
#include "common.h"

using namespace v8;

Handle<Value>
ErrorException(const char *msg)
{
    Isolate *isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    return Exception::Error(String::NewFromUtf8(isolate, msg));
}

Handle<Value>
VException(const char *msg) {
    Isolate *isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    return isolate->ThrowException(ErrorException(msg));
}

bool str_eq(const char *s1, const char *s2)
{
    return strcmp(s1, s2) == 0;
}

