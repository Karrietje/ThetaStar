#pragma once
#include "../SteeringBehaviors.h"

class TheFlock;

//SEPARATION - FLOCKING
//*********************
class Separation : public ISteeringBehavior
{
public:
	Separation(TheFlock* flock) : m_pFlock{ flock } {};
	virtual ~Separation() = default;

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	TheFlock* m_pFlock;
};

//COHESION - FLOCKING
//*******************
class Cohesion : public Seek
{
public:
	Cohesion(TheFlock* flock) : m_pFlock{ flock } {};
	virtual ~Cohesion() = default;

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	TheFlock* m_pFlock;
};

//VELOCITY MATCH - FLOCKING
//************************
class Alignment : public ISteeringBehavior
{
public:
	Alignment(TheFlock* flock) : m_pFlock{ flock } {};
	virtual ~Alignment() = default;

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	TheFlock* m_pFlock;
};
