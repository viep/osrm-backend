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
    const auto &data = node_based_graph.GetEdgeData(via_edge);
    const auto lane_string = turn_lane_strings.GetNameForID(data.lane_id);
    std::cout << "Lane Data: " << turn_lane_strings.GetNameForID(data.lane_id) << std::endl;
    for( const auto &turn : Intersection )
    {
        std::cout << "Turn: " << toString(turn) << std::endl;
    }
}

} // namespace guidance
} // namespace extractor
} // namespace osrm
