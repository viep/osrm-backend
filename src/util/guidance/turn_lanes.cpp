#include "util/guidance/turn_lanes.hpp"

#include <algorithm>
#include <iostream>

#include <boost/assert.hpp>

namespace osrm
{
namespace util
{
namespace guidance
{
LaneTupel::LaneTupel()
    : lanes_at_turn(0), lanes_in_turn(0), first_lane_from_the_right(0), padding(0)
{
    // basic constructor, set everything to zero
}

LaneTupel::LaneTupel(std::uint8_t lanes_at_turn,
                     std::uint8_t lanes_in_turn,
                     std::uint8_t first_lane_from_the_right)
    : lanes_at_turn(lanes_at_turn), lanes_in_turn(lanes_in_turn),
      first_lane_from_the_right(first_lane_from_the_right), padding(0)
{
}

// comparation based on interpretation as unsigned 32bit integer
bool LaneTupel::operator==(const LaneTupel other) const
{
    static_assert(sizeof(LaneTupel) == sizeof(std::uint32_t),
                  "Comparation requires LaneTupel to be the of size 32Bit");
    return *reinterpret_cast<const std::uint32_t *>(this) ==
           *reinterpret_cast<const std::uint32_t *>(&other);
}

// comparation based on interpretation as unsigned 32bit integer
bool LaneTupel::operator<(const LaneTupel other) const
{
    static_assert(sizeof(LaneTupel) == sizeof(std::uint32_t),
                  "Comparation requires LaneTupel to be the of size 32Bit");
    return *reinterpret_cast<const std::uint32_t *>(this) <
           *reinterpret_cast<const std::uint32_t *>(&other);
}

std::vector<LaneTupel> makeLaneTupels(const std::string &lane_string)
{
    BOOST_ASSERT(!lane_string.empty());
    if (lane_string.front() == '|' || lane_string.back() == '|')
        std::cout << "Invalid lane string: " << lane_string << std::endl;
 //   BOOST_ASSERT(lane_string.front() != '|' && lane_string.back() != '|');
    // the string we expect has the format of `left|through;right|right`
    // every `|` character represents a new lane. The number of lanes equals the number of
    // separators + 1
    std::vector<LaneTupel> result;

    std::uint8_t num_lanes = std::count(lane_string.begin(), lane_string.end(), '|') + 1;
    std::uint8_t first_lane = 0, lanes_in_turn = 0;

    // tokenize the strings
    if (num_lanes > 1)
    {
        const std::vector<std::string> lanes = [](const std::string &lane_string) {
            std::vector<std::string> result;
            std::size_t pos = 0;
            while (std::string::npos != pos)
            {
                const auto sep = lane_string.find_first_of('|', pos);
                if (sep != std::string::npos)
                {
                    result.push_back(lane_string.substr(pos, sep - pos));
                    pos = sep + 1;
                }
                else
                {
                    result.push_back(lane_string.substr(pos));
                    pos = sep;
                }
            }
            return result;
        }(lane_string);
    }
    else
    {
        result.push_back({num_lanes, first_lane, lanes_in_turn});
    }
/*
    std::cout << "Turn Lane Tupels: " << std::endl;
    for (auto lane : result)
    {
        std::cout << (int)lane.lanes_at_turn << " " << (int)lane.lanes_in_turn << " "
                  << (int)lane.first_lane_from_the_right << std::endl;
    }
*/
    return result;
}

} // namespace guidance
} // namespace util
} // namespace osrm
