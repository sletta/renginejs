#include "renginejs.h"

#include <fstream>
#include <sstream>

RENGINE_DEFINE_GLOBALS

using namespace rengine;
using namespace std;

static void populateGlobalObject(duk_context *ctx);
static bool loadFile(duk_context *ctx, const char *file);

class JSRengineSurface : public StandardSurfaceInterface
{
public:
    Node *update(Node *old) {

        cout << "cpp: update called.." << endl;
        duk_context *ctx = m_rjs->jsContext();
        bool ok = duk_get_global_string(ctx, "update");
        if (duk_pcall(ctx, 0) != 0) {
            cerr << "Call to 'update' returned with error state: '"
                 << duk_safe_to_string(ctx, -1) << "'" << endl;
            Backend::get()->quit();
            return 0;
        }

        Node *node = 0;
        if (duk_is_object(ctx, -1)) {
            cout << " - return value is an object.." << endl;
            duk_get_prop_string(ctx, -1, "__ptr");
            if (duk_is_pointer(ctx, -1)) {
                cout << " - return value has __ptr value.." << endl;
                node = (Node *) duk_get_pointer(ctx, -1);
                cout << " - returned (Node *)" << node << endl;
            }
        }

        return node;
    }

    RengineJS *m_rjs = 0;
};

int main(int argc, char **argv)
{
    if (argc < 2) {
        cout << "please specify a filename as input.." << endl;
        return 0;
    }

    std::unique_ptr<Backend> backend(Backend::get());
    JSRengineSurface iface;
    std::unique_ptr<Surface> surface(backend->createSurface(&iface));

    // Evaluate the code..
    duk_context *ctx = duk_create_heap_default();
    for (int i=1; i<argc; ++i) {
        if (!loadFile(ctx, argv[i]))
            return 1;
    }

    if (!duk_get_global_string(ctx, "update")) {
        cout << "No 'update' function..." << endl;
        return 2;
    }

    populateGlobalObject(ctx);

    RengineJS rjs;
    rjs.setJsContext(ctx);
    iface.m_rjs = &rjs;

    surface->show();

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

duk_ret_t rjs_RectangleNode_constructor(duk_context *ctx)
{
    if (!duk_is_constructor_call(ctx)) {
        duk_push_string(ctx, "Function 'RectangleNode' not called as constructor!");
        duk_throw(ctx);
    }

    RectangleNode *rn = RectangleNode::create();
    rn->setX(100);
    rn->setY(100);
    rn->setWidth(100);
    rn->setHeight(100);
    rn->setColor(vec4(1, 0, 0, 1));

    cout << __PRETTY_FUNCTION__ << " is handing out" << (Node *) rn << endl;

    duk_push_this(ctx);
    duk_push_pointer(ctx, (Node *) rn);
    duk_put_prop_string(ctx, -2, "__ptr");
    duk_pop(ctx);

    return 1;
}

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
    duk_push_global_object(ctx);

    addFunction(ctx, "RectangleNode", rjs_RectangleNode_constructor, 0);

}
