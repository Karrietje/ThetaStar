#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "TheFlock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
    int amountOfNeighbours = m_pFlock->GetNrOfNeighbors();
    Elite::Vector2 Vector{};
    SteeringOutput steering{};
    const vector<SteeringAgent*>& neighbours = m_pFlock->GetNeighbors();

    for (int i{ 0 }; i < amountOfNeighbours; i++)
    {
        Vector = pAgent->GetPosition() - neighbours[i]->GetPosition();
        float distance = Vector.Magnitude();
        if (distance > 0)
        {
            steering.LinearVelocity += (Vector.GetNormalized() / distance);
        }
    }

    steering.LinearVelocity = steering.LinearVelocity.GetNormalized() * pAgent->GetMaxLinearSpeed();
    return steering;
}

//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	m_Target = m_pFlock->GetAverageNeighborPos();
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
