#pragma once

#include "Scripting/Statements/statement.h"

namespace Engine {
namespace Scripting {
namespace Statements {


class Assignment : public Statement {
public:
    Assignment(int line, const std::string &varName, const std::string &op, Ogre::unique_ptr<const Statement> &&value);

    virtual ValueType run(Scope *rootScope, Scope *scope, VarSet &stack, bool *) const;

private:
	enum {
		ASSIGNMENT,
		APPEND,
		SUBTRACT
	} mOp;

    std::string mVarName;
    Ogre::unique_ptr<const Statement> mValue;
};

}
}
}
