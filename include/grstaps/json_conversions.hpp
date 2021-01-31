#ifndef GRSTAPS_JSON_CONVERSIONS_HPP
#define GRSTAPS_JSON_CONVERSIONS_HPP

// external
#include <box2d/b2_polygon_shape.h>
#include <nlohmann/json.hpp>
#include <clipper/clipper.hpp>

void from_json(const nlohmann::json& j, b2Vec2& v);
void to_json(nlohmann::json& j, const b2Vec2& v);
void from_json(const nlohmann::json& j, b2PolygonShape& p);
void to_json(nlohmann::json& j, const b2PolygonShape& p);
namespace ClipperLib2
{
    void from_json(const nlohmann::json& j, ClipperLib2::IntPoint& p);
    void to_json(nlohmann::json& j, const ClipperLib2::IntPoint& p);
}
#endif  // GRSTAPS_JSON_CONVERSIONS_HPP
