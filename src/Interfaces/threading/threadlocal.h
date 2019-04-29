#pragma once

#include "../generic/any.h"

namespace Engine
{	
	namespace Threading
	{

		struct INTERFACES_EXPORT ThreadLocalStorage {

			static size_t registerLocalVariable(Any &&var);

			static const Any &localVariable(size_t index);

			static void init();

		};

		template <typename T, typename G = ThreadLocalStorage>
		struct ThreadLocal
		{
			
			ThreadLocal(T initial = {})
			{
				mIndex = G::registerLocalVariable(initial);
			}

			T *operator->()
			{
				return &data();
			}

			T &operator*()
			{
				return data();
			}

			operator T&()
			{
				return data();
			}

			void operator=(const T &t)
			{
				data() = t;
			}

			T &data()
			{
				return G::localVariable(mIndex).template as<T>();
			}

		private:
			size_t mIndex;
		};

		template <typename T>
		struct Proxy
		{
			
			Proxy(T t = {}) :
				mData(t) {}

			T *operator->()
			{
				return &mData;
			}

			T &operator*()
			{
				return mData;
			}

			operator T&()
			{
				return mData;
			}

			void operator=(const T &t)
			{
				mData = t;
			}

		private:
			T mData;
	};

#if !EMSCRIPTEN
#	define THREADLOCAL(T) thread_local ::Engine::Threading::Proxy<T>
#else
#	define THREADLOCAL(T) ::Engine::Threading::ThreadLocal<T>
#	define thread_local provoke syntax error
#endif

	}
}