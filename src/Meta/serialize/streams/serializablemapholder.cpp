#include "../../metalib.h"

#include "serializablemapholder.h"
#include "serializestreambuf.h"
#include "serializestream.h"

namespace Engine {
namespace Serialize {

    SerializableMapHolder::SerializableMapHolder(SerializeOutStream &out)
    {
        if (!out.data().mSerializableMap) {
            mData = &out.data();
            mData->mSerializableMap = &mMap;
        }
    }

    SerializableMapHolder::~SerializableMapHolder()
    {
        if (mData) {
            assert(mData->mSerializableMap == &mMap);
            mData->mSerializableMap = nullptr;
        }
    }
    
    SerializableListHolder::SerializableListHolder(SerializeInStream &in)
    {
        if (!in.data().mSerializableList) {
            mData = &in.data();
            mData->mSerializableList = &mList;
        }
    }

    SerializableListHolder::~SerializableListHolder()
    {
        if (mData) {
            assert(mData->mSerializableList == &mList);
            mData->mSerializableList = nullptr;
        }
    }

}
}