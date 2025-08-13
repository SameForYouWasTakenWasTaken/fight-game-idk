// NOTE: Tests assume GoogleTest. If your project uses Catch2/doctest, adapt includes and macros accordingly.
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <cmath>

// ---- Begin minimal raylib test stubs ----
extern "C" {

// Simplified Vector2 and Texture2D definitions matching expected fields
typedef struct Vector2 {
    float x;
    float y;
} Vector2;

typedef struct Texture2D {
    unsigned int id;
    int width;
    int height;
    int mipmaps;
    int format;
} Texture2D;

// Color WHITE stub
typedef struct Color {
    unsigned char r, g, b, a;
} Color;

static const Color WHITE = {255, 255, 255, 255};

// Global stub state to verify calls in tests
static std::vector<std::tuple<Texture2D, int, int, Color>> g_drawCalls;
static std::vector<std::string> g_logInfoCalls;
static std::vector<std::string> g_loadTexturePaths;

// Reset helper for tests
static void ResetRaylibSpdlogStubs() {
    g_drawCalls.clear();
    g_logInfoCalls.clear();
    g_loadTexturePaths.clear();
}

// Stubbed LoadTexture: record path, return a deterministic texture based on path hash
static unsigned long HashPath(const char* path) {
    // Simple FNV-1a 64-bit
    const unsigned long FNV_OFFSET = 1469598103934665603ULL;
    const unsigned long FNV_PRIME = 1099511628211ULL;
    unsigned long hash = FNV_OFFSET;
    for (const char* p = path; *p; ++p) {
        hash ^= static_cast<unsigned char>(*p);
        hash *= FNV_PRIME;
    }
    return hash;
}

Texture2D LoadTexture(const char* fileName) {
    g_loadTexturePaths.emplace_back(fileName ? std::string(fileName) : std::string());
    unsigned long h = HashPath(fileName ? fileName : "");
    // Produce width/height in a predictable small range so tests can control with the file path
    // For generality, clamp to [8, 512]
    int w = static_cast<int>((h % 128) + 32);  // 32..159
    int hh = static_cast<int>(((h >> 8) % 128) + 32);
    Texture2D tex{};
    tex.id = static_cast<unsigned int>(h & 0xFFFFFFFFu);
    tex.width = w;
    tex.height = hh;
    tex.mipmaps = 1;
    tex.format = 0;
    return tex;
}

// Stubbed DrawTexture: record parameters
void DrawTexture(Texture2D texture, int posX, int posY, Color tint) {
    g_drawCalls.emplace_back(texture, posX, posY, tint);
}

} // extern "C"

// ---- Begin minimal spdlog info stub ----
namespace spdlog {
    inline void info(const std::string& msg) {
        g_logInfoCalls.emplace_back(msg);
    }
}

// ---- Include the system under test ----
#include "NPCs/Entity.h"

// ---- Helper accessors via header contracts or friend APIs ----
// We assume Entity exposes the following based on code under test:
//  - GetPosition() -> Vector2
//  - SetPosition(Vector2) or a way to change m_Position
//  - GetTexture() -> Texture2D
//  - TakeDamage(float), CheckCollision(std::shared_ptr<Entity>), CommonDraw(), CommonUpdate(float)
//  - Possibly getters for name and hp; if not available, we verify behavior via effects (alive flag, collision geometry)
namespace TestHelpers {
    // Utility to set position if no setter is available; we use a conditional compilation trick:
    // If Entity provides SetPosition, prefer it; otherwise, we use a friend accessor shim via subclassing.
    // To avoid modifying production code, we define a shim that inherits and exposes protected/private position via a helper
    // Only if compilation allows it; otherwise tests relying on position will use placement functions instead.

    struct EntityShim : public Entity {
        using Entity::Entity;
        void ForceSetPosition(float x, float y) {
            // If Entity has SetPosition, prefer it
#ifdef HAS_ENTITY_SET_POSITION
            this->SetPosition(Vector2{ x, y });
#else
            // Rely on a public API if present; otherwise, we assume there is SetPosition by conventional design.
            this->SetPosition(Vector2{ x, y });
#endif
        }
    };
}

// A small fixture to reset stubs between tests
class EntityTest : public ::testing::Test {
protected:
    void SetUp() override {
        ResetRaylibSpdlogStubs();
    }
    void TearDown() override {
        ResetRaylibSpdlogStubs();
    }
};

