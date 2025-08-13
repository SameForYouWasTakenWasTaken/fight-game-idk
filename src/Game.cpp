#include "Game.h"
#include "NPCs/Player.h"

Game::Game(int height, int width, const char* title)
	: m_Height(height), m_Width(width), m_Title(title)
{}

void Game::run()
{
	InitWindow(m_Width, m_Height, m_Title);
	SetTraceLogLevel(TraceLogLevel::LOG_ERROR);

	Player player;
	Entity enemy("resources/Player/idle.png", "Enemy", 100.f);
	enemy.GetPosition() = { 500, 0 };
	SetTargetFPS(1000);
	while (!WindowShouldClose())
	{
		float dt = GetFrameTime();
		// Update
		player.Update(dt);
		enemy.Update(dt);
		// Draw
		BeginDrawing();
		ClearBackground(RED);
		player.Draw();
		enemy.Draw();

		for (const auto& bullet : player.m_Bullets)
			bullet->Draw();

		EndDrawing();
		
	}
	
	CloseWindow();
}