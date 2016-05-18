#include "extractor/guidance/constants.hpp"
#include "extractor/guidance/turn_lane_matcher.hpp"

#include <cstdint>
#include <iomanip>

#include <boost/algorithm/string/predicate.hpp>

namespace osrm
{
namespace extractor
{
namespace guidance
{

TurnLaneMatcher::TurnLaneMatcher(const util::NodeBasedDynamicGraph &node_based_graph,
                                 const util::NameTable &turn_lane_strings,
                                 const std::vector<QueryNode> &node_info_list,
                                 const TurnAnalysis &turn_analysis)
    : node_based_graph(node_based_graph), turn_lane_strings(turn_lane_strings),
      node_info_list(node_info_list), turn_analysis(turn_analysis)
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
            road.turn.instruction.lane_tupel = {INVALID_LANEID, INVALID_LANEID};

        return;
    }

    // the list of allowed entries for turn_lane strings
    const constexpr char *turn_directions[] = {
        "left",        "slight_left", "sharp_left",    "through",        "right", "slight_right",
        "sharp_right", "reverse",     "merge_to_left", "merge_to_right", "none"};

    auto getNextTag = [](std::string &string, const char *separators) {
        auto pos = string.find_last_of(separators);
        auto result = pos != std::string::npos ? string.substr(pos + 1) : string;

        string.resize(pos == std::string::npos ? 0 : pos);
        return result;
    };

    auto setLaneData = [&](LaneMap &map, std::string lane, const int current_lane) {
        do
        {
            auto identifier = getNextTag(lane, ";");
            if (identifier.empty())
                identifier = "none";
            auto map_iterator = map.find(identifier);
            if (map_iterator == map.end())
                map[identifier] = std::make_pair(current_lane, current_lane);
            else
            {
                map_iterator->second.second = current_lane;
            }
        } while (!lane.empty());
    };

    auto lane_data = [&](std::string lane_string) {
        LaneMap lane_map;
        // FIXME this is a workaround due to https://github.com/cucumber/cucumber-js/issues/417,
        // need to switch statements when fixed
        // const auto num_lanes = std::count(lane_string.begin(), lane_string.end(), '|') + 1;
        const auto num_lanes = std::count(lane_string.begin(), lane_string.end(), '|') + 1 +
                               std::count(lane_string.begin(), lane_string.end(), '&');
        int lane_nr = 0;
        do
        {
            // FIXME this is a cucumber workaround, since escaping does not work properly in
            // cucumber.js (see https://github.com/cucumber/cucumber-js/issues/417). Needs to be
            // changed to "|" only, when the bug is fixed
            auto lane = getNextTag(lane_string, "|&");
            setLaneData(lane_map, lane, lane_nr);
            ++lane_nr;
        } while (lane_nr < num_lanes);

        LaneDataVector lane_data;
        for (const auto tag : lane_map)
        {
            lane_data.push_back({tag.first, tag.second.first, tag.second.second});
        }

        std::sort(lane_data.begin(), lane_data.end());

        return lane_data;
    }(turn_lane_string);

    // might be reasonable to handle multiple turns, if we know of a sequence of lanes
    // e.g. one direction per lane, if three lanes and right, through, left available
    if (lane_data.size() == 1 && lane_data[0].tag == "none")
    {
        for (auto &road : intersection)
            road.turn.instruction.lane_tupel = {INVALID_LANEID, INVALID_LANEID};
        return;
    }

    // check whether we are at a simple intersection
    const bool is_simple = isSimpleIntersection(lane_data, intersection);

    auto node = node_based_graph.GetTarget(via_edge);
    auto coordinate = node_info_list[node];

    if (is_simple)
        lane_data = handleNoneValueAtSimpleTurn(node_based_graph.GetTarget(via_edge),
                                                std::move(lane_data), intersection);
    else
    {

        /*  FIXME
            We need to check whether the turn lanes have to be propagated further at some points.
            Some turn lanes are given on a segment prior to the one where the turn actually happens.
            These have to be pushed along to the segment where the data is actually used.

                      |    |           |    |
            ----------      -----------      -----



            ----------      -----------      -----
                      |    |           |    |
                      | vv |           | ^^ |
            ----------      -----------      ------
             (a)-----^
             (b)----->
             (c)-----v
            ----------      -----------      ------
                     |      |          |     |

            Both (a) and (b) are targeting not only the intersection they are at. The correct
           representation for routing is:

                      |    |           |    |
            ----------      -----------      -----



            ----------      -----------      -----
                      |    |           |    |
                      | vv |           | ^^ |
            ----------      -----------      ------
             (a)-------------------------^
             (b)----->      ---------->
             (c)-----v
            ----------      -----------      ------
                     |      |          |     |


        */

        /*
        std::cout << "Skipping complex intersection, for now (" << std::setprecision(12)
                  << util::toFloating(coordinate.lat) << " " << util::toFloating(coordinate.lon)
                  << ") " << turn_lane_string << std::endl;
        */
        for (auto &road : intersection)
            road.turn.instruction.lane_tupel = {INVALID_LANEID, INVALID_LANEID};

        return;
    }

