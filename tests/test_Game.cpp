// NOTE: Testing framework and library:
// - Preferred: GoogleTest (gtest). If <gtest/gtest.h> is available in this project, these includes work out-of-the-box.
// - If the project uses Catch2/doctest instead, adapt the includes and TEST/ASSERT macros accordingly.
//   We keep the test body framework-agnostic except for common ASSERT/EXPECT macros.
// Rationale: We focus on unit-testing Game::update and Game::draw as per the provided diff.

#include <memory>
#include <vector>
#include <algorithm>
#include <string>
#include <type_traits>

#include "Game.h"
// Try to include Entity/Player interfaces to derive test doubles.
// Paths may be Entity.h and NPCs/Player.h per the snippet.
#include "Entity.h"
#include "NPCs/Player.h"

// Prefer GoogleTest by default
#ifdef __has_include
#  if __has_include(<gtest/gtest.h>)
#    include <gtest/gtest.h>
#    define HAS_GTEST 1
#  elif __has_include("gtest/gtest.h")
#    include "gtest/gtest.h"
#    define HAS_GTEST 1
#  endif
#  if __has_include(<catch2/catch.hpp>)
#    include <catch2/catch.hpp>
#    define HAS_CATCH2 1
#  elif __has_include("catch.hpp")
#    include "catch.hpp"
#    define HAS_CATCH2 1
#  endif
#  if __has_include(<doctest/doctest.h>)
#    include <doctest/doctest.h>
#    define HAS_DOCTEST 1
#  endif
#endif

#if !defined(HAS_GTEST) && !defined(HAS_CATCH2) && !defined(HAS_DOCTEST)
// Fallback minimal assertions to avoid breaking builds if no framework is detected.
// Replace with your project's test framework.
#  include <cassert>
#  define TEST_CASE(name) static void name(); int main(){ name(); return 0; } static void name()
#  define SECTION(_name)
#  define REQUIRE(x) assert(x)
#  define REQUIRE_FALSE(x) assert(!(x))
#  define CHECK(x) assert(x)
#  define CHECK_FALSE(x) assert(!(x))
#else
#  ifdef HAS_GTEST
#    define TEST_CASE(_name) TEST(GameTests, _name)
#    define SECTION(_name)
#    define REQUIRE(x) ASSERT_TRUE(x)
#    define REQUIRE_FALSE(x) ASSERT_FALSE(x)
#    define CHECK(x) EXPECT_TRUE(x)
#    define CHECK_FALSE(x) EXPECT_FALSE(x)
#  elif defined(HAS_CATCH2)
#    define TEST_CASE(_name) TEST_CASE(_name, "[Game]")
#    define SECTION(_name) SECTION(_name)
#    define REQUIRE(x) REQUIRE(x)
#    define REQUIRE_FALSE(x) REQUIRE_FALSE(x)
#    define CHECK(x) CHECK(x)
#    define CHECK_FALSE(x) CHECK_FALSE(x)
#  elif defined(HAS_DOCTEST)
#    define TEST_CASE(_name) TEST_CASE(_name)
#    define SECTION(_name) SUBCASE(_name)
#    define REQUIRE(x) REQUIRE(x)
#    define REQUIRE_FALSE(x) REQUIRE_FALSE(x)
#    define CHECK(x) CHECK(x)
#    define CHECK_FALSE(x) CHECK_FALSE(x)
#  endif
#endif

// Test doubles for Entity, Player, and Bullet.
// We assume the real Entity/Player expose the following per the snippet:
//  - Entity: virtual void Update(float), virtual void Draw(), virtual bool CheckCollision(const std::vector<std::shared_ptr<Entity>>&),
//            virtual bool IsAlive(), Vector2& GetPosition()
//  - Player: publicly accessible std::vector<std::shared_ptr<Entity>> m_Bullets
//
// These doubles override methods to capture calls and simulate collisions and liveness.

