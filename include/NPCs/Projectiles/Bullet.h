#include <vector>
#include <memory>
#include "NPCs/Entity.h"
#include "NPCs/Player.h"

class Bullet : public Entity
{
public:
	Bullet(Entity* parent, float velocity, bool positiveXdirection);
private:
	Entity* m_Parent;
	bool m_positiveXdirection;
	void OnUpdate(float dt) override;
	bool CheckCollision(std::shared_ptr<Entity> other) override;
	bool CheckCollision(std::vector<std::shared_ptr<Entity>> others) override;
};