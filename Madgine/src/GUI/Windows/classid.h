#pragma once

#include "windowclass.h"

namespace Engine {
	namespace GUI {

		template <class T>
		struct ClassId {

		};

		template <>
		struct ClassId<Window> {
			static constexpr Class id = Class::WINDOW_CLASS;
		};

		template <>
		struct ClassId<TextureDrawer> {
			static constexpr Class id = Class::TEXTUREDRAWER_CLASS;
		};

		template <>
		struct ClassId<Combobox> {
			static constexpr Class id = Class::COMBOBOX_CLASS;
		};

		template <>
		struct ClassId<TabWindow> {
			static constexpr Class id = Class::TABWINDOW_CLASS;
		};

		template <>
		struct ClassId<Checkbox> {
			static constexpr Class id = Class::CHECKBOX_CLASS;
		};

		template <>
		struct ClassId<Textbox> {
			static constexpr Class id = Class::TEXTBOX_CLASS;
		};

		template <>
		struct ClassId<Button> {
			static constexpr Class id = Class::BUTTON_CLASS;
		};

		template <>
		struct ClassId<Layout> {
			static constexpr Class id = Class::LAYOUT_CLASS;
		};

		template <>
		struct ClassId<Label> {
			static constexpr Class id = Class::LABEL_CLASS;
		};

		template <>
		struct ClassId<Bar> {
			static constexpr Class id = Class::BAR_CLASS;
		};

	}
}