#pragma once


namespace Ogre {
	template <class T>
	class Singleton;

	template <class T>
	class SharedPtr;
}

namespace Engine {

	namespace App {
		class ConfigSet;
		struct OgreAppSettings;
		enum class ContextMask : unsigned int;
		class Application;
	}

	namespace UI {

		class Handler;
		class GameHandlerBase;
		class GuiHandlerBase;
		class UIManager;


	}

	namespace GUI {

		class GUISystem;
		struct WindowSizeRelVector;
		class WindowContainer;
		class Window;
		class Bar;
		class Button;
		class TextureDrawer;
		class Checkbox;
		class Combobox;
		class TabWindow;
		class Textbox;
		class Label;

		namespace MouseButton {
			enum MouseButton : unsigned char;
		}

	}

	namespace Scene {

		class OgreSceneManager;
		class BaseSceneComponent;

		namespace Entity {
			class Entity;
			class BaseEntityComponent;
		}


	}

	namespace Math {
		class Bounds;
	}

	namespace Resources {
		class TextureListener;
		class TextureComponent;

		class ResourceLoader;
		namespace Shading {
			class ShaderFactoryBase;
		}
	}

	namespace Util {
		class Profiler;
		class ProcessStats;
		class Util;
	}

	namespace Input {
		class InputHandler;
	}


}



