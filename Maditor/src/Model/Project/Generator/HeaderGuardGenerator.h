#pragma once

#include "Generator.h"
#include <functional>

namespace Maditor {
	namespace Model {
		class Module;

		namespace Generator {

			class HeaderGuardGenerator : public Generator {

			public:
				HeaderGuardGenerator(Module *module, const QString &name);

				// Geerbt �ber Generator
				virtual QStringList paths() override;
				virtual void write(QTextStream & stream, int index) override;

				const QString &name();

				QString fileName();
				QString guardName();
				static QString fileName(const QString &name);
				static QString guardName(const QString &name);

			private:
				Module *mModule;
				QString mName;

			};

		}
	}
}