#ifndef OSRM_EXTRACTOR_GUIDANCE_TURN_LANE_MATCHER_HPP_
#define OSRM_EXTRACTOR_GUIDANCE_TURN_LANE_MATCHER_HPP_

#include "extractor/guidance/intersection.hpp"
#include "util/name_table.hpp"
#include "util/node_based_graph.hpp"

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
  public:
    TurnLaneMatcher(const util::NodeBasedDynamicGraph &node_based_graph,
                    const util::NameTable &turn_lane_strings);

    void assignTurnLanes(const EdgeID via_edge, Intersection Intersection) const;

  private:
    const util::NodeBasedDynamicGraph &node_based_graph;
    const util::NameTable &turn_lane_strings;
};

} // namespace guidance
} // namespace extractor
} // namespace osrm

#endif // OSRM_EXTRACTOR_GUIDANCE_TURN_LANE_MATCHER_HPP_
