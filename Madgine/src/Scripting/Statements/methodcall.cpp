#include "madginelib.h"
#include "methodcall.h"
#include "Scripting/Types/scope.h"
#include "valuetype.h"

namespace Engine {
namespace Scripting {
namespace Statements {


MethodCall::MethodCall(int line, const std::string &methodName,
                       std::list<Ogre::unique_ptr<const Statement>> &&arguments) :
    mMethodName(methodName),
    mArguments(std::forward<std::list<Ogre::unique_ptr<const Statement>>>(arguments)),
	Statement(line)
{

}

ValueType MethodCall::run(Scope *rootScope, Scope *scope, Stack &stack, bool *) const
{
    if (!scope) scope = rootScope;
    ArgumentList args(mArguments.size());
    unsigned int i = 0;
    for (const Ogre::unique_ptr<const Statement> &s : mArguments) {
        args[i++] = s->run(rootScope, 0, stack);
    }
    return scope->methodCall(mMethodName, args);

}

}
}
}
