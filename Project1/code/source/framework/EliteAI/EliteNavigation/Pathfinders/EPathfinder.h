/*=============================================================================*/
// Copyright 2017-2018 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// EPathfinder.h: Elite pathfinder base
/*=============================================================================*/
#ifndef ELITE_NAVIGATION_PATHFINDERBASE
#define ELITE_NAVIGATION_PATHFINDERBASE
namespace Elite
{
	template<class NodeType, class = std::enable_if<std::is_base_of<Node, NodeType>::value>>
	class Pathfinder
	{
	public:
		//--- Constructor & Destructor ---
		Pathfinder() = default;
		~Pathfinder() = default; //Non virtual Destructor

		//--- Pathfinder Template Functions ---
		std::vector<Vector2> FindPath(
			Graph<NodeType>* pGraph, 
			NodeType* pStartNode, NodeType* pEndNode,
			Heuristic heuristicFunction);

	protected:
		void CalculateCosts(Graph<NodeType>* pGraph, Connection* pC, Node* pStartNode, Node* pTargetNode, Heuristic heuristicFunction, std::vector<Connection*> openList)
		{
			//Calculate the g and h cost (f is calculate when requested)
			//g = current.g + cost(displacement vector current to this)
			float currentGCost = 0;
			if (pC->GetHeadConnection() != nullptr)
				currentGCost = pC->GetHeadConnection()->GetGCost();

			Vector2 parentPos = pStartNode->GetPosition();
			Node* pParentNode = pStartNode;
			Connection* pHeadConnection{ pC->GetHeadConnection() };
			if (pHeadConnection != nullptr)
			{
				pParentNode = pHeadConnection->GetStartNode();
				parentPos = pParentNode->GetPosition();
			}

			if (pHeadConnection && InLineOfSight(pGraph, pParentNode, pC->GetEndNode(), heuristicFunction))
			{
				//pC->SetStartNode(pParentNode);
				Connection* pNewConnection{ pParentNode->AddConnection(pC->GetEndNode()) };
				pNewConnection->SetHeadConnection(nullptr);
				if (pHeadConnection && pHeadConnection->GetHeadConnection())
				{
					pNewConnection->SetHeadConnection(pHeadConnection->GetHeadConnection());
				}
				openList.push_back(pNewConnection);

				Vector2 tempV = GetAbs(pNewConnection->GetEndNode()->GetPosition() - parentPos);
				float gCost = heuristicFunction(tempV.x, tempV.y);
				pNewConnection->SetGCost(currentGCost + gCost);

				//h = distance from absolute displacement vector from this to goal, using the 'heuristicFunction'
				tempV = GetAbs(pNewConnection->GetEndNode()->GetPosition() - pTargetNode->GetPosition());
				float hCost = heuristicFunction(tempV.x, tempV.y);
				pNewConnection->SetHCost(hCost);
			}
			else
			{
				Vector2 tempV = GetAbs(pC->GetEndNode()->GetPosition() - parentPos);
				float gCost = heuristicFunction(tempV.x, tempV.y);
				pC->SetGCost(currentGCost + gCost);

				//h = distance from absolute displacement vector from this to goal, using the 'heuristicFunction'
				tempV = GetAbs(pC->GetEndNode()->GetPosition() - pTargetNode->GetPosition());
				float hCost = heuristicFunction(tempV.x, tempV.y);
				pC->SetHCost(hCost);
			}

		}

	private:
		bool InLineOfSight(Graph<NodeType>* pGraph, Node* pParentNode, Node* pNeighbourNode, Heuristic heuristicFunction)
		{
			Vector2 direction{ GetNormalized(pParentNode->GetPosition() - pNeighbourNode->GetPosition()) };
			Vector2 position{ pNeighbourNode->GetPosition() };
			Node* closeNode{ pGraph->GetClosestNode(position, heuristicFunction) };

			while (closeNode != pParentNode)
			{
				position += direction;
				position.x = ((position.x - int(position.x)) < 2.5f ? int(position.x) : int(position.x + 5));
				position.y = ((position.y - int(position.y)) < 2.5f ? int(position.y) : int(position.y + 5));
				closeNode = pGraph->GetClosestNode(position, heuristicFunction);
				if (!closeNode->IsWalkable())
					return false;
			}
			return true;
		}
	};
}
#endif