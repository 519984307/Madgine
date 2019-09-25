#pragma once

#include "widget.h"
#include "Modules/generic/virtualbase.h"

namespace Engine
{
	namespace GUI
	{
		class MADGINE_CLIENT_EXPORT Label : public Widget<Label>
		{
		public:
			using Widget::Widget;
			virtual ~Label() = default;

			virtual std::string getText();
			virtual void setText(const std::string& text);

			virtual WidgetClass getClass() const override;

			//KeyValueMapList maps() override;
		};
	}
}


