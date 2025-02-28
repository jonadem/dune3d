#include "tool_constrain_perpendicular.hpp"
#include "document/document.hpp"
#include "document/entity/entity.hpp"
#include "document/entity/entity_line3d.hpp"
#include "document/constraint/constraint_line_points_perpendicular.hpp"
#include "core/tool_id.hpp"
#include <optional>
#include "util/selection_util.hpp"
#include <iostream>
#include <array>
#include <algorithm>
#include "tool_common_impl.hpp"

namespace dune3d {

struct LineAndPoints {
    UUID line;
    std::array<EntityAndPoint, 2> points;
};

static std::optional<LineAndPoints> line_and_points_from_selection(const Document &doc,
                                                                   const std::set<SelectableRef> &sel)
{
    if (sel.size() != 3)
        return {};
    auto it = sel.begin();
    auto &sr1 = *it++;
    auto &sr2 = *it++;
    auto &sr3 = *it;

    if (sr1.type != SelectableRef::Type::ENTITY)
        return {};
    if (sr2.type != SelectableRef::Type::ENTITY)
        return {};
    if (sr3.type != SelectableRef::Type::ENTITY)
        return {};

    std::array<const SelectableRef *, 3> srs = {&sr1, &sr2, &sr3};

    for (size_t i = 0; i < 6; i++) {
        std::ranges::next_permutation(srs);

        auto &en1 = doc.get_entity(srs.at(0)->item);

        LineAndPoints r;
        if (en1.get_type() == Entity::Type::LINE_3D && srs.at(0)->point == 0) {
            r.line = en1.m_uuid;
            if (srs.at(1)->point == 0)
                return {};
            if (srs.at(2)->point == 0)
                return {};
            r.points.at(0).entity = srs.at(1)->item;
            r.points.at(0).point = srs.at(1)->point;
            r.points.at(1).entity = srs.at(2)->item;
            r.points.at(1).point = srs.at(2)->point;
            return r;
        }
    }


    return {};
}

bool ToolConstrainPerpendicular::can_begin()
{
    return line_and_points_from_selection(get_doc(), m_selection).has_value();
}

ToolResponse ToolConstrainPerpendicular::begin(const ToolArgs &args)
{
    auto &doc = get_doc();
    if (auto tp = line_and_points_from_selection(doc, m_selection)) {
        auto &line = doc.get_entity<EntityLine3D>(tp->line);
        for (const auto lpt : {1, 2}) {
            auto line_point = line.get_point(lpt, doc);
            for (auto pt : {0, 1}) {
                auto p = doc.get_entity(tp->points.at(pt).entity).get_point(tp->points.at(pt).point, doc);
                std::cout << glm::length(line_point - p) << std::endl;
                if (glm::length(line_point - p) < 1e-6) {
                    auto &constraint = add_constraint<ConstraintLinePointsPerpendicular>();
                    const auto other_pt = !pt;
                    constraint.m_line = line.m_uuid;
                    constraint.m_point_line = tp->points.at(pt);
                    constraint.m_point = tp->points.at(other_pt);
                    return ToolResponse::commit();
                }
            }
        }
    }

    return ToolResponse::end();
}

ToolResponse ToolConstrainPerpendicular::update(const ToolArgs &args)
{
    return ToolResponse();
}
} // namespace dune3d
