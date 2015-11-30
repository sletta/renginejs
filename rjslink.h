#pragma once

#define RJS_LINK_PROPERTY "\xff""rjs_link"

class RJSContext;

class RJSLink
{
protected:
    RJSLink() : m_nativeType(UnknownType), m_scriptOwnership(true) { }
    ~RJSLink() { }

public:
    enum Ownership {
        NativeOwnership = 0,
        ScriptOwnership = 1
    };

    enum NativeType {
        UnknownType,
        SurfaceType,
        SurfaceInterfaceType,
        NodeType
    };

    void setOwnership(Ownership ownership) { m_scriptOwnership = (bool) ownership; }
    Ownership ownership() const { return (Ownership) m_scriptOwnership;  }

    void setNativeType(NativeType type) { m_nativeType = type; }
    NativeType nativeType() const { return (NativeType) m_nativeType; }

    void setNativeValue(void *nval) { m_native = nval; }
    void *nativeValue() { return m_native; }

    void setScriptValue(void *sval) { m_script = sval; }
    void *scriptValue() { return m_script; }

    static RJSLink *create() { return new RJSLink(); }
    void destroy() { delete this; }

private:
    void *m_native = 0;
    void *m_script = 0;

    unsigned m_nativeType : 8;
    bool m_scriptOwnership : 1;
    // space for an additional 25 (+32) bytes worth of stuff..
};