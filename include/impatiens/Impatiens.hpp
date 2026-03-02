#pragma once
#include "Traits.hpp"

namespace impatiens {

    template <class Derived, class Base>
    struct Modify : Base {
        static_assert(
            traits::alwaysFalse<Derived, Base>,
            "A modify specialization does not exist for this class."
        );
    };
}