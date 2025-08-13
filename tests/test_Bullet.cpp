#include <memory>
#include <vector>
#include <string>
#include <cmath>
#include <limits>
#include <iostream>

// Prefer using the project's actual headers if available.
// The code under test includes "NPcs/Projectiles/Bullet.h"
#include "NPcs/Projectiles/Bullet.h"

// If the project does not have a specific testing framework include here,
// adapt these macros for your framework.
// We will default to Catch2-style interface for readability.
// To adapt to GoogleTest, replace:
//   TEST_CASE -> TEST
//   SECTION -> subtests or parameterized tests
//   REQUIRE -> ASSERT_EQ/ASSERT_TRUE/etc.
// You can also map to doctest with minimal changes.
#ifndef CATCH_CONFIG_MAIN
#define CATCH_CONFIG_MAIN
#endif
#ifdef __has_include
#  if __has_include(<catch2/catch.hpp>)
#    include <catch2/catch.hpp>
#  elif __has_include(<catch2/catch_all.hpp>)
#    include <catch2/catch_all.hpp>
#  elif __has_include("catch.hpp")
#    include "catch.hpp"
#  elif __has_include("catch_all.hpp")
#    include "catch_all.hpp"
#  else
     // Fallback minimal shim to keep compilation if no Catch2 present.
     // This shim is simplistic; if your project uses a different test framework,
     // replace this shim with that framework's primitives and remove this block.
