#pragma once


namespace Engine
{
	struct TypeInfo {
		static const char *fix(const char* s)
		{
			const char* f = strrchr(s, ':');
			return f ? f + 1 : s;
		}

		TypeInfo(const char *fullName, const char *headerPath, const TypeInfo *decayType = nullptr) :
			mFullName(fullName),
			mTypeName(fix(mFullName)),
			mHeaderPath(headerPath),
			mDecayType(decayType)
		{}

		inline std::string namespaceName() const {
			return std::string{ mFullName, strlen(mFullName) - 2 - strlen(mTypeName) };
		}

		const char *mFullName;
		const char *mTypeName;		
		const char *mHeaderPath;
		const TypeInfo *mDecayType;
	};

	

	template <class T>
	struct AccessClassInfo {
		static const TypeInfo &get();
	};
	

#define RegisterType(T) template<> inline const Engine::TypeInfo &Engine::AccessClassInfo<::T>::get(){static TypeInfo sInfo{ #T, __FILE__ }; return sInfo;}

	template <class T>
	inline const TypeInfo &typeInfo() {
		return AccessClassInfo<T>::get();
	}
	
	template <class T>
	inline const char *typeName() {
		return typeInfo<T>().mTypeName;
	}

	

}