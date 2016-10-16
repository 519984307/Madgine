#include "Editor.h"
#include <qdebug.h>
#include <qdiriterator.h>
#include "Engine/Watcher/LogsWatcher.h"
#include "Engine/Watcher/ResourceWatcher.h"
#include "Engine/Watcher/ApplicationWatcher.h"
#include "Engine/ApplicationWrapper.h"
#include "Editors\VSLink.h"
#include "Project\Generator\ClassGeneratorFactory.h"
#include "Editors\ScriptEditorModel.h"

namespace Maditor {
	namespace Model {

		Editor::Editor() :
		mApplicationWrapper(0),
		mApplicationWatcher(0),
		mVS(0),
		mScriptEditor(0),
		mSettings("MadMan Studios", "Maditor") {

			mRecentProjects = mSettings.value("recentProjects").toStringList();
			

			mLoader = new ModuleLoader;
			mApplicationWatcher = new Watcher::ApplicationWatcher(mLoader);
			mApplicationWrapper = new ApplicationWrapper(mApplicationWatcher, mLoader);

			mVS = new Editors::VSLink;
			mScriptEditor = new Editors::ScriptEditorModel;
			mClassGeneratorFactory = new Generator::ClassGeneratorFactory;

			connect(mApplicationWatcher->logsWatcher(), &Watcher::LogsWatcher::openScriptFile, mScriptEditor, &Editors::ScriptEditorModel::openScriptFile);
			connect(mApplicationWatcher->resourceWatcher(), &Watcher::ResourceWatcher::openScriptFile, mScriptEditor, &Editors::ScriptEditorModel::openScriptFile);
		}

		Editor::~Editor()
		{
			mSettings.setValue("recentProjects", mRecentProjects);

			delete mApplicationWrapper;
			delete mApplicationWatcher;
			delete mLoader;			
		}

		void Editor::newProject(const QString &path, const QString &name, QWindow *target)
		{

			openProject(std::make_unique<Project>(path, name), target);

		}

		void Editor::loadProject(const QString & path, QWindow * target)
		{

			openProject(std::unique_ptr<Project>(Project::load(path)), target);

		}



		ApplicationWrapper *Editor::application()
		{
			return mApplicationWrapper;
		}

		Watcher::ApplicationWatcher * Editor::watcher()
		{
			return mApplicationWatcher;
		}

		Generator::ClassGeneratorFactory *Editor::classGeneratorFactory()
		{
			return mClassGeneratorFactory;
		}

		Editors::ScriptEditorModel *Editor::scriptEditor()
		{
			return mScriptEditor;
		}

		Editors::VSLink *Editor::vs()
		{
			return mVS;
		}

		Project *Editor::project()
		{
			return mProject.get();
		}

		QSettings & Editor::settings()
		{
			return mSettings;
		}

		const QStringList & Editor::recentProjects()
		{
			return mRecentProjects;
		}

		void Editor::openProject(std::unique_ptr<Project> &&project, QWindow *target)
		{
			if (!project->isValid()) return;

			mProject = std::forward<std::unique_ptr<Project>>(project);

			QString path = mProject->root();
			mRecentProjects.removeAll(path);
			mRecentProjects.push_front(path);

			emit recentProjectsChanged(mRecentProjects);


			emit projectOpened(mProject.get());

			mApplicationWrapper->load(mProject.get(), target);

			//mApplicationWrapper.go();
		}

		
	}
}