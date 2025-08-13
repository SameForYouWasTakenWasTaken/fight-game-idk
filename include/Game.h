#pragma once
#include <vector>
#include <memory>
#include "raylib.h"
#include "spdlog/spdlog.h"
#include "NPCs/Player.h"

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