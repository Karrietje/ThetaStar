#pragma once

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class AStar
	{
	public:
		AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		// stores the optimal connection to a node and its total costs related to the start and end node of the path
		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnection = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode
					&& pConnection == other.pConnection
					&& costSoFar == other.costSoFar
					&& estimatedTotalCost == other.estimatedTotalCost;
			};

			bool operator<(const NodeRecord& other) const
			{
				return estimatedTotalCost < other.estimatedTotalCost;
			};
		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;
		bool ParentInSight(T_NodeType* pParentNode, T_NodeType* pNeighbourNode) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	AStar<T_NodeType, T_ConnectionType>::AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> AStar<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{
		std::vector<T_NodeType*> nPath;

		std::vector<NodeRecord> openList;
		std::vector<NodeRecord> closedList;

		NodeRecord currentRecord;
		NodeRecord startRecord;
		startRecord.pNode = pStartNode;
		startRecord.pConnection = nullptr;
		startRecord.costSoFar = 0;
		startRecord.estimatedTotalCost = GetHeuristicCost(pStartNode, pGoalNode);

		openList.push_back(startRecord);

		//if in loop
		while (!openList.empty())
		{
			//Look at the 4 options, which one is the best/shortest
			currentRecord = *min_element(openList.cbegin(), openList.cend());

			//If you are at your end node, you break!
			if (currentRecord.pNode == pGoalNode)
				break;

			for (T_ConnectionType* connection : m_pGraph->GetNodeConnections(currentRecord.pNode->GetIndex()))
			{
				float tempCost = currentRecord.costSoFar + connection->GetCost();

				vector<NodeRecord>::iterator closedIt = std::find_if(closedList.begin(), closedList.end(), [&connection](NodeRecord lhs)
					{
						return (connection->GetTo() == lhs.pNode->GetIndex());
					});
				vector<NodeRecord>::iterator openIt = std::find_if(openList.begin(), openList.end(), [&connection](NodeRecord lhs)
					{
						return (connection->GetTo() == lhs.pNode->GetIndex());
					});

				if (closedIt != closedList.end())
				{
					NodeRecord closedTemp = *closedIt;
					if (closedTemp.costSoFar <= tempCost)
					{
						continue;
					}
					else closedList.erase(closedIt);
				}
				else if (openIt != openList.end())
				{
					NodeRecord openTemp = *openIt;
					if (openTemp.costSoFar <= tempCost)
					{
						continue;
					}
					else
						openList.erase(openIt);
				}

				NodeRecord temporaryRecord;

				if (currentRecord.pConnection && ParentInSight(m_pGraph->GetNode(currentRecord.pConnection->GetFrom()), m_pGraph->GetNode(connection->GetTo())))
				{
					std::cout << "hello" << std::endl;

					temporaryRecord.pNode = m_pGraph->GetNode(connection->GetTo());
					temporaryRecord.pConnection = currentRecord.pConnection;
					temporaryRecord.costSoFar = currentRecord.costSoFar + currentRecord.pConnection->GetCost();
					temporaryRecord.estimatedTotalCost = temporaryRecord.costSoFar + GetHeuristicCost(m_pGraph->GetNode(currentRecord.pConnection->GetFrom()), pGoalNode);
				}
				else
				{
					temporaryRecord.pNode = m_pGraph->GetNode(connection->GetTo());
					temporaryRecord.pConnection = connection;
					temporaryRecord.costSoFar = currentRecord.costSoFar + connection->GetCost();
					temporaryRecord.estimatedTotalCost = temporaryRecord.costSoFar + GetHeuristicCost(currentRecord.pNode, pGoalNode);
				}

				openList.push_back(temporaryRecord);

			}

			vector<NodeRecord>::iterator it = std::find(openList.begin(), openList.end(), currentRecord);
			if (it != openList.end())
			{
				openList.erase(it);
			}
			closedList.push_back(currentRecord);

		}

		while (currentRecord.pNode != pStartNode)
		{
			nPath.push_back(currentRecord.pNode);
			vector<NodeRecord>::iterator it = std::find_if(closedList.begin(), closedList.end(), [currentRecord, this](const auto lhs)
				{
					return lhs.pNode == m_pGraph->GetNode(currentRecord.pConnection->GetFrom());
				});
			if (it != closedList.end())
			{
				currentRecord = *it;
			}
			else
				break;
		}

		nPath.push_back(pStartNode);
		std::reverse(nPath.begin(), nPath.end());
		return nPath;
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}

	template<class T_NodeType, class T_ConnectionType>
	inline bool AStar<T_NodeType, T_ConnectionType>::ParentInSight(T_NodeType* pParentNode, T_NodeType* pNeighbourNode) const
	{
		Vector2 direction{ GetNormalized(m_pGraph->GetNodePos(pParentNode) - m_pGraph->GetNodePos(pNeighbourNode)) };
		Vector2 position{ m_pGraph->GetNodePos(pNeighbourNode)};
		std::vector<T_NodeType*> pNodes{ m_pGraph->GetAllNodes() };
		T_NodeType* closeNode{ nullptr };
		for (T_NodeType* node : pNodes)
		{
			if (m_pGraph->GetNodePos(node) == position)
				closeNode = node;
		}

		while (closeNode != pParentNode)
		{
			position += direction;
			position.x = int(position.x);
			position.y = int(position.y);
			
			for (size_t i{}; i < pNodes.size(); i++)
			{
				if (m_pGraph->GetNodePos(i) == position)
				{
					closeNode = m_pGraph->GetNode(i);
					GridTerrainNode* pNode{ dynamic_cast<GridTerrainNode*>(closeNode) };
					if (pNode && (pNode->GetTerrainType() == TerrainType::Mud || pNode->GetTerrainType() == TerrainType::Water))
						return false;
					break;
				}
			}
		}
		return true;
	}
}