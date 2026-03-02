#pragma once

#define IMPATIENS_EXPAND_INNER(...) __VA_ARGS__
#define IMPATIENS_EXPAND(...) IMPATIENS_EXPAND_INNER(__VA_ARGS__)

#define IMPATIENS_STRINGIFY_INNER(...) #__VA_ARGS__
#define IMPATIENS_STRINGIFY(...) IMPATIENS_STRINGIFY_INNER(__VA_ARGS__)

#define IMPATIENS_INVOKE_INNER(Macro_, ...) Macro_(__VA_ARGS__)
#define IMPATIENS_INVOKE(Macro_, ...) IMPATIENS_INVOKE_INNER(Macro_, __VA_ARGS__)

#define IMPATIENS_CONCAT_INNER(A_, B_) A_##B_
#define IMPATIENS_CONCAT(A_, B_) IMPATIENS_CONCAT_INNER(A_, B_)

#define IMPATIENS_NUM_ARGS_INNER(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N

#define IMPATIENS_NUM_ARGS(...) \
    IMPATIENS_NUM_ARGS_INNER(__VA_OPT__(__VA_ARGS__, ) 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define IMPATIENS_APPLY_0(Macro_, ...)
#define IMPATIENS_APPLY_1(Macro_, X_) Macro_(1, X_)
#define IMPATIENS_APPLY_2(Macro_, X_, ...) Macro_(2, X_), IMPATIENS_APPLY_1(Macro_, __VA_ARGS__)
#define IMPATIENS_APPLY_3(Macro_, X_, ...) Macro_(3, X_), IMPATIENS_APPLY_2(Macro_, __VA_ARGS__)
#define IMPATIENS_APPLY_4(Macro_, X_, ...) Macro_(4, X_), IMPATIENS_APPLY_3(Macro_, __VA_ARGS__)
#define IMPATIENS_APPLY_5(Macro_, X_, ...) Macro_(5, X_), IMPATIENS_APPLY_4(Macro_, __VA_ARGS__)
#define IMPATIENS_APPLY_6(Macro_, X_, ...) Macro_(6, X_), IMPATIENS_APPLY_5(Macro_, __VA_ARGS__)
#define IMPATIENS_APPLY_7(Macro_, X_, ...) Macro_(7, X_), IMPATIENS_APPLY_6(Macro_, __VA_ARGS__)
#define IMPATIENS_APPLY_8(Macro_, X_, ...) Macro_(8, X_), IMPATIENS_APPLY_7(Macro_, __VA_ARGS__)
#define IMPATIENS_APPLY_9(Macro_, X_, ...) Macro_(9, X_), IMPATIENS_APPLY_8(Macro_, __VA_ARGS__)
#define IMPATIENS_APPLY_10(Macro_, X_, ...) Macro_(10, X_), IMPATIENS_APPLY_9(Macro_, __VA_ARGS__)

#define IMPATIENS_APPLY(Macro_, ...)                                                             \
    IMPATIENS_EXPAND(                                                                            \
        IMPATIENS_CONCAT(IMPATIENS_APPLY_, IMPATIENS_NUM_ARGS(__VA_ARGS__))(Macro_, __VA_ARGS__) \
    )

#define IMPATIENS_ADD_DECLTYPE(Index_, X_) decltype(X_)
#define IMPATIENS_ADD_AUTO(Index_, X_) auto X_
#define IMPATIENS_ADD_PARAMETER(Index_, X_) X_ IMPATIENS_CONCAT(param, Index_)
#define IMPATIENS_GET_PARAMETER(Index_, X_) IMPATIENS_CONCAT(param, Index_)

#define IMPATIENS_EXPAND_CALL_PARAMETERS(...) this __VA_OPT__(, __VA_ARGS__)
#define IMPATIENS_EXPAND_CALL_PARAMETERS_STATIC(...) __VA_ARGS__

#define IMPATIENS_HANDLE_FUNCTION_OVERLOADED_INNER(                                           \
    Line_, CallerExpand_, Class_, Function_, FunctionID_, ...                                 \
)                                                                                             \
    impatiens::OpaqueFunctionHandle nextFunctionHandle;                                       \
    {                                                                                         \
        using ClassType = Class_;                                                             \
        using ResolveType =                                                                   \
            impatiens::traits::Resolve<IMPATIENS_APPLY(IMPATIENS_ADD_DECLTYPE, __VA_ARGS__)>; \
        static auto constexpr Function = ResolveType::resolve(&Class_::Function_);            \
        using MetadataType = impatiens::traits::Metadata<decltype(Function)>;                 \
        static auto constexpr UUID = impatiens::traits::Comparator<Function>::uuid;           \
        (void)impatiens::traits::StaticRegister<[]() {                                        \
            impatiens::Runtime::get().registerMapping(                                        \
                UUID, IMPATIENS_STRINGIFY(Class_) "::" FunctionID_                            \
            );                                                                                \
        }>{};                                                                                 \
        nextFunctionHandle = impatiens::Runtime::get().getNextFunction(UUID);                 \
        if (auto function = static_cast<impatiens::RuntimeFunctionType<MetadataType> const*>( \
                nextFunctionHandle.function                                                   \
            )) {                                                                              \
            return function->function(CallerExpand_(__VA_ARGS__));                            \
        }                                                                                     \
    }

