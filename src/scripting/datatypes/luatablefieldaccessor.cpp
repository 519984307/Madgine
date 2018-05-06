#include "../../interfaceslib.h"
#include "luatablefieldaccessor.h"
#include "luatableinstance.h"
#include "../../generic/valuetype.h"

namespace Engine
{
	namespace Scripting
	{

		LuaTableFieldAccessor::LuaTableFieldAccessor(const std::shared_ptr<LuaTableInstance>& instance, const std::string & name) :
			mInstance(instance),
			mName(name)
		{
		}

		void LuaTableFieldAccessor::operator=(const ValueType &v) {
			mInstance->setValue(mName, v);
		}

		LuaTableFieldAccessor::operator ValueType(){
			return mInstance->getValue(mName);
		}

	}
}
