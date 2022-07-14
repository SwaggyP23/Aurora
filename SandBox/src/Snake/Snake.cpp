#include "Snake.h"

void Snake::IncParts()
{
	BodyPart newPart;

	if (m_Arr[m_Arr.size() - 1].position.x == m_Arr[m_Arr.size() - 2].position.x)
		newPart = { { m_Arr[m_Arr.size() - 1].position.x - 1.0f,  m_Arr[m_Arr.size() - 1].position.y } };
	else if(m_Arr[m_Arr.size() - 1].position.y == m_Arr[m_Arr.size() - 2].position.y)
		newPart = { { m_Arr[m_Arr.size() - 1].position.x,  m_Arr[m_Arr.size() - 1].position.y - 1.0f } };

	m_Arr.emplace_back(newPart);
	AR_WARN("Snake grew to {0}", m_Arr.size());
}

bool Snake::checkCollision(const glm::vec2& appPos)
{
	if ((m_Arr[0].position.x >= appPos.x - 0.1 && m_Arr[0].position.x <= appPos.x + 0.9)
		&& (m_Arr[0].position.y >= appPos.y - 0.1 && m_Arr[0].position.y <= appPos.y + 0.9))
	{
		IncParts();

		return true;
	}

	return false;
}

void Snake::moveUp(Aurora::TimeStep ts)
{
	for (size_t i = m_Arr.size() - 1; i > 0; i--)
	{
		m_Arr[i].position.x = m_Arr[i - 1].position.x;
		m_Arr[i].position.y = m_Arr[i - 1].position.y;
	}

	m_Arr[0].position.y += m_Velocity * UNITSIZE;// *ts;

	if (m_Arr[0].position.y >= PANELHEIGHT / 2 - 0.5f)
		m_Arr[0].position.y = -PANELHEIGHT / 2 + 0.5f;
}

void Snake::moveDown(Aurora::TimeStep ts)
{
	for (size_t i = m_Arr.size() - 1; i > 0; i--)
	{
		m_Arr[i].position.x = m_Arr[i - 1].position.x;
		m_Arr[i].position.y = m_Arr[i - 1].position.y;
	}

	m_Arr[0].position.y -= m_Velocity * UNITSIZE;// *ts;

	if (m_Arr[0].position.y <= -PANELHEIGHT / 2 + 0.5f)
		m_Arr[0].position.y = PANELHEIGHT / 2 - 0.5f;
}

void Snake::moveLeft(Aurora::TimeStep ts)
{
	for (size_t i = m_Arr.size() - 1; i > 0; i--)
	{
		m_Arr[i].position.x = m_Arr[i - 1].position.x;
		m_Arr[i].position.y = m_Arr[i - 1].position.y;
	}

	m_Arr[0].position.x -= m_Velocity * UNITSIZE;// *ts;

	if (m_Arr[0].position.x <= -PANELWIDTH/ 2 + 0.5f)
		m_Arr[0].position.x = PANELWIDTH / 2 - 0.5f;
}

void Snake::moveRight(Aurora::TimeStep ts)
{
	for (size_t i = m_Arr.size() - 1; i > 0; i--)
	{
		m_Arr[i].position.x = m_Arr[i - 1].position.x;
		m_Arr[i].position.y = m_Arr[i - 1].position.y;
	}

	m_Arr[0].position.x += m_Velocity * UNITSIZE;// *ts;

	if (m_Arr[0].position.x >= PANELWIDTH / 2 - 0.5f)
		m_Arr[0].position.x = -PANELWIDTH / 2 + 0.5;
}