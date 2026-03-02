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

    class OpaqueFunction {};

    template <class Return, class... Params>
    class RuntimeFunction : public OpaqueFunction {
    public:
        Return (*function)(Params...);

        RuntimeFunction(Return (*func)(Params...)) : function(func) {}

        // void invoke(OpaqueReturn* returnValue, OpaqueParameters* parameters) {
        //     auto* typedReturnValue = static_cast<RuntimeReturn<Return>*>(returnValue);
        //     auto* typedReturn = &typedReturnValue->data;
        //     auto* typedParameters = static_cast<RuntimeParameters<Params...>*>(parameters);

        //     if constexpr (!std::is_void_v<Return>) {
        //         new (std::launder(typedReturn)) Return(
        //             std::apply(
        //                 [this](auto&&... args) {
        //                     return RuntimeReturn<Return>{
        //                         .data = std::bit_cast<std::array<std::byte, sizeof(Return)>>(
        //                             function(std::forward<decltype(args)>(args)...)
        //                         )
        //                     };
        //                 },
        //                 typedParameters->data
        //             )
        //         );
        //     }
        //     else {
        //         std::apply(
        //             [this](auto&&... args) {
        //                 return RuntimeReturn<Return>{
        //                     .data = std::bit_cast<std::array<std::byte, sizeof(Return)>>(
        //                         function(std::forward<decltype(args)>(args)...)
        //                     )
        //                 };
        //             },
        //             typedParameters->data
        //         );
        //     }
        // }
    };

    template <class Return, template <class...> class ParamList, class... Params>
    struct RuntimeFunction<Return, ParamList<Params...>> : RuntimeFunction<Return, Params...> {};

    template <class MetadataType>
    using RuntimeFunctionType =
        RuntimeFunction<typename MetadataType::ReturnType, typename MetadataType::LambdaArgsType>;

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

        bool insertHook(traits::UUIDType uuid, OpaqueFunction const* function);

        void registerMapping(traits::UUIDType uuid, std::string name);

        OpaqueFunctionHandle getNextFunction(traits::UUIDType uuid);

        friend class OpaqueFunctionHandle;
    };
}