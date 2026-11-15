#pragma once

#include <array>
#include <memory>
#include <utility>
#include <vector>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/View.hpp>

#include "engine/core/Assert.hpp"
#include "engine/core/NonCopyable.hpp"
#include "engine/events/EventHandle.hpp"
#include "Camera.hpp"

namespace engine::camera
{
	class CCameraManager : public engine::core::NonCopyable
	{
	public:

		explicit CCameraManager(sf::Vector2u _viewport_size);
		~CCameraManager() = default;

		template<typename T, typename... Args>
		T& Register(Args&&... _args)
		{
			static_assert(std::is_base_of_v<ICamera, T>, "T must derive from ICamera");

			std::unique_ptr<T> camera = std::make_unique<T>(std::forward<Args>(_args)...);
			const size_t index = static_cast<size_t>(camera->GetType());

			ASSERT(index < m_Cameras.size(), "Invalid camera type");
			ASSERT(m_Cameras[index] == nullptr, "Camera of this type is already registered");

			camera->m_Owner = this;
			T& ref = *camera;
			m_Cameras[index] = std::move(camera);
			return ref;
		}

		[[nodiscard]] ICamera* Find(ECameraType _type) const;

		template<typename T>
		[[nodiscard]] T* Get(ECameraType _type) const
		{
			return static_cast<T*>(Find(_type));
		}

		void Push(ECameraType _type);
		void Pop();
		void Replace(ECameraType _type);

		[[nodiscard]] bool HasActive() const { return !m_Stack.empty(); }
		[[nodiscard]] ECameraType GetActiveType() const;
		[[nodiscard]] ICamera& GetActive() const;
		[[nodiscard]] bool IsActive(ECameraType _type) const;
		[[nodiscard]] const std::vector<ECameraType>& GetStack() const { return m_Stack; }

		void Frame(float _dt);

		void SetViewportSize(sf::Vector2u _size);
		[[nodiscard]] sf::Vector2u GetViewportSize() const { return m_ViewportSize; }

		[[nodiscard]] const sf::View& GetView() const { return m_View; }
		void ApplyTo(sf::RenderTarget& _target) const { _target.setView(m_View); }

		[[nodiscard]] sf::Vector2f ScreenToWorld(sf::Vector2i _pixel) const;
		[[nodiscard]] sf::Vector2i WorldToScreen(sf::Vector2f _world) const;

		[[nodiscard]] sf::FloatRect GetVisibleWorldRect() const;
		[[nodiscard]] sf::FloatRect GetStreamingWorldRect() const;
		[[nodiscard]] sf::Vector2f GetVelocity() const { return m_Velocity; }

	public:

		float StreamingLookahead = 0.75f; // [s]

	private:

		void RefreshView();
		void SwitchActive(ICamera* _from, ICamera* _to);

	private:

		std::array<std::unique_ptr<ICamera>, static_cast<size_t>(ECameraType::Count)> m_Cameras;
		std::vector<ECameraType> m_Stack;

		sf::View m_View;
		sf::Vector2u m_ViewportSize;
		sf::Vector2f m_Velocity{ 0.f, 0.f };
		bool m_KeyboardInputEnabled = true;

		engine::events::CEventHandle m_ResizeEventHandle;
	};
}