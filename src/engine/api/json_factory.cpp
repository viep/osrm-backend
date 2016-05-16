#include "engine/api/json_factory.hpp"

#include "engine/hint.hpp"
#include "engine/polyline_compressor.hpp"
#include "util/integer_range.hpp"

#include "util/guidance/bearing_class.hpp"
#include "util/guidance/entry_class.hpp"
#include "util/guidance/toolkit.hpp"

#include <boost/assert.hpp>
#include <boost/optional.hpp>

#include <algorithm>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

using TurnType = osrm::extractor::guidance::TurnType;
using DirectionModifier = osrm::extractor::guidance::DirectionModifier;
using TurnInstruction = osrm::extractor::guidance::TurnInstruction;

namespace osrm
{
namespace engine
{
namespace api
{
namespace json
{
namespace detail
{

const constexpr char *modifier_names[] = {"uturn",    "sharp right", "right", "slight right",
                                          "straight", "slight left", "left",  "sharp left"};

// translations of TurnTypes. Not all types are exposed to the outside world.
// invalid types should never be returned as part of the API
const constexpr char *turn_type_names[] = {"invalid",
                                           "new name",
                                           "continue",
                                           "turn",
                                           "merge",
                                           "on ramp",
                                           "off ramp",
                                           "fork",
                                           "end of road",
                                           "notification",
                                           "roundabout",
                                           "roundabout",
                                           "rotary",
                                           "rotary",
                                           "roundabout turn",
                                           "roundabout turn",
                                           "invalid",
                                           "invalid",
                                           "invalid",
                                           "invalid",
                                           "invalid",
                                           "invalid",
                                           "invalid",
                                           "invalid",
                                           "invalid"};

const constexpr char *waypoint_type_names[] = {"invalid", "arrive", "depart"};

// Check whether to include a modifier in the result of the API
inline bool isValidModifier(const guidance::StepManeuver maneuver)
{
    if (maneuver.waypoint_type != guidance::WaypointType::None &&
        maneuver.instruction.direction_modifier == DirectionModifier::UTurn)
        return false;
    return true;
}

std::string instructionTypeToString(const TurnType type)
{
    return turn_type_names[static_cast<std::size_t>(type)];
}

std::string instructionModifierToString(const DirectionModifier modifier)
{
    return modifier_names[static_cast<std::size_t>(modifier)];
}

std::string waypointTypeToString(const guidance::WaypointType waypoint_type)
{
    return waypoint_type_names[static_cast<std::size_t>(waypoint_type)];
}

util::json::Array coordinateToLonLat(const util::Coordinate coordinate)
{
    util::json::Array array;
    array.values.push_back(static_cast<double>(toFloating(coordinate.lon)));
    array.values.push_back(static_cast<double>(toFloating(coordinate.lat)));
    return array;
}

util::json::Object getConnection(const bool entry_allowed, const double bearing)
{
    util::json::Object result;
    result.values["entry_allowed"] = entry_allowed ? "true" : "false";
    result.values["bearing"] = bearing;
    return result;
}

util::json::Array getConnections(const util::guidance::EntryClass entry_class,
                                 const util::guidance::BearingClass bearing_class)
{
    util::json::Array result;
    const auto bearings = bearing_class.getAvailableBearings();
    for (size_t connection = 0; connection < bearings.size(); ++connection)
    {
        result.values.push_back(
            getConnection(entry_class.allowsEntry(connection), bearings[connection]));
    }
    return result;
}

util::json::Object getIntersection(const guidance::StepManeuver maneuver)
{
    util::json::Object result;
    // bearings are oriented in the direction of driving. For the in-bearing, we actually need to
    // find the bearing from the view of the intersection. This means we have to rotate the bearing
    // by 180 degree.
    const auto rotated_bearing_before = (maneuver.bearing_before >= 180.0)
                                            ? (maneuver.bearing_before - 180.0)
                                            : (maneuver.bearing_before + 180.0);
    result.values["from_bearing"] =
        getMatchingDiscreteBearing(false, rotated_bearing_before, maneuver.entry_class,
                                   maneuver.bearing_class.getAvailableBearings());
    result.values["to_bearing"] =
        getMatchingDiscreteBearing(true, maneuver.bearing_after, maneuver.entry_class,
                                   maneuver.bearing_class.getAvailableBearings());

    result.values["connections"] = getConnections(maneuver.entry_class, maneuver.bearing_class);
    return result;
}

// FIXME this actually needs to be configurable from the profiles
std::string modeToString(const extractor::TravelMode mode)
{
    std::string token;
    switch (mode)
    {
    case TRAVEL_MODE_INACCESSIBLE:
        token = "inaccessible";
        break;
    case TRAVEL_MODE_DRIVING:
        token = "driving";
        break;
    case TRAVEL_MODE_CYCLING:
        token = "cycling";
        break;
    case TRAVEL_MODE_WALKING:
        token = "walking";
        break;
    case TRAVEL_MODE_FERRY:
        token = "ferry";
        break;
    case TRAVEL_MODE_TRAIN:
        token = "train";
        break;
    case TRAVEL_MODE_PUSHING_BIKE:
        token = "pushing bike";
        break;
    case TRAVEL_MODE_STEPS_UP:
        token = "steps up";
        break;
    case TRAVEL_MODE_STEPS_DOWN:
        token = "steps down";
        break;
    case TRAVEL_MODE_RIVER_UP:
        token = "river upstream";
        break;
    case TRAVEL_MODE_RIVER_DOWN:
        token = "river downstream";
        break;
    case TRAVEL_MODE_ROUTE:
        token = "route";
        break;
    default:
        token = "other";
        break;
    }
    return token;
}

} // namespace detail

util::json::Object makeStepManeuver(const guidance::StepManeuver &maneuver)
{
    util::json::Object step_maneuver;
    if (maneuver.waypoint_type == guidance::WaypointType::None)
    {
        step_maneuver.values["type"] = detail::instructionTypeToString(maneuver.instruction.type);
        step_maneuver.values["intersection"] = detail::getIntersection(maneuver);
    }
    else
        step_maneuver.values["type"] = detail::waypointTypeToString(maneuver.waypoint_type);

    if (detail::isValidModifier(maneuver))
        step_maneuver.values["modifier"] =
            detail::instructionModifierToString(maneuver.instruction.direction_modifier);

    step_maneuver.values["location"] = detail::coordinateToLonLat(maneuver.location);
    step_maneuver.values["bearing_before"] = std::round(maneuver.bearing_before);
    step_maneuver.values["bearing_after"] = std::round(maneuver.bearing_after);
    if (maneuver.exit != 0)
        step_maneuver.values["exit"] = maneuver.exit;

    // TODO currently we need this to comply with the api.
    // We should move this to an additional entry, the moment we
    // actually compute the correct locations of the intersections
    if (!maneuver.intersections.empty() && maneuver.exit == 0)
        step_maneuver.values["exit"] = maneuver.intersections.size();

    return step_maneuver;
}

util::json::Object makeRouteStep(guidance::RouteStep step, util::json::Value geometry)
{
    util::json::Object route_step;
    route_step.values["distance"] = std::round(step.distance * 10) / 10.;
    route_step.values["duration"] = std::round(step.duration * 10) / 10.;
    route_step.values["name"] = std::move(step.name);
    if (!step.rotary_name.empty())
        route_step.values["rotary_name"] = std::move(step.rotary_name);

    route_step.values["mode"] = detail::modeToString(std::move(step.mode));
    route_step.values["maneuver"] = makeStepManeuver(std::move(step.maneuver));
    route_step.values["geometry"] = std::move(geometry);
    return route_step;
}

util::json::Object makeRoute(const guidance::Route &route,
                             util::json::Array legs,
                             boost::optional<util::json::Value> geometry)
{
    util::json::Object json_route;
    json_route.values["distance"] = std::round(route.distance * 10) / 10.;
    json_route.values["duration"] = std::round(route.duration * 10) / 10.;
    json_route.values["legs"] = std::move(legs);
    if (geometry)
    {
        json_route.values["geometry"] = *std::move(geometry);
    }
    return json_route;
}

util::json::Object makeWaypoint(const util::Coordinate location, std::string name, const Hint &hint)
{
    util::json::Object waypoint;
    waypoint.values["location"] = detail::coordinateToLonLat(location);
    waypoint.values["name"] = std::move(name);
    waypoint.values["hint"] = hint.ToBase64();
    return waypoint;
}

util::json::Object makeRouteLeg(guidance::RouteLeg leg, util::json::Array steps)
{
    util::json::Object route_leg;
    route_leg.values["distance"] = std::round(leg.distance * 10) / 10.;
    route_leg.values["duration"] = std::round(leg.duration * 10) / 10.;
    route_leg.values["summary"] = std::move(leg.summary);
    route_leg.values["steps"] = std::move(steps);
    return route_leg;
}

util::json::Array makeRouteLegs(std::vector<guidance::RouteLeg> legs,
                                std::vector<util::json::Value> step_geometries)
{
    util::json::Array json_legs;
    auto step_geometry_iter = step_geometries.begin();
    for (const auto idx : util::irange<std::size_t>(0UL, legs.size()))
    {
        auto leg = std::move(legs[idx]);
        util::json::Array json_steps;
        json_steps.values.reserve(leg.steps.size());
        std::transform(
            std::make_move_iterator(leg.steps.begin()), std::make_move_iterator(leg.steps.end()),
            std::back_inserter(json_steps.values), [&step_geometry_iter](guidance::RouteStep step) {
                return makeRouteStep(std::move(step), std::move(*step_geometry_iter++));
            });
        json_legs.values.push_back(makeRouteLeg(std::move(leg), std::move(json_steps)));
    }
    return json_legs;
}
} // namespace json
} // namespace api
} // namespace engine
} // namespace osrm
