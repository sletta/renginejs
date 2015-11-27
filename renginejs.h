#pragma once

#include "duktape.h"
#include "rengine.h"

#include <unordered_map>

class RengineJS
{
public:

    void setJsContext(duk_context *js) { m_js = js; }
    duk_context *jsContext() const { return m_js; }

private:
    duk_context *m_js = 0;

};