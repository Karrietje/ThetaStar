#pragma once

#include <vector>
#include "framework/EliteGeometry/EGeometry2DTypes.h"
#include "framework/EliteAI/EliteGraphs/EGraphNodeTypes.h"

namespace Elite
{
	//Portal struct (only contains line info atm, you can expand this if needed)
	struct Portal
	{
		Portal() {}
		explicit Portal(const Elite::Line& line) :
			Line(line)
		{}
		Elite::Line Line = {};
	};


	class SSFA final
	{
	public:
		//=== SSFA Functions ===
		//--- References ---
		//http://digestingduck.blogspot.be/2010/03/simple-stupid-funnel-algorithm.html
		//https://gamedev.stackexchange.com/questions/68302/how-does-the-simple-stupid-funnel-algorithm-work
		static std::vector<Portal> FindPortals(
			const std::vector<NavGraphNode*>& nodePath,
			Polygon* navMeshPolygon)
		{
			//Container
			std::vector<Portal> vPortals = {};

			vPortals.push_back(Portal(Line(nodePath[0]->GetPosition(), nodePath[0]->GetPosition())));

			//For each node received, get it's corresponding line
			for (size_t i = 1; i < nodePath.size() - 1; ++i)
			{
				//Local variables
				auto pNode = nodePath[i]; //Store node, except last node, because this is our target node!
				auto pLine = navMeshPolygon->GetLines()[pNode->GetLineIndex()];

				//Redetermine it's "orientation" based on the required path (left-right vs right-left) - p1 should be right point
				auto centerLine = (pLine->p1 + pLine->p2) / 2.0f;
				auto previousPosition = i == 0 ? nodePath[0]->GetPosition() : nodePath[i - 1]->GetPosition();
				auto cp = Cross((centerLine - previousPosition), (pLine->p1 - previousPosition));
				Line portalLine = {};
				if (cp > 0)//Left
					portalLine = Line(pLine->p2, pLine->p1);
				else //Right
					portalLine = Line(pLine->p1, pLine->p2);

				//Store portal
				vPortals.push_back(Portal(portalLine));
			}
			//Add degenerate portal to force end evaluation
			vPortals.push_back(Portal(Line(nodePath[nodePath.size()-1]->GetPosition(), nodePath[nodePath.size() - 1]->GetPosition())));

			return vPortals;
		}

		static std::vector<Elite::Vector2> OptimizePortals(const std::vector<Portal>& portals)
		{
			//P1 == right point of portal, P2 == left point of portal
			std::vector<Elite::Vector2> vPath = {};
			auto apex = portals[0].Line.p1;
			auto apexIndex = 0, leftLegIndex = 1, rightLegIndex = 1;
			auto rightLeg = portals[rightLegIndex].Line.p1 - apex;
			auto leftLeg = portals[leftLegIndex].Line.p2 - apex;

			for (unsigned int i = 1; i < static_cast<unsigned int>(portals.size()); ++i)
			{
				//Local
				const auto &portal = portals[i];

				//--- RIGHT CHECK ---
				//1. See if moving funnel inwards - RIGHT
		
				Elite::Vector2 newRightLeg = portal.Line.p1 - apex; 
				if (Cross(newRightLeg, rightLeg) < 0)
				{
					//2. See if new line degenerates a line segment - RIGHT
					if (Cross(newRightLeg, leftLeg) > 0)
					{
						rightLeg = newRightLeg;
						rightLegIndex = i;
					}
					else
					{
						apex += leftLeg;
						apexIndex = leftLegIndex;
						apexIndex += 1; 
						i = apexIndex;
						rightLegIndex = apexIndex; 
						leftLegIndex = apexIndex; 
						
						if (std::find_if(vPath.begin(), vPath.end(), [&apex](const Elite::Vector2& lhs) { return  lhs == apex; }) == vPath.end())
						{
							vPath.push_back(apex);
						}

						if (i < int(portals.size()))
						{
							rightLeg = portals[rightLegIndex].Line.p1 - apex;
							leftLeg = portals[leftLegIndex].Line.p2 - apex;
							continue;
						}

					}

				}

				//--- LEFT CHECK ---
				//1. See if moving funnel inwards - LEFT
				Elite::Vector2 newLeftLeg = portal.Line.p2 - apex;
				if (Cross(newLeftLeg, leftLeg) > 0)
				{
					//2. See if new line degenerates a line segment - LEFT
					if (Cross(newLeftLeg, rightLeg) < 0)
					{
						leftLeg = newLeftLeg;
						leftLegIndex = i;
					}
					else
					{
						apex += rightLeg;
						apexIndex = rightLegIndex;
						i = leftLegIndex = rightLegIndex = apexIndex + 1;
						if (std::find_if(vPath.begin(), vPath.end(), [&apex](const Elite::Vector2& lhs) { return  lhs == apex; }) == vPath.end())
						{
							vPath.push_back(apex);
						}

						if (i < int(portals.size()))
						{
							rightLeg = portals[rightLegIndex].Line.p1 - apex;
							leftLeg = portals[leftLegIndex].Line.p2 - apex;
							continue;
						}

					}

				}
			}
			
			// Add last path point (You can use the last portal p1 or p2 points as both are equal to the endPoint of the path
			vPath.push_back(portals[portals.size() - 1].Line.p1);
			return vPath;
		}
	private:
		SSFA() {};
		~SSFA() {};
	};
}