#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\App_Steering\SteeringAgent.h"

// --- Cell ---
// ------------
Cell::Cell(float left, float bottom, float width, float height)
{
	boundingBox.bottomLeft = { left, bottom };
	boundingBox.width = width;
	boundingBox.height = height;
}

std::vector<Elite::Vector2> Cell::GetRectPoints() const
{
	auto left = boundingBox.bottomLeft.x;
	auto bottom = boundingBox.bottomLeft.y;
	auto width = boundingBox.width;
	auto height = boundingBox.height;

	std::vector<Elite::Vector2> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(float width, float height, int rows, int cols, int maxEntities)
	: m_SpaceWidth(width)
	, m_SpaceHeight(height)
	, m_NrOfRows(rows)
	, m_NrOfCols(cols)
	, m_pNeighbors(maxEntities)
	, m_NrOfNeighbours(0)
{
	m_CellWidth = m_SpaceWidth / m_NrOfRows;
	m_CellHeight = m_SpaceWidth / m_NrOfCols; 

	for (int cols = 0; cols < m_NrOfCols; cols++)
	{
		for (int rows = 0; rows < m_NrOfRows; rows++)
		{
			m_Cells.push_back(Cell{ m_CellWidth * rows, m_CellHeight * cols, m_CellWidth, m_CellHeight }); 
		}
	}
}

void CellSpace::AddAgent(SteeringAgent* agent)
{
	int cellIndex = PositionToIndex(agent->GetPosition());
	m_Cells[cellIndex].agents.push_back(agent);
}

void CellSpace::UpdateAgentCell(SteeringAgent* agent, const Elite::Vector2& oldPos)
{
	int oldIndex = PositionToIndex(oldPos);
	int newIndex = PositionToIndex(agent->GetPosition());

	if (oldIndex != newIndex)
	{
		m_Cells[newIndex].agents.push_back(agent);
		m_Cells[oldIndex].agents.remove(agent);
	}
}

void CellSpace::RegisterNeighbors(SteeringAgent* pAgent, float queryRadius)
{
	/*m_NrOfNeighbors = 0;
	int cornersIdx[3]{};
	cornersIdx[0] = PositionToIndex(Elite::Vector2(pAgent->GetPosition().x - queryRadius, pAgent->GetPosition().y - queryRadius));
	cornersIdx[1] = PositionToIndex(Elite::Vector2(pAgent->GetPosition().x - queryRadius, pAgent->GetPosition().y + queryRadius));
	cornersIdx[2] = PositionToIndex(Elite::Vector2(pAgent->GetPosition().x + queryRadius, pAgent->GetPosition().y - queryRadius));
	for (int row{ cornersIdx[0] / m_NrOfRows }; row < (cornersIdx[1] / m_NrOfRows); row++)
	{
		for (int col{ cornersIdx[0] % m_NrOfCols }; col < (cornersIdx[2] % m_NrOfCols); col++)
		{
			int index{ row * m_NrOfCols + col };
			for (SteeringAgent* pAgents : m_Cells[index].agents)
			{
				float distance{ Elite::Distance(pAgents->GetPosition(), pAgent->GetPosition()) };
				if (pAgents != nullptr && distance < queryRadius)
				{
					m_pNeighbors[m_NrOfNeighbors] = pAgents;
					m_NrOfNeighbors++;
				}
			}
		}
	}*/
	m_NrOfNeighbours = 0;
	for (float rows{ pAgent->GetPosition().y - queryRadius }; rows < int(pAgent->GetPosition().y + queryRadius); rows += m_CellHeight)
	{
		for (float cols{ pAgent->GetPosition().x - queryRadius }; cols < int(pAgent->GetPosition().x + queryRadius); cols += m_CellWidth)
		{
			int index = PositionToIndex(Elite::Vector2{ cols, rows });
			for (SteeringAgent* agent : m_Cells[index].agents)
			{
				if (agent && Elite::DistanceSquared(agent->GetPosition(), pAgent->GetPosition()) <= queryRadius * queryRadius)
				{
					m_pNeighbors[m_NrOfNeighbours] = agent;
					m_NrOfNeighbours++;
				}
			}
		}
	}
}

void CellSpace::RenderCells() const
{
	float height{ 1.f };
	for (size_t i{}; i < m_Cells.size(); i++)
	{
		std::vector<Elite::Vector2> points = m_Cells[i].GetRectPoints();
		DEBUGRENDERER2D->DrawSegment(points[0], points[1], Elite::Color(1, 0, 0, 1));
		DEBUGRENDERER2D->DrawSegment(points[1], points[2], Elite::Color(1, 0, 0, 1));
		DEBUGRENDERER2D->DrawSegment(points[2], points[3], Elite::Color(1, 0, 0, 1));
		DEBUGRENDERER2D->DrawSegment(points[3], points[0], Elite::Color(1, 0, 0, 1));
		DEBUGRENDERER2D->DrawString(m_Cells[i].GetRectPoints()[1], std::to_string(m_Cells[i].agents.size()).data());
		height += 3;
	}
}

int CellSpace::PositionToIndex(const Elite::Vector2 pos) const
{

	int row = int(pos.y / m_CellHeight);
	int col = int(pos.x / m_CellWidth);
	if (row < 0 || row > m_NrOfRows )
	{
		row = 0; 
	}
	
	if (col < 0 || col > m_NrOfCols)
	{
		col = 0;
	}
	return Elite::Clamp(col + m_NrOfCols * row, 0, int(m_Cells.size() - 1));
}