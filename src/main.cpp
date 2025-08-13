#include "Game.h"

int main()
{
	Game* game = new Game(1080, 1920, "Game");
	game->run();

	delete game;
	return 0;
}