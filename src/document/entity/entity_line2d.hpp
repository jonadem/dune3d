#pragma once
#include "entity.hpp"
#include "ientity_in_workplane.hpp"
#include <glm/glm.hpp>

namespace dune3d {
class EntityLine2D : public Entity, public IEntityInWorkplane {
public:
    explicit EntityLine2D(const UUID &uu);
    explicit EntityLine2D(const UUID &uu, const json &j);
    static constexpr Type s_type = Type::LINE_2D;
    Type get_type() const override
    {
        return s_type;
    }
    json serialize() const override;
    std::unique_ptr<Entity> clone() const override;

    double get_param(unsigned int point, unsigned int axis) const override;
    void set_param(unsigned int point, unsigned int axis, double value) override;

    glm::dvec3 get_point(unsigned int point, const Document &doc) const override;
    bool is_valid_point(unsigned int point) const override;
    glm::dvec2 get_point_in_workplane(unsigned int point) const override;

    glm::dvec2 get_tangent_at_point(unsigned int point) const;

    void accept(EntityVisitor &visitor) const override;

    glm::dvec2 m_p1;
    glm::dvec2 m_p2;
    UUID m_wrkpl;

    const UUID &get_workplane() const override
    {
        return m_wrkpl;
    }

    std::set<UUID> get_referenced_entities() const override;
};

} // namespace dune3d
