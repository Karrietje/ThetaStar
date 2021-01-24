#pragma once
#include "../SteeringHelpers.h"
#include "FlockingSteeringBehaviors.h"
#include "projects/App_Steering/SteeringBehaviors.h"

class ISteeringBehavior;
class SteeringAgent;
class BlendedSteering;
class PrioritySteering;

using namespace Elite;

class TheFlock
{
public:
	TheFlock(
		TargetData* pTargetRef = nullptr, 
		int flockSize = 50, 
		float worldSize = 100.f, 
		SteeringAgent* pAgentToEvade = nullptr, 
		bool trimWorld = false);

	~TheFlock();

	void Update(float deltaT);
	void Render(float deltaT) const;

	void RegisterNeighbors(SteeringAgent* pAgent);
	int GetNrOfNeighbors() const { return m_NrOfNeighbors; }
	const vector<SteeringAgent*>& GetNeighbors() const { return m_Neighbors; }

	Elite::Vector2 GetNeighborhoodCenter() const;
	Elite::Vector2 GetNeighborhoodAverageVelocity() const;

	float& GetWeight_Separation() { return GetWeight(reinterpret_cast<ISteeringBehavior*>(m_pSeparation)); }
	float& GetWeight_Cohesion() { return GetWeight(reinterpret_cast<ISteeringBehavior*>(m_pCohesion)); }
	float& GetWeight_VelocityMatch() { return GetWeight(reinterpret_cast<ISteeringBehavior*>(m_pAlignment)); }
	float& GetWeight_Seek() { return GetWeight(reinterpret_cast<ISteeringBehavior*>(m_pSeek)); }
	//float& GetWeight_Wandering() { return GetWeight(reinterpret_cast<ISteeringBehavior*>(--- insert var ---)); }
	float& GetWeight(ISteeringBehavior* pBehaviour);

private:
	bool m_TrimWorld = false;
	float m_WorldSize = 0.f;

	// Flock agents
	int m_FlockSize = 0;
	vector<SteeringAgent*> m_Boids;

	// Neighbors
	vector<SteeringAgent*> m_Neighbors;
	float m_NeighborhoodRadius = 15.f;
	int m_NrOfNeighbors = 0;

	BlendedSteering* m_pBlendedSteering = nullptr;
	PrioritySteering* m_pPrioritySteering = nullptr;
	Cohesion* m_pCohesion = nullptr;
	Separation* m_pSeparation = nullptr;
	Alignment* m_pAlignment = nullptr;
	Seek* m_pSeek = nullptr;
	Evade* m_pEvade = nullptr;

	TargetData* m_pTarget = nullptr;
	SteeringAgent* m_pEvading = nullptr;

private:
	TheFlock(const TheFlock& other);
	TheFlock& operator=(const TheFlock& other);
};