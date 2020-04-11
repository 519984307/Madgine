#pragma once

#include "../serialize/formatter.h"

namespace Engine {
namespace Ini {

struct MODULES_EXPORT IniFormatter : Serialize::Formatter {
        
		IniFormatter();

        virtual void beginPrimitive(Serialize::SerializeOutStream &, const char *name, size_t typeId, bool closeExtended) override;
                virtual void endPrimitive(Serialize::SerializeOutStream &, const char *name, size_t typeId) override;

        virtual void beginPrimitive(Serialize::SerializeInStream &, const char *name, size_t typeId, bool closeExtended) override;
                virtual void endPrimitive(Serialize::SerializeInStream &, const char *name, size_t typeId) override;

		virtual std::string lookupFieldName(Serialize::SerializeInStream &) override;
};

}
}