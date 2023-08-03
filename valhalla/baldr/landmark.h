#pragma once

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>

#include <valhalla/baldr/graphconstants.h>

namespace valhalla {
namespace baldr {

// obvious landmark types for vehicle routing
enum class LandmarkType : uint8_t {
  // these will almost always be obvious by their function and don't require a name
  fuel = 1,
  post_office = 2,
  police = 3,
  fire_station = 4,
  car_wash = 5,
  // could be obvious if its a chain but could also just be a building
  restaurant = 6,
  // this will always be a chain in which case the name is a required distinguisher (as they are often
  // next to each other on either side of a street)
  fast_food = 7,
  // this could be a chain making it easier to distinguish (starbucks) but could be a one off cafe in
  // which case the name will be required
  cafe = 8,
  // on the fence about this one, these are often obvious without a name, maybe should go in the upper
  // list?
  bank = 10,
  // same with this one, in some countries these are obvious and have the little plus sign, in america
  // its not the case and they can be different chains or could be part of a grocery store or
  // anything... speaking of grocery store, why isnt that on the list? also why arent department
  // stores on the list, surely stores would be a good landmark to use (again when they are named)
  pharmacy = 11,
  // again these could just be a random building with nothing other than a sign like "early years" or
  // something, they better have a name or it will be ambiguous
  kindergarten = 12,
  // same thing, often just a building
  bar = 13,
  // this could be very very big like a college campus, i think we should remove this
  hospital = 14,
  // same as bar could be just a building need at least a name to have a chance of seeing it
  pub = 16,
  // generic, could be many many types of clinics also often there are multiple of these located in
  // same "complex", you might have a dentist, orthodontist, ear nose throat and eye specialists all
  // next to each other.
  clinic = 17,
  // on the fence about this one, to me this would be in the same category as "museum" or something
  // where its so specialized maybe it doesnt need a name. what keeps me from say ing that is you
  // often cant tell from the outside what function these amenities serve until you read the name
  theatre = 16,
  // this could work without a name in my opinion, they often have a board out front that shows what
  // movies are playing which would probably remove the need for a name
  cinema = 17,
  // these are usually quite large as well but not usually multiple buildings, how often are they
  // located near one another though? if its often perhaps we need a name to differentiate between
  // this casino and the next one down on the next block? maybe that argument is a good argument for
  // everything has to have a name?
  casino = 18,
};

inline LandmarkType string_to_landmark_type(const std::string& s) {
  static const std::unordered_map<std::string, LandmarkType> string_to_landmark_type =
      {{"restaurant", LandmarkType::restaurant},
       {"fast_food", LandmarkType::fast_food},
       {"cafe", LandmarkType::cafe},
       {"fuel", LandmarkType::fuel},
       {"bank", LandmarkType::bank},
       {"pharmacy", LandmarkType::pharmacy},
       {"kindergarten", LandmarkType::kindergarten},
       {"bar", LandmarkType::bar},
       {"hospital", LandmarkType::hospital},
       {"post_office", LandmarkType::post_office},
       {"pub", LandmarkType::pub},
       {"clinic", LandmarkType::clinic},
       {"police", LandmarkType::police},
       {"fire_station", LandmarkType::fire_station},
       {"car_wash", LandmarkType::car_wash},
       {"theatre", LandmarkType::theatre},
       {"cinema", LandmarkType::cinema},
       {"casino", LandmarkType::casino}};

  auto it = string_to_landmark_type.find(s);
  if (it == string_to_landmark_type.cend()) {
    throw std::runtime_error("unknown landmark type");
  }
  return it->second;
}

struct Landmark {
  int64_t id;
  std::string name;
  LandmarkType type;
  double lng;
  double lat;

  std::string to_str() const;
  Landmark from_str(const std::string& str) const;
};

/**
 * Convert a Landmark object to a string which consists of 1 byte for kLandmark tag,
 * 1 byte for landmark type, 4 bytes for location (lng and lat), and landmark name at last.
 *
 * @param landmark The Landmark object to be converted.
 */
inline std::string Landmark::to_str() const {
  std::string tagged_value(1, static_cast<char>(baldr::TaggedValue::kLandmark));
  tagged_value.push_back(static_cast<std::string::value_type>(type));
  uint32_t location = uint32_t((lng + 180) * 1e7) << 16 | uint32_t((lat + 90) * 1e7)
                                                              << 1; // leaves one spare bit
  tagged_value += std::string(static_cast<const char*>(static_cast<void*>(&location)), 4);
  tagged_value += name;

  return tagged_value;
}

/**
 * Convert a string to a Landmark object.
 * The input string should consist of at least 6 bytes: 1 byte for kLandmark tag,
 * 1 byte for landmark type, 4 bytes for location (lng and lat), and the remaining for landmark name.
 *
 * @param str The string to be converted.
 */
inline Landmark Landmark::from_str(const std::string& str) const {
  // ensure that the string has the minimum expected size to represent a Landmark
  if (str.size() < 6) { // 6 = 1 + 1 + 4, name may be null
    throw std::runtime_error("Invalid Landmark string: too short");
  }

  // ensure that the first byte is the kLandmark tag
  if (str[0] != static_cast<char>(baldr::TaggedValue::kLandmark)) {
    throw std::runtime_error("Invalid Landmark string: missing kLandmark tag");
  }

  // extract the LandmarkType (second byte in the string)
  LandmarkType landmark_type = static_cast<LandmarkType>(static_cast<uint8_t>(str[1]));

  // extract the location (next 4 bytes) - lng and lat
  uint32_t location = 0;
  std::memcpy(&location, str.data() + 2, 4);
  double lng = (static_cast<double>((location >> 16) & 0x7FFFFFFF) / 1e7) - 180;
  double lat = (static_cast<double>((location >> 1) & 0x7FFFFFFF) / 1e7) - 90;

  // extract the name (rest of the string after the first 6 bytes)
  std::string name = str.substr(6);

  return Landmark{0, name, landmark_type, lng, lat}; // note: fake id should not be used!
}

} // namespace baldr
} // namespace valhalla
