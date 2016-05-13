#ifndef OSRM_UTIL_GUIDANCE_TURN_LANES_HPP
#define OSRM_UTIL_GUIDANCE_TURN_LANES_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

#include <boost/functional/hash.hpp>

namespace osrm
{
namespace util
{
namespace guidance
{
class LaneTupel;
} // namespace guidance
} // namespace util
} // namespace osrm

namespace std
{
template <> struct hash<::osrm::util::guidance::LaneTupel>
{
    inline std::size_t operator()(const ::osrm::util::guidance::LaneTupel &bearing_class) const;
};
} // namespace std

namespace osrm
{
namespace util
{
namespace guidance
{

// The mapping of turn lanes can be done using a tupel as described here.
// Given a road like this:
//           |   |   |
//           |   |   |
// -----------       |
//      -^           |
// -----------        -------------
//      -^ ->
// --------------------------------
//      -v       |
// ----------    |
//          |    |
//
// we generate a set of tuples in the form of:
//
// <3,2,1,0> Three lanes at the turn, two allow turning left, the first one is lane 1 (0,1,2) and it
// maps to lanes 0,1 at the destination
// <1,1,1,0> A single lane goes straight
// <1,1,1,0> A single lane turns right

class LaneTupel
{
  public:
    LaneTupel();
    LaneTupel(std::uint8_t lanes_at_turn,
              std::uint8_t lanes_in_turn,
              std::uint8_t first_lane_from_the_right);

    bool operator==(const LaneTupel other) const;
    bool operator<(const LaneTupel other) const;

//  private:
    std::uint8_t lanes_at_turn;
    std::uint8_t lanes_in_turn;
    std::uint8_t first_lane_from_the_right;
    std::uint8_t padding;

    friend std::size_t std::hash<LaneTupel>::operator()(const LaneTupel &) const;
};

std::vector<LaneTupel> makeLaneTupels(const std::string &turn_lane_string);

} // namespace guidance
} // namespace util
} // namespace osrm



// make Bearing Class hasbable
namespace std
{
inline size_t hash<::osrm::util::guidance::LaneTupel>::
operator()(const ::osrm::util::guidance::LaneTupel &lane_tupel) const
{
    return boost::hash_value(*reinterpret_cast<const std::uint32_t *>(&lane_tupel));
}
} // namespace std

#endif /* OSRM_UTIL_GUIDANCE_TURN_LANES_HPP */
