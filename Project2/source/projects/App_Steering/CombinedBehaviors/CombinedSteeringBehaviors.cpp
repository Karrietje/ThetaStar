#include "stdafx.h"
#include "CombinedSteeringBehaviors.h"
#include <algorithm>

BlendedSteering::BlendedSteering(vector<WeightedBehavior> weightedBehaviors)
	:m_WeightedBehaviors(weightedBehaviors)
{
};

//****************
//BLENDED STEERING
SteeringOutput BlendedSteering::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	auto totalWeight = 0.f;

	for (auto pair : m_WeightedBehaviors)
	{
		auto retSteering = pair.pBehavior->CalculateSteering(deltaT, pAgent);
		steering.LinearVelocity += pair.weight * retSteering.LinearVelocity;
		steering.AngularVelocity += pair.weight * retSteering.AngularVelocity;

		totalWeight += pair.weight;
	}

	if (totalWeight > 0.f)
	{
		auto scale = 1.f / totalWeight;
		steering *= scale;
	}

	return steering;
}

//*****************
//PRIORITY STEERING
SteeringOutput PrioritySteering::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	for (auto pBehavior : m_PriorityBehaviors)
	{
		steering = pBehavior->CalculateSteering(deltaT, pAgent);

		if (steering.IsValid)
			break;
	}

	//If non of the behavior return a valid output, last behavior is returned
	return steering;
}