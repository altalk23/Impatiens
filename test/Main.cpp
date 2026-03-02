#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <impatiens/Impatiens.hpp>

struct A {
    void foo() {}

    void foo(int) {}

    void bar() const {}
};

static bool s_modifiedFoo = false;
static bool s_modifiedFooInt = false;
static bool s_modifiedBar = false;

template <class Derived>
struct impatiens::Modify<Derived, A> : A {
    using Base = A;

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
            static constexpr auto DerivedFunction = traits::Resolve<>::resolve(&Derived::bar);
            static constexpr auto BaseFunction = traits::Resolve<>::resolve(&A::bar);

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
    void foo() {}
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