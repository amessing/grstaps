//
// Created by andrew on 9/11/20.
//
#include "grstaps/json_conversions.hpp"

void from_json(const nlohmann::json& j, b2Vec2& v)
{
    v.Set(j["x"], j["y"]);
}

void to_json(nlohmann::json& j, const b2Vec2& v)
{
    j = {
        {"x", v.x},
        {"y", v.y}
    };
}

void from_json(const nlohmann::json& j, b2PolygonShape& p)
{
    auto vertices = j.get<std::vector<b2Vec2>>();
    p.Set(vertices.data(), vertices.size());
}

void to_json(nlohmann::json& j, const b2PolygonShape& p)
{
    std::vector<b2Vec2> vertices;
    vertices.reserve(p.m_count);
    for(uint i = 0; i < p.m_count; ++i)
    {
        vertices.push_back(p.m_vertices[i]);
    }
    j = vertices;
}

namespace ClipperLib2
{
    void from_json(const nlohmann::json& j, ClipperLib2::IntPoint& p)
    {
        p.X = j["x"];
        p.Y = j["y"];
    }
    void to_json(nlohmann::json& j, const ClipperLib2::IntPoint& p)
    {
        j["x"] = p.X;
        j["y"] = p.Y;
    }
}