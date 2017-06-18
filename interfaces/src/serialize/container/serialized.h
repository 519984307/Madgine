#pragma once

#include "../serializable.h"
#include "unithelper.h"

namespace Engine {
	namespace Serialize {

		template <class T>
		class Serialized : public Serializable, protected UnitHelper<T> {
		public:
			template <class... _Ty>
			Serialized(_Ty&&... args) :
				mData(std::forward<_Ty>(args)...)
			{				
				this->postConstruct(mData);
				if (topLevel())
					this->setItemTopLevel(mData, topLevel());
			}

			template <class Ty>
			void operator =(Ty&& v) {
				if (mData != v) {
					mData = std::forward<Ty>(v);
					notify();
				}
			}

			T *operator->() {
				return &mData;
			}

			T *ptr() {
				return &mData;
			}

			operator const T &() const {
				return mData;
			}

			virtual void readState(SerializeInStream &in) override {
				this->read_id(in, mData);
				this->read_state(in, mData);
				notify();
			}

			virtual void writeState(SerializeOutStream &out) const override {
				this->write_id(out, mData);
				this->write_state(out, mData);
			}

			template <class Ty>
			void setCallback(Ty &slot) {
				mNotifySignal.connect(slot);
			}

		protected:
			void notify() {
				mNotifySignal.emit(mData);
			}

		private:
			T mData;		
			SignalSlot::Signal<const T &> mNotifySignal;			
		};

	}
}