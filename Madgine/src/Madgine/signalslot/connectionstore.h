#pragma once

namespace Engine
{
	namespace SignalSlot
	{
		class INTERFACES_EXPORT ConnectionStore
		{
		public:
			typedef std::list<std::shared_ptr<ConnectionBase>>::const_iterator const_iterator;

			template <class Con, class... Args>
			static std::shared_ptr<Con> create(std::shared_ptr<ConnectionBase> *prev, Args&&... args)
			{
				
				return std::static_pointer_cast<Con>(
						make_shared_connection(
							std::make_unique<Con>(prev, std::forward<Args>(args)...)
						)
					);
			}

			template <class Con, class... Args>
			std::weak_ptr<Con> emplace_front(Args&&... args)
			{
				std::shared_ptr<Con> ptr = create<Con>(&mBegin, std::forward<Args>(args)...);
				mBegin = ptr;
				return ptr;
			}

			ConnectionStore();

			void clear();

			static ConnectionStore& globalStore();

		private:
			static std::shared_ptr<ConnectionBase> make_shared_connection(std::unique_ptr<ConnectionBase> &&conn);

			std::shared_ptr<ConnectionBase> mBegin;
		};

		template <class T, class = void>
		struct has_store : std::false_type {};

		template <class T>
		struct has_store<T, std::void_t<std::enable_if_t<std::is_same_v<decltype(&T::connectionStore), ConnectionStore &(T::*)()>>>> : std::true_type {};

	}
}
