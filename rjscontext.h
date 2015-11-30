#pragma once

#include "rengine.h"
#include "rjslink.h"

#include "duktape.h"

#include <unordered_map>

#define RJS_CONTEXT_PROPERTY "\xff""rjs_context"

class RJSContext
{
public:
    RJSLink *create(rengine::SurfaceInterface *iface, void *jsValue, RJSLink::Ownership ownership);
    void clear(RJSLink *link);

    RJSLink *fromNative(void *ptr) const;
    RJSLink *fromScript(void *ptr) const;

    static RJSContext *from(duk_context *ctx);
    static void initialize(duk_context *ctx);

    duk_context *scriptContext() const { return m_ctx; }

private:
    RJSContext(duk_context *ctx) : m_ctx(ctx) { }
    typedef std::unordered_map<void *, RJSLink *> LinkMap;
    LinkMap m_linkFromNative;
    LinkMap m_linkFromScript;

    duk_context *m_ctx;
};

inline RJSLink *RJSContext::create(rengine::SurfaceInterface *iface, void *jsValue, RJSLink::Ownership ownership)
{
    RJSLink *link = RJSLink::create();
    link->setNativeValue(iface);
    link->setNativeType(RJSLink::SurfaceInterfaceType);
    link->setScriptValue(jsValue);
    link->setOwnership(ownership);

    m_linkFromScript.insert(std::pair<void *, RJSLink *>(jsValue, link));
    m_linkFromNative.insert(std::pair<void *, RJSLink *>(iface, link));
    assert(m_linkFromNative.size() == m_linkFromNative.size());

    return link;
}

inline void RJSContext::clear(RJSLink *link)
{
    m_linkFromNative.erase(link->nativeValue());
    m_linkFromScript.erase(link->scriptValue());

    assert(m_linkFromScript.size() == m_linkFromNative.size());
}

inline RJSLink *RJSContext::fromNative(void *ptr) const
{
    auto it = m_linkFromNative.find(ptr);
    if (it != m_linkFromNative.end())
        return it->second;
    return 0;
}

inline RJSLink *RJSContext::fromScript(void *ptr) const
{
    auto it = m_linkFromScript.find(ptr);
    if (it != m_linkFromScript.end())
        return it->second;
    return 0;
}

inline void RJSContext::initialize(duk_context *ctx)
{
    RJSContext *rjs = new RJSContext(ctx);

    // Put the RJSContext into the global stash of the duk_context, so we can
    // acquire it from any location..
    duk_push_global_stash(ctx);
    duk_push_pointer(ctx, rjs);
    duk_put_prop_string(ctx, -2, RJS_CONTEXT_PROPERTY);
    duk_pop(ctx);
}

inline RJSContext *RJSContext::from(duk_context *ctx)
{
    duk_push_global_stash(ctx);
    duk_get_prop_string(ctx, -1, RJS_CONTEXT_PROPERTY);
    assert(duk_is_pointer(ctx, -1));
    RJSContext *rjs = (RJSContext *) duk_get_pointer(ctx, -1);
    duk_pop(ctx); // the pointer
    duk_pop(ctx); // the global stash
    return rjs;
}

