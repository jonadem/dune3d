#include "group.hpp"
#include "nlohmann/json.hpp"
#include "all_groups.hpp"
#include "document/document.hpp"
#include "document/entity/entity.hpp"
#include "document/constraint/constraint.hpp"
#include "util/json_util.hpp"

namespace dune3d {

static void to_json(json &j, const std::optional<Body> &b)
{
    if (b.has_value())
        j = b->serialize();
    else
        j = nullptr;
}

static void from_json(const json &j, std::optional<Body> &b)
{
    if (j == nullptr)
        b = std::nullopt;
    else
        b.emplace(j);
}


Body::Body(const json &j) : m_name(j.at("name").get<std::string>())
{
}

Body::Body()
{
}

json Body::serialize() const
{
    return json{{"name", m_name}};
}

Group::Group(const UUID &uu) : m_uuid(uu)
{
}
Group::Group(const UUID &uu, const json &j)
    : m_uuid(uu), m_name(j.at("name").get<std::string>()), m_body(j.at("body").get<std::optional<Body>>()),
      m_index(j.at("index").get<int>())
{
    if (j.contains("active_wrkpl"))
        j.at("active_wrkpl").get_to(m_active_wrkpl);
}

NLOHMANN_JSON_SERIALIZE_ENUM(Group::Type, {
                                                  {Group::Type::REFERENCE, "reference"},
                                                  {Group::Type::SKETCH, "sketch"},
                                                  {Group::Type::EXTRUDE, "extrude"},
                                                  {Group::Type::FILLET, "fillet"},
                                                  {Group::Type::CHAMFER, "chamfer"},
                                                  {Group::Type::LATHE, "lathe"},
                                          })

json Group::serialize() const
{
    return json{
            {"type", get_type()},
            {"index", m_index},
            {"name", m_name},
            {"body", m_body},
            {"active_wrkpl", static_cast<std::string>(m_active_wrkpl)},
    };
}

std::unique_ptr<Group> Group::new_from_json(const UUID &uu, const json &j)
{
    const auto type = j.at("type").get<Type>();
    switch (type) {
    case Type::REFERENCE:
        return std::make_unique<GroupReference>(uu, j);
    case Type::SKETCH:
        return std::make_unique<GroupSketch>(uu, j);
    case Type::EXTRUDE:
        return std::make_unique<GroupExtrude>(uu, j);
    case Type::FILLET:
        return std::make_unique<GroupFillet>(uu, j);
    case Type::CHAMFER:
        return std::make_unique<GroupChamfer>(uu, j);
    case Type::LATHE:
        return std::make_unique<GroupLathe>(uu, j);
    }
    throw std::runtime_error("unknown entity type");
}

Group::BodyAndGroup Group::find_body(const Document &doc) const
{
    const Group *body_group = nullptr;
    for (auto group : doc.get_groups_sorted()) {
        if (group->m_body)
            body_group = group;
        if (group == this)
            return {body_group->m_body.value(), *body_group};
    }
    throw std::runtime_error("body not found");
}

GroupStatusMessage::Status GroupStatusMessage::summarize(const std::list<GroupStatusMessage> &msgs)
{
    Status status = Status::NONE;
    for (const auto &msg : msgs) {
        status = static_cast<Status>(std::max(static_cast<int>(status), static_cast<int>(msg.status)));
    }
    return status;
}

std::set<UUID> Group::get_referenced_entities(const Document &doc) const
{
    std::set<UUID> r;
    if (m_active_wrkpl)
        r.insert(m_active_wrkpl);
    for (const auto &[uu, en] : doc.m_entities) {
        if (en->m_group == m_uuid) {
            auto refs = en->get_referenced_entities();
            r.insert(refs.begin(), refs.end());
        }
    }
    for (const auto &[uu, co] : doc.m_constraints) {
        if (co->m_group == m_uuid) {
            auto refs = co->get_referenced_entities();
            r.insert(refs.begin(), refs.end());
        }
    }
    return r;
}

std::set<UUID> Group::get_referenced_groups(const Document &doc) const
{
    auto ents = get_referenced_entities(doc);
    std::set<UUID> r;
    for (auto &en_uu : ents) {
        auto &en = doc.get_entity(en_uu);
        r.insert(en.m_group);
    }

    return r;
}

std::set<UUID> Group::get_required_entities(const Document &doc) const
{
    return {};
}

std::set<UUID> Group::get_required_groups(const Document &doc) const
{
    return {};
}

json Group::serialize(const Document &doc) const
{
    return serialize();
}

Group::~Group() = default;
} // namespace dune3d
