#pragma once

#include "CmakeGenerator.h"

namespace Maditor {
	namespace Model {
		namespace Generator {

			class CmakeSubProject : public CmakeGenerator {
			public:
				CmakeSubProject(CmakeGenerator *parent, const QString &name);

				// Geerbt �ber CmakeGenerator
				virtual QString root() override;

				

			private:
				CmakeGenerator *mParent;
				
			};

		}
	}
}