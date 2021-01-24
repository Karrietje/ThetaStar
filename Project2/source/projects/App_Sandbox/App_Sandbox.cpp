#include "stdafx.h"

//Includes
#include "App_Sandbox.h"
#include "SandboxAgent.h"

//Destructor
App_Sandbox::~App_Sandbox()
{
	SAFE_DELETE(m_pAgent);
}

//Functions
void App_Sandbox::Start()
{
	m_pAgent = new SandboxAgent();
}

void App_Sandbox::Update(float deltaTime)
{
	if (INPUTMANAGER->IsMouseButtonUp(Elite::InputMouseButton::eLeft) )//Een macro dat je kunt gebruiken voor als ge uw mousinput gaat gebruiken
	{
		auto mouseData = INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eLeft);
			m_pAgent->SetTarget(DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld(Elite::Vector2(float(mouseData.X), float(mouseData.Y)))); //Convert van screen naar worldscreen!
	}
	m_pAgent->Update(deltaTime);
}

void App_Sandbox::Render(float deltaTime) const
{
	m_pAgent->Render(deltaTime);
}