#include "../../moduleslib.h"
#include "luatable.h"
#include "luatableiterator.h"
#include "intluatableinstance.h"
#include "luatablefieldaccessor.h"

extern "C"
{
#include "../../lua/lua.h"
}

namespace Engine
{
	namespace Scripting
	{
		LuaTable::LuaTable()
		{
		}

		LuaTable LuaTable::global(LuaThread *thread)
		{
			return std::static_pointer_cast<LuaTableInstance>(std::make_shared<IntLuaTableInstance>(thread, LUA_RIDX_GLOBALS));
		}

		LuaTable LuaTable::registry(LuaThread *thread)
		{
			return std::static_pointer_cast<LuaTableInstance>(std::make_shared<IntLuaTableInstance>(thread, LUA_REGISTRYINDEX));
		}

		void LuaTable::clear()
		{
			mInstance.reset();
		}

		LuaTableFieldAccessor LuaTable::operator[](const std::string &name) {
			return LuaTableFieldAccessor(mInstance, name);
		}

		LuaTableFieldAccessor LuaTable::operator[](const char *name) {
			return LuaTableFieldAccessor(mInstance, name);
		}

		ValueType LuaTable::operator[](const std::string &name) const {
			return getValue(name);
		}

		ValueType LuaTable::operator[](const char *name) const {
			return getValue(name);
		}

		LuaTable::operator bool() const
		{
			return mInstance.operator bool();
		}

		void LuaTable::setValue(const std::string& name, const ValueType& value)
		{
			mInstance->setValue(name, value);
		}

		ValueType LuaTable::getValue(const std::string& name) const
		{
			return mInstance ? mInstance->getValue(name) : ValueType();
		}

		void LuaTable::setLightUserdata(const std::string& name, void* userdata)
		{
			mInstance->setLightUserdata(name, userdata);
		}

		void LuaTable::setMetatable(const LuaTable& metatable)
		{
			mInstance->setMetatable(metatable);
		}

		void LuaTable::setMetatable(const std::string& metatable)
		{
			mInstance->setMetatable(metatable);
		}

		bool LuaTable::hasFunction(const std::string& name) const
		{
			return mInstance ? mInstance->hasFunction(name) : false;
		}

		ArgumentList LuaTable::callMethod(const std::string& name, const ArgumentList& args)
		{
			return mInstance->callMethod(name, args);
		}

		void LuaTable::push(lua_State* state) const
		{
			mInstance->push(state);
		}

		LuaTable LuaTable::createTable(LuaThread *thread)
		{
			return mInstance->createTable(thread, mInstance);
		}

		LuaTable LuaTable::createTable(const std::string& name)
		{
			return mInstance->createTable(name, mInstance);
		}

		LuaTable LuaTable::registerTable(lua_State* state, int index)
		{
			return mInstance->registerTable(state, index, mInstance);
		}

		LuaTable::LuaTable(const std::shared_ptr<LuaTableInstance>& instance) :
			mInstance(instance)
		{
		}

		LuaThread* LuaTable::thread() const
		{
			return mInstance->thread();
		}

		LuaTable::iterator LuaTable::begin() const
		{
			return iterator(mInstance);
		}

		LuaTable::iterator LuaTable::end() const
		{
			return iterator();
		}
	}
}