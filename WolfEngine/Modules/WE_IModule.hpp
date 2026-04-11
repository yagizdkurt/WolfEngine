#pragma once

class IModule {
public:
    const char* const Name;
    const int Priority;
    virtual ~IModule() = default;
private:
    template<typename, int> friend class TModule;
    IModule(const char* name, int priority) : Name(name), Priority(priority) {}
    friend class ModuleSystem;
    virtual void OnInit()     {}
    virtual void OnUpdate()   {}
    virtual void OnShutdown() {}
};

template<typename T, int Priority> class TModule : public IModule {
public:
    static T& Get() { return *s_instance; }
protected:
    TModule(const char* name) : IModule(name, Priority) { s_instance = static_cast<T*>(this); }
private:
    inline static T* s_instance = nullptr;
};
