#pragma once

#include "../observable.h"
#include "serialized.h"


namespace Engine {
		namespace Serialize {

			template <class T>
			class Observed : public Observable, public Serialized<T>{
			public:
				Observed(SerializableUnit *parent, const T &data = T()) :
					mData(data),
					Observable(parent),
					Serialized(parent, data){

				}

				template <class T>
				void operator =(T&& v) {
					if (mData != v) {
						mData = std::forward<T>(v);
						notify();
					}
				}

				virtual void readChanges(SerializeInStream &in) override {
					in >> mData;
				}

				virtual void writeChanges(SerializeOutStream &out) const override {
					out << mData;
				}

				void notify() {
					Network::BaseNetworkStream &stream = mParent->prepareActionMessage(mIndex);
					writeChanges(stream.out());
					mParent->sendAction(stream);
				}

			};

			template <>
			class Observed<std::string> : public Observable, public Serialized<std::string> {
			public:
				Observed(SerializableUnit *parent, const std::string &data = "") :
					Observable(parent),
					Serialized(parent, data) {

				}

				template <class T>
				void operator =(T&& v) {
					if (mData != v) {
						mData = std::forward<T>(v);
						notify();
					}
				}


				bool empty() const {
					return mData.empty();
				}

				void clear() {
					if (!mData.empty()) {
						mData.clear();
						notify();
					}
				}

				virtual void readChanges(SerializeInStream &in) override {
					in >> mData;
				}

				void notify() {

					for (SerializeOutStream *out : getMessageTargets(true)) {
						*out << mData;
					}
					
				}

			};

		}
}