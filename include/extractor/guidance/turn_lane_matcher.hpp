#ifndef OSRM_EXTRACTOR_GUIDANCE_TURN_LANE_MATCHER_HPP_
#define OSRM_EXTRACTOR_GUIDANCE_TURN_LANE_MATCHER_HPP_

#include "extractor/guidance/intersection.hpp"
#include "extractor/guidance/turn_analysis.hpp"
#include "extractor/query_node.hpp"
#include "util/name_table.hpp"
#include "util/node_based_graph.hpp"
#include "util/typedefs.hpp"
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace osrm
{
namespace extractor
{
namespace guidance
{

// Given an Intersection, the graph to access the data and  the turn lanes, the turn lane matcher
// assigns appropriate turn tupels to the different turns.

class TurnLaneMatcher
{
    struct TurnLaneData
    {
        std::string tag;
        int from;
        int to;

        inline bool operator<(const TurnLaneData &other) const
        {
            return from < other.from || (from == other.from && to < other.to);
        }
    };

  public:
    TurnLaneMatcher(const util::NodeBasedDynamicGraph &node_based_graph,
                    const util::NameTable &turn_lane_strings,
                    const std::vector<QueryNode> &node_info_list,
                    const TurnAnalysis &turn_analysis);

    void assignTurnLanes(const EdgeID via_edge, Intersection intersection) const;

  private:
    typedef std::vector<TurnLaneData> LaneDataVector;
    typedef std::map<std::string, std::pair<int, int>> LaneMap;
    const util::NodeBasedDynamicGraph &node_based_graph;
    const util::NameTable &turn_lane_strings;
    const std::vector<QueryNode> &node_info_list;
    const TurnAnalysis &turn_analysis;

    bool isSimpleIntersection(const std::string &turn_lane_string,
                              const Intersection &intersection) const;

    LaneDataVector handleNoneValueAtSimpleTurn(const NodeID at,
                                   LaneDataVector lane_data,
                                   const Intersection &intersection) const;
};

} // namespace guidance
} // namespace extractor
} // namespace osrm

#endif // OSRM_EXTRACTOR_GUIDANCE_TURN_LANE_MATCHER_HPP_