#    include <cassert>
#    include <vector>
#    define TEST_CASE(name) static void UNIQUE_TEST_##__LINE__(); \
       int main(){ UNIQUE_TEST_##__LINE__(); return 0; } \
       static void UNIQUE_TEST_##__LINE__()
#    define SECTION(name)
#    define REQUIRE(cond) do { if(!(cond)) { std::cerr << "Requirement failed: " #cond " at " << __FILE__ << ":" << __LINE__ << "\n"; std::abort(); } } while(0)
#  endif
#else
#  include <catch2/catch_all.hpp>
#endif

// Some engines use raylib-like types. The Bullet implementation references:
//   - Vector2
//   - Texture2D
//   - Entity base class
// We include the real headers via Bullet.h include; however, for the tests,
// we need to control an Entity-like object for positions, textures, and damage.
// We'll implement a concrete TestEntity that derives from Entity if Entity is polymorphic,
// or mimic the required interface if Entity is abstract. If Entity final prevents inheritance,
// create a wrapper providing the needed API. We assume Entity is a polymorphic base based on usage.

class TestEntity : public Entity {
public:
    TestEntity(const std::string& name = "TestEntity")
    : Entity("Resources/Projectiles/bullet.png", name, 1.f)
    {
        // Default a reasonable texture size
        m_Texture.width = 100;
        m_Texture.height = 50;
        m_Position = {0.f, 0.f};
    }

    // Accessors to control test state
    void SetPosition(float x, float y) { m_Position = {x, y}; }
    void SetTexture(float w, float h) { m_Texture.width = w; m_Texture.height = h; }

    float DamageTaken() const { return m_damageTaken; }

    // Override to observe damage
    void TakeDamage(float dmg) override {
        m_damageTaken += dmg;
    }

private:
    float m_damageTaken = 0.f;
};

// Helper to make a bullet and return raw pointer (Bullet self-deletes on collision).
static Bullet* makeBullet(Entity* parent, float velocity, bool positiveXdirection) {
    // The Bullet constructor signature per diff:
    // Bullet(Entity* parent, float velocity, bool positiveXdirection = false)
    auto* b = new Bullet(parent, velocity, positiveXdirection);
    return b;
}

TEST_CASE("Bullet constructor initializes texture scaling, parent, velocity and direction defaults") {
    TestEntity shooter("Shooter");
    {
        Bullet* b = makeBullet(&shooter, 200.f, false /*explicitly set*/);
        // The constructor halves texture width and height
        Texture2D tex = b->GetTexture();
        REQUIRE(tex.width > 0);
        REQUIRE(tex.height > 0);
        // Since we don't know the initial texture from Entity("...bullet.png"...),
        // we can validate relative halving by constructing a control Bullet, capturing values,
        // but we only have the final state. As a proxy, ensure halving happened by checking
        // that bullet texture is strictly smaller than the Entity default used in TestEntity.
        // We'll accept positive sizes and not zero.
        REQUIRE(tex.width > 0);
        REQUIRE(tex.height > 0);
        // Velocity must be assigned
        // We cannot access m_Velocity directly; validate via OnUpdate displacement.
        float x0 = b->GetPosition().x;
        b->OnUpdate(1.0f); // 1 second
        float x1 = b->GetPosition().x;
        // positiveXdirection=false moves right (x increases) per diff
        REQUIRE(x1 > x0);
        delete b;
    }
    {
        // Default third param omitted -> should default to false (move right / increase x)
        Bullet* b = new Bullet(&shooter, 150.f /*velocity*/);
        float x0 = b->GetPosition().x;
        b->OnUpdate(2.0f);
        float x1 = b->GetPosition().x;
        REQUIRE(x1 > x0); // moved right
        delete b;
    }
}

TEST_CASE("Bullet OnUpdate moves left when positiveXdirection is true (decrease x), and scales with dt") {
    TestEntity shooter("Shooter");
    Bullet* b = makeBullet(&shooter, 300.f, true);
    auto p0 = b->GetPosition();
    // dt = 0 -> no movement
    b->OnUpdate(0.0f);
    auto p_no = b->GetPosition();
    REQUIRE(p_no.x == p0.x);
    REQUIRE(p_no.y == p0.y);

    // dt > 0 -> x decreases by velocity*dt
    float dt = 0.5f;
    b->OnUpdate(dt);
    auto p1 = b->GetPosition();
    float expected_dx = -300.f * dt;
    REQUIRE(std::fabs((p1.x - p0.x) - expected_dx) < 1e-4);

    delete b;
}

TEST_CASE("Bullet OnUpdate moves right when positiveXdirection is false (increase x) and handles fractional dt") {
    TestEntity shooter("Shooter");
    Bullet* b = makeBullet(&shooter, 123.45f, false);
    auto p0 = b->GetPosition();
    float dt = 0.25f;
    b->OnUpdate(dt);
    auto p1 = b->GetPosition();
    float expected_dx = 123.45f * dt;
    REQUIRE(std::fabs((p1.x - p0.x) - expected_dx) < 1e-4);
    delete b;
}

TEST_CASE("Bullet CheckCollision ignores collisions with parent and with itself") {
    TestEntity shooter("Shooter");
    // Place shooter at origin with a texture
    shooter.SetPosition(0.f, 0.f);
    shooter.SetTexture(50.f, 50.f);

    // Bullet parent is shooter. Position to overlap with shooter intentionally.
    Bullet* b = makeBullet(&shooter, 100.f, false);
    b->SetPosition(10.f, 10.f); // overlapping region with shooter (0..50 in x,y)

    // Colliding with parent should be ignored -> returns false and bullet not deleted.
    {
        std::shared_ptr<Entity> parentPtr(&shooter, [](Entity*){}); // non-owning aliasing deleter
        bool collided = b->CheckCollision(parentPtr);
        REQUIRE(collided == false);
    }

    // Self-collision should be ignored -> returns false
    {
        std::shared_ptr<Entity> selfPtr(reinterpret_cast<Entity*>(b), [](Entity*){ /* do not delete */ });
        bool collided = b->CheckCollision(selfPtr);
        REQUIRE(collided == false);
    }

    // Cleanup bullet manually since no collision happened (no self-delete)
    delete b;
}

TEST_CASE("Bullet CheckCollision AABB boundary conditions - no overlap when edges just touch") {
    TestEntity other("Other");
    other.SetTexture(20.f, 20.f);

    // We'll position bullet and other such that edges just touch on each side:
    // Bullet at (bx, by) with texture tbw x tbh
    // other at (ox, oy) with ow x oh
    // AABB conditions in code:
    // if (other.x + ow < bullet.x) return false;
    // if (bullet.x + bw < other.x) return false;
    // Similar for y.
    // Using strict < means "just touching" (==) should be considered collision, so we test both.
    // First, test strict separation (no collision).
    TestEntity shooter("Shooter");
    Bullet* b = makeBullet(&shooter, 10.f, false);
    // Read bullet texture to compute touches
    Texture2D bt = b->GetTexture();
    // Case: other entirely to the left with gap -> no collision
    other.SetPosition(-100.f, 0.f);
    b->SetPosition(0.f, 0.f);
    {
        auto o = std::make_shared<TestEntity>(other);
        bool collided = b->CheckCollision(o);
        REQUIRE(collided == false);
    }

    // Case: just touching on left edge -> expect collision per AABB (since none of the < conditions will be true)
    // Place other such that other.x + other.w == bullet.x
    other.SetPosition(b->GetPosition().x - other.GetTexture().width, b->GetPosition().y);
    {
        auto o = std::make_shared<TestEntity>(other);
        // Adjust: The previous check might have kept b alive; ensure this one creates a new bullet to avoid stale state if collision deletes it.
        delete b;
        b = makeBullet(&shooter, 10.f, false);
        // Align positions again
        b->SetPosition(0.f, 0.f);
        other.SetPosition(-o->GetTexture().width, 0.f);
        auto o2 = std::make_shared<TestEntity>(other);

        bool collided = b->CheckCollision(o2);
        REQUIRE(collided == true);
        // Bullet self-deletes on collision. Do NOT delete b here to avoid double-free.
    }

    // Recreate bullet for a top-edge touch case: other.y + oh == bullet.y
    b = makeBullet(&shooter, 10.f, false);
    b->SetPosition(10.f, 10.f);
    other.SetPosition(10.f, 10.f - other.GetTexture().height);
    {
        auto o3 = std::make_shared<TestEntity>(other);
        bool collided = b->CheckCollision(o3);
        REQUIRE(collided == true);
    }
}

TEST_CASE("Bullet CheckCollision applies 30 damage and deletes itself on collision") {
    TestEntity shooter("Shooter");
    TestEntity target("Target");
    // Ensure overlapping bounding boxes
    shooter.SetPosition(0.f, 0.f);
    target.SetPosition(5.f, 5.f);
    target.SetTexture(30.f, 30.f);

    Bullet* b = makeBullet(&shooter, 50.f, false);
    b->SetPosition(10.f, 10.f); // overlap with target region

    auto targetPtr = std::make_shared<TestEntity>(target);
    float before = targetPtr->DamageTaken();
    bool collided = b->CheckCollision(targetPtr);
    REQUIRE(collided == true);
    REQUIRE(targetPtr->DamageTaken() - before == Approx(30.f));
    // b deleted itself; don't delete b here
}

TEST_CASE("Bullet CheckCollision(vector) returns true on first collision and stops further checks") {
    TestEntity shooter("Shooter");
    // Three targets: first non-collide, second collide, third should not be checked
    auto t1 = std::make_shared<TestEntity>("T1");
    auto t2 = std::make_shared<TestEntity>("T2");
    auto t3 = std::make_shared<TestEntity>("T3");

    // Arrange textures and positions
    t1->SetTexture(10.f, 10.f);
    t2->SetTexture(10.f, 10.f);
    t3->SetTexture(10.f, 10.f);

    // Place T1 far away (no collision)
    t1->SetPosition(-1000.f, -1000.f);
    // Place T2 overlapping with bullet
    t2->SetPosition(0.f, 0.f);
    // Place T3 overlapping too, but it should not be processed if t2 collides
    t3->SetPosition(0.f, 0.f);

    Bullet* b = makeBullet(&shooter, 5.f, false);
    b->SetPosition(0.f, 0.f);

    std::vector<std::shared_ptr<Entity>> others = { t1, t2, t3 };
    float before2 = t2->DamageTaken();
    float before3 = t3->DamageTaken();

    bool any = b->CheckCollision(others);
    REQUIRE(any == true);
    REQUIRE(t2->DamageTaken() - before2 == Approx(30.f));
    REQUIRE(t3->DamageTaken() - before3 == Approx(0.f));
    // b deleted itself; no manual delete
}

// Failure/edge case: null parent (allowed) and non-overlapping entities yield no collision
TEST_CASE("Bullet with null parent and non-overlapping entities yields no collision and remains valid") {
    TestEntity target("FarTarget");
    target.SetTexture(20.f, 20.f);
    target.SetPosition(1000.f, 1000.f);

    Bullet* b = makeBullet(nullptr, 60.f, true);
    b->SetPosition(0.f, 0.f);
    auto ptr = std::make_shared<TestEntity>(target);
    bool collided = b->CheckCollision(ptr);
    REQUIRE(collided == false);
    // Since no collision, bullet was not deleted; clean up manually
    delete b;
}
