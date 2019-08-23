#pragma once

#include "widget.h"
#include "Modules/generic/virtualbase.h"

namespace Engine
{
	namespace GUI
	{
		class MADGINE_CLIENT_EXPORT Label : public Widget
		{
		public:
			using Widget::Widget;
			virtual ~Label() = default;

			virtual std::string getText();
			virtual void setText(const std::string& text);

			//KeyValueMapList maps() override;
		};
	}
}

