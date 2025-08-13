#include "NPCs/Entity.h"

class Bullet : public Entity
{
public:
	Bullet(float velocity, bool positiveXdirection);

private:
	bool m_positiveXdirection;
	void OnUpdate(float dt) override;

	bool CheckCollision(Entity& other) override;
};