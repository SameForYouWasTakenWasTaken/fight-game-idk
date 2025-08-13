#include <typeinfo>
#include "Game.h"
#include "NPCs/Player.h"

Game::Game(int height, int width, const char* title)
	: m_Height(height), m_Width(width), m_Title(title)
{}

void Game::run()
{
	InitWindow(m_Width, m_Height, m_Title);
	SetTraceLogLevel(TraceLogLevel::LOG_ERROR);

	std::shared_ptr<Player> player = std::make_shared<Player>();
	std::shared_ptr<Entity> enemy = std::make_shared<Entity>("resources/Player/idle.png", "Enemy", 100.f);

	m_Entities.push_back(player);
	m_Entities.push_back(enemy);
	enemy->GetPosition() = { 500, 0 };
	SetTargetFPS(144);
	while (!WindowShouldClose())
	{
		float dt = GetFrameTime();
		// Update
		update(dt);
		// Draw stuff
		BeginDrawing();
		ClearBackground(RED);

		draw(); // Draw all essentials
		
		EndDrawing();
		
	}
	
	CloseWindow();
}

void Game::update(float dt)
{
	for (const auto& entity : m_Entities)
	{
		if (!entity) continue;

		entity->Update(dt);
		entity->CheckCollision(m_Entities);

		if (auto player = dynamic_cast<Player*>(entity.get()))
		{
			player->m_Bullets.erase(
				std::remove_if(player->m_Bullets.begin(), player->m_Bullets.end(),
					[&](auto& bullet) {
						return bullet->CheckCollision(m_Entities);
					}),
				player->m_Bullets.end()
			);
		}
	}

	m_Entities.erase(
		std::remove_if(m_Entities.begin(), m_Entities.end(),
			[](const std::shared_ptr<Entity>& e) {
				return !e->IsAlive();
			}),
		m_Entities.end()
	);
}


void Game::draw()
{
	for (const auto& entity : m_Entities)
	{
		entity->Draw();
	}
}