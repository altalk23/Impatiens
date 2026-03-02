#pragma once

namespace impatiens::traits {
    /// A utility class to represent a list of types.
    template <class... Types>
    struct TypeList {};

    static inline void nullUuidFunction() {}

    template <auto FunctionValue>
    struct Comparator {
    private:
        static void uuidFunction() {}

        template <auto OtherValue>
        struct ResolveUUID {
            static constexpr auto value = &uuidFunction;
        };

        template <>
        struct ResolveUUID<nullptr> {
            static constexpr auto value = &nullUuidFunction;
        };

        static inline constexpr auto uuid = ResolveUUID<FunctionValue>::value;

        constexpr bool isNull() const {
            return uuid == &nullUuidFunction;
        }

    public:
        template <class OtherComparator>
        constexpr bool operator==(OtherComparator const& other) const {
            if (this->isNull() || other.isNull()) return true;

            return uuid == other.uuid;
        }

        template <class OtherComparator>
        constexpr bool operator!=(OtherComparator const& other) const {
            if (this->isNull() || other.isNull()) return false;

            return uuid != other.uuid;
        }

        template <auto OtherValue>
        friend class Comparator;
    };

    /// A utility class to contain information about a function.
    template <class FunctionType>
    struct Metadata {
        static inline constexpr bool isConst = false;
        static inline constexpr bool isLvalue = false;
        static inline constexpr bool isRvalue = false;

        using ClassType = void;
        using ReturnType = void;
        using ArgsType = TypeList<>;
    };

    template <class Return, class... Args>
    struct Metadata<Return (*)(Args...)> : Metadata<void> {
        using ReturnType = Return;
        using ArgsType = TypeList<Args...>;
    };

    template <class Return, class Class, class... Args>
    struct Metadata<Return (Class::*)(Args...)> : Metadata<void> {
        using ClassType = Class;
        using ReturnType = Return;
        using ArgsType = TypeList<Args...>;
    };

    template <class Return, class Class, class... Args>
    struct Metadata<Return (Class::*)(Args...) &> : Metadata<Return (Class::*)(Args...)> {
        static inline constexpr bool isLvalue = true;
    };

    template <class Return, class Class, class... Args>
    struct Metadata<Return (Class::*)(Args...) &&> : Metadata<Return (Class::*)(Args...)> {
        static inline constexpr bool isRvalue = true;
    };

    template <class Return, class Class, class... Args>
    struct Metadata<Return (Class::*)(Args...) const> : Metadata<Return (Class::*)(Args...)> {
        static inline constexpr bool isConst = true;
    };

    template <class Return, class Class, class... Args>
    struct Metadata<Return (Class::*)(Args...) const&> : Metadata<Return (Class::*)(Args...) const> {
        static inline constexpr bool isLvalue = true;
    };

    template <class Return, class Class, class... Args>
    struct Metadata<Return (Class::*)(Args...) const&&> : Metadata<Return (Class::*)(Args...) const> {
        static inline constexpr bool isRvalue = true;
    };

    /// A utility class to resolve class type and return type
    /// of an overloaded function based on the provided arguments.
    template <class... Args>
    struct Resolve {
        template <class Return>
        static constexpr auto resolve(Return (*value)(Args...)) {
            return value;
        }

        template <class Return, class Class>
        static constexpr auto resolve(Return (Class::*value)(Args...)) {
            return value;
        }

        template <class Return, class Class>
        static constexpr auto resolve(Return (Class::*value)(Args...) &) {
            return value;
        }

        template <class Return, class Class>
        static constexpr auto resolve(Return (Class::*value)(Args...) &&) {
            return value;
        }

        template <class Return, class Class>
        static constexpr auto resolve(Return (Class::*value)(Args...) const) {
            return value;
        }

        template <class Return, class Class>
        static constexpr auto resolve(Return (Class::*value)(Args...) const&) {
            return value;
        }

        template <class Return, class Class>
        static constexpr auto resolve(Return (Class::*value)(Args...) const&&) {
            return value;
        }

        static constexpr auto resolve(...) {
            return nullptr;
        }
    };

    /// A utility that always evaluates to false, used for static assertions in templates.
    template <class...>
    inline constexpr bool alwaysFalse = false;
}