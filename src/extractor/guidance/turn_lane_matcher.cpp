#include "extractor/guidance/turn_lane_matcher.hpp"

#include <cstdint>

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

/*
    Turn lanes are given in the form of strings that closely correspond to the direction modifiers
   we use for our turn types. However, we still cannot simply perform a 1:1 assignment.

    In this function we match the turn lane strings to the actual turns.
    The input contains of a string of the format |left|through;right|right| for a setup like

    ----------
    A -^
    ----------
    B -> -v
    ----------
    C -v
    ----------

    For this setup, we coul get a set of turns of the form

    (130, turn slight right), (180, ramp straight), (320, turn sharp left)

    Here we need to augment to:
    (130, turn slight right, <3,2,0>) (180,ramp straight,<3,1,1>), and (320, turn sharp left,
   <3,1,2>)
 */
void TurnLaneMatcher::assignTurnLanes(const EdgeID via_edge, Intersection intersection) const
{
    const auto &data = node_based_graph.GetEdgeData(via_edge);
    const auto lane_string = turn_lane_strings.GetNameForID(data.lane_id);

    if( lane_string.empty() )
    {
        for( auto &road : intersection )
        {
            road.turn.instruction.lane_tupel_id = 0;
        }
        return;
    }

    // the list of allowed entries for turn_lane strings
    const constexpr char *turn_directions[] = {
        "left",        "slight_left", "sharp_left",    "through",        "right", "slight_right",
        "sharp_right", "reverse",     "merge_to_left", "merge_to_right", "none"};

    std::cout << "Lane Data: " << turn_lane_strings.GetNameForID(data.lane_id) << std::endl;
    for (const auto &turn : intersection)
    {
        std::cout << "Turn: " << toString(turn) << std::endl;
    }
}

} // namespace guidance
} // namespace extractor
} // namespace osrm
