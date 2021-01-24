//=== General Includes ===
#include "stdafx.h"
#include "EAStarPathfinder.h"
using namespace Elite;

//=== Pathfinder Functions ===
std::vector<Elite::Vector2> AStarPathfinder::FindPath(
	Graph<Node>* pGraph,
	Node* pStartNode, Node* pEndNode,
	Heuristic heuristicFunction)
{
	//Variables
	std::vector<Vector2> vPath;

	std::vector<Connection*> openList;
	std::vector<Connection*> closedList;

	for (Connection* pConnection : pStartNode->GetConnections())
	{
		CalculateCosts(pGraph, pConnection, pStartNode, pEndNode, heuristicFunction, openList);
		openList.push_back(pConnection);
	}
	
	Connection* pCurrentConnection = nullptr;
	while (!openList.empty())
	{
		auto connectionLowestF = std::min_element(openList.begin(), openList.end(), [](Connection* firstConnect, Connection* secondConnect)
			{
				return firstConnect->GetFCost() < secondConnect->GetFCost();
			});
		pCurrentConnection = *connectionLowestF;
		closedList.push_back(pCurrentConnection);
		
		auto removeOpenlist = std::remove(openList.begin(), openList.end(), pCurrentConnection);
		openList.erase(removeOpenlist);
		std::vector<Connection*> tempConnection = pCurrentConnection->GetEndNode()->GetConnections();

		for (Connection* pConnection : tempConnection)
		{
			if (pConnection->GetEndNode() == pEndNode)
			{
				pConnection->SetHeadConnection(pCurrentConnection);
				openList.clear(); 
				break;
			}
			else
			{
				if (std::find(closedList.begin(), closedList.end(), pConnection) != closedList.end())
				{
					continue;
				}
				else if (std::find(openList.begin(), openList.end(), pConnection) == openList.end())
				{
					CalculateCosts(pGraph, pConnection, pStartNode, pEndNode, heuristicFunction, openList);
					pConnection->SetHeadConnection(pCurrentConnection);
					openList.push_back(pConnection);
				}
			}
		}
	}
	
	while (pCurrentConnection->GetStartNode() != pStartNode)
	{
		vPath.push_back(pCurrentConnection->GetEndNode()->GetPosition());
		pCurrentConnection = pCurrentConnection->GetHeadConnection(); 
	}

	vPath.push_back(pCurrentConnection->GetEndNode()->GetPosition());
	vPath.push_back(pStartNode->GetPosition());
	
	std::reverse(vPath.begin(), vPath.end());
	return vPath;
}