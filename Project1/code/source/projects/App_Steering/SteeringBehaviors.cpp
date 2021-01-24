//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "SteeringAgent.h"
#include "Behaviors/App_SteeringBehaviors.h"
#include "Obstacle.h"

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{}; 

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition(); 
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	//Debug rendering
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0.f,1.f,0.f,0.5f }, 0.4f);
	}

	return steering;
}

//WANDER (base> SEEK)
//******
SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{

	Elite::Vector2 offset = pAgent->GetLinearVelocity();
	offset.Normalize(); 
	offset *= m_OffsetDistance;

	//Change wander angle
	m_WanderAngle += randomFloat() * m_MaxAngleChange - m_MaxAngleChange * 0.5f; 
	Elite::Vector2 circleOffset = { cos(m_WanderAngle) * m_Radius, sin(m_WanderAngle) * m_Radius }; 

	m_Target = TargetData(pAgent->GetPosition() + offset + circleOffset); 

	//Debug rendering
	if (pAgent->CanRenderBehavior())
	{
		Elite::Vector2 pos = pAgent->GetPosition();
		DEBUGRENDERER2D->DrawSegment(pos, pos + offset, { 0.f,0.f,1.f,0.5f }, 0.4f);
		DEBUGRENDERER2D->DrawCircle(pos + offset, m_Radius, { 0.f,0.f,1.f,0.5f }, 0.4f);
		DEBUGRENDERER2D->DrawSolidCircle(pos + offset + circleOffset, 0.5f, { 0,0 }, { 0,1,0,0.5 }, 0.3f); 
	}

	return Seek::CalculateSteering(deltaT, pAgent); 
}

SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{ Seek::CalculateSteering(deltaT, pAgent) };
	steering.AngularVelocity = -steering.AngularVelocity; 
	steering.LinearVelocity = { -steering.LinearVelocity.x, -steering.LinearVelocity.y }; 

	return steering;
}

SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{ };

	Elite::Vector2 toTarget = m_Target.Position - pAgent->GetPosition(); 
	float distance = toTarget.Normalize(); 
	if (distance < m_arrivalRadius)
	{
		steering.LinearVelocity = Elite::ZeroVector2; 
		return steering; 
	}

	if (distance < m_slowRadius)
	{
		toTarget *= m_maxSpeed * distance / m_slowRadius;
	}
	else toTarget *= m_maxSpeed; 

	steering.LinearVelocity = toTarget; 

	//Debug rendering
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawCircle(m_Target.Position, m_arrivalRadius, { 1.f,0.f,0.f,0.5f }, 0.4f);
	}
	return steering;
}

SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	float angle{ pAgent->GetOrientation() };
	Elite::Vector2 target{ m_Target.Position - pAgent->GetPosition() };
	target.Normalize();
	float targetAngle{ atan2(target.x, -target.y) };
	float angularVelocity{ targetAngle - angle };

	if (angularVelocity > E_PI)
	{
		angularVelocity = -(float(E_PI_2) - angularVelocity);
	}
	else if (angularVelocity < -E_PI)
	{
		angularVelocity = float(E_PI_2) + angularVelocity;
	}

	angularVelocity = Clamp(angularVelocity, -1.f, 1.f);

	steering.AngularVelocity = angularVelocity * pAgent->GetMaxAngularSpeed();

	return steering;
}

SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	float distance{ (m_Target.Position - pAgent->GetPosition()).Magnitude() }; //Magnitude is groote
	float t{ distance / pAgent->GetMaxLinearSpeed() };
	
	m_Target.Position += m_Target.LinearVelocity * t; 

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawCircle(m_Target.Position, 1.f, { 1.f, 0.f, 0.f, 1.f }, 0.4f);
	}

	return Seek::CalculateSteering(deltaT, pAgent);
}

SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	float distance{ Elite::Distance(m_Target.Position, pAgent->GetPosition()) };

	if (distance > m_Radius)
	{
		SteeringOutput steering{};
		steering.IsValid = false;
		return steering;
	}

	float t{ distance / pAgent->GetMaxLinearSpeed() };
	m_Target.Position += m_Target.LinearVelocity * t;

	return Flee::CalculateSteering(deltaT, pAgent);
}

SteeringOutput Hide::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering;

	float distance{ 10000.f };
	Elite::Vector2 targetPos{ m_Target.Position };
	Elite::Vector2 hidePos{};

	for (Obstacle* pObs : m_App->m_Obstacles)
	{
		Elite::Vector2 goHide{ pObs->GetCenter() - targetPos };
		float length = goHide.Normalize();
		length += pObs->GetRadius();
		length += 5.f;

		Elite::Vector2 tempPos{ targetPos + (length * goHide) };
		float tempDistance{ Elite::Distance(pAgent->GetPosition(), tempPos) };

		if (tempDistance < distance)
		{
			distance = tempDistance;
			hidePos = tempPos;
		}
	}

	m_Target.Position = hidePos;

	return Arrive::CalculateSteering(deltaT, pAgent);
}
