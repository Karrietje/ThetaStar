#include "stdafx.h"
#include "SandboxAgent.h"

using namespace Elite;

SandboxAgent::SandboxAgent(): BaseAgent()
{
	m_Target = GetPosition();
}

void SandboxAgent::Update(float dt)
{
	const float maxSpeed = 50.f;
	const float arrivalRadius = 1.f; 
	const float slowRadius = 15.f; 

	Elite::Vector2 toTarget = m_Target - GetPosition(); 
	float distance = toTarget.Magnitude(); 
	if (distance < arrivalRadius)
	{
		SetLinearVelocity(Elite::ZeroVector2);
		return; 
	}

	toTarget.Normalize();//naar de target normalizen

	if (distance < slowRadius)
	{
		toTarget *= maxSpeed * distance / slowRadius; 
	}
	else toTarget *= maxSpeed; //eerst normalizen dan vermengivuldigen met maxspeed

	SetLinearVelocity(toTarget); //Lineair zetten met de vector.
	
	//Orientation
	const auto angle = Orientation(GetRotation(), GetLinearVelocity());
	m_pRigidBody->SetTransform(Transform(GetPosition(), Elite::Vector2(angle, angle)));
}

void SandboxAgent::Render(float dt)
{
	BaseAgent::Render(dt); //Default Agent Rendering
	const Elite::Vector2 position = GetPosition();
	const Elite::Vector2 offset = { 1.f,1.f };
	DEBUGRENDERER2D->DrawString(position + offset, "x: %.2fn y: %.2f", position.x, position.y); //x is vervangen door float met precisie 2

	const Elite::Vector2 velocity = GetLinearVelocity();
	Elite::Vector2 segment = velocity; 
	segment.Normalize(); 
	segment *= 1.5f; 
	DEBUGRENDERER2D->DrawSegment(position, position + segment, Color(0.f, 1.f, 0.f)); 
}

float SandboxAgent::Orientation(float currentOrientation, Elite::Vector2 velocity) const
{
	//Determine angle based on direction
	if (velocity.Magnitude() > 0)
	{
		velocity.Normalize();
		return atan2(velocity.x, -velocity.y);
	}
	return currentOrientation;
}