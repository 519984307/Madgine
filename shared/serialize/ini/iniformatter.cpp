#include "inilib.h"

#include "iniformatter.h"

#include "Meta/serialize/streams/serializestream.h"

#include "Meta/serialize/primitivetypes.h"

namespace Engine {
namespace Ini {

    IniFormatter::IniFormatter()
        : Formatter(false, true)
    {
    }

    void IniFormatter::beginPrimitive(Serialize::SerializeOutStream &out, const char *name, uint8_t typeId)
    {
        out.writeUnformatted(std::string(name));
        out.writeUnformatted("="s);
    }

    void IniFormatter::endPrimitive(Serialize::SerializeOutStream &out, const char *name, uint8_t typeId)
    {
        out.writeUnformatted("\n"s);
    }

    void IniFormatter::beginPrimitive(Serialize::SerializeInStream &in, const char *name, uint8_t typeId)
    {

        std::string prefix;
        if (name) {
            prefix = in.readN(strlen(name) + 1);
            if (prefix != std::string(name) + "=")
                std::terminate();
        } else
            prefix = in.readUntil("=");

        if (typeId == Serialize::PrimitiveTypeIndex_v<std::string>) {
            in.setNextFormattedStringDelimiter('\n');
        }
    }

    void IniFormatter::endPrimitive(Serialize::SerializeInStream &in, const char *name, uint8_t typeId)
    {
    }

    std::string IniFormatter::lookupFieldName(Serialize::SerializeInStream &in)
    {
        std::string name = in.peekUntil("=");
        if (!name.empty())
            name = name.substr(0, name.size() - 1);
        return name;
    }

}
}