#if defined(DEBUG)
#pragma once

#include <string_view>

namespace engine::editor
{
	class IWidget
	{
	public:
		virtual ~IWidget() = default;

		[[nodiscard]] virtual std::string_view GetName() const = 0;
		[[nodiscard]] virtual std::string_view GetMenuName() const = 0;

		virtual void Render() = 0;
		virtual void Frame(float _dt) = 0;

		[[nodiscard]] bool IsVisible() const { return m_Visible; }
		void SetVisible(bool _visible) { m_Visible = _visible; }

		[[nodiscard]] virtual bool GetRenderInMenu() const = 0;

	protected:

		bool m_Visible = false;
	};
}
#endif