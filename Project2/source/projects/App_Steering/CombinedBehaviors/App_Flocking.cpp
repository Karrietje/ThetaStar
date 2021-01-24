//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_Flocking.h"
#include "../SteeringAgent.h"
#include "TheFlock.h"

//Destructor
App_Flocking::~App_Flocking()
{	
	SAFE_DELETE(m_pFlock);
	SAFE_DELETE(m_pEvadingAgent);
	SAFE_DELETE(m_pWander);
}

//Functions
void App_Flocking::Start()
{
	m_pWander = new Wander();
	m_pEvadingAgent = new SteeringAgent();
	m_pEvadingAgent->SetSteeringBehavior(m_pWander);
	m_pEvadingAgent->SetMaxLinearSpeed(50.f);
	m_pEvadingAgent->SetAutoOrient(true);
	m_pEvadingAgent->SetMass(0.1f);
	m_pEvadingAgent->SetBodyColor({ 0, 1, 0 });

	m_pFlock = new TheFlock(&m_MouseTarget, 1500, m_TrimWorldSize, m_pEvadingAgent, true);
}

void App_Flocking::Update(float deltaTime)
{
	//Update that is being called after the physics simulation
	(void)deltaTime;

	//INPUT
	if (INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eLeft) && m_VisualizeMouseTarget)
	{
		auto const mouseData = INPUTMANAGER->GetMouseData(InputType::eMouseButton, InputMouseButton::eLeft);
		m_MouseTarget.Position = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y) });
	}

#ifdef PLATFORM_WINDOWS

	m_pFlock->Update(deltaTime);

	m_pEvadingAgent->Update(deltaTime);
	m_pEvadingAgent->TrimToWorld({ 0, 0 }, { m_TrimWorldSize, m_TrimWorldSize });
#endif
}

void App_Flocking::Render(float deltaTime) const
{
	std::vector<Elite::Vector2> points =
	{
		{ 0,m_TrimWorldSize },
		{ m_TrimWorldSize,m_TrimWorldSize },
		{ m_TrimWorldSize,0 },
		{0,0 }
	};
	DEBUGRENDERER2D->DrawPolygon(&points[0], 4, { 1,0,0,1 }, 0.4f);

	m_pFlock->Render(deltaTime);

	m_pEvadingAgent->Render(deltaTime);
	m_pFlock->UpdateAndRenderUI();

	//Render Target
	if(m_VisualizeMouseTarget)
		DEBUGRENDERER2D->DrawSolidCircle(m_MouseTarget.Position, 0.3f, { 0.f,0.f }, { 1.f,0.f,0.f },-0.8f);
}
