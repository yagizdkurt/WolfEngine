#pragma once
#include <vector>
#include "WE_IModule.hpp"

class ModuleRegistry {
public:
    // Insert in descending priority order (highest priority runs first).
    static void Register(IModule* mod) { 
        auto& list = Modules();
        auto it = list.begin();
        while (it != list.end() && (*it)->GetPriority() >= mod->GetPriority()) ++it;
        list.insert(it, mod);
    }

    static void InitAll()               { for (auto* mod : Modules()) mod->OnInit(); }
    static void UpdateAll()             { for (auto* mod : Modules()) mod->OnUpdate(); }
    static void ShutdownAll() {
        auto& list = Modules();
        for (int i = list.size() - 1; i >= 0; i--) list[i]->OnShutdown();
    }

    template<typename T> static T* Get() {
        for (auto* mod : Modules())
            if (auto* typed = dynamic_cast<T*>(mod))
                return typed;
        return nullptr;
    }

private:
    static std::vector<IModule*>& Modules() {
        static std::vector<IModule*> s_modules;
        return s_modules;
    }
};

/**
 * @brief RAII helper that registers a single module instance before `main()`.
 *
 * @details Declare one file-scope `static ModuleRegistrar<T>` in the module's
 *          `.cpp` file (inside the appropriate `#ifdef` guard).  The constructor
 *          creates a `static T` instance — no heap allocation — and passes its
 *          address to `ModuleRegistry::Register`.
 *
 * @tparam T  Concrete module type to instantiate and register; must inherit `IModule`.
 *
 * @note  The `static T s_instance` inside the constructor has program lifetime.
 *        Destruction order relative to other statics is undefined — do not access
 *        other static objects from `T`'s destructor.
 *
 * @warning Registering the same type twice (two `ModuleRegistrar<T>` objects for
 *          the same `T`) will insert two separate instances into the registry.
 */
template<typename T> struct ModuleRegistrar {
    ModuleRegistrar() {
        static T s_instance;
        ModuleRegistry::Register(&s_instance);
    }
};
