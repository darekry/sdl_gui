#include "../lib/catch_amalgamated.hpp"

#include "../src/easing.hpp"

using Catch::Approx;

TEST_CASE("Easing - Linear Function", "[easing][linear]") {
    SECTION("linear(0) returns 0") {
        REQUIRE(Easing::linear(0.0f) == 0.0f);
    }

    SECTION("linear(1) returns 1") {
        REQUIRE(Easing::linear(1.0f) == 1.0f);
    }

    SECTION("linear(0.5) returns 0.5") {
        REQUIRE(Easing::linear(0.5f) == 0.5f);
    }

    SECTION("linear preserves input exactly") {
        REQUIRE(Easing::linear(0.25f) == 0.25f);
        REQUIRE(Easing::linear(0.75f) == 0.75f);
        REQUIRE(Easing::linear(0.1f) == 0.1f);
        REQUIRE(Easing::linear(0.9f) == 0.9f);
    }

    SECTION("linear is monotonic") {
        float prev = Easing::linear(0.0f);
        for (float t = 0.1f; t <= 1.0f; t += 0.1f) {
            float curr = Easing::linear(t);
            REQUIRE(curr >= prev);
            prev = curr;
        }
    }

    SECTION("linear handles values outside [0,1]") {
        REQUIRE(Easing::linear(-0.5f) == -0.5f);
        REQUIRE(Easing::linear(1.5f) == 1.5f);
    }
}

TEST_CASE("Easing - EaseInQuad Function", "[easing][easeinquad]") {
    SECTION("easeInQuad(0) returns 0") {
        REQUIRE(Easing::easeInQuad(0.0f) == 0.0f);
    }

    SECTION("easeInQuad(1) returns 1") {
        REQUIRE(Easing::easeInQuad(1.0f) == 1.0f);
    }

    SECTION("easeInQuad(0.5) returns 0.25") {
        REQUIRE(Easing::easeInQuad(0.5f) == 0.25f);
    }

    SECTION("easeInQuad is quadratic (t squared)") {
        REQUIRE(Easing::easeInQuad(0.25f) == 0.0625f);  // 0.25^2
        REQUIRE(Easing::easeInQuad(0.75f) == Approx(0.5625f));  // 0.75^2
    }

    SECTION("easeInQuad starts slow (accelerates)") {
        // First half of animation covers only 25% of distance
        REQUIRE(Easing::easeInQuad(0.5f) == 0.25f);
        // Second half covers remaining 75%
        float firstHalfDistance = Easing::easeInQuad(0.5f);
        float fullDistance = Easing::easeInQuad(1.0f);
        REQUIRE(firstHalfDistance == 0.25f * fullDistance);
    }

    SECTION("easeInQuad is monotonic increasing") {
        float prev = Easing::easeInQuad(0.0f);
        for (float t = 0.1f; t <= 1.0f; t += 0.1f) {
            float curr = Easing::easeInQuad(t);
            REQUIRE(curr > prev);
            prev = curr;
        }
    }

    SECTION("easeInQuad values are within [0,1] for t in [0,1]") {
        for (float t = 0.0f; t <= 1.0f; t += 0.05f) {
            float result = Easing::easeInQuad(t);
            REQUIRE(result >= 0.0f);
            REQUIRE(result <= 1.0f);
        }
    }
}

TEST_CASE("Easing - EaseOutQuad Function", "[easing][easeoutquad]") {
    SECTION("easeOutQuad(0) returns 0") {
        REQUIRE(Easing::easeOutQuad(0.0f) == 0.0f);
    }

    SECTION("easeOutQuad(1) returns 1") {
        REQUIRE(Easing::easeOutQuad(1.0f) == 1.0f);
    }

    SECTION("easeOutQuad(0.5) returns 0.75") {
        REQUIRE(Easing::easeOutQuad(0.5f) == 0.75f);
    }

    SECTION("easeOutQuad starts fast (decelerates)") {
        // First half of animation covers 75% of distance
        REQUIRE(Easing::easeOutQuad(0.5f) == 0.75f);
    }

    SECTION("easeOutQuad is monotonic increasing") {
        float prev = Easing::easeOutQuad(0.0f);
        for (float t = 0.1f; t <= 1.0f; t += 0.1f) {
            float curr = Easing::easeOutQuad(t);
            REQUIRE(curr > prev);
            prev = curr;
        }
    }

    SECTION("easeOutQuad values are within [0,1] for t in [0,1]") {
        for (float t = 0.0f; t <= 1.0f; t += 0.05f) {
            float result = Easing::easeOutQuad(t);
            REQUIRE(result >= 0.0f);
            REQUIRE(result <= 1.0f);
        }
    }

    SECTION("easeOutQuad is inverse shape of easeInQuad") {
        // At t=0.5, easeIn is at 25%, easeOut is at 75%
        // Together they form a complete ease-in-out curve
        REQUIRE(Easing::easeInQuad(0.5f) + Easing::easeOutQuad(0.5f) == Approx(1.0f));
    }
}

