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
	m_pEvadingAgent->SetMaxLinearSpeed(15.f);
	m_pEvadingAgent->SetAutoOrient(true);
	m_pEvadingAgent->SetMass(1.f);
	m_pEvadingAgent->SetBodyColor({ 0, 1, 0 });

	m_pFlock = new TheFlock(&m_MouseTarget, 1000, m_TrimWorldSize, m_pEvadingAgent, true);
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
	#pragma region UI
	//UI
	{
		//Setup
		int menuWidth = 235;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
		ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: place target");
		ImGui::Text("RMB: move cam.");
		ImGui::Text("Scrollwheel: zoom cam.");
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Checkbox("Debug Rendering", &m_CanDebugRender);
		ImGui::SliderFloat("Trim Size", &m_TrimWorldSize, 0.f, 500.f, "%1.");
		ImGui::Spacing();

		ImGui::SliderFloat("Cohesion", &m_pFlock->GetWeight_Cohesion(), 0.f, 500.f, "%1.");
		ImGui::SliderFloat("Separation", &m_pFlock->GetWeight_Separation(), 0.f, 500.f, "%1.");
		ImGui::SliderFloat("Alignment", &m_pFlock->GetWeight_VelocityMatch(), 0.f, 500.f, "%1.");
		ImGui::SliderFloat("Seek", &m_pFlock->GetWeight_Seek(), 0.f, 500.f, "%1.");

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
	#pragma endregion

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

	//Render Target
	if(m_VisualizeMouseTarget)
		DEBUGRENDERER2D->DrawSolidCircle(m_MouseTarget.Position, 0.3f, { 0.f,0.f }, { 1.f,0.f,0.f },-0.8f);
}
