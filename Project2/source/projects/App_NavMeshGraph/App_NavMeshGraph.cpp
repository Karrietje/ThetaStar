//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"
using namespace Elite;

//Includes
#include "App_NavMeshGraph.h"
#include "projects/Shared/NavigationColliderElement.h"

#include "../App_Steering/SteeringAgent.h"
#include "../App_Steering/SteeringBehaviors.h"

#include "framework\EliteAI\EliteGraphs\ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"



//Statics
bool App_NavMeshGraph::sShowPolygon = true;
bool App_NavMeshGraph::sShowGraph = false;
bool App_NavMeshGraph::sDrawPortals = false;
bool App_NavMeshGraph::sDrawFinalPath = true;
bool App_NavMeshGraph::sDrawNonOptimisedPath = false;

//Destructor
App_NavMeshGraph::~App_NavMeshGraph()
{
	for (auto pNC : m_vNavigationColliders)
		SAFE_DELETE(pNC);
	m_vNavigationColliders.clear();

	SAFE_DELETE(m_pNavGraph);
	SAFE_DELETE(m_pSeekBehavior);
	SAFE_DELETE(m_pArriveBehavior);
	SAFE_DELETE(m_pAgent);
}

//Functions
void App_NavMeshGraph::Start()
{
	//Initialization of your application. 
	//----------- CAMERA ------------
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(36.782f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(12.9361f, 0.2661f));

	//----------- WORLD ------------
	m_vNavigationColliders.push_back(new NavigationColliderElement(Elite::Vector2(15.f, 10.f), 14.0f, 1.0f));
	m_vNavigationColliders.push_back(new NavigationColliderElement(Elite::Vector2(-25.f, 10.f), 14.0f, 1.0f));
	m_vNavigationColliders.push_back(new NavigationColliderElement(Elite::Vector2(-13.f, -8.f), 30.0f, 2.0f));
	m_vNavigationColliders.push_back(new NavigationColliderElement(Elite::Vector2(15.f, -21.f), 14.0f, 1.0f));

	//----------- NAVMESH  ------------
	std::list<Elite::Vector2> baseBox
	{ { -60, 30 },{ -60, -30 },{ 60, -30 },{ 60, 30 } };

	m_pNavGraph = new Elite::NavGraph(Elite::Polygon(baseBox), m_AgentRadius);

	//----------- AGENT ------------
	m_pSeekBehavior = new Seek();
	m_pArriveBehavior = new Arrive();
	m_Target = TargetData(Elite::ZeroVector2);
	m_pAgent = new SteeringAgent();
	m_pAgent->SetSteeringBehavior(m_pSeekBehavior);
	m_pAgent->SetMaxLinearSpeed(m_AgentSpeed);
	m_pAgent->SetAutoOrient(true);
	m_pAgent->SetMass(0.1f);

}

void App_NavMeshGraph::Update(float deltaTime)
{
	//Update target/path based on input
	if (INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eMiddle))
	{
		auto mouseData = INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eMiddle);
		Elite::Vector2 mouseTarget = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld(
			Elite::Vector2((float)mouseData.X, (float)mouseData.Y));
		m_vPath = FindPath(m_pAgent->GetPosition(), mouseTarget);
	}

	//Check if a path exist and move to the following point
	if (m_vPath.size() > 0)
	{
		if (m_vPath.size() == 1)
		{
			m_pAgent->SetSteeringBehavior(m_pArriveBehavior);
			m_pArriveBehavior->SetTarget(m_vPath[0]);
		}
		else
		{
			m_pAgent->SetSteeringBehavior(m_pSeekBehavior);
			m_pSeekBehavior->SetTarget(m_vPath[0]);
		}

		if (Elite::DistanceSquared(m_pAgent->GetPosition(), m_vPath[0]) < m_AgentRadius * m_AgentRadius)
		{
			//If we reached the next point of the path. Remove it 
			m_vPath.erase(std::remove(m_vPath.begin(), m_vPath.end(), m_vPath[0]));
		}
	}
	
	UpdateImGui();
	m_pAgent->Update(deltaTime);
}

