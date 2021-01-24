//=== General Includes ===
#include "stdafx.h"
#include "ENavigationMeshPathfinder.h"
using namespace Elite;

//=== Static Debug Variable Initialization ===
#ifdef _DEBUG
	bool NavigationMeshPathfinder::sDrawDebugInfoQueries = false;
	bool NavigationMeshPathfinder::sDrawPortals = false;
#endif

//=== Pathfinder Functions ===
std::vector<Elite::Vector2> NavigationMeshPathfinder::FindPath(
	Graph<NavigationMeshNode>* pGraph,
	NavigationMeshNode StartNode, NavigationMeshNode EndNode,
	Heuristic heuristicFunction)
{
	//Variables
	std::vector<Elite::Vector2> vPath;
	NavigationMesh* pNavigationMesh = reinterpret_cast<NavigationMesh*>(pGraph);
	if (pNavigationMesh == nullptr)
	{
		std::cout << "ERROR: searching path on NavigationMesh without passing a NavigationMesh Graph!" << std::endl;
		return vPath;
	}

	//--------------- STEP 1: PREPARATIONS ---------------
	// - Resetting the Graph, adding initial path nodes & setting up variables
	// - Kickstarting the while loop by creating nodes/connections from the startTriangle
	//--------------------------------------------
	//....

	pNavigationMesh->ResetNavigationMesh();

	//Add the start and endNode
	NavigationMeshNode* pStartNode = pNavigationMesh->AppendNode(StartNode);
	NavigationMeshNode* pEndNode = pNavigationMesh->AppendNode(EndNode);

	//A* containers and variables
	std::vector<Connection*> openList;
	std::vector<Connection*> closedList;
	Connection* pCurrentConnection = nullptr;

	const Triangle* pStartTriangle = pNavigationMesh->GetCurrentTriangleFromPosition(pStartNode->GetPosition());
	const Triangle* pEndTriangle = pNavigationMesh->GetCurrentTriangleFromPosition(pEndNode->GetPosition());

	if (pStartTriangle == nullptr)
	{
		pStartTriangle = pNavigationMesh->GetClosestTriangle(pStartNode->GetPosition());
	}
	if (pEndTriangle == nullptr)
	{
		pEndTriangle = pNavigationMesh->GetClosestTriangle(pEndNode->GetPosition());
	}

	if (pStartTriangle == nullptr || pEndTriangle == nullptr)
	{
		return vPath;
	}
	
	if (pStartTriangle == pEndTriangle)
	{
		vPath.push_back(pEndNode->GetPosition());
		return vPath; 
	}
	
	std::vector<const Line*> pLines = pNavigationMesh->GetPlausibleLines(pStartTriangle); 
	for (const Line* pLine : pLines)
	{
		Vector2 projectLine = ProjectOnLineSegment(pLine->p1, pLine->p2, pStartNode->GetPosition(), 1.f);
		NavigationMeshNode* pNewNode = pNavigationMesh->AppendNode(NavigationMeshNode(projectLine, pLine->index, pStartTriangle));
		pStartNode->AddConnection(pNewNode);
		pNavigationMesh->SetLineReviewed(pLine->index);
	}

	for (Connection* pConnection : pStartNode->GetConnections())
	{
		CalculateCosts(pGraph, pConnection, pStartNode, pEndNode, heuristicFunction, openList);
		openList.push_back(pConnection);
	}


	//--------------- CALCULATIONS ---------------
	//Do the actual path calculations. We generate our "graph" (== query) on the fly, 
	//using the A* (heuristics) algorithm to determine next connections.
	//Generate query on the fly - expanding based on the current closest node (through connection).
	//We loop while we still have nodes in the openlist.
	//We stop when we hit the triangle that holds the targetNode 
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

		NavigationMeshNode* pTempNavMesh = static_cast<NavigationMeshNode*>(pCurrentConnection->GetEndNode());
		int index = pTempNavMesh->GetAssociatedLineIndex(); 
		if ((index == pEndTriangle->metaData.IndexLine1) || (index == pEndTriangle->metaData.IndexLine2) || (index == pEndTriangle->metaData.IndexLine3))
		{
			Connection* pLastConnection = pTempNavMesh->AddConnection(pEndNode);
			pLastConnection->SetHeadConnection(pCurrentConnection);
			pCurrentConnection = pLastConnection; 
			openList.clear(); 
			break; 
		}

		std::vector <const Triangle*> pTriangles = pNavigationMesh->GetTrianglesFromLineIndex(index);
		const Triangle* pCurrentTriangle{}; 
		if (pTriangles.size() == 1)
		{
			pCurrentTriangle = pTriangles[0]; 
		}
		else if (pTriangles.size() == 2)
		{
			int numberOfReviewed{0};
			if (pNavigationMesh->IsLineReviewed(pTriangles[0]->metaData.IndexLine1))
			{
				numberOfReviewed++;
			}

			if (pNavigationMesh->IsLineReviewed(pTriangles[0]->metaData.IndexLine2))
			{
				numberOfReviewed++;

			}

			if (pNavigationMesh->IsLineReviewed(pTriangles[0]->metaData.IndexLine3))
			{
				numberOfReviewed++;

			}

			if (numberOfReviewed == 1)
			{
				pCurrentTriangle = pTriangles[0];
			}
			else pCurrentTriangle = pTriangles[1]; 
		}

		for (const Line* pPlausibleLine : pNavigationMesh->GetPlausibleLines(pCurrentTriangle))
		{
			if (pNavigationMesh->IsLineReviewed(pPlausibleLine->index))
			{
				continue;
			}
			Vector2 positionOnLine = ProjectOnLineSegment(pPlausibleLine->p1, pPlausibleLine->p2, pCurrentConnection->GetEndNode()->GetPosition(), 1.f);
			NavigationMeshNode* pNewNode = pNavigationMesh->AppendNode(NavigationMeshNode(positionOnLine, pPlausibleLine->index, pCurrentTriangle));
			Connection* pNewConnection = pCurrentConnection->GetEndNode()->AddConnection(pNewNode); 
			pNewConnection->SetHeadConnection(pCurrentConnection);
			pNavigationMesh->SetLineReviewed(pPlausibleLine->index); 
		}

		for (Connection* pNewConnection : pCurrentConnection->GetEndNode()->GetConnections())
		{
			if (std::find(closedList.begin(), closedList.end(), pNewConnection) == closedList.end())
			{
				if (std::find(openList.begin(), openList.end(), pNewConnection) == openList.end())
				{
					CalculateCosts(pGraph, pNewConnection, pStartNode, pEndNode, heuristicFunction, openList);
					openList.push_back(pNewConnection); 
				}
			}
		}
	}

	//--- DEBUG RENDERING
