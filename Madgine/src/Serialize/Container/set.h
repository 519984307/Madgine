#pragma once

#include "serializablecontainer.h"
#include "creationhelper.h"
#include "observablecontainer.h"
#include "unithelper.h"

namespace Engine {
	namespace Serialize {

		template <class T>
		class Keyed {
		public:
			typedef T KeyType;

			virtual KeyType key() const = 0;
		};

		template <class T>
		struct KeyHelper {
			typedef typename T::KeyType KeyType;

			static KeyType key(const T &t) {
				return t.key();
			}
		};

		template <class T>
		struct KeyHelper<Ogre::unique_ptr<T>> {
			typedef typename KeyHelper<T>::KeyType KeyType;

			static KeyType key(const Ogre::unique_ptr<T> &t) {
				return KeyHelper<T>::key(*t);
			}
		};
			

		template <class T>
		class KeyHolder {
		public:
			typedef typename KeyHelper<T>::KeyType KeyType;

			template <class... _Ty>
			KeyHolder(_Ty&&... args) :
				mItem(std::forward<_Ty>(args)...){
				mKey = KeyHelper<T>::key(mItem);
			}

			const KeyType &key() const {
				return mKey;
			}

			operator T &() const {
				return mItem;
			}

			bool operator < (const KeyHolder<T> &other) const {
				return mKey < other.mKey;
			}

			mutable T mItem;
		private:
			KeyType mKey;			
		};

		template <class T>
		class SetConstIterator;

		template <class T>
		class SetIterator : public std::iterator<typename std::iterator_traits<typename std::set<T>::iterator>::iterator_category, T>{
		public:
			typedef typename std::set<KeyHolder<T>>::iterator It;
			typedef typename KeyHolder<T>::KeyType KeyType;

			SetIterator() {}
			SetIterator(It&& it) :
				mIterator(std::forward<It>(it)) {}
			SetIterator(const It &it) :
				mIterator(it) {}

			T &operator *() const {
				return mIterator->mItem;
			}

			T *operator ->() const {
				return &mIterator->mItem;
			}

			const KeyType &key() const {
				return mIterator->key();
			}

			bool operator !=(const SetIterator<T> &other) const {
				return mIterator != other.mIterator;
			}
			bool operator ==(const SetIterator<T> &other) const {
				return mIterator == other.mIterator;
			}

			void operator++() {
				++mIterator;
			}

		private:
			friend class SetConstIterator<T>;
			friend class BaseContainer<std::set<T>>;

			It mIterator;
		};

		template <class T>
		class SetConstIterator : public std::iterator<typename std::iterator_traits<typename std::set<T>::iterator>::iterator_category, T> {
		public:
			typedef typename std::set<KeyHolder<T>>::const_iterator It;
			typedef typename KeyHolder<T>::KeyType KeyType;

			SetConstIterator(It &&it) :
				mIterator(std::forward<It>(it)) {}
			SetConstIterator(const It &it) :
				mIterator(it) {}
			SetConstIterator(const SetIterator<T> &it) :
				mIterator(it.mIterator) {}

			const T &operator *() const {
				return mIterator->mItem;
			}

			const T *operator ->() const {
				return &mIterator->mItem;
			}

			const KeyType &key() const {
				return mIterator->key();
			}

			bool operator !=(const SetConstIterator<T> &other) const {
				return mIterator != other.mIterator;
			}
			bool operator ==(const SetConstIterator<T> &other) const {
				return mIterator == other.mIterator;
			}
			

			void operator++() {
				++mIterator;
			}

		private:
			friend class BaseContainer<std::set<T>>;

			It mIterator;
		};

		template <class T>
		class BaseContainer<std::set<T>> : protected UnitHelper<T> {
		public:
			static constexpr const bool sorted = true;
		protected:
			typedef typename SetIterator<Type> iterator;
			typedef typename SetConstIterator<Type> const_iterator;


			

			template <class... _Ty>
			iterator insert_where(const iterator &where, _Ty&&... args) {
				return mData.emplace_hint(where.mIterator, std::forward<_Ty>(args)...);
			}

			const_iterator erase(const const_iterator &it) {
				return mData.erase(it.mIterator);
			}


		protected:
			std::set<KeyHolder<Type>> mData;

		};


		template <class T, class... Args>
		class SerializableSet : public SerializableContainer<std::set<T>, Creator<Args...>> {
		public:
			using SerializableContainer::SerializableContainer;

			typedef typename KeyHolder<T>::KeyType KeyType;

			iterator find(const T &item) {
				return std::find(begin(), end(), item);
			}

			const_iterator find(const T &item) const {
				return std::find(begin(), end(), item);
			}

			bool contains(const T &item) const {
				return find(item) != end();
			}

			iterator find(const KeyType &key) {				
				for (iterator it = begin(); it != end(); ++it) {
					if (it.key() == key)
						return it; ////TODO  O(log(n))
				}
				return end();
			}

			bool contains(const KeyType &key) {
				return find(key) != end();
			}

			template <class... _Ty>
			iterator emplace(_Ty&&... args) {
				return insert_where(end(), std::forward<_Ty>(args)...);
			}

		};

		template <class T, class... Args>
		class ObservableSet : public ObservableContainer<SerializableSet<T, Args...>> {
		public:
			using ObservableContainer::ObservableContainer;

		};





	}
}


