#pragma once

#include "connection.h"
#include "connectionmanager.h"

namespace Engine
{
	namespace SignalSlot
	{
		template <class... _Ty>
		class Signal
		{
		public:
			Signal() = default;

			Signal(const Signal<_Ty...>& other)
			{
			}

			Signal(Signal<_Ty...>&&) noexcept
			{
			}

			~Signal()
			{
				disconnectAll();
			}

			Signal<_Ty...>& operator=(const Signal<_Ty...>& other) = delete;

			void emit(_Ty ... args)
			{
				auto end = mConnectedSlots.end();

				mConnectedSlots.erase(
					std::remove_if(
						mConnectedSlots.begin(), 
						end,
						[&](const std::weak_ptr<ConnectionInstance<_Ty...>> &p) {
							if (std::shared_ptr<ConnectionInstance<_Ty...>> ptr = p.lock())
							{
								(*ptr)(p, args...);
								return false;
							}
							else
							{
								return true;
							}
						}
					),
					end
				);

			}


			template <class T, class _ = std::enable_if_t<has_store<T>::value>>
			std::weak_ptr<ConnectionBase> connect(T& slot, DirectConnectionType = {})
			{
				std::weak_ptr<ConnectionInstance<_Ty...>> conn = slot.connectionStore().template create<
					DirectConnection, _Ty...>(&slot);
				mConnectedSlots.emplace_back(
					conn
				);
				return conn;
			}

			template <class T, class _ = std::enable_if_t<has_store<T>::value>>
			std::weak_ptr<ConnectionBase> connect(T& slot, QueuedConnectionType)
			{
				std::weak_ptr<ConnectionInstance<_Ty...>> conn = slot.connectionStore().template create<QueuedConnection, _Ty...>(
					&slot, slot.manager());
				mConnectedSlots.emplace_back(
					conn
				);
				return conn;
			}

			template <class T, class _ = std::enable_if_t<!has_store<T>::value>>
			std::weak_ptr<ConnectionBase> connect(T&& slot, DirectConnectionType = {})
			{
				std::weak_ptr<ConnectionInstance<_Ty...>> conn = ConnectionStore::globalStore().create<DirectConnection, _Ty...>(
					std::forward<T>(slot));
				mConnectedSlots.emplace_back(
					conn
				);
				return conn;
			}

			template <class T, class _ = std::enable_if_t<!has_store<T>::value>>
			std::weak_ptr<ConnectionBase> connect(T&& slot, QueuedConnectionType)
			{
				std::weak_ptr<ConnectionInstance<_Ty...>> conn = ConnectionStore::globalStore().create<QueuedConnection, _Ty...>(
					std::forward<T>(slot), ConnectionManager::getSingleton());
				mConnectedSlots.emplace_back(
					conn
				);
				return conn;
			}

			void disconnectAll()
			{
				for (const std::weak_ptr<ConnectionInstance<_Ty...>>& conn : mConnectedSlots)
				{
					if (std::shared_ptr<ConnectionInstance<_Ty...>> ptr = conn.lock())
					{
						ptr->disconnect();
					}
				}
				mConnectedSlots.clear();
			}

			void copyConnections(const Signal<_Ty...>& other)
			{
				for (const std::weak_ptr<ConnectionInstance<_Ty...>>& conn : other.mConnectedSlots)
				{
					if (std::shared_ptr<ConnectionInstance<_Ty...>> ptr = conn.lock())
					{
						mConnectedSlots.emplace_back(ptr->clone());
					}
				}
			}

		private:
			std::vector<std::weak_ptr<ConnectionInstance<_Ty...>>> mConnectedSlots;
		};
	}
}
