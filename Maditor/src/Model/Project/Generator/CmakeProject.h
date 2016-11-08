#pragma once

#include "CmakeGenerator.h"

namespace Maditor {
	namespace Model {
		namespace Generator {

			class MADITOR_EXPORT CmakeProject : public CmakeGenerator {
			public:
				CmakeProject(const QString &root, const QString &buildDir, const QString &name);

				void build();

				// Geerbt �ber CmakeGenerator
				virtual QString root() override;

				void addLibrary(const QString &lib);

				QString solutionName();

			protected:
				virtual QString preTargetCode() override;

			private:
				QString mBuildDir;
				QString mRoot;

				QStringList mLibraries;

			};

		}
	}
}