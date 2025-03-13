#pragma once

#include <glm/glm.hpp>
#include <nlohmann/json_fwd.hpp>

namespace glm
{
    void to_json(nlohmann::json &j, const glm::vec3 &v);
    void from_json(const nlohmann::json &j, glm::vec3 &v);

    void to_json(nlohmann::json &j, const glm::vec4 &v);
    void from_json(const nlohmann::json &j, glm::vec4 &v);

    void to_json(nlohmann::json &j, const glm::quat &q);
    void from_json(const nlohmann::json &j, glm::quat &q);
}