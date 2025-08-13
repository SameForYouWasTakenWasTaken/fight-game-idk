#pragma once

#include <vector>

#include "NPCs/Entity.h"

#define IDLE "resources/Player/idle.png"
#define LEFT "resources/Player/left.png"
#define RIGHT "resources/Player/right.png"
#define UP "resources/Player/up.png"

class Player : public Entity
{
public:
	std::vector<Entity*> m_Bullets;
	Player();
private:
	void OnUpdate(float dt) override;
	void OnDraw() override;
};