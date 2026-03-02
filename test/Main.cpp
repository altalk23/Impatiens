#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <impatiens/Impatiens.hpp>

struct A {
    int foo() {
        return 5;
    }

    void foo(int) {}

    void bar(int a, float& b, bool c) const {
        {
            using ClassType = A;
            using ResolveType = impatiens::traits::Resolve<decltype(a), decltype(b), decltype(c)>;
            static auto constexpr Function = ResolveType::resolve(&A::bar);
            using MetadataType = impatiens::traits::Metadata<decltype(Function)>;
            static auto constexpr Lambda = impatiens::traits::LambdaResolver<MetadataType>::lambda(
                [](auto self, auto a, auto b, auto c) {
                    self->ClassType::bar(a, b, c);
                },
                [](auto a, auto b, auto c) {
                    static_cast<ClassType*>(nullptr)->ClassType::bar(a, b, c);
                }
            );
        }
    }

    static void baz(int a, float& b, bool c) {
        {
            using ClassType = A;
            using ResolveType = impatiens::traits::Resolve<decltype(a), decltype(b), decltype(c)>;
            static auto constexpr Function = ResolveType::resolve(&A::baz);
            using MetadataType = impatiens::traits::Metadata<decltype(Function)>;
            static auto constexpr Lambda = impatiens::traits::LambdaResolver<MetadataType>::lambda(
                [](auto self, auto a, auto b, auto c) {
                    self->ClassType::baz(a, b, c);
                },
                [](auto a, auto b, auto c) {
                    static_cast<ClassType*>(nullptr)->ClassType::baz(a, b, c);
                }
            );
        }
    }
};

static bool s_modifiedFoo = false;
static bool s_modifiedFooInt = false;
static bool s_modifiedBar = false;

template <class Derived>
struct impatiens::Modify<Derived, A> : impatiens::traits::BaseExposer<A> {
    static constexpr auto modify() {
        {
            static constexpr auto DerivedFunction = traits::Resolve<>::resolve(&Derived::foo);
            static constexpr auto BaseFunction = traits::Resolve<>::resolve(&A::foo);

            using DerivedMetadata = traits::Metadata<decltype(DerivedFunction)>;
            using BaseMetadata = traits::Metadata<decltype(BaseFunction)>;

            using DerivedComparator = traits::Comparator<DerivedFunction>;
            using BaseComparator = traits::Comparator<BaseFunction>;

            if (DerivedComparator{} != BaseComparator{}) {
                // overriden
                s_modifiedFoo = true;
            }
        }

        {
            static constexpr auto DerivedFunction = traits::Resolve<int>::resolve(&Derived::foo);
            static constexpr auto BaseFunction = traits::Resolve<int>::resolve(&A::foo);

            using DerivedMetadata = traits::Metadata<decltype(DerivedFunction)>;
            using BaseMetadata = traits::Metadata<decltype(BaseFunction)>;

            using DerivedComparator = traits::Comparator<DerivedFunction>;
            using BaseComparator = traits::Comparator<BaseFunction>;

            if (DerivedComparator{} != BaseComparator{}) {
                // overriden
                s_modifiedFooInt = true;
            }
        }

        {
            static constexpr auto DerivedFunction =
                traits::Resolve<int, float, bool>::resolve(&Derived::bar);
            static constexpr auto BaseFunction = traits::Resolve<int, float, bool>::resolve(&A::bar);

            using DerivedMetadata = traits::Metadata<decltype(DerivedFunction)>;
            using BaseMetadata = traits::Metadata<decltype(BaseFunction)>;

            using DerivedComparator = traits::Comparator<DerivedFunction>;
            using BaseComparator = traits::Comparator<BaseFunction>;

            if (DerivedComparator{} != BaseComparator{}) {
                // overriden
                s_modifiedBar = true;
            }
        }
    }
};

struct B : impatiens::Modify<B, A> {
    int foo() {
        return 10;
    }
};

TEST_CASE("Modify") {
    SECTION("B") {
        REQUIRE(!s_modifiedFoo);
        REQUIRE(!s_modifiedFooInt);
        REQUIRE(!s_modifiedBar);
        B::modify();
        REQUIRE(s_modifiedFoo);
        REQUIRE(!s_modifiedFooInt);
        REQUIRE(!s_modifiedBar);
    }
}