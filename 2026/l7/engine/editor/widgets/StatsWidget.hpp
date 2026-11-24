#if defined(DEBUG)
#pragma once

#include "Widget.hpp"

namespace engine::editor
{
	class CStatsWidget : public IWidget
	{
	public:
		CStatsWidget() = default;
		~CStatsWidget() = default;

		[[nodiscard]] std::string_view GetName() const override { return "Stats"; }
		[[nodiscard]] std::string_view GetMenuName() const override { return "Debug"; }

		void Frame(float _dt) override {}
		void Render() override;

		[[nodiscard]] bool GetRenderInMenu() const override { return true; }
	};
}
#endif