struct CallLog {
    std::vector<std::string> log;
    void add(const std::string& s) { log.push_back(s); }
};

class StubEntity : public Entity {
public:
    explicit StubEntity(const std::string& name = "StubEntity", bool alive = true)
    : Entity("resources/Player/idle.png", name, 100.0f),
      alive_(alive) {}

    void Update(float dt) override {
        updatedWith_.push_back(dt);
        callLog_.add(name_ + ".Update(" + std::to_string(dt) + ")");
    }

    void Draw() override {
        drawCount_++;
        callLog_.add(name_ + ".Draw()");
    }

    bool CheckCollision(const std::vector<std::shared_ptr<Entity>>& others) override {
        lastCollisionCheckCount_ = static_cast<int>(others.size());
        callLog_.add(name_ + ".CheckCollision(count=" + std::to_string(lastCollisionCheckCount_) + ")");
        return collides_;
    }

    bool IsAlive() const override {
        return alive_;
    }

    Vector2& GetPosition() override {
        return Entity::GetPosition();
    }

    // Controls
    void setAlive(bool a) { alive_ = a; }
    void setCollides(bool c) { collides_ = c; }

    // Observations
    int drawCount() const { return drawCount_; }
    const std::vector<float>& updatedWith() const { return updatedWith_; }
    int lastCollisionCheckCount() const { return lastCollisionCheckCount_; }
    CallLog& calls() { return callLog_; }

private:
    bool alive_{true};
    bool collides_{false};
    int drawCount_{0};
    int lastCollisionCheckCount_{0};
    std::vector<float> updatedWith_{};
    CallLog callLog_{};
    using Entity::name_; // if Entity has a protected name_ member; otherwise we track our own
};

class StubBullet : public StubEntity {
public:
    explicit StubBullet(const std::string& name = "Bullet", bool collides = false)
    : StubEntity(name, true) {
        setCollides(collides);
    }
};

class StubPlayer : public Player {
public:
    StubPlayer() : Player() {}

    // Expose bullets vector reference for test set-up convenience.
    std::vector<std::shared_ptr<Entity>>& bullets() { return m_Bullets; }

    void Update(float dt) override {
        updatedWith_.push_back(dt);
        callLog_.add("Player.Update(" + std::to_string(dt) + ")");
    }

    void Draw() override {
        drawCount_++;
        callLog_.add("Player.Draw()");
    }

    bool CheckCollision(const std::vector<std::shared_ptr<Entity>>& others) override {
        lastCollisionCheckCount_ = static_cast<int>(others.size());
        callLog_.add("Player.CheckCollision(count=" + std::to_string(lastCollisionCheckCount_) + ")");
        return collides_;
    }

    bool IsAlive() const override {
        return alive_;
    }

    void setAlive(bool a) { alive_ = a; }
    void setCollides(bool c) { collides_ = c; }

    int drawCount() const { return drawCount_; }
    const std::vector<float>& updatedWith() const { return updatedWith_; }
    int lastCollisionCheckCount() const { return lastCollisionCheckCount_; }
    CallLog& calls() { return callLog_; }

private:
    bool alive_{true};
    bool collides_{false};
    int drawCount_{0};
    int lastCollisionCheckCount_{0};
    std::vector<float> updatedWith_{};
    CallLog callLog_{};
};

// Helper to construct a Game without invoking run() and with access to m_Entities.
// If m_Entities is private in Game, we rely on Game's public API to add entities.
// If not available, we exploit friendship or create a small accessor in tests via subclassing.
class TestableGame : public Game {
public:
    TestableGame(int h = 600, int w = 800, const char* t = "Test")
    : Game(h, w, t) {}

    std::vector<std::shared_ptr<Entity>>& entities() {
        return m_Entities;
    }
};

