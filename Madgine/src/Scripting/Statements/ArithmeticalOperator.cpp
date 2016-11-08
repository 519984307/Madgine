#include "libinclude.h"
#include "ArithmeticalOperator.h"

#include "Database\exceptionmessages.h"
#include "Scripting\Datatypes\valuetype.h"
#include "Scripting\scriptexception.h"

namespace Engine {
	namespace Scripting {
		namespace Statements {
			
			ArithmeticalOperator::ArithmeticalOperator(int line, const std::string & op, Ogre::unique_ptr<const Statement>&& first, Ogre::unique_ptr<const Statement>&& second) :
				mFirst(std::forward<Ogre::unique_ptr<const Statement>>(first)),
				mSecond(std::forward<Ogre::unique_ptr<const Statement>>(second)),
				Statement(line)
			{
				if (op == "+") {
					mType = Plus;
				}
				else if (op == "/") {
					mType = Divide;
				}
				else {
					throw ScriptingException(Database::Exceptions::unknownBoolOperator(op));
				}
			}

			ValueType ArithmeticalOperator::run(Scope * rootScope, Scope *, VarSet & stack, bool *) const
			{
				ValueType first = mFirst->run(rootScope, 0, stack);
				ValueType second = mSecond->run(rootScope, 0, stack);
				switch (mType) {
				case Plus:
					return first + second;
				case Divide:
					return first / second;
				default:
					throw 0;
				}
			}
		}
	}
}