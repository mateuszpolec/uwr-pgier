#include "Entity.hpp"

namespace engine::ecs
{
	CEntity::CEntity(entity_id_t _entity_id, CEntityManager& _entity_manager)
		: m_EntityId(_entity_id)
		, m_EntityManager(_entity_manager)
	{

	}

	bool CEntity::IsValid() const
	{
		return m_EntityId != INVALID_ENTITY_ID;
	}
}