#ifdef _DEBUG
	if (sDrawDebugInfoQueries)
	{
		for (auto pN : pGraph->GetNodes())
		{
			for (auto pC : pN->GetConnections())
			{	
				DEBUGRENDERER2D->DrawPoint(pC->GetStartNode()->GetPosition(), 5.0f, { 0,1.f,1.f }, 0.3f);
				DEBUGRENDERER2D->DrawPoint(pC->GetEndNode()->GetPosition(), 5.0f, { 0,1.f,1.f }, 0.3f);
				DEBUGRENDERER2D->DrawSegment(pC->GetStartNode()->GetPosition(),
					pC->GetEndNode()->GetPosition(), { 0,1.f,1.f }, 0.3f);
			}
		}
	}
#endif

	//IMPORTANT: The code below is given as a gift. If you reach this point, you will get a path that is not the optimal path but the agent will already get to the end point.
	//Finish the OptimizePortals() functions to get an optimized path!!
	//--------------- RECONSTRUCT ---------------
	//Retracing the path and store this path in a container.
	//--------------------------------------------
	std::vector<NavigationMeshNode*> vPathNodes;
	//pCurrentConnection && is a safety check for when you run before implementing the actual pathfinder
	if(pCurrentConnection)
	{ 
		while (pCurrentConnection->GetStartNode() != pStartNode)
		{
			vPathNodes.push_back(static_cast<NavigationMeshNode*>(pCurrentConnection->GetEndNode()));
			pCurrentConnection = pCurrentConnection->GetHeadConnection();
		}
		vPathNodes.push_back(static_cast<NavigationMeshNode*>(pCurrentConnection->GetEndNode()));
		std::reverse(vPathNodes.begin(), vPathNodes.end());
	}
	//--------------- OPTIMIZE ---------------
	//Retrace the path and store this path in a container. Then optimize it using the funnel algorithm and
	//return the optimized path! 
	//--------------------------------------------
	//COMMENT THE FOLLOWING LINE OF CODE (#define NOT_OPTIMIZED) TO ENABLE THE OPTIMIZED PART!
	//Otherwise it will just copy over the nodes so you can test earlier functionality
//#define NOT_OPTIMIZED
#ifdef NOT_OPTIMIZED
	for (auto n : vPathNodes)
		vPath.push_back(n->GetPosition());
#else
	//Optimize our path using funnel algorithm!
	auto const portals = FindPortals(pNavigationMesh, vPathNodes, pStartNode->GetPosition(), pEndNode->GetPosition());
	vPath = OptimizePortals(portals, pStartNode->GetPosition());
	vPath.push_back(vPathNodes[vPathNodes.size() - 1]->GetPosition()); //Push goal as part of the path
