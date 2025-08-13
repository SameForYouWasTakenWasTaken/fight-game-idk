#include "NPCs/Player.h"
#include "NPCs/Projectiles/Bullet.h"
static bool aiming_left = false;

/**
 * @brief Constructs a Player with the default visual and movement settings.
 *
 * Initializes a Player entity using the idle texture ("resources/Player/idle.png"),
 * sets its name to "Player", and configures its movement speed to 300.f.
 */
Player::Player()
	: Entity("resources/Player/idle.png", "Player", 300.f)
{ }

/**
 * @brief Renders all bullets owned by the player.
 *
 * Calls each bullet's Draw() method so the player's active projectiles are rendered
 * to the current render target.
 */
void Player::OnDraw()
{
	for (auto bullet : m_Bullets)
		bullet->Draw();
}

/**
 * @brief Process input, update player movement, handle firing, and manage bullets for this frame.
 *
 * This updates the player's position and texture based on keyboard input (W/A/S/D),
 * sets the shooting direction flag, spawns bullets when firing input is received,
 * removes out-of-bounds bullets, and updates all active bullets.
 *
 * Movement:
 * - A/D move left/right and set the shooting direction (aiming_left).
 * - W/S take priority over A/D and force the shooting direction to right.
 *
 * Firing:
 * - Pressing F or the left mouse button creates a new Bullet owned by this Player,
 *   positioned at the center of the player's current texture area and added to m_Bullets.
 *
 * Bullet lifecycle:
 * - Bullets whose x position is > 5000 or < -5000 are removed from m_Bullets and deleted.
 * - Remaining bullets are updated each frame via bullet->Update(dt).
 *
 * Side effects: modifies m_Position, m_Texture, aiming_left, allocates/deletes Bullet instances,
 * and mutates m_Bullets.
 *
 * @param dt Frame delta time in seconds.
 */
void Player::OnUpdate(float dt)
{

	if (IsKeyDown(KEY_A))
	{
		aiming_left = true; // Shoot left
		m_Texture = LoadTexture(LEFT);
		m_Position.x -= m_Velocity * dt;
	}

	if (IsKeyDown(KEY_D))
	{
		aiming_left = false; // Shoot right
		m_Texture = LoadTexture(RIGHT);
		m_Position.x += m_Velocity * dt;
	}
	// Priorities W and S keybinds over A and D
	if (IsKeyDown(KEY_W))
	{
		aiming_left = false; // Force to shoot right by default if not holding A or D
		m_Texture = LoadTexture(UP);
		m_Position.y -= m_Velocity * dt;
	}

	if (IsKeyDown(KEY_S))
	{
		aiming_left = false; // Force to shoot right by default if not holding A or D
		m_Texture = LoadTexture(IDLE);
		m_Position.y += m_Velocity * dt;
	}

	if (IsKeyPressed(KEY_F) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	{
		Bullet* bullet = new Bullet(this, 1000.f, aiming_left);
		// Set the bullet position in the middle of the player position
		bullet->GetPosition() = 
		{
			static_cast<float>(m_Texture.width / 2) + m_Position.x,
			static_cast<float>(m_Texture.height / 2) + m_Position.y
		};
		m_Bullets.push_back(bullet);
	}

	for (auto bullet : m_Bullets)
	{
		const float pos = bullet->GetPosition().x;
		if (pos > 5000 || pos < -5000)
		{
			// Remove the bullet if its position is out of the screen
			m_Bullets.erase(std::remove(m_Bullets.begin(), m_Bullets.end(), bullet), m_Bullets.end());
			delete bullet;
		}
	}

	for (auto bullet : m_Bullets)
		bullet->Update(dt);

}