    // Now assign the turn lanes to their respective turns

    /*
    std::cout << "Location: " << std::setprecision(12) << util::toFloating(coordinate.lat) << " "
              << util::toFloating(coordinate.lon) << std::endl;
    for (auto tag : lane_data)
        std::cout << "Lane Information: " << tag.tag << " " << tag.from << "-" << tag.to
                  << std::endl;

    std::cout << "Lane Data: " << turn_lane_strings.GetNameForID(data.lane_id) << std::endl;
    for (const auto &turn : intersection)
    {
        std::cout << "Turn: " << toString(turn) << std::endl;
    }
    */
}

/*
    Lanes can have the tag none. While a nice feature for visibility, it is a terrible feature for
   parsing. None can be part of neighboring turns, or not. We have to look at both the intersection
   and the lane data to see what turns we have to augment by the none-lanes
 */
TurnLaneMatcher::LaneDataVector TurnLaneMatcher::handleNoneValueAtSimpleTurn(
    const NodeID at, LaneDataVector lane_data, const Intersection &intersection) const
{
    bool has_right = false;
    bool has_through = false;
    bool has_left = false;
    std::size_t connection_count = 0;
    for (const auto &road : intersection)
    {
        if (!road.entry_allowed)
            continue;

        ++connection_count;
        const auto modifier = road.turn.instruction.direction_modifier;
        has_right |= modifier == DirectionModifier::Right;
        has_right |= modifier == DirectionModifier::SlightRight;
        has_right |= modifier == DirectionModifier::SharpRight;
        has_through |= modifier == DirectionModifier::Straight;
        has_left |= modifier == DirectionModifier::Left;
        has_left |= modifier == DirectionModifier::SlightLeft;
        has_left |= modifier == DirectionModifier::SharpLeft;
    }

    if (intersection[0].entry_allowed)
        --connection_count;

    const constexpr char *tag_by_modifier[] = {"reverse", "sharp_right", "right", "slight_right",
                                               "through", "slight_left", "left",  "sharp_left"};

    /*
    if (lane_data.size() < connection_count)
    {
        std::cout << "Input" << std::endl;
        for (auto tag : lane_data)
            std::cout << "Lane Information: " << tag.tag << " " << tag.from << "-" << tag.to
                      << std::endl;
    }
    */

    // TODO check for impossible turns to see whether the turn lane is at the correct place

    for (std::size_t index = 0; index < lane_data.size(); ++index)
    {
        if (lane_data[index].tag == "none")
        {
            bool print = false;
            // we have to create multiple turns
            if (connection_count > lane_data.size())
            {
                // a none-turn is allowing multiple turns. we have to add a lane-data entry for
                // every possible turn
            }
            // we have to reduce it, assigning it to neighboring turns
            else if (connection_count < lane_data.size())
            {
                // a prerequisite is simple turns. Larger differences should not end up here
                BOOST_ASSERT(connection_count + 1 == lane_data.size());
                // an additional line at the side is only reasonable if it is targeting public
                // service vehicles. Otherwise, we should not have it
                // TODO what about lane numbering. Should we count differently?
                if (index == 0 || index + 1 == lane_data.size())
                {
                    lane_data.erase(lane_data.begin() + index);
                }
                else if (lane_data[index].to - lane_data[index].from <= 1)
                {
                    lane_data[index - 1].to = lane_data[index].from;
                    lane_data[index + 1].from = lane_data[index].to;

                    lane_data.erase(lane_data.begin() + index);
                }
            }
            // we have to rename and possibly augment existing ones. The pure count remains the
            // same.
            else
            {
                // find missing tag and augment neighboring, if possible
                if (index == 0)
                {
                    if (has_right &&
                        (lane_data.size() == 1 || (lane_data[index + 1].tag != "sharp_right" &&
                                                   lane_data[index + 1].tag != "right")))
                    {
                        lane_data[index].tag = "right";
                        if (lane_data.size() > 1 && lane_data[index + 1].tag == "through")
                        {
                            lane_data[index + 1].from = lane_data[index].from;
                            // turning right through a possible through lane is not possible
                            lane_data[index].to = lane_data[index].from;
                        }
                    }
                }
                else if (index + 1 == lane_data.size())
                {
                    if (has_left &&
                        (lane_data.size() == 1 || (lane_data[index + 1].tag != "sharp_left" &&
                                                   lane_data[index + 1].tag != "left")))
                    {
                        lane_data[index].tag = "left";
                        if (lane_data[index - 1].tag == "through")
                        {
                            lane_data[index - 1].to = lane_data[index].to;
                            // turning left through a possible through lane is not possible
                            lane_data[index].from = lane_data[index].to;
                        }
                    }
                }
                else
                {
                    print = true;
                    std::cout << "Input [" << index << "]" << std::endl;
                    for (auto tag : lane_data)
                        std::cout << "Lane Information: " << tag.tag << " " << tag.from << "-"
                                  << tag.to << std::endl;

                    if ((lane_data[index + 1].tag == "left" ||
                         lane_data[index + 1].tag == "slight_left" ||
                         lane_data[index + 1].tag == "sharp_left") &&
                        (lane_data[index - 1].tag == "right" ||
                         lane_data[index - 1].tag == "slight_right" ||
                         lane_data[index - 1].tag == "sharp_right"))
                    {
                        lane_data[index].tag = "through";
                    }
                }
            }
            std::sort(lane_data.begin(), lane_data.end());

            if (print)
            {
                auto coordinate = node_info_list[at];

                std::cout << "Location: " << std::setprecision(12)
                          << util::toFloating(coordinate.lat) << " "
                          << util::toFloating(coordinate.lon) << std::endl;

                std::cout << "Output" << std::endl;
                for (auto tag : lane_data)
                    std::cout << "Lane Information: " << tag.tag << " " << tag.from << "-" << tag.to
                              << std::endl;

                for (const auto &turn : intersection)
                {
                    std::cout << "Turn: " << toString(turn) << std::endl;
                }
            }
            break;
        }
    }

    // BOOST_ASSERT( lane_data.size() + 1 >= intersection.size() );
    return lane_data;
}

/* A simple intersection does not depend on the next intersection coming up. This is important for
 * turn lanes, since traffic signals and/or segregated intersections can influence the
 * interpretation of turn-lanes at a given turn.
 *
 * Here we check for simple Intersections. A simple intersection has a long enough segment following
 * the turn, offers no straight turn, or only non-trivial turn operations.
 */
bool TurnLaneMatcher::isSimpleIntersection(const LaneDataVector &lane_data,
                                           const Intersection &intersection) const
{
    // if we are on a straight road, turn lanes are only reasonable in connection to the next
    // intersection, or in case of a merge. If not all but one (straight) are merges, we don't
    // consider the intersection simple
    if (intersection.size() == 2)
        return std::count_if(
                   lane_data.begin(), lane_data.end(),
                   [](const TurnLaneData &data) { return boost::starts_with(data.tag, "merge"); }) +
                   std::size_t{1} >=
               lane_data.size();

    // in case an intersection offers far more lane data items than actual turns, some of them have
    // to be for another intersection. A single additional item can be for an invalid bus lane.
    const auto num_turns =
        std::count_if(intersection.begin(), intersection.end(),
                      [](const ConnectedRoad &road) { return road.entry_allowed; });

    // more than two additional lane data entries -> lanes target a different intersection
    if (num_turns + std::size_t{2} <= lane_data.size())
        return false;

    // single additional lane data entry is alright, if it is none at the side. This usually refers
    // to a bus-lane
    if (num_turns + std::size_t{1} == lane_data.size())
        return lane_data.front().tag == "none" || lane_data.back().tag == "none";

    // find straightmost turn
    const auto straightmost_index = [&]() {
        const auto itr = std::min_element(intersection.begin(), intersection.end(),
                                          [](const ConnectedRoad &lhs, const ConnectedRoad &rhs) {
                                              return angularDeviation(lhs.turn.angle, 180) <
                                                     angularDeviation(rhs.turn.angle, 180);
                                          });
        return std::distance(intersection.begin(), itr);
    }();

    const auto &straightmost_turn = intersection[straightmost_index];

    // if we onlye have real turns, it cannot be a simple intersection
    // TODO this has to be handled for the ingoing edge as well. Might be we have to get the turn
    // lane string from our predecessor
    if (angularDeviation(straightmost_turn.turn.angle, 180) > NARROW_TURN_ANGLE)
        return false;

    const auto &data = node_based_graph.GetEdgeData(straightmost_turn.turn.eid);
    if (data.distance > 30)
        return true;

    // better save than sorry
    return false;
}

} // namespace guidance
} // namespace extractor
} // namespace osrm