TEST_CASE("Easing - EaseInOutQuad Function", "[easing][easeinoutquad]") {
    SECTION("easeInOutQuad(0) returns 0") {
        REQUIRE(Easing::easeInOutQuad(0.0f) == 0.0f);
    }

    SECTION("easeInOutQuad(1) returns 1") {
        REQUIRE(Easing::easeInOutQuad(1.0f) == 1.0f);
    }

    SECTION("easeInOutQuad(0.5) returns 0.5") {
        REQUIRE(Easing::easeInOutQuad(0.5f) == 0.5f);
    }

    SECTION("easeInOutQuad uses easeInQuad for first half") {
        // For t < 0.5, it should behave like easeInQuad scaled
        REQUIRE(Easing::easeInOutQuad(0.25f) == Approx(0.125f)); // 2*t*t for t=0.25
        REQUIRE(Easing::easeInOutQuad(0.1f) == Approx(0.02f));   // 2*t*t for t=0.1
    }

    SECTION("easeInOutQuad uses easeOutQuad for second half") {
        // For t >= 0.5, it should behave like easeOutQuad scaled
        REQUIRE(Easing::easeInOutQuad(0.75f) == Approx(0.875f));
        REQUIRE(Easing::easeInOutQuad(0.9f) == Approx(0.98f));
    }

    SECTION("easeInOutQuad is symmetric around t=0.5") {
        REQUIRE(Easing::easeInOutQuad(0.3f) == Approx(1.0f - Easing::easeInOutQuad(0.7f)));
        REQUIRE(Easing::easeInOutQuad(0.2f) == Approx(1.0f - Easing::easeInOutQuad(0.8f)));
    }

    SECTION("easeInOutQuad is monotonic increasing") {
        float prev = Easing::easeInOutQuad(0.0f);
        for (float t = 0.05f; t <= 1.0f; t += 0.05f) {
            float curr = Easing::easeInOutQuad(t);
            REQUIRE(curr >= prev);
            prev = curr;
        }
    }

    SECTION("easeInOutQuad values are within [0,1] for t in [0,1]") {
        for (float t = 0.0f; t <= 1.0f; t += 0.05f) {
            float result = Easing::easeInOutQuad(t);
            REQUIRE(result >= 0.0f);
            REQUIRE(result <= 1.0f);
        }
    }
}

TEST_CASE("Easing - Function Comparison", "[easing][comparison]") {
    SECTION("All easing functions start at 0") {
        REQUIRE(Easing::linear(0.0f) == 0.0f);
        REQUIRE(Easing::easeInQuad(0.0f) == 0.0f);
        REQUIRE(Easing::easeOutQuad(0.0f) == 0.0f);
        REQUIRE(Easing::easeInOutQuad(0.0f) == 0.0f);
    }

    SECTION("All easing functions end at 1") {
        REQUIRE(Easing::linear(1.0f) == 1.0f);
        REQUIRE(Easing::easeInQuad(1.0f) == 1.0f);
        REQUIRE(Easing::easeOutQuad(1.0f) == 1.0f);
        REQUIRE(Easing::easeInOutQuad(1.0f) == 1.0f);
    }

    SECTION("At t=0.25, linear is fastest, easeInQuad is slowest") {
        float linearVal = Easing::linear(0.25f);
        float easeInVal = Easing::easeInQuad(0.25f);
        float easeOutVal = Easing::easeOutQuad(0.25f);
        float easeInOutVal = Easing::easeInOutQuad(0.25f);

        REQUIRE(easeInVal < easeInOutVal);
        REQUIRE(easeInOutVal < linearVal);
        REQUIRE(linearVal < easeOutVal);
    }

    SECTION("At t=0.75, easeOutQuad is fastest, linear is in middle") {
        float linearVal = Easing::linear(0.75f);
        float easeInVal = Easing::easeInQuad(0.75f);
        float easeOutVal = Easing::easeOutQuad(0.75f);
        float easeInOutVal = Easing::easeInOutQuad(0.75f);

        REQUIRE(easeInVal < linearVal);
        REQUIRE(linearVal < easeInOutVal);
        REQUIRE(easeInOutVal < easeOutVal);
    }
}

TEST_CASE("Easing - Mathematical Properties", "[easing][math]") {
    SECTION("linear identity property") {
        for (float t = 0.0f; t <= 1.0f; t += 0.1f) {
            REQUIRE(Easing::linear(t) == t);
        }
    }

    SECTION("All easing functions are continuous") {
        // Test continuity by checking small increments
        for (float t = 0.05f; t < 1.0f; t += 0.1f) {
            float linearDiff = Easing::linear(t + 0.001f) - Easing::linear(t);
            REQUIRE(std::abs(linearDiff) < 0.01f);

            float easeInDiff = Easing::easeInQuad(t + 0.001f) - Easing::easeInQuad(t);
            REQUIRE(std::abs(easeInDiff) < 0.01f);

            float easeOutDiff = Easing::easeOutQuad(t + 0.001f) - Easing::easeOutQuad(t);
            REQUIRE(std::abs(easeOutDiff) < 0.01f);

            float easeInOutDiff = Easing::easeInOutQuad(t + 0.001f) - Easing::easeInOutQuad(t);
            REQUIRE(std::abs(easeInOutDiff) < 0.01f);
        }
    }

    SECTION("easeInOutQuad continuity at t=0.5") {
        // Check that there's no discontinuity at the midpoint
        float justBefore = Easing::easeInOutQuad(0.49f);
        float atMidpoint = Easing::easeInOutQuad(0.5f);
        float justAfter = Easing::easeInOutQuad(0.51f);

        REQUIRE(justBefore <= atMidpoint);
        REQUIRE(atMidpoint <= justAfter);
        REQUIRE(std::abs(atMidpoint - 0.5f) < 0.001f);
    }
}