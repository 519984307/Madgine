#pragma once

#include "handler.h"
#include "../gui/windownames.h"
#include "../ogreuniquecomponentcollector.h"
#include "../uniquecomponent.h"

#include "../scripting/types/scope.h"

namespace Engine
{
	namespace UI
	{
		class MADGINE_CLIENT_EXPORT GuiHandlerBase : public Handler
		{
		public:
			enum class WindowType
			{
				MODAL_OVERLAY,
				NONMODAL_OVERLAY,
				ROOT_WINDOW
			};

			GuiHandlerBase(UIManager &ui, const std::string& windowName, WindowType type);


			bool init() override;
			void finalize() override;

			virtual void open();
			virtual void close();
			bool isOpen() const;

			bool isRootWindow() const;

			Scene::ContextMask context() const;

		protected:			
			void setContext(Scene::ContextMask context);


		private:

			const WindowType mType;


			Scene::ContextMask mContext;
		};

		using GuiHandlerCollector = OgreUniqueComponentCollector<GuiHandlerBase, std::vector, UIManager &>;
		template <class T>
		using GuiHandler = Scripting::Scope<T, UniqueComponent<T, GuiHandlerCollector>>;
	} // namespace GuiHandler

	PLUGIN_COLLECTOR_EXPORT(GuiHandler, UI::GuiHandlerCollector);

} // namespace Cegui
