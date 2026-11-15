#if defined(DEBUG)
#pragma once

#include "Widget.hpp"

namespace engine::editor
{
	class CCameraWidget : public IWidget
	{
	public:
		CCameraWidget() = default;
		~CCameraWidget() = default;

		[[nodiscard]] std::string_view GetName() const override { return "Camera"; }
		[[nodiscard]] std::string_view GetMenuName() const override { return "Windows"; }

		void Frame(float _dt) override {}
		void Render() override;

		[[nodiscard]] bool GetRenderInMenu() const override { return true; }

	private:

		void RenderStack();
		void RenderActiveState();
		void RenderFocusCameraSettings();
		void RenderStreamingInfo();
	};
}
#endif