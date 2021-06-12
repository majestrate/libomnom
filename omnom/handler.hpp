#pragma once
#include "crypto_hash.hpp"
#include "entity.hpp"
#include <oxenmq/oxenmq.h>
#include <functional>
#include <unordered_map>
#include <variant>

namespace omnom
{
    class Context;

    class EntityHandler
    {
        Context* const _ctx;
        std::unordered_map<EntityKind, std::function<void(Entity)>> m_Handlers;

       public:
        explicit EntityHandler(Context* ctx);

        EntityHandler(const EntityHandler&) = delete;
        EntityHandler(EntityHandler&&) = delete;
    };

}  // namespace omnom