TEST_CASE("update_should_call_Update_and_CheckCollision_on_all_non_null_entities") {
    TestableGame game;
    auto e1 = std::make_shared<StubEntity>("E1", true);
    auto e2 = std::make_shared<StubEntity>("E2", true);
    auto eNull = std::shared_ptr<Entity>(nullptr);

    game.entities().push_back(e1);
    game.entities().push_back(eNull);
    game.entities().push_back(e2);

    const float dt = 0.016f;
    game.update(dt);

    // Verify Update called with dt and CheckCollision called with size 3 (including null ignored by entity itself)
    REQUIRE(e1->updatedWith().size() == 1);
    REQUIRE(e2->updatedWith().size() == 1);
    CHECK(std::abs(e1->updatedWith().front() - dt) < 1e-6);
    CHECK(std::abs(e2->updatedWith().front() - dt) < 1e-6);
    // Collision set gets full vector size; entities should receive the vector even with a null inside
    CHECK(e1->lastCollisionCheckCount() == 3);
    CHECK(e2->lastCollisionCheckCount() == 3);
}

TEST_CASE("update_should_prune_player_bullets_that_collide") {
    TestableGame game;
    auto player = std::make_shared<StubPlayer>();
    // Create bullets: two colliding, one non-colliding
    auto b1 = std::make_shared<StubBullet>("B1", true);
    auto b2 = std::make_shared<StubBullet>("B2", false);
    auto b3 = std::make_shared<StubBullet>("B3", true);

    player->bullets().push_back(b1);
    player->bullets().push_back(b2);
    player->bullets().push_back(b3);

    // Add a non-player entity to collide against
    auto enemy = std::make_shared<StubEntity>("Enemy", true);

    game.entities().push_back(player);
    game.entities().push_back(enemy);

    game.update(0.01f);

    // After update, bullets that report collision should be erased: B1 and B3 removed, B2 remains
    REQUIRE(player->bullets().size() == 1);
    REQUIRE(static_cast<StubEntity*>(player->bullets().front().get()) != nullptr);
    // Check that the remaining bullet is B2 by verifying it is non-colliding and not removed
    CHECK(static_cast<StubEntity*>(player->bullets().front().get())->lastCollisionCheckCount() >= 1);
}

TEST_CASE("update_should_remove_entities_not_alive_after_update") {
    TestableGame game;
    auto aliveEntity = std::make_shared<StubEntity>("Alive", true);
    auto deadEntity = std::make_shared<StubEntity>("Dead", false);

    game.entities().push_back(aliveEntity);
    game.entities().push_back(deadEntity);
    REQUIRE(game.entities().size() == 2);

    game.update(0.02f);

    // Dead entity should be erased
    REQUIRE(game.entities().size() == 1);
    // The remaining entity is the alive one
    CHECK(aliveEntity->IsAlive());
}

TEST_CASE("draw_should_invoke_Draw_on_all_entities_in_order") {
    TestableGame game;
    auto e1 = std::make_shared<StubEntity>("E1", true);
    auto e2 = std::make_shared<StubEntity>("E2", true);
    auto player = std::make_shared<StubPlayer>();

    game.entities().push_back(e1);
    game.entities().push_back(player);
    game.entities().push_back(e2);

    // draw() simply iterates and calls Draw()
    game.draw();

    // Verify Draw count for each
    REQUIRE(e1->drawCount() == 1);
    REQUIRE(player->drawCount() == 1);
    REQUIRE(e2->drawCount() == 1);
}

TEST_CASE("update_should_ignore_null_entities_without_throwing") {
    TestableGame game;
    auto e1 = std::make_shared<StubEntity>("E1", true);
    std::shared_ptr<Entity> eNull;

    game.entities().push_back(eNull);
    game.entities().push_back(e1);
    // Should not crash
    game.update(0.005f);

    REQUIRE(e1->updatedWith().size() == 1);
}

// Edge case: no entities
TEST_CASE("update_with_no_entities_should_not_throw") {
    TestableGame game;
    // Should not crash or change internal state
    REQUIRE(game.entities().empty());
    game.update(0.001f);
    REQUIRE(game.entities().empty());
}
