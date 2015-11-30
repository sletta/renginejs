#include "rjssurfaceinterface.h"
#include "rjscontext.h"

#include <fstream>
#include <sstream>

RENGINE_DEFINE_GLOBALS

using namespace rengine;
using namespace std;

static void populateGlobalObject(duk_context *ctx);
static bool loadFile(duk_context *ctx, const char *file);

int main(int argc, char **argv)
{
    if (argc < 2) {
        cout << "please specify a filename as input.." << endl;
        return 0;
    }

    std::unique_ptr<Backend> backend(Backend::get());

    // Evaluate the code..
    duk_context *ctx = duk_create_heap_default();
    populateGlobalObject(ctx);
    for (int i=1; i<argc; ++i) {
        if (!loadFile(ctx, argv[i]))
            return 1;
    }

    backend->run();
    duk_destroy_heap(ctx);

    return 0;
}


bool loadFile(duk_context *ctx, const char *fileName)
{
    string code = (stringstream() << ifstream(fileName).rdbuf()).str();
    if (code.length() == 0) {
        cerr << "File does not exist or is empty: " << fileName << endl;
        return false;
    }
    duk_push_string(ctx, code.c_str());
    duk_push_string(ctx, fileName);
    if (duk_pcompile(ctx, 0) != 0) {
        cerr << "File does not compile: " << fileName << endl
             << duk_safe_to_string(ctx, -1) << endl;
        return false;
    }

    if (duk_pcall(ctx, 0) != 0) {
        cerr << "File failed to execute: " << fileName << endl
             << duk_safe_to_string(ctx, -1) << endl;
        return false;
    }

    return true;
}

// static void putNodeToThis(duk_context *ctx, Node *node)
// {
//     duk_push_this(ctx);
//     duk_push_pointer(ctx, (Node *) node);
//     duk_put_prop_string(ctx, -2, RENGINEJS_NODE_POINTER_PROPERTY);
//     duk_pop(ctx);
// }

// static Node* getNodeFromThis(duk_context *ctx)
// {
//     duk_get_prop_string(ctx,

// #define RJS_DECLARE_NODE_CONSTRUCTOR(Class)                                                 \
//     duk_ret_t rjs_##Class##_constructor(duk_context *ctx) {                                 \
//     if (!duk_is_constructor_call(ctx)) {                                                    \
//         duk_push_string(ctx, "Function '" #Class "' not called as constructor!");           \
//         duk_throw(ctx);                                                                     \
//         return 0;                                                                           \
//     }                                                                                       \
//     putNodeToThis(ctx, Class::create());                                                    \
//     return 1;                                                                               \
// }

// #define RJS_DECLARE_NODE_PROPERTY(Class, Type, Setter, Getter)                              \
//     duk_ret_t rjs_##Class##_##Setter(duk_context *ctx) {                                    \


// duk_ret_t rjs_##

// RJS_DECLARE_NODE_CONSTRUCTOR(Node);
// RJS_DECLARE_NODE_CONSTRUCTOR(RectangleNode);
// RJS_DECLARE_NODE_CONSTRUCTOR(TextureNode);

duk_ret_t rjs_Backend_quit(duk_context *ctx)
{
    Backend::get()->quit();
    return 1;
}

// duk_ret_t rjs_Backend_createSurface(duk_context *ctx)
// {
//     duk_require_object(ctx, 0);

//     return 1;
// }

/*
    Pushes 'function' to the topmost object on the stack under then name
    'functionName' with expected arguments count begin 'argCount'
 */
static void addFunction(duk_context *ctx, const char *functionName, duk_c_function function, int argCount)
{
    duk_push_c_function(ctx, function, argCount);
    duk_put_prop_string(ctx, -2, functionName);
}

void populateGlobalObject(duk_context *ctx)
{
    RJSContext::initialize(ctx);

    duk_push_global_object(ctx);

    // // addFunction(ctx, "Node", rjs_Node_constructor, 0);
    // // addFunction(ctx, "RectangleNode", rjs_RectangleNode_constructor, 0);
    // // addFunction(ctx, "TextureNode", rjs_TextureNode_constructor, 0);

    addFunction(ctx, "SurfaceInterface", rjs_SurfaceInterface_constructor, 0);

    // The backend object..
    duk_push_object(ctx);
    duk_push_c_function(ctx, rjs_Backend_quit, 0);
    duk_put_prop_string(ctx, -2, "quit");
    // duk_push_c_function(ctx, rjs_Backend_createSurface, 1);
    // duk_put_prop_string(ctx, -2, "createSurface");
    duk_put_prop_string(ctx, -2, "Backend");

    // Pop off the global object..
    duk_pop(ctx);

    duk_dump_context_stdout(ctx);

}
