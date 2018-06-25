#pragma once


#include "../model/project/generators/classgeneratorfactory.h"


namespace Maditor {

	namespace Model {


			class MADITOR_MODEL_EXPORT DialogManager : public QObject
			{
				Q_OBJECT

			public:
				DialogManager() {
					sSingleton = this;
				};
				~DialogManager() = default;

				virtual bool confirmFileOverwrite(const QString &filePath, QMessageBox::StandardButton *result = 0) = 0;
				static bool confirmFileOverwriteStatic(const QString &filePath, QMessageBox::StandardButton *result = 0) {
					return sSingleton->confirmFileOverwrite(filePath, result);
				}

				virtual View::Dialogs::SettingsDialog *settingsDialog() = 0;

				virtual void showError(const QString &title, const QString &msg) = 0;
				static void showErrorStatic(const QString &title, const QString &msg)
				{
					sSingleton->showError(title, msg);
				}
					

				virtual bool showNewProjectDialog(QString &path, QString &name) = 0;
				static bool showNewProjectDialogStatic(QString &path, QString &name) {
					return sSingleton->showNewProjectDialog(path, name);
				}
				virtual bool showLoadProjectDialog(QString &path) = 0;
				static bool showLoadProjectDialogStatic(QString &path) {
					return sSingleton->showLoadProjectDialog(path);
				}
				virtual bool showNewConfigDialog(Model::ConfigList *list, QString &name) = 0;
				static bool showNewConfigDialogStatic(Model::ConfigList *list, QString &name) {
					return sSingleton->showNewConfigDialog(list, name);
				}
				virtual bool showDeleteConfigDialog(Model::ApplicationConfig *config) = 0;
				static bool showDeleteConfigDialogStatic(Model::ApplicationConfig *config) {
					return sSingleton->showDeleteConfigDialog(config);
				}
				virtual bool showNewModuleDialog(Model::ModuleList *list, QString &name) = 0;
				static bool showNewModuleDialogStatic(Model::ModuleList *list, QString &name) {
					return sSingleton->showNewModuleDialog(list, name);
				}
				virtual bool showDeleteModuleDialog(Model::Module *module, bool &deleteFiles) = 0;
				static bool showDeleteModuleDialogStatic(Model::Module *module, bool &deleteFiles) {
					return sSingleton->showDeleteModuleDialog(module, deleteFiles);
				}
				virtual bool showNewClassDialog(Model::Module *module, QString &name, Model::Generators::ClassGeneratorFactory::ClassType &type) = 0;
				static bool showNewClassDialogStatic(Model::Module *module, QString &name, Model::Generators::ClassGeneratorFactory::ClassType &type) {
					return sSingleton->showNewClassDialog(module, name, type);
				}

				virtual bool showNewGuiHandlerDialog(Model::Module *module, const QString &name, QString &window, int &type, bool &hasLayout) = 0;
				virtual bool showNewGlobalAPIDialog(Model::Module *module, const QString &name) = 0;
				virtual bool showNewEntityComponentDialog(Model::Module *module, const QString &name, QString &componentName) = 0;
				virtual bool showNewSceneComponentDialog(Model::Module *module, const QString &name) = 0;
				virtual bool showNewGameHandlerDialog(Model::Module *module, const QString &name) = 0;
				virtual bool showNewOtherClassDialog(Model::Module *module, const QString &name, bool &headerOnly) = 0;
				virtual bool showNewServerClassDialog(Model::Module *module, const QString &name) = 0;
				virtual bool showDeleteClassDialog(Generators::ClassGenerator *generator, bool &deleteFiles) = 0;
				static bool showNewGuiHandlerDialogStatic(Model::Module *module, const QString &name, QString &window, int &type, bool &hasLayout) {
					return sSingleton->showNewGuiHandlerDialog(module, name, window, type, hasLayout);
				}
				static bool showNewGlobalAPIDialogStatic(Model::Module *module, const QString &name) {
					return sSingleton->showNewGlobalAPIDialog(module, name);
				}
				static bool showNewEntityComponentDialogStatic(Model::Module *module, const QString &name, QString &componentName) {
					return sSingleton->showNewEntityComponentDialog(module, name, componentName);
				}
				static bool showNewSceneComponentDialogStatic(Model::Module *module, const QString &name) {
					return sSingleton->showNewSceneComponentDialog(module, name);
				}
				static bool showNewGameHandlerDialogStatic(Model::Module *module, const QString &name) {
					return sSingleton->showNewGameHandlerDialog(module, name);
				}
				static bool showNewOtherClassDialogStatic(Model::Module *module, const QString &name, bool &headerOnly) {
					return sSingleton->showNewOtherClassDialog(module, name, headerOnly);
				}
				static bool showNewServerClassDialogStatic(Model::Module *module, const QString &name) {
					return sSingleton->showNewServerClassDialog(module, name);
				}
				static bool showDeleteClassDialogStatic(Generators::ClassGenerator *generator, bool &deleteFiles) {
					return sSingleton->showDeleteClassDialog(generator, deleteFiles);
				}

				virtual void showSettingsDialog() = 0;

			private:
				static DialogManager *sSingleton;

				
			};
	}
}

