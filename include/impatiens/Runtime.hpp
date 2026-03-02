#pragma once
#include "Traits.hpp"

#include <memory>
#include <string>

namespace impatiens {
    // class OpaqueReturn {};

    // template <class Return>
    // class RuntimeReturn : public OpaqueReturn {
    // public:
    //     alignas(alignof(Return)) std::array<std::byte, sizeof(Return)> data;
    // };

    // template <>
    // class RuntimeReturn<void> {
    // public:
    //     std::monostate data;
    // };

    // class OpaqueParameters {};

    // template <class... Params>
    // class RuntimeParameters : public OpaqueParameters {
    // public:
    //     std::tuple<Params...> data;
    // };

    class OpaqueFunction {
    public:
        virtual ~OpaqueFunction() = default;
    };

    template <class Return, class... Params>
    class RuntimeFunction : public OpaqueFunction {
    public:
        virtual Return operator()(Params... params) const = 0;
    };

    template <class Return, template <class...> class ParamList, class... Params>
    struct RuntimeFunction<Return, ParamList<Params...>> : RuntimeFunction<Return, Params...> {};

    template <class MetadataType>
    using RuntimeFunctionType =
        RuntimeFunction<typename MetadataType::ReturnType, typename MetadataType::LambdaArgsType>;

    template <class Return, class... Params>
    class PointerFunction : public RuntimeFunction<Return, Params...> {
    public:
        Return (*function)(Params...);

        PointerFunction(Return (*func)(Params...)) : function(func) {}

        Return operator()(Params... params) const override {
            return this->function(params...);
        }
    };

    template <class Return, template <class...> class ParamList, class... Params>
    struct PointerFunction<Return, ParamList<Params...>> : PointerFunction<Return, Params...> {
        using PointerFunction<Return, Params...>::PointerFunction;
    };

    template <class MetadataType>
    using PointerFunctionType =
        PointerFunction<typename MetadataType::ReturnType, typename MetadataType::LambdaArgsType>;

    struct OpaqueFunctionHandle {
    private:
        traits::UUIDType uuid;
        bool isValid;

        OpaqueFunctionHandle(OpaqueFunction const* func, traits::UUIDType uuid);

    public:
        OpaqueFunction const* function;

        ~OpaqueFunctionHandle();
        OpaqueFunctionHandle();
        OpaqueFunctionHandle(OpaqueFunctionHandle const&) = delete;
        OpaqueFunctionHandle(OpaqueFunctionHandle&& other) noexcept;
        OpaqueFunctionHandle& operator=(OpaqueFunctionHandle const&) = delete;
        OpaqueFunctionHandle& operator=(OpaqueFunctionHandle&& other) noexcept;

        friend class Runtime;
    };

    class Runtime {
        class Impl;
        std::unique_ptr<Impl> m_impl;

        void returnFunctionHandle(OpaqueFunctionHandle const* handle);
        Runtime();
        ~Runtime();

    public:
        static Runtime& get();

        bool insertHook(std::string_view name, OpaqueFunction const* function);

        void registerMapping(traits::UUIDType uuid, std::string name);

        OpaqueFunctionHandle getNextFunction(traits::UUIDType uuid);

        friend class OpaqueFunctionHandle;
    };
}