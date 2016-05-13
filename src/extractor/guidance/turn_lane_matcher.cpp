#include "extractor/guidance/turn_lane_matcher.hpp"

namespace osrm
{
namespace extractor
{
namespace guidance
{

TurnLaneMatcher::TurnLaneMatcher(const util::NodeBasedDynamicGraph &node_based_graph,
                                 const util::NameTable &turn_lane_strings)
    : node_based_graph(node_based_graph), turn_lane_strings(turn_lane_strings)
{
}

void TurnLaneMatcher::assignTurnLanes(const EdgeID via_edge, Intersection Intersection) const
{
    for( const auto &turn : Intersection )
    {
        const auto &data = node_based_graph.GetEdgeData(turn.turn.eid);
        if( 
        const auto lane_string = turn_lane_strings.GetEdgeData(data.lane_id);
        std::cout << "Lane Data: " << turn_lane_strings.GetNameForID(data.lane_id) << std::endl;
    }
}

} // namespace guidance
} // namespace extractor
} // namespace osrm
