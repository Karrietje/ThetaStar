/*=============================================================================*/
// Copyright 2017-2018 Elite Engine
// Authors: Matthieu Delaere, Thomas Goussaert
/*=============================================================================*/
// SteeringBehaviors.h: SteeringBehaviors interface and different implementations
/*=============================================================================*/
#ifndef ELITE_STEERINGBEHAVIORS
#define ELITE_STEERINGBEHAVIORS

//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "SteeringHelpers.h"
class App_SteeringBehaviors;
class SteeringAgent;
class TheFlock;
using namespace Elite;

#pragma region **ISTEERINGBEHAVIOR** (BASE)
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	virtual SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) = 0;

	//Seek Functions
	void SetTarget(const TargetData& target) { m_Target = target; }

	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehavior, T>::value>::type* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	TargetData m_Target;
};
#pragma endregion

///////////////////////////////////////
//SEEK
//****
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() = default;

	//Seek Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};

//////////////////////////
//WANDER
//******
class Wander : public Seek
{
public:
	Wander() = default;
	virtual ~Wander() = default;

	//Wander Behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
	void SetWanderOffset(float offset) { m_OffsetDistance = offset; };
	
protected: 
	float m_OffsetDistance = 6.f;
	float m_Radius = 4.f; 
	float m_MaxAngleChange = ToRadians(45); 
	float m_WanderAngle = 0.f; //Current angle 
};

//////////////////////////
//Flee
//******
class Flee : public Seek
{
public:
	Flee() = default; 
	virtual ~Flee() = default; 

	void SetFleeRadius(float radius) { m_Radius = radius; };

	//Flee behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override; 
protected:
	float m_Radius = 15.f;
};

//////////////////////////
//Arrive
//******
class Arrive : public ISteeringBehavior
{
public:
	Arrive() = default; 
	virtual ~Arrive() = default; 

	//Arrive Behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
	

protected:
	float m_maxSpeed = 15.f;
	float m_arrivalRadius = 1.f;
	float m_slowRadius = 15.f;
};

//////////////////////////
//Face
//******
class Face : public Seek
{
public:
	Face() = default;
	virtual ~Face() = default;

	//Arrive Behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

};


//////////////////////////
//Face
//******
class Pursuit : public Seek
{
public:
	Pursuit() = default;
	virtual ~Pursuit() = default;

	//Arrive Behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

};

class Evade : public Flee
{
public:
	Evade() = default;
	virtual ~Evade() = default;

	//Arrive Behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

};

class Hide : public Arrive
{
public:
	Hide(App_SteeringBehaviors* app) : m_App{ app } {}
	virtual ~Hide() = default;

	//Arrive Behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
private:
	App_SteeringBehaviors* m_App;
};
#endif


