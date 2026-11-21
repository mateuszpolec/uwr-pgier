
#include "AnimationSystem.hpp"

#include "engine/game/Game.hpp"
#include "engine/animation/AnimationsManager.hpp"
#include "engine/assets/TilesetManager.hpp"

#include "engine/ecs/components/Component_Animatable.hpp"
#include "engine/ecs/components/Component_Renderable.hpp"

namespace engine::ecs::systems
{
	void CAnimationSystem::Frame(CEntityManager& _components, float _dt)
	{
		using namespace engine::ecs::components;

		_components.ForEach<SCoAnimatable, SCoRenderable>(
			[this, _dt](entity_id_t, SCoAnimatable& _animatable, SCoRenderable& _renderable)
			{
				if (_animatable.Animation.empty() || _animatable.Sequence.empty())
				{
					return;
				}

				const engine::animation::CAnimationManager& animations_manager = engine::game::CGame::Get().GetAnimationManager();
				const engine::assets::CTilesetManager& tileset_manager = engine::game::CGame::Get().GetTilesetManager();

				const engine::animation::SAnimation* animation = animations_manager.FindAnimation(_animatable.Animation);
				if (!animation)
				{
					return;
				}

				const engine::animation::SAnimationSequence* sequence = animation->FindSequence(_animatable.Sequence);
				if (!sequence || sequence->Frames.empty())
				{
					return;
				}

				const engine::assets::STileset* tileset = tileset_manager.FindTileset(animation->Tileset);
				if (!tileset || !tileset->IsValid())
				{
					return;
				}

				const bool sequence_changed = _animatable.ActiveAnimation != _animatable.Animation || _animatable.ActiveSequence != _animatable.Sequence;

				if (sequence_changed)
				{
					_animatable.ActiveAnimation = _animatable.Animation;
					_animatable.ActiveSequence = _animatable.Sequence;
					_animatable.Playback.Play(*sequence, !_animatable.AutoPlay);

					const bool needs_sprite = !_renderable.GetAsSprite() || &_renderable.TextureHandle.Get() != &tileset->Texture.Get();
					if (needs_sprite)
					{
						_renderable.SetSprite(tileset->Texture);
					}
				}

				_animatable.Playback.Frame(*sequence, _dt, _animatable.SpeedMultiplier);

				// On sequence change the first frame rect must be applied immediately,
				// otherwise the sprite would show the whole texture until the first frame advance.
				if (!sequence_changed && !_animatable.Playback.HasFrameChanged())
				{
					return;
				}

				const engine::animation::SAnimationFrame& frame = sequence->Frames[_animatable.Playback.GetFrameIndex()];

				if (const std::optional<sf::IntRect> rect = tileset->GetTileRect(frame.TileIndex))
				{
					if (sf::Sprite* sprite = _renderable.GetAsSprite())
					{
						sprite->setTextureRect(*rect);
					}
				}
			});
	}
}