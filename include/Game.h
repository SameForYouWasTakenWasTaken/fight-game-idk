#pragma once
#include "raylib.h"
#include "spdlog/spdlog.h"

class Game {
public:
	Game(int width, int height, const char* title);
	void run();
private:
	int m_Width;
	int m_Height;
	const char* m_Title;
};