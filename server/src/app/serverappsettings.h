#pragma once

#include "app/appsettings.h"

namespace Engine{
namespace App{

/**
 * Contains the information needed to start the Application.
 */
struct ServerAppSettings : AppSettings {

	/**
	 * Sets up default values for the settings.
	 */
	ServerAppSettings(lua_State *state) :
		mState(state)
	{

	}

	lua_State *mState;


};


}
}