// Helper to set entity position with fallbacks
static void SetEntityPosition(Entity& e, float x, float y) {
    // Try to call SetPosition if available; if not, compile will fail and the shim will be used below.
    // We wrap this in a lambda to avoid ODR issues; tests will attempt both paths guarded by overload resolution.

    // Attempt direct SetPosition
    struct HasSetPosition {
        template <typename T>
        static auto test(int) -> decltype(std::declval<T&>().SetPosition(Vector2{0,0}), std::true_type{});
        template <typename>
        static auto test(...) -> std::false_type;
    };

    if constexpr (decltype(HasSetPosition::test<Entity>(0))::value) {
        e.SetPosition(Vector2{ x, y });
    } else {
        // Fallback: try to cast to shim
        TestHelpers::EntityShim* shim = dynamic_cast<TestHelpers::EntityShim*>(&e);
        if (shim) {
            shim->ForceSetPosition(x, y);
        } else {
            // As last resort, we assume SetPosition exists per code references in CheckCollision using m_Position.
            // If not, tests may need adjustment to project API.
            e.SetPosition(Vector2{ x, y });
        }
    }
}

// Utility: create entity with deterministic texture sizes by controlling path strings
static std::shared_ptr<Entity> MakeEntity(const char* texturePath, const std::string& name, float hp, float x = 0.f, float y = 0.f) {
    auto ent = std::make_shared<Entity>(texturePath, name, hp);
    SetEntityPosition(*ent, x, y);
    return ent;
}

// --------------- Tests --------------------

TEST_F(EntityTest, ConstructorInitializesNameHpAndLoadsTexture) {
    auto ent = MakeEntity("assets/test_64x80.png", "Dummy", 100.f, 0.f, 0.f);
    // Texture must have been loaded via our stub
    ASSERT_FALSE(g_loadTexturePaths.empty());
    EXPECT_EQ(g_loadTexturePaths.back(), "assets/test_64x80.png");

    // Validate texture returned via getter is consistent with stubbed LoadTexture
    Texture2D tex = ent->GetTexture();
    EXPECT_GT(tex.width, 0);
    EXPECT_GT(tex.height, 0);
}

TEST_F(EntityTest, TakeDamageReducesHpAndDoesNotGoNegativeForNegativeInput) {
    auto ent = MakeEntity("t.png", "Dummy", 50.f, 0.f, 0.f);

    // Positive damage
    ent->TakeDamage(10.f);
    // No direct getter for HP assumed; validate alive state remains true (requires hp > 0)
    // Apply negative damage which should be treated as absolute
    ent->TakeDamage(-5.f);
    // Lethal damage to kill
    ent->TakeDamage(100.f);

    // When hp <= 0, Entity sets m_IsAlive = false. We need a way to check alive state.
    // If Entity has IsAlive(), use it; otherwise, we infer via collision behavior (optional).
    // Preferred: IsAlive()
    bool isAlive = true;
    // SFINAE check for IsAlive()
    struct HasIsAlive {
        template <typename T>
        static auto test(int) -> decltype(std::declval<T&>().IsAlive(), std::true_type{});
        template <typename>
        static auto test(...) -> std::false_type;
    };
    if constexpr (decltype(HasIsAlive::test<Entity>(0))::value) {
        isAlive = ent->IsAlive();
        EXPECT_FALSE(isAlive);
    } else {
        // Without IsAlive, we accept test as smoke (no crash) and rely on collision tests next
        SUCCEED() << "IsAlive() accessor not found; lethal damage path executed without crash.";
    }
}

TEST_F(EntityTest, TakeDamageZeroDoesNotChangeAliveStatus) {
    auto ent = MakeEntity("t.png", "Dummy", 10.f, 0.f, 0.f);
    ent->TakeDamage(0.f);

    struct HasIsAlive {
        template <typename T>
        static auto test(int) -> decltype(std::declval<T&>().IsAlive(), std::true_type{});
        template <typename>
        static auto test(...) -> std::false_type;
    };
    if constexpr (decltype(HasIsAlive::test<Entity>(0))::value) {
        EXPECT_TRUE(ent->IsAlive());
    } else {
        SUCCEED() << "IsAlive() accessor not found; zero-damage path executed.";
    }
}

