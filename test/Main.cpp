#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <impatiens/Impatiens.hpp>

std::vector<int> g_vec;

struct A {
    int foo() {
        IMPATIENS_HANDLE_OVERLOADED(A, foo, foo);
        return 5;
    }

    void foo(int a) {
        IMPATIENS_HANDLE_OVERLOADED(A, foo, foo2, a);
    }

    void bar(int a, float& b, bool c) const {
        IMPATIENS_HANDLE(A, bar, a, b, c);

        g_vec.push_back(67);
    }

    static float baz(double d) {
        IMPATIENS_HANDLE_STATIC(A, baz, d);
        return static_cast<float>(d);
    }
};

template <class Derived>
struct impatiens::Modify<Derived, A> : impatiens::traits::BaseExposer<A> {
    static constexpr auto modify() {
        IMPATIENS_MODIFY(foo);
        IMPATIENS_MODIFY(foo, int);
        IMPATIENS_MODIFY(bar, int, float&, bool);
        IMPATIENS_MODIFY(baz, double);
    }
};

struct B : impatiens::Modify<B, A> {
    int foo() {
        return 10;
    }

    void bar(int a, float& b, bool c) const {
        A::bar(a, b, c);
        g_vec.push_back(42);
        A::bar(a, b, c);
    }
};

TEST_CASE("Modify") {
    SECTION("B") {
        int a = 0;
        float b = 0;
        bool c = false;
        A aObj;

        aObj.bar(a, b, c);
        REQUIRE(g_vec.size() == 1);
        REQUIRE(g_vec[0] == 67);

        REQUIRE(aObj.foo() == 5);

        B::modify();

        aObj.bar(a, b, c);
        REQUIRE(g_vec.size() == 4);
        REQUIRE(g_vec[1] == 67);
        REQUIRE(g_vec[2] == 42);
        REQUIRE(g_vec[3] == 67);

        REQUIRE(aObj.foo() == 10);
    }
}