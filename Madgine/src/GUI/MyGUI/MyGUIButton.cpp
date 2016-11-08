#include "libinclude.h"

#include "MyGUIButton.h"

#include "MyGUIWindow.h"

namespace Engine {
	namespace GUI {
		namespace MyGui {
			MyGUIButton::MyGUIButton(MyGUIWindow * w) :
				Button(w),
				mButton(w->window()->castType<MyGUI::Button>())
			{
			}
			void MyGUIButton::setText(const std::string & text)
			{
				mButton->setCaption(text);
			}
		}
	}
}