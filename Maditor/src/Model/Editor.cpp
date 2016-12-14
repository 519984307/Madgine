#include "madgineinclude.h"

#include "Editor.h"
#include "Logsmodel.h"
#include "Engine/ApplicationWrapper.h"
#include "Editors\EditorManager.h"
#include "Project\Generators\ClassGeneratorFactory.h"
#include "Project\Generators\CommandLine.h"
#include "Engine\ModuleLoader.h"
#include "Engine\Watcher\ResourceWatcher.h"
#include "Engine/Watcher/LogWatcher.h"
#include "Editors\ScriptEditorModel.h"
#include "Addons\Addon.h"

SharedMemory *SharedMemory::msSingleton = 0;

namespace Maditor {
	namespace Model {

		Editor::Editor(Addons::AddonCollector *collector) :
			mMemory(SharedMemory::create),
			mApplicationWrapper(0),
			mApplicationWatcher(0),
			mEditorManager(0),
			mSettings("MadMan Studios", "Maditor"),
			mLogs(0)
		{

			//mLog = new Watcher::OgreLogWatcher(Watcher::OgreLogWatcher::GuiLog, "Madgine.log");

			mSettings.beginGroup("Editor");
			mRecentProjects = mSettings.value("recentProjects").toStringList();
			mReloadProject = mSettings.value("reloadProject").toBool();
			mSettings.endGroup();

			mApplicationWrapper = new ApplicationWrapper();

			mEditorManager = new Editors::EditorManager(collector);
			mClassGeneratorFactory = new Generators::ClassGeneratorFactory;
			mLogs = new LogsModel;

			/*mApplicationWatcher->init();

			connect(mApplicationWatcher, &Watcher::ApplicationWatcher::logCreated, mLogs, &LogsModel::addLog);
			connect(mApplicationWatcher, &Watcher::ApplicationWatcher::logRemoved, mLogs, &LogsModel::removeLog);
			connect(mEditorManager->scriptEditor(), &Editors::ScriptEditorModel::documentSaved, mApplicationWatcher->resourceWatcher(), &Watcher::ResourceWatcher::reloadScriptFile);*/

			mLogs->addLog(mLog);
			Generators::CommandLine::setLog(mLog);
		}

		Editor::~Editor()
		{
			mSettings.beginGroup("Editor");
			mSettings.setValue("recentProjects", mRecentProjects);
			mSettings.setValue("reloadProject", mReloadProject);
			mSettings.endGroup();

			delete mApplicationWrapper;
			delete mApplicationWatcher;
			delete mEditorManager;
			delete mLoader;		
			delete mLogs;

			delete mLog;
		}

		void Editor::onStartup()
		{

			if (mReloadProject && !mRecentProjects.isEmpty()) {
				loadProject(mRecentProjects.front());
			}
		}

		void Editor::newProject(const QString &path, const QString &name)
		{

			openProject(std::make_unique<Project>(path, name));

		}

		void Editor::loadProject(const QString & path)
		{

			openProject(std::unique_ptr<Project>(Project::load(path)));

		}



		ApplicationWrapper *Editor::application()
		{
			return mApplicationWrapper;
		}

		Watcher::ApplicationWatcher * Editor::watcher()
		{
			return mApplicationWatcher;
		}

		Generators::ClassGeneratorFactory *Editor::classGeneratorFactory()
		{
			return mClassGeneratorFactory;
		}

		Editors::EditorManager * Editor::editorManager()
		{
			return mEditorManager;
		}

		LogsModel * Editor::logsModel()
		{
			return mLogs;
		}

		Project *Editor::project()
		{
			return mProject.get();
		}

		void Editor::clearRecentProjects()
		{
			mRecentProjects.clear();
			emit recentProjectsChanged(mRecentProjects);
		}


		QSettings & Editor::settings()
		{
			return mSettings;
		}

		const QStringList & Editor::recentProjects()
		{
			return mRecentProjects;
		}

		bool Editor::reloadProjectProperty()
		{
			return mReloadProject;
		}

		void Editor::setReloadProjectProperty(bool b)
		{
			mReloadProject = b;
		}

		void Editor::openProject(std::unique_ptr<Project> &&project)
		{
			if (!project->isValid()) return;

			mProject = std::forward<std::unique_ptr<Project>>(project);

			QString path = mProject->path();
			mRecentProjects.removeAll(path);
			mRecentProjects.push_front(path);

			emit recentProjectsChanged(mRecentProjects);


			emit projectOpened(mProject.get());

			mEditorManager->setCurrentRoot(mProject->moduleList()->path());
			
			//mApplicationWrapper.go();
		}

		
	}
}