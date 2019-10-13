#include "Madgine/clientlib.h"

#include "mainmenuhandler.h"

#include "Modules/keyvalue/metatable_impl.h"

#include "Madgine/gui/widgets/button.h"

#include "gamehandler.h"

#include "gamemanager.h"

UNIQUECOMPONENT(ClickBrick::UI::MainMenuHandler)

RegisterType(ClickBrick::UI::MainMenuHandler)

    METATABLE_BEGIN_BASE(ClickBrick::UI::MainMenuHandler, Engine::UI::GuiHandlerBase)
        METATABLE_END(ClickBrick::UI::MainMenuHandler)

            namespace ClickBrick
{
    namespace UI {

        MainMenuHandler::MainMenuHandler(Engine::UI::UIManager &ui)
            : Engine::UI::GuiHandler<MainMenuHandler>(ui, Engine::UI::GuiHandlerBase::WindowType::ROOT_WINDOW)
            , mStartGameSlot(this)
			, mStartGameButton(this, "StartGameButton")            
        {
        }

        const char *MainMenuHandler::key() const
        {
            return "MainMenuHandler";
        }

        void ClickBrick::UI::MainMenuHandler::setWidget(Engine::GUI::WidgetBase *w)
        {
            if (widget()) {
                mStartGameSlot.disconnectAll();
            }
            Engine::GuiHandlerBase::setWidget(w);
            if (widget()) {
                mStartGameButton->clickEvent().connect(mStartGameSlot);
            }
        }

        void ClickBrick::UI::MainMenuHandler::startGame()
        {
            getGameHandler<GameManager>().start();
            getGuiHandler<GameHandler>().open();
        }

    }
}