void App_NavMeshGraph::Render(float deltaTime) const
{
	if (sShowGraph)
	{
		m_GraphRenderer.RenderGraph(m_pNavGraph, true, true);
	}

	if (sShowPolygon)
	{
		DEBUGRENDERER2D->DrawPolygon(m_pNavGraph->GetNavMeshPolygon(),
			Color(0.1f, 0.1f, 0.1f));
		DEBUGRENDERER2D->DrawSolidPolygon(m_pNavGraph->GetNavMeshPolygon(),
			Color(0.0f, 0.5f, 0.1f, 0.05f), 0.4f);
	}

	if (sDrawPortals)
	{
		for (const auto &portal : m_Portals)
		{
			DEBUGRENDERER2D->DrawSegment(portal.Line.p1, portal.Line.p2, Color(1.f, .5f, 0.f), -0.1f);
			//Draw just p1 p2
			std::string p1{ "p1" };
			std::string p2{ "p2" };
			//Add the positions to the debugdrawing
			//p1 +=" x:" + std::to_string(portal.Line.p1.x) + ", y: " + std::to_string(portal.Line.p1.y);
			//p2 +=" x:" + std::to_string(portal.Line.p2.x) + ", y: " + std::to_string(portal.Line.p2.y);
			DEBUGRENDERER2D->DrawString(portal.Line.p1, p1.c_str(), Color(1.f, .5f, 0.f), -0.1f);
			DEBUGRENDERER2D->DrawString(portal.Line.p2, p2.c_str(), Color(1.f, .5f, 0.f), -0.1f);

		}
	}

	if (sDrawNonOptimisedPath && m_DebugNodePositions.size() > 0)
	{
		for (auto pathNode : m_DebugNodePositions)
			DEBUGRENDERER2D->DrawCircle(pathNode, 2.0f, Color(0.f, 0.f, 1.f), -0.45f);
	}

	if (sDrawFinalPath && m_vPath.size() > 0)
	{

		for (auto pathPoint : m_vPath)
			DEBUGRENDERER2D->DrawCircle(pathPoint, 2.0f, Color(1.f, 0.f, 0.f), -0.2f);

		DEBUGRENDERER2D->DrawSegment(m_pAgent->GetPosition(), m_vPath[0], Color(1.f, 0.0f, 0.0f), -0.2f);
		for (size_t i = 0; i < m_vPath.size() - 1; i++)
		{
			float g = float(i) / m_vPath.size();
			DEBUGRENDERER2D->DrawSegment(m_vPath[i], m_vPath[i+1], Color(1.f, g, g), -0.2f);
		}
			
	}


}

std::vector<Elite::Vector2> App_NavMeshGraph::FindPath(Elite::Vector2 startPos, Elite::Vector2 endPos)
{
	//Create the path to return
	std::vector<Elite::Vector2> endPath{};

	//Get the start and endTriangle
	const Triangle* startTriangle = m_pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(startPos);
	const Triangle* endTriangle = m_pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(endPos);
	if (startTriangle == nullptr || endTriangle == nullptr)
		return endPath;
	if (startTriangle == endTriangle)
	{
		endPath.push_back(endPos);
		return endPath; 
	}
	
	//If we have valid start/end triangles and they are not the same
	//=> Start looking for a path
	//Copy the graph
	auto tempGraph{ m_pNavGraph->Clone() };
		
	//Create extra node for the Start Node (Agent's position)
	int startIndex = tempGraph->GetNextFreeNodeIndex();
	NavGraphNode* startNode = new NavGraphNode{ startIndex, -1, startPos };
	tempGraph->AddNode(startNode);
	for (int lineIndex : startTriangle->metaData.IndexLines )
	{
		int tempIndex = -1;
		if (lineIndex != 1)
		{
			tempIndex = m_pNavGraph->GetNodeIdxFromLineIdx(lineIndex);
		}
		if (tempIndex != -1)
		{
			tempGraph->AddConnection(new GraphConnection2D{ startIndex, tempIndex });
		}
	}
	
	//Create extra node for the End Node
	int endIndex = tempGraph->GetNextFreeNodeIndex();
	NavGraphNode* endNode = new NavGraphNode{ endIndex, -1 , endPos };
	tempGraph->AddNode(endNode);
	for (int lineIdx : endTriangle->metaData.IndexLines )
	{
		int tempIdx = -1;
		if (lineIdx != -1)
		{
			tempIdx = m_pNavGraph->GetNodeIdxFromLineIdx(lineIdx);
		}
		if (tempIdx != -1)
		{
			tempGraph->AddConnection(new GraphConnection2D{ endIndex, tempIdx });
		}
	}
	//Run A star on new graph
	vector<NavGraphNode*> pAStarPath{ AStar<NavGraphNode, GraphConnection2D>(tempGraph.get(), HeuristicFunctions::Chebyshev).FindPath(startNode, endNode) };
	m_DebugNodePositions.clear(); 

	for (NavGraphNode* temporary : pAStarPath)
	{
		m_DebugNodePositions.push_back(temporary->GetPosition()); 
	}

	for (size_t i{ 1 };  i < pAStarPath.size(); i++)
	{
		endPath.push_back(pAStarPath[i]->GetPosition());
	}

	//Extra: Run optimiser on new graph, Make sure the A star path is fine before uncommenting this!
	m_Portals = SSFA::FindPortals(pAStarPath, m_pNavGraph->GetNavMeshPolygon());
	endPath = SSFA::OptimizePortals(m_Portals);

	endPath.push_back(endNode->GetPosition());
	return endPath;
}

void App_NavMeshGraph::UpdateImGui()
{
	//------- UI --------
#ifdef PLATFORM_WINDOWS
#pragma region UI
	{
		//Setup
		int menuWidth = 150;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 90));
		ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);
		ImGui::SetWindowFocus();
		ImGui::PushItemWidth(70);
		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
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

		ImGui::Checkbox("Show Polygon", &sShowPolygon);
		ImGui::Checkbox("Show Graph", &sShowGraph);
		ImGui::Checkbox("Show Portals", &sDrawPortals);
		ImGui::Checkbox("Show Path Nodes", &sDrawNonOptimisedPath);
		ImGui::Checkbox("Show Final Path", &sDrawFinalPath);
		ImGui::Spacing();
		ImGui::Spacing();

		if (ImGui::SliderFloat("AgentSpeed", &m_AgentSpeed, 0.0f, 22.0f))
		{
			m_pAgent->SetMaxLinearSpeed(m_AgentSpeed);
		}
		
		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
#pragma endregion
#endif
}
