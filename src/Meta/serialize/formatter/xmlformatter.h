#pragma once

#include "Meta/serialize/formatter.h"

namespace Engine {
namespace Serialize {

    struct META_EXPORT XMLFormatter : Formatter {

        XMLFormatter();

        virtual void setupStream(std::istream &) override;
        virtual void setupStream(std::ostream &) override; 

        virtual void beginExtended(SerializeOutStream &, const char *name, size_t count) override;

        virtual void beginCompound(SerializeOutStream &, const char *name) override;
        virtual void endCompound(SerializeOutStream &, const char *name) override;

        virtual StreamResult beginExtended(SerializeInStream &, const char *name, size_t count) override;

        virtual StreamResult beginCompound(SerializeInStream &, const char *name) override;
        virtual StreamResult endCompound(SerializeInStream &, const char *name) override;

        virtual void beginPrimitive(SerializeOutStream &, const char *name, uint8_t typeId) override;
        virtual void endPrimitive(SerializeOutStream &, const char *name, uint8_t typeId) override;

        virtual StreamResult beginPrimitive(SerializeInStream &, const char *name, uint8_t typeId) override;
        virtual StreamResult endPrimitive(SerializeInStream &, const char *name, uint8_t typeId) override;

        virtual std::string lookupFieldName(SerializeInStream &) override;

        virtual void beginContainer(SerializeOutStream &, const char *name, uint32_t size) override;
        virtual void endContainer(SerializeOutStream &, const char *name) override;
        virtual StreamResult beginContainer(SerializeInStream &, const char *name, bool sized) override;
        virtual StreamResult endContainer(SerializeInStream &, const char *name) override;
        virtual bool hasContainerItem(SerializeInStream &) override;

        std::string indent();

    private:
        StreamResult prefetchAttributes(SerializeInStream &in, const char *name = nullptr);
        StreamResult skipValue(SerializeInStream &);

    private:
        size_t mLevel = 0;
        bool mCurrentExtended = false;
        size_t mCurrentExtendedCount = 0;
        std::istream::pos_type mExtendedLookupPos = -1;
        std::map<std::string, std::istream::pos_type, std::less<>> mPrefetchedAttributes;
    };

}
}