#define IMPATIENS_HANDLE(Class_, Function_, ...) \
    IMPATIENS_HANDLE_FUNCTION_OVERLOADED_INNER(  \
        __LINE__,                                \
        IMPATIENS_EXPAND_CALL_PARAMETERS,        \
        Class_,                                  \
        Function_,                               \
        IMPATIENS_STRINGIFY(Function_),          \
        __VA_ARGS__                              \
    )

#define IMPATIENS_HANDLE_STATIC(Class_, Function_, ...) \
    IMPATIENS_HANDLE_FUNCTION_OVERLOADED_INNER(         \
        __LINE__,                                       \
        IMPATIENS_EXPAND_CALL_PARAMETERS_STATIC,        \
        Class_,                                         \
        Function_,                                      \
        IMPATIENS_STRINGIFY(Function_),                 \
        __VA_ARGS__                                     \
    )

#define IMPATIENS_HANDLE_OVERLOADED(Class_, Function_, FunctionID_, ...) \
    IMPATIENS_HANDLE_FUNCTION_OVERLOADED_INNER(                          \
        __LINE__,                                                        \
        IMPATIENS_EXPAND_CALL_PARAMETERS,                                \
        Class_,                                                          \
        Function_,                                                       \
        IMPATIENS_STRINGIFY(FunctionID_),                                \
        __VA_ARGS__                                                      \
    )

#define IMPATIENS_HANDLE_OVERLOADED_STATIC(Class_, Function_, FunctionID_, ...) \
    IMPATIENS_HANDLE_FUNCTION_OVERLOADED_INNER(                                 \
        __LINE__,                                                               \
        IMPATIENS_EXPAND_CALL_PARAMETERS_STATIC,                                \
        Class_,                                                                 \
        Function_,                                                              \
        IMPATIENS_STRINGIFY(FunctionID_),                                       \
        __VA_ARGS__                                                             \
    )

#define IMPATIENS_MODIFY(Function_, ...)                                                                \
    {                                                                                                   \
        static constexpr auto DerivedFunction =                                                         \
            traits::Resolve<__VA_ARGS__>::resolve(&Derived::Function_);                                 \
        static constexpr auto BaseFunction =                                                            \
            traits::Resolve<__VA_ARGS__>::resolve(&Base::Function_);                                    \
        using DerivedMetadata = traits::Metadata<decltype(DerivedFunction)>;                            \
        using BaseMetadata = traits::Metadata<decltype(BaseFunction)>;                                  \
        using DerivedComparator = traits::Comparator<DerivedFunction>;                                  \
        using BaseComparator = traits::Comparator<BaseFunction>;                                        \
        if constexpr (DerivedFunction != nullptr && BaseFunction != nullptr) {                          \
            static auto constexpr DerivedLambda =                                                       \
                impatiens::traits::LambdaResolver<DerivedMetadata>::lambda(                             \
                    [](auto self __VA_OPT__(, IMPATIENS_APPLY(IMPATIENS_ADD_PARAMETER, __VA_ARGS__))) { \
                        return self->Derived::Function_(                                                \
                            IMPATIENS_APPLY(IMPATIENS_GET_PARAMETER, __VA_ARGS__)                       \
                        );                                                                              \
                    },                                                                                  \
                    [](IMPATIENS_APPLY(IMPATIENS_ADD_PARAMETER, __VA_ARGS__)) {                         \
                        return static_cast<Derived*>(nullptr)->Derived::Function_(                      \
                            IMPATIENS_APPLY(IMPATIENS_GET_PARAMETER, __VA_ARGS__)                       \
                        );                                                                              \
                    }                                                                                   \
                );                                                                                      \
            static constexpr auto BaseUUID = BaseComparator::uuid;                                      \
            if (DerivedComparator{} != BaseComparator{}) {                                              \
                Runtime::get().insertHook(                                                              \
                    BaseUUID, new RuntimeFunctionType<DerivedMetadata>(DerivedLambda)                   \
                );                                                                                      \
            }                                                                                           \
        }                                                                                               \
    }