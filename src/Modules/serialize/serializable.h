#pragma once

namespace Engine {
namespace Serialize {

    struct SerializableBase {
        
    };

    template <typename OffsetPtr>
    struct Serializable : SerializableBase {
        bool isSynced() const
        {
            return OffsetPtr::parent(this) && OffsetPtr::parent(this)->isSynced();
        }

        bool isActive() const
        {
            return !OffsetPtr::parent(this) || OffsetPtr::parent(this)->isActive(OffsetPtr::parent(this)->serializeType()->getIndex(OffsetPtr::template offset<SerializableUnitBase>()));
        }
    };
}
}
