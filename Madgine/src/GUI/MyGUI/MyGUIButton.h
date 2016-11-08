#pragma once
#include "GUI\Windows\Button.h"
#include "MyGUIWindow.h"

namespace Engine {
	namespace GUI {
		namespace MyGui {
			class MyGUIButton :
				public Button
			{
			public:
				MyGUIButton(MyGUIWindow *w);

				virtual void setText(const std::string &text) override;

			private:
				MyGUI::Button *mButton;
			};
		}
	}
}

