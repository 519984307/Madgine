#pragma once

#include <qobject.h>
#include <Qsci\qsciscintilla.h>
#include <map>
#include "ScriptEditor.h"

namespace Maditor {
	namespace Model {
		namespace Editors {

			class ScriptEditorModel : public QObject {
				Q_OBJECT
				
			public:

			signals:
				void documentAdded(const QString &name, ScriptEditor *document);
				void documentClosed(int index);
				void showDoc(ScriptEditor *widget);

			public slots :
				void openScriptFile(const QString &path, int line);
				void closeDocument(int index, const QString &path);

			private:
				std::map<QString, ScriptEditor *> mDocuments;

			};
		}
	}
}