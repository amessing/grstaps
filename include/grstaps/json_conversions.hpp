//
// Created by andrew on 9/11/20.
//

#ifndef GRSTAPS_JSON_CONVERSIONS_HPP
#define GRSTAPS_JSON_CONVERSIONS_HPP

// external
#include <box2d/b2_polygon_shape.h>
#include <nlohmann/json.hpp>

void from_json(const nlohmann::json& j, b2Vec2& v);
void to_json(nlohmann::json& j, const b2Vec2& v);
void from_json(const nlohmann::json& j, b2PolygonShape& p);
void to_json(nlohmann::json& j, const b2PolygonShape& p);

#endif  // GRSTAPS_JSON_CONVERSIONS_HPP
