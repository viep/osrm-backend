#include "extractor/guidance/turn_lane_matcher.hpp"

#include <cstdint>
#include <map>
#include <utility>

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
    const auto turn_lane_string = turn_lane_strings.GetNameForID(data.lane_id);

    if (turn_lane_string.empty())
    {
        for (auto &road : intersection)
        {
            road.turn.instruction.lane_tupel_id = 0;
        }
        return;
    }

    // the list of allowed entries for turn_lane strings
    const constexpr char *turn_directions[] = {
        "left",        "slight_left", "sharp_left",    "through",        "right", "slight_right",
        "sharp_right", "reverse",     "merge_to_left", "merge_to_right", "none"};

    auto getNextTag = [](std::string &string, const char separator) {
        auto pos = string.find_last_of(separator);
        auto result = pos != std::string::npos ? string.substr(pos + 1) : string;

        string.resize(pos == std::string::npos ? 0 : pos);
        return result;
    };

    auto setLaneData = [&](std::map<std::string, std::pair<int, int>> &map, std::string lane, const int current_lane) {
        do
        {
            auto identifier = getNextTag(lane,';');
            if( identifier.empty() )
                identifier = "none";
            auto map_iterator = map.find(identifier);
            if( map_iterator == map.end() )
                map[identifier] = std::make_pair(current_lane,current_lane);
            else
                map_iterator->second.second = current_lane;
        }while(!lane.empty());
    };

    auto lane_map = [&](std::string lane_string) {
        std::map<std::string, std::pair<int, int>> result;
        auto num_lanes = std::count(lane_string.begin(),lane_string.end(),'|')+1;
        int lane_nr = 0;
        do
        {
            auto lane = getNextTag(lane_string,'|');
            setLaneData(result,lane,lane_nr);
            ++lane_nr;
        } while (lane_nr < num_lanes);

        return result;
    }(turn_lane_string);

    for( auto tag : lane_map )
        std::cout << "Lane Information: " << tag.first << " " << tag.second.first << "-" << tag.second.second << std::endl;

    std::cout << "Lane Data: " << turn_lane_strings.GetNameForID(data.lane_id) << std::endl;
    for (const auto &turn : intersection)
    {
        std::cout << "Turn: " << toString(turn) << std::endl;
    }
}

} // namespace guidance
} // namespace extractor
} // namespace osrm
