#if defined(DEBUG)
#pragma once

#include "Widget.hpp"

namespace engine::editor
{
	class CJobSystemWidget : public IWidget
	{
	public:
		CJobSystemWidget() = default;
		~CJobSystemWidget() = default;

		[[nodiscard]] std::string_view GetName() const override { return "Job System"; }
		[[nodiscard]] std::string_view GetMenuName() const override { return "Debug"; }

		void Frame(float _dt) override {}
		void Render() override;

		[[nodiscard]] bool GetRenderInMenu() const override { return true; }
	};
}
#endif
