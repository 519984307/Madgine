#pragma once

namespace Maditor {
	namespace Model {
		class ApplicationWrapper;
		class Project;
		class Module;

		class Editor;

		class TreeItem;

		class Log;
		class LogsModel;

		class ProjectLog;

		namespace Watcher {
			class ApplicationWatcher;
			
			class PerformanceWatcher;
			class StatsProfilerNode;
			
			class ResourceWatcher;
			class ResourceItem;
			class ResourceGroupItem;
			
			class OgreSceneWatcher;
			class OgreSceneNodeItem;
			
			class OgreLogWatcher;		

			class InputWrapper;
			
		}

		namespace Generator {
			class ClassGeneratorFactory;
			class ClassGenerator;
			class CmakeSubProject;
		}

		namespace Editors {
			class ScriptEditorModel;
			class VSLink;
			class ScriptEditor;
		}
		
		class ModuleLoader;

	}

	namespace View {
		class OgreWindow;
	}
}