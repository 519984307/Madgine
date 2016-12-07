#pragma once

#include "statement.h"

namespace Engine {
	namespace Scripting {
		namespace Statements {

			class ArithmeticalOperator : public Statement 
			{
			public:
				ArithmeticalOperator(int line, const std::string &op, Ogre::unique_ptr<const Statement> &&first, Ogre::unique_ptr<const Statement> &&second);
				
				virtual ValueType run(Scope *rootScope, Scope *, Stack &stack, bool *) const override;

			private:
				enum {
					Add,
					Subtract,
					Multiply,
					Divide
				} mType;
				Ogre::unique_ptr<const Statement> mFirst, mSecond;
			};

		}
	}
}

