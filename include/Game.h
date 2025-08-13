#pragma once
#include <vector>
#include <memory>
#include "raylib.h"
#include "spdlog/spdlog.h"
#include "NPCs/Player.h"

/**
 * Construct a Game with the given window size and title.
 * @param width Window width in pixels.
 * @param height Window height in pixels.
 * @param title Null-terminated window title string.
 */
 
/**
 * Enter and run the main game loop until the window is closed.
 * This continuously processes input, updates game state, and renders frames.
 */
 
/**
 * Advance the game simulation by the specified delta time.
 * @param dt Time elapsed since the last update call, in seconds.
 */
 
/**
 * Render the current game state to the window.
 */
class Game {
public:
	Game(int width, int height, const char* title);
	void run();
	void update(float dt);
	void draw();
private:
	std::vector<std::shared_ptr<Entity>> m_Entities;
	int m_Width;
	int m_Height;
	const char* m_Title;
};