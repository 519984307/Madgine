#pragma once

#include "widget.h"

namespace Engine
{
namespace Widgets
	{
		class Checkbox : public Widget<Checkbox>
		{
		public:
			using Widget::Widget;
			virtual ~Checkbox() = default;

			virtual bool isChecked();
			virtual void setChecked(bool b);

			virtual WidgetClass getClass() const override;
		};
	}
}
