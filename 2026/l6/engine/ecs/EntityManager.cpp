#include "EntityManager.hpp"
#include "Entity.hpp"

namespace engine::ecs
{
	CEntity CEntityManager::CreateEntity()
	{
		const entity_id_t entity_id = m_NextEntityId++;
		m_Entities.insert(entity_id);
		return CEntity(entity_id, *this);
	}

	void CEntityManager::DestroyEntity(entity_id_t _entity_id)
	{
		for (auto& [type, pool] : m_ComponentPools)
		{
			pool->RemoveComponent(_entity_id);
		}
		m_Entities.erase(_entity_id);
	}

	bool CEntityManager::IsValid(entity_id_t _entity_id) const
	{
		return m_Entities.find(_entity_id) != m_Entities.end();
	}
}
