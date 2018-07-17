#include "../serverlib.h"

#include "serverlightmanager.h"




namespace Engine
{

	API_IMPL(Scene::ServerLightManager);

	namespace Scene
	{
		ServerLightManager::ServerLightManager(SceneManager &sceneMgr) :
			VirtualSceneComponentImpl<Engine::Scene::ServerLightManager, Engine::Scene::LightManager>(sceneMgr)
		{
		}

		ServerLightManager::~ServerLightManager()
		{
		}

		std::unique_ptr<Light> ServerLightManager::createLightImpl()
		{
			return std::make_unique<Light>();
		}

	}
}
