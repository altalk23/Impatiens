#pragma once
#include <compare>

namespace impatiens::traits {
    /// A utility class to represent a list of types.
    template <class... Types>
    struct TypeList {};

    /// The null value for the Comparator.
    static inline void nullUuidFunction() {}

    /// A utility class to compare function pointers, including virtual ones.
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
        constexpr auto operator<=>(OtherComparator const& other) const {
            if (this->isNull() || other.isNull()) return std::strong_ordering::equal;
            return (uuid <=> other.uuid);
        }

        template <auto OtherValue>
        friend class Comparator;
    };

    /// A utility class to contain information about a function.
    template <class FunctionType>
    struct Metadata {
        static inline constexpr bool isStatic = true;
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

        static constexpr auto lambda(Return (*value)(Args...)) {
            return value;
        }
    };

    template <class Return, class Class, class... Args>
    struct Metadata<Return (Class::*)(Args...)> : Metadata<void> {
        static inline constexpr bool isStatic = false;

        using ClassType = Class;
        using ReturnType = Return;
        using ArgsType = TypeList<Args...>;

        static constexpr auto lambda(Return (*value)(Class*, Args...)) {
            return value;
        }
    };

    template <class Return, class Class, class... Args>
    struct Metadata<Return (Class::*)(Args...) &> : Metadata<Return (Class::*)(Args...)> {
        static inline constexpr bool isLvalue = true;

        static constexpr auto lambda(Return (*value)(Class const*, Args...)) {
            return value;
        }
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

    template <class FunctionType>
    struct Metadata<FunctionType const> : Metadata<FunctionType> {};

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

    /// A utility class to expose the base class in a generalized way for the Modify class.
    template <class Class>
    struct BaseExposer : Class {
        using Base = Class;
    };

    template <class MetadataType>
    struct LambdaResolver {
        template <bool IsStatic>
        struct Lambda {
            static constexpr auto lambda(auto member, auto nonMember) {
                return MetadataType::lambda(nonMember);
            }
        };

        template <>
        struct Lambda<false> {
            static constexpr auto lambda(auto member, auto nonMember) {
                return MetadataType::lambda(member);
            }
        };

        static constexpr auto lambda(auto member, auto nonMember) {
            return Lambda<MetadataType::isStatic>::lambda(member, nonMember);
        }
    };

    /// A utility that always evaluates to false, used for static assertions in templates.
    template <class...>
    inline constexpr bool alwaysFalse = false;
}