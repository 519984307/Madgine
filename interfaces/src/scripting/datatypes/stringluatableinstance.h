#pragma once

#include "luatableinstance.h"

namespace Engine
{
	namespace Scripting
	{
		class StringLuaTableInstance : public LuaTableInstance
		{
		public:
			StringLuaTableInstance(lua_State* state, const std::string& index,
			                       const std::shared_ptr<LuaTableInstance>& parent = {});
			virtual ~StringLuaTableInstance();
			void push(lua_State* state = nullptr) const override;
		private:
			std::string mIndex;
		};
	}
}