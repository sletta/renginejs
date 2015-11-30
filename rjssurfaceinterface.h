#pragma once

#include "rjscontext.h"

#define RJS_SURFACEINTERFACE_POINTER_PROPERTY "\xFFrjs_SurfaceInterface"

class RJSSurfaceInterface : public rengine::StandardSurfaceInterface
{
public:
    RJSSurfaceInterface(RJSContext *rjs)
        : m_context(rjs)
        , m_surface(0)
    {
        assert(rjs);
    }

    ~RJSSurfaceInterface()
    {
        printf("RJSSurfaceInterface::~RJSSurfaceInterface()\n");
    }

    rengine::Node *update(rengine::Node *old);

        // std::cout << "(cpp) update called.." << std::endl;
        // duk_context *ctx = m_context->scriptContext();
        // bool ok = duk_get_global_string(ctx, "update");
        // if (duk_pcall(ctx, 0) != 0) {
        //     std::cerr << "Call to 'update' returned with error state: '"
        //              << duk_safe_to_string(ctx, -1) << "'" << std::endl;
        //     return 0;
        // }

        // rengine::Node *node = 0;
        // // if (duk_is_object(ctx, -1)) {
        // //     std::cout << " - return value is an object.." << std::endl;
        // //     duk_get_prop_string(ctx, -1, RENGINEJS_NODE_POINTER_PROPERTY);
        // //     if (duk_is_pointer(ctx, -1)) {
        // //         std::cout << " - return value has " << RENGINEJS_NODE_POINTER_PROPERTY << " value.." << std::endl;
        // //         node = (rengine::Node *) duk_get_pointer(ctx, -1);
        // //         std::cout << " - returned (Node *)" << node << std::endl;
        // //     }
        // // }

    void show() {
        if (!m_surface)
            m_surface = rengine::Backend::get()->createSurface(this);
        m_surface->show();
    }

private:
    RJSContext *m_context;;
    rengine::Surface *m_surface;
};

inline rengine::Node *RJSSurfaceInterface::update(rengine::Node *old)
{
    printf("RJSSurfaceInterface(%p)::update(%p)\n", this, old);

    // Resolve an 'update' function if present...
    RJSLink *link = m_context->fromNative(this);
    assert(link);

    duk_context *ctx = m_context->scriptContext();
    duk_push_heapptr(ctx, link->scriptValue());
    duk_get_prop_string(ctx, -1, "onUpdate");

    if (duk_is_function(ctx, -1)) {
        duk_dup(ctx, -2);
        duk_push_undefined(ctx);
        duk_pcall_method(ctx, 1);
        // ignore return value..
        duk_pop(ctx);
    } else {
        // pop whatever was there which was not a function, usually undefined...
        duk_pop(ctx);
    }

    // Then pop the heapptr..
    duk_pop(ctx);
    return 0;
};

inline duk_ret_t rjs_SurfaceInterface_finalize(duk_context *ctx)
{
    duk_get_prop_string(ctx, 0, RJS_LINK_PROPERTY);
    RJSLink *link = (RJSLink *) duk_require_pointer(ctx, -1);
    RJSContext::from(ctx)->clear(link);

    assert(link->ownership() == RJSLink::ScriptOwnership);
    assert(link->nativeType() == RJSLink::SurfaceInterfaceType);
    assert(link->nativeValue());
    delete (RJSSurfaceInterface *) link->nativeValue();

    return 1;
}

inline duk_ret_t rjs_SurfaceInterface_show(duk_context *ctx)
{
    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, RJS_LINK_PROPERTY);
    RJSLink *link = (RJSLink *) duk_require_pointer(ctx, -1);
    assert(link);
    assert(link->nativeValue());
    assert(link->nativeType() == RJSLink::SurfaceInterfaceType);
    ((RJSSurfaceInterface *) link->nativeValue())->show();
    return 1;
}

inline duk_ret_t rjs_SurfaceInterface_constructor(duk_context *ctx)
{
    if (!duk_is_constructor_call(ctx)) {
        duk_push_string(ctx, "Function 'StandardSurfaceInterface' not called as a constructor");
        duk_throw(ctx);
        return 0;
    }

    RJSContext *rjs = RJSContext::from(ctx);

    duk_push_this(ctx);
    void *ptr = duk_get_heapptr(ctx, -1);

    RJSSurfaceInterface *iface = new RJSSurfaceInterface(rjs);
    RJSLink *link = rjs->create(iface, ptr, RJSLink::ScriptOwnership);

    duk_push_pointer(ctx, link);
    duk_put_prop_string(ctx, -2, RJS_LINK_PROPERTY);
    duk_push_c_function(ctx, rjs_SurfaceInterface_finalize, 1);
    duk_set_finalizer(ctx, -2);
    duk_push_c_function(ctx, rjs_SurfaceInterface_show, 0);
    duk_put_prop_string(ctx, -2, "show");

    duk_pop(ctx);
    duk_dump_context_stdout(ctx);
    return 1;
}

