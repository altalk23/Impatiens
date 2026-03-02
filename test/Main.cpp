#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <impatiens/Impatiens.hpp>

std::vector<int> g_vec;

struct A {
    int foo() {
        return 5;
    }

    void foo(int) {}

    void bar(int a, float& b, bool c) const {
        impatiens::OpaqueFunctionHandle nextFunctionHandle;
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

            static auto constexpr UUID = impatiens::traits::Comparator<Function>::uuid;

            (void)impatiens::traits::StaticRegister<[]() {
                impatiens::Runtime::get().registerMapping(UUID, "A::bar");
            }>{};

            nextFunctionHandle = impatiens::Runtime::get().getNextFunction(UUID);

            // if function is not nullptr, we have a hook, so let's call that instead
            if (auto function = static_cast<impatiens::RuntimeFunctionType<MetadataType> const*>(
                    nextFunctionHandle.function
                )) {
                return function->function(this, a, b, c);
            }
            // otherwise we just continue the original function
            // the handle should handle ordering (getNextFunction increments and the destructor decrements)
        }

        g_vec.push_back(67);
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
                traits::Resolve<int, float&, bool>::resolve(&Derived::bar);
            static constexpr auto BaseFunction = traits::Resolve<int, float&, bool>::resolve(&A::bar);

            using DerivedMetadata = traits::Metadata<decltype(DerivedFunction)>;
            using BaseMetadata = traits::Metadata<decltype(BaseFunction)>;

            using DerivedComparator = traits::Comparator<DerivedFunction>;
            using BaseComparator = traits::Comparator<BaseFunction>;

            static auto constexpr DerivedLambda =
                impatiens::traits::LambdaResolver<DerivedMetadata>::lambda(
                    [](auto self, auto a, auto b, auto c) {
                        self->Derived::bar(a, b, c);
                    },
                    [](auto a, auto b, auto c) {
                        static_cast<Derived*>(nullptr)->Derived::bar(a, b, c);
                    }
                );

            static constexpr auto BaseUUID = BaseComparator::uuid;

            if (DerivedComparator{} != BaseComparator{}) {
                // overriden
                s_modifiedBar = true;
                Runtime::get().insertHook(
                    BaseUUID, new RuntimeFunctionType<DerivedMetadata>(DerivedLambda)
                );
            }
        }
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

        REQUIRE(!s_modifiedFoo);
        REQUIRE(!s_modifiedFooInt);
        REQUIRE(!s_modifiedBar);
        B::modify();
        REQUIRE(s_modifiedFoo);
        REQUIRE(!s_modifiedFooInt);
        REQUIRE(s_modifiedBar);

        aObj.bar(a, b, c);
        REQUIRE(g_vec.size() == 4);
        REQUIRE(g_vec[1] == 67);
        REQUIRE(g_vec[2] == 42);
        REQUIRE(g_vec[3] == 67);
    }
}