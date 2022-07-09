#pragma once

#include <Aurora.h>

#include <vector>
#include <glm/glm.hpp>

#define PANELWIDTH 50 // So technically this is a 10x10 grid
#define PANELHEIGHT 50
#define UNITSIZE 5
#define GAMEUNITS (PANELWIDTH * PANELHEIGHT) / UNITSIZE

struct BodyPart
{
	glm::vec2 position;
};

class Snake
{
public:
	void IncParts();
	void moveUp(TimeStep ts);
	void moveDown(TimeStep ts);
	void moveLeft(TimeStep ts);
	void moveRight(TimeStep ts);
	bool checkCollision(const glm::vec2& appPos);

	const std::vector<BodyPart>& getBodyParts() const { return m_Arr; }

private:
	std::vector<BodyPart> m_Arr{ { { 5.0f, 5.0f } },
								 { { 5.0f, 4.0f } } };

	//float m_Velocity = 10.0f;
	float m_Velocity = 0.05f;
	
};