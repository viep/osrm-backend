#ifndef OSRM_UTIL_GUIDANCE_TOOLKIT_HPP_
#define OSRM_UTIL_GUIDANCE_TOOLKIT_HPP_

/* A set of tools required for guidance in both pre and post-processing */

#include "engine/guidance/route_step.hpp"
#include "engine/phantom_node.hpp"
#include "extractor/guidance/turn_instruction.hpp"
#include "util/guidance/bearing_class.hpp"
#include "util/guidance/entry_class.hpp"
#include "util/simple_logger.hpp"

#include <algorithm>
#include <vector>

namespace osrm
{
namespace util
{
namespace guidance
{

inline double angularDeviation(const double angle, const double from)
{
    const double deviation = std::abs(angle - from);
    return std::min(360 - deviation, deviation);
}

inline extractor::guidance::DirectionModifier getTurnDirection(const double angle)
{
    // An angle of zero is a u-turn
    // 180 goes perfectly straight
    // 0-180 are right turns
    // 180-360 are left turns
    if (angle > 0 && angle < 60)
        return extractor::guidance::DirectionModifier::SharpRight;
    if (angle >= 60 && angle < 140)
        return extractor::guidance::DirectionModifier::Right;
    if (angle >= 140 && angle < 170)
        return extractor::guidance::DirectionModifier::SlightRight;
    if (angle >= 165 && angle <= 195)
        return extractor::guidance::DirectionModifier::Straight;
    if (angle > 190 && angle <= 220)
        return extractor::guidance::DirectionModifier::SlightLeft;
    if (angle > 220 && angle <= 300)
        return extractor::guidance::DirectionModifier::Left;
    if (angle > 300 && angle < 360)
        return extractor::guidance::DirectionModifier::SharpLeft;
    return extractor::guidance::DirectionModifier::UTurn;
}

} // namespace guidance
} // namespace util
} // namespace osrm

#endif /* OSRM_UTIL_GUIDANCE_TOOLKIT_HPP_ */
