#include "stdafx.h"
#include "TheFlock.h"

#include "../SteeringAgent.h"
#include "../SteeringBehaviors.h"
#include "CombinedSteeringBehaviors.h"
#include "FlockingSteeringBehaviors.h"

//Constructor & Destructor
TheFlock::TheFlock(
	TargetData* pTargetRef, 
	int flockSize /*= 50*/, 
	float worldSize /*= 100.f*/, 
	SteeringAgent* pAgentToEvade /*= nullptr*/, 
	bool trimWorld /*= false*/)

	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld { trimWorld }
	, m_NeighborhoodRadius{ 15 }
	, m_NrOfNeighbors{0}
	, m_pTarget{ pTargetRef }
	, m_pEvading{pAgentToEvade}
	, m_OldPositions{}
	, m_pCellSpace{new CellSpace(worldSize, worldSize, 15, 15, flockSize)}
{
	m_pCohesion = new Cohesion(this);
	m_pSeparation = new Separation(this);
	m_pAlignment = new Alignment(this);
	m_pSeek = new Seek();
	m_pWander = new Wander();
	m_pEvade = new Evade();
	m_pEvade->SetFleeRadius(25.f);
	m_OldPositions.resize(flockSize);
	m_pBlendedSteering = new BlendedSteering({ {m_pCohesion, 0.5f}, {m_pSeparation, 0.5f}, {m_pAlignment, 0.5f}, {m_pWander, 0.5f}, {m_pSeek, 0.0f} });
	m_pPrioritySteering = new PrioritySteering({ m_pEvade, m_pBlendedSteering });


	SteeringAgent* pAgent{};

	for (int i{}; i < m_FlockSize; i++)
	{
		pAgent = new SteeringAgent();
		if (m_pEvading)
		{
			pAgent->SetSteeringBehavior(m_pPrioritySteering);
		}
		else
		{
			pAgent->SetSteeringBehavior(m_pBlendedSteering);
		}
		pAgent->SetMaxLinearSpeed(50.f);
		pAgent->SetAutoOrient(true);
		pAgent->SetMass(0.1f);
		pAgent->SetBodyColor({ 1, 0, 0 });
		pAgent->SetPosition(Elite::Vector2(float(rand() % int(m_WorldSize)), float(rand() % int(m_WorldSize))));
		m_Agents.push_back(pAgent);
		m_pCellSpace->AddAgent(pAgent); 
		m_OldPositions[i] = pAgent->GetPosition(); 
	}
	
	m_Neighbors.resize(m_FlockSize);
}

TheFlock::~TheFlock()
{
	for (int i{}; i < m_FlockSize; i++)
		SAFE_DELETE(m_Agents[i]);

	SAFE_DELETE(m_pCohesion);
	SAFE_DELETE(m_pSeparation);
	SAFE_DELETE(m_pAlignment);
	SAFE_DELETE(m_pSeek);
	SAFE_DELETE(m_pEvade);
	SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pPrioritySteering);
}

void TheFlock::Update(float deltaT)
{
	m_pSeek->SetTarget(*m_pTarget);

	TargetData evade{};
	evade.LinearVelocity = m_pEvading->GetLinearVelocity();
	evade.Position = m_pEvading->GetPosition();
	m_pEvade->SetTarget(evade);

	for (size_t i{}; i < m_Agents.size(); i++)
	{

		if (m_PartitionSpacingEnabled)
		{
			m_pCellSpace->UpdateAgentCell(m_Agents[i], m_OldPositions[i]);
			m_pCellSpace->RegisterNeighbors(m_Agents[i], m_NeighborhoodRadius);
			m_Neighbors = m_pCellSpace->GetNeighbors(); 
			m_NrOfNeighbors = m_pCellSpace->GetNrOfNeighbors(); 
			m_OldPositions[i] = m_Agents[i]->GetPosition();
		}
		else
		{
			RegisterNeighbors(m_Agents[i]);
		}
		m_Agents[i]->Update(deltaT);
		if (m_TrimWorld)
		{
			m_Agents[i]->TrimToWorld({ 0, 0 }, { m_WorldSize, m_WorldSize });
		}
	}
}

void TheFlock::UpdateAndRenderUI()
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

	ImGui::Text("Flocking");
	ImGui::Spacing();

	// Implement checkboxes and sliders here
	ImGui::Spacing();

	ImGui::Checkbox("Partitioning", &m_PartitionSpacingEnabled);
	
	ImGui::Spacing();
	ImGui::SliderFloat("Cohesion", GetWeight_Cohesion(), 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Separation", GetWeight_Separation(), 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Alignment", GetWeight_VelocityMatch(), 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Wander", GetWeight_Wander(), 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Seek", GetWeight_Seek(), 0.f, 1.f, "%.2");

	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
}

void TheFlock::Render(float deltaT) const
{
	if (m_PartitionSpacingEnabled)
	{
		m_pCellSpace->RenderCells(); 
	}
	/*
	for (SteeringAgent* pBoid : m_Agents)
	{
		pBoid->Render(deltaT);
	}*/
}

void TheFlock::RegisterNeighbors(SteeringAgent* pAgent)
{
	// register the agents neighboring the currently evaluated agent
	// store how many they are, so you know which part of the vector to loop over
	m_NrOfNeighbors = 0;

	for (SteeringAgent* pBoid : m_Agents)
	{
		float distance{ Elite::Distance(pBoid->GetPosition(), pAgent->GetPosition()) };
		if (pBoid && distance < m_NeighborhoodRadius)
		{
			m_Neighbors[m_NrOfNeighbors] = pBoid;
			m_NrOfNeighbors++;
		}
	}
}

Elite::Vector2 TheFlock::GetAverageNeighborPos() const
{
	Elite::Vector2 allCenters{};
	for (int i{}; i < m_NrOfNeighbors; i++)
	{
		allCenters += m_Neighbors[i]->GetPosition();
	}
	return (allCenters / float(m_NrOfNeighbors));
}

Elite::Vector2 TheFlock::GetNeighborhoodAverageVelocity() const
{
	Elite::Vector2 allVelocities{};
	for (int i{}; i < m_NrOfNeighbors; i++)
	{
		allVelocities += m_Neighbors[i]->GetLinearVelocity();
	}
	return (allVelocities / float(m_NrOfNeighbors));
}

float* TheFlock::GetWeight(ISteeringBehavior* pBehaviour)
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->m_WeightedBehaviors;
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehaviour](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehaviour;
			}
		);

		if (it != weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}


