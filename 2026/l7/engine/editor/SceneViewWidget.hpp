#if defined(DEBUG)
#pragma once

#include "Widget.hpp"

namespace engine::editor
{
	class CSceneViewWidget : public IWidget
	{
	public:
		CSceneViewWidget() = default;
		~CSceneViewWidget() = default;

		[[nodiscard]] std::string_view GetName() const override { return "Scene View"; }
		[[nodiscard]] std::string_view GetMenuName() const override { return "Windows"; }

		void Frame(float _dt) override {}
		void Render() override;

		[[nodiscard]] bool GetRenderInMenu() const override { return true; }

	private:

		void RenderHeader();
		void RenderLayers();
		void RenderChunks();
	};
}
#endif