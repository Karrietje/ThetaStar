#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "TheFlock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	std::vector<SteeringAgent*> pNeighbors = m_pFlock->GetNeighbors();

	for (int i{}; i < m_pFlock->GetNrOfNeighbors(); i++)
	{
		Elite::Vector2 velocity{ pAgent->GetPosition() - pNeighbors[i]->GetPosition() };
		float distance = velocity.Normalize();
		if (distance < FLT_EPSILON)
		{
			velocity /= distance;
			steering.LinearVelocity += velocity;
		}
	}

	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	return steering;
}

//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	m_Target = m_pFlock->GetNeighborhoodCenter();
	return Seek::CalculateSteering(deltaT, pAgent);
}

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput Alignment::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	steering.LinearVelocity = m_pFlock->GetNeighborhoodAverageVelocity();

	return steering;
}
