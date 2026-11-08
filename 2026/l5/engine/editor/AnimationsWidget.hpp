#if defined(DEBUG)
#pragma once

#include <string>

#include "Widget.hpp"
#include "engine/animation/Animation.hpp"
#include "engine/animation/AnimationPlayback.hpp"
#include "engine/assets/Tileset.hpp"

namespace engine::editor
{
	class CAnimationsWidget : public IWidget
	{
	public:

		void Frame(float _dt) override;
		void Render() override;

		[[nodiscard]] std::string_view GetName() const override { return "Animations"; }
		[[nodiscard]] std::string_view GetMenuName() const override { return "Windows"; }
		[[nodiscard]] bool GetRenderInMenu() const override { return true; }

	private:

		void RenderMenuBar();
		void RenderAnimationList();
		void RenderSequenceList();
		void RenderSequenceEditor();
		void RenderPreview();
		void RenderFramesTable();
		void RenderTilePicker();
		void RenderCreateAnimationPopup();

		void SelectAnimation(const std::string& _name);
		void SelectSequence(int _index);
		void RestartPreview();

		[[nodiscard]] engine::animation::SAnimationSequence* GetSelectedSequence();
		[[nodiscard]] const engine::assets::STileset* GetTileset() const;

	private:

		std::string m_SelectedAnimation;
		int m_SelectedSequence{ -1 };
		engine::animation::SAnimation m_Draft;
		bool m_Dirty{ false };

		engine::animation::CAnimationPlayback m_Preview;
		float m_PreviewScale{ 4.f };
		int m_PickerTarget{ -1 };

		bool m_OpenCreatePopup{ false };
		std::string m_NewAnimationName;
		std::string m_NewSequenceName;
	};
}
#endif