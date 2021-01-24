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
{
	m_pCohesion = new Cohesion(this);
	m_pSeparation = new Separation(this);
	m_pAlignment = new Alignment(this);
	m_pSeek = new Seek();
	m_pEvade = new Evade();
	m_pEvade->SetFleeRadius(15.f);

	m_pBlendedSteering = new BlendedSteering({ {m_pCohesion, 0.25f}, {m_pSeparation, 0.25f}, {m_pAlignment, 0.25f}, {m_pSeek, 0.25f} });
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
		pAgent->SetMaxLinearSpeed(15.f);
		pAgent->SetAutoOrient(true);
		pAgent->SetMass(1.f);
		pAgent->SetBodyColor({ 1, 0, 0 });
		pAgent->SetPosition(Elite::Vector2((rand() % int(m_WorldSize)), (rand() % int(m_WorldSize))));
		m_Boids.push_back(pAgent);
	}
	
	m_Neighbors.resize(m_FlockSize);
}

TheFlock::~TheFlock()
{
	for (int i{}; i < m_FlockSize; i++)
		SAFE_DELETE(m_Boids[i]);

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

	for (size_t i{}; i < m_Boids.size(); i++)
	{
		RegisterNeighbors(m_Boids[i]);
		m_Boids[i]->Update(deltaT);
		if (m_TrimWorld)
			m_Boids[i]->TrimToWorld({ 0, 0 }, { m_WorldSize, m_WorldSize });
	}
}

void TheFlock::Render(float deltaT) const
{
	for (SteeringAgent* pBoid : m_Boids)
	{
		pBoid->Render(deltaT);
	}
}

void TheFlock::RegisterNeighbors(SteeringAgent* pAgent)
{
	// register the agents neighboring the currently evaluated agent
	// store how many they are, so you know which part of the vector to loop over
	m_NrOfNeighbors = 0;

	for (SteeringAgent* pBoid : m_Boids)
	{
		float distance{ Elite::Distance(pBoid->GetPosition(), pAgent->GetPosition()) };
		if (distance < m_NeighborhoodRadius)
		{
			m_Neighbors[m_NrOfNeighbors] = pBoid;
			m_NrOfNeighbors++;
		}
	}
}

Elite::Vector2 TheFlock::GetNeighborhoodCenter() const
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


float& TheFlock::GetWeight(ISteeringBehavior* pBehavior)
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->m_WeightedBehaviors;
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if(it!= weightedBehaviors.end())
			return it->weight;
	}

	float weight = 0.f;
	return weight;
}
