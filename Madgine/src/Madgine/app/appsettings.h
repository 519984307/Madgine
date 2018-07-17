#pragma once


namespace Engine
{
	namespace App
	{
		/**
		 * Contains the information needed to start the Application.
		 */
		struct AppSettings
		{
			/**
			 * Sets up default values for the settings.
			 */
			AppSettings() :
			mRunMain(true)
			{
			}

			std::string mAppName;

			bool mRunMain;

		};
	}
}