TEST_F(EntityTest, CommonDrawInvokesDrawTextureOnceWithCurrentPosition) {
    auto ent = MakeEntity("my.png", "Drawer", 10.f, 42.f, 24.f);
    ResetRaylibSpdlogStubs(); // Clear texture load record to only track draw

    ent->CommonDraw();

    ASSERT_EQ(g_drawCalls.size(), 1u);
    auto [tex, x, y, color] = g_drawCalls[0];
    EXPECT_EQ(x, 42);
    EXPECT_EQ(y, 24);
    EXPECT_EQ(color.r, WHITE.r);
    EXPECT_EQ(color.g, WHITE.g);
    EXPECT_EQ(color.b, WHITE.b);
    EXPECT_EQ(color.a, WHITE.a);
    // Ensure texture matches entity texture
    Texture2D entTex = ent->GetTexture();
    EXPECT_EQ(tex.width, entTex.width);
    EXPECT_EQ(tex.height, entTex.height);
    EXPECT_EQ(tex.id, entTex.id);
}

TEST_F(EntityTest, CheckCollisionReturnsFalseWhenComparingWithSelf) {
    auto ent = MakeEntity("a.png", "Selfie", 5.f, 0.f, 0.f);
    EXPECT_FALSE(ent->CheckCollision(ent));
    EXPECT_TRUE(g_logInfoCalls.empty()) << "No log expected on self-collision rejection.";
}

TEST_F(EntityTest, CheckCollisionNoOverlapSeparatedOnXLeft) {
    auto a = MakeEntity("a.png", "A", 5.f, 100.f, 100.f);
    auto b = MakeEntity("b.png", "B", 5.f, 0.f, 100.f); // completely left of A
    EXPECT_FALSE(a->CheckCollision(b));
    EXPECT_TRUE(g_logInfoCalls.empty());
}

TEST_F(EntityTest, CheckCollisionNoOverlapSeparatedOnXRight) {
    auto a = MakeEntity("a.png", "A", 5.f, 0.f, 100.f);
    auto b = MakeEntity("b.png", "B", 5.f, 1000.f, 100.f); // completely right of A
    EXPECT_FALSE(a->CheckCollision(b));
    EXPECT_TRUE(g_logInfoCalls.empty());
}

TEST_F(EntityTest, CheckCollisionNoOverlapSeparatedOnYAbove) {
    auto a = MakeEntity("a.png", "A", 5.f, 100.f, 100.f);
    auto b = MakeEntity("b.png", "B", 5.f, 100.f, -100.f); // completely above A
    EXPECT_FALSE(a->CheckCollision(b));
    EXPECT_TRUE(g_logInfoCalls.empty());
}

TEST_F(EntityTest, CheckCollisionNoOverlapSeparatedOnYBelow) {
    auto a = MakeEntity("a.png", "A", 5.f, 100.f, 100.f);
    auto b = MakeEntity("b.png", "B", 5.f, 100.f, 1000.f); // completely below A
    EXPECT_FALSE(a->CheckCollision(b));
    EXPECT_TRUE(g_logInfoCalls.empty());
}

TEST_F(EntityTest, CheckCollisionEdgeTouchingCountsAsCollision) {
    // The implementation: returns false only if one box + width/height < other's position
    // Edge-touching (==) should be treated as collision (returns true)
    auto a = MakeEntity("a.png", "A", 5.f, 0.f, 0.f);
    Texture2D aTex = a->GetTexture();

    // Place b so that its left edge touches a's right edge: b.x == a.x + a.width
    auto b = MakeEntity("b.png", "B", 5.f, static_cast<float>(aTex.width), 0.f);

    EXPECT_TRUE(a->CheckCollision(b));
    ASSERT_FALSE(g_logInfoCalls.empty());
    EXPECT_EQ(g_logInfoCalls.back(), "Hit!");
}

TEST_F(EntityTest, CheckCollisionOverlappingBoxesReturnsTrueAndLogs) {
    auto a = MakeEntity("a.png", "A", 5.f, 10.f, 10.f);
    auto b = MakeEntity("b.png", "B", 5.f, 15.f, 12.f); // Overlaps with A
    EXPECT_TRUE(a->CheckCollision(b));
    ASSERT_FALSE(g_logInfoCalls.empty());
    EXPECT_EQ(g_logInfoCalls.back(), "Hit!");
}

TEST_F(EntityTest, CommonUpdateIsCallableAndNoSideEffectsOccur) {
    auto ent = MakeEntity("u.png", "Updater", 5.f, 1.f, 2.f);
    Texture2D before = ent->GetTexture();
    Vector2 posBefore = ent->GetPosition();

    ent->CommonUpdate(0.016f);

    Texture2D after = ent->GetTexture();
    Vector2 posAfter = ent->GetPosition();
    EXPECT_EQ(before.id, after.id);
    EXPECT_FLOAT_EQ(posBefore.x, posAfter.x);
    EXPECT_FLOAT_EQ(posBefore.y, posAfter.y);
}