#endif

	return vPath;
}

//=== Private Pathfinder Functions ===
//--- References ---
//http://digestingduck.blogspot.be/2010/03/simple-stupid-funnel-algorithm.html
//https://gamedev.stackexchange.com/questions/68302/how-does-the-simple-stupid-funnel-algorithm-work
std::vector<Elite::Vector2> NavigationMeshPathfinder::OptimizePortals(const std::vector<Portal>& portals,
	const Elite::Vector2& startPos) const
{
	//P1 == right point of portal, P2 == left point of portal
	std::vector<Vector2> vPath = {};
	Vector2 apex = startPos;
	int apexIndex = 0, leftLegIndex = 0, rightLegIndex = 0;
	Vector2 rightLeg = portals[rightLegIndex].Line.p1 - apex;
	Vector2 leftLeg = portals[leftLegIndex].Line.p2 - apex;

	for (unsigned int i = 1; i < static_cast<unsigned int>(portals.size()); ++i)
	{
		//--- GET CURRENT PORTAL ---
		const Portal& currentPortal = portals[i];

		//--- RIGHT CHECK ---
		Vector2 newRightLeg = currentPortal.Line.p1 - apex; //Apex is middelpunt waaruit twee benen vertrekken
		if (Cross(rightLeg, newRightLeg) > 0)
		{
			if (Cross(newRightLeg, leftLeg) > 0)
			{
				rightLeg = newRightLeg;
				rightLegIndex = i;
			}
			else
			{
				apex += leftLeg;
				apexIndex = leftLegIndex;
				int newIt{};
				newIt = apexIndex + 1;
				i = newIt;
				leftLegIndex = newIt;
				rightLegIndex = newIt;
				vPath.push_back(apex);

				if (newIt < static_cast<int>(portals.size()))
				{
					rightLeg = portals[rightLegIndex].Line.p1 - apex;
					leftLeg = portals[leftLegIndex].Line.p2 - apex;
					continue;
				}
			}
		}

		//--- LEFT CHECK ---
		Vector2 newLeftLeg = currentPortal.Line.p2 - apex; //Apex is middelpunt waaruit twee benen vertrekken
		if (Cross(leftLeg, newLeftLeg) < 0)
		{
			if (Cross(newLeftLeg, rightLeg) < 0)
			{
				leftLeg = newLeftLeg;
				leftLegIndex = i;
			}
			else
			{
				apex += rightLeg;
				apexIndex = rightLegIndex;
				int newIt{};
				newIt = apexIndex + 1;
				i = newIt;
				leftLegIndex = newIt;
				rightLegIndex = newIt;
				vPath.push_back(apex);

				if (newIt < static_cast<int>(portals.size()))
				{
					rightLeg = portals[rightLegIndex].Line.p1 - apex;
					leftLeg = portals[leftLegIndex].Line.p2 - apex;
					continue;
				}
			}
		}
	}
	return vPath;
}

std::vector<Portal> NavigationMeshPathfinder::FindPortals(const NavigationMesh* pNavigationMesh,
	const std::vector<NavigationMeshNode*>& nodePath,
	const Elite::Vector2& startPos, const Elite::Vector2& endPos) const
{
	//Container
	std::vector<Portal> vPortals = {};

	//For each node received, get it's corresponding line
	const Triangle* pTriangle = nullptr;
	for (size_t i = 0; i < nodePath.size() - 1; ++i)
	{
		//Local variables
		auto pNode = nodePath[i]; //Store node, except last node, because this is our target node!
		auto pLine = pNavigationMesh->GetPolygon()->GetLines()[pNode->GetAssociatedLineIndex()];
		
		//Redetermine it's "orientation" based on the required path (left-right vs right-left) - p1 should be right point
		auto centerLine = (pLine->p1 + pLine->p2) / 2.0f;
		//Find the corresponding triangled based on node linking
		pTriangle = pNode->GetTraversedTriangle();
		auto centerTriangle = pTriangle->GetCenter();
		auto cp = Cross((centerLine - centerTriangle), (pLine->p1 - centerTriangle));
		Line portalLine = {};
		if (cp > 0)//Left
			portalLine = Line(pLine->p2, pLine->p1);
		else //Right
			portalLine = Line(pLine->p1, pLine->p2);

		//Store portal
		vPortals.push_back(Portal(portalLine));
	}
	//Add degenerate portal to force end evaluation
	vPortals.push_back(Portal(Line(endPos, endPos)));

#ifdef _DEBUG
	if (sDrawPortals)
	{
		for (auto portal : vPortals)
			DEBUGRENDERER2D->DrawSegment(portal.Line.p1, portal.Line.p2, Color(1.f, .5f, 0.f));
	}
#endif

	return vPortals;
}