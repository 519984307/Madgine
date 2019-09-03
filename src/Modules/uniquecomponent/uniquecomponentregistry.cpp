#include "../moduleslib.h"

#if ENABLE_PLUGINS

#include "uniquecomponentregistry.h"

#include "../plugins/binaryinfo.h"

#include "Interfaces/stringutil.h"

#include "Interfaces/filesystem/api.h"

namespace Engine {

	static size_t sLevel = 0;

struct CompareTypeInfo {
    bool operator()(const TypeInfo *t1, const TypeInfo *t2) const
    {
        return strcmp(t1->mFullName, t2->mFullName) == -1;
    }
};

struct GuardGuard {
    GuardGuard(std::ostream &o, const Plugins::BinaryInfo *b)
        : out(o)
        , bin(b)
    {
        if (bin) {
            out << "#" << std::string(4 * sLevel, ' ') << "ifdef BUILD_" << bin->mName << "\n";
            ++sLevel;
        }
    }

    ~GuardGuard()
    {
        if (bin) {
            --sLevel;
            out << "#" << std::string(4 * sLevel, ' ') << "endif\n";
        }
    }

    std::ostream &out;
    const Plugins::BinaryInfo *bin;
};

std::string fixInclude(const char *pStr, const Plugins::BinaryInfo *binInfo)
{
    Filesystem::Path p = Filesystem::makeNormalized(pStr);
    return p.relative(binInfo->mSourceRoot).str();
};

void include(std::ostream &out, std::string header,
    const Plugins::BinaryInfo *bin = nullptr)
{
    GuardGuard g(out, bin);
    out << "#" << std::string(4 * sLevel, ' ') << "include \"" << StringUtil::replace(header, ".cpp", ".h") << "\"\n";
}

static std::vector<const TypeInfo *> &sSkip()
{
    static std::vector<const TypeInfo *> dummy;
    return dummy;
}

void skipUniqueComponentOnExport(const TypeInfo *t) {
    sSkip().push_back(t);
}

void exportStaticComponentHeader(const Filesystem::Path &outFile)
{
    LOG("Exporting uniquecomponent configuration source file '" << outFile << "'");

    std::set<const Plugins::BinaryInfo *> binaries;

    auto notInSkip = [&](const TypeInfo *v) {
        return std::find_if(sSkip().begin(), sSkip().end(), [=](const TypeInfo *v2) {
            return strcmp(v->mFullName, v2->mFullName) == 0;
        }) == sSkip().end();
    };

    for (auto &[name, reg] : registryRegistry()) {
        binaries.insert(reg->mBinary);

        for (CollectorInfo *collector : *reg) {
            binaries.insert(collector->mBinary);
        }
    }

    std::ofstream file(outFile.str());
    assert(file);

    include(file, "Modules/moduleslib.h");

    for (const Plugins::BinaryInfo *bin : binaries) {
        if (strlen(bin->mPrecompiledHeaderPath)) {
            include(file, bin->mPrecompiledHeaderPath, bin);
        }
    }

    for (auto &[name, reg] : registryRegistry()) {
        const Plugins::BinaryInfo *bin = reg->mBinary;
        include(file, fixInclude(reg->type_info()->mHeaderPath, bin), bin);

        for (CollectorInfo *collector : *reg) {
            for (const TypeInfo *typeInfo : collector->mElementInfos) {
                if (notInSkip(typeInfo))
                    include(
                        file,
                        fixInclude(typeInfo->mHeaderPath, collector->mBinary),
                        collector->mBinary);
            }
        }
    }

    file << R"(

namespace Engine {

)";

    for (auto &[name, reg] : registryRegistry()) {
        GuardGuard g2(file, reg->mBinary);

        file << R"(template <>
const std::vector<const Engine::MetaTable *> &)" << name
             << R"(::sTables() 
{
	static std::vector<const Engine::MetaTable *> dummy = {
)";

        for (CollectorInfo *collector : *reg) {
            GuardGuard g(file, collector->mBinary);
            for (const TypeInfo *typeInfo : collector->mElementInfos) {
                while (typeInfo->mDecayType)
                    typeInfo = typeInfo->mDecayType;
                if (notInSkip(typeInfo))
                    file << "		&table<"
                         << typeInfo->mFullName << ">(),\n";
            }
        }

        file << R"(
	}; 
	return dummy;
}
)";

        file << R"(template <>
std::vector<)" << name << "::F> " << name
             << R"(::sComponents()
{
	return {
)";

        for (CollectorInfo *collector : *reg) {
            GuardGuard g(file, collector->mBinary);
            for (const TypeInfo *typeInfo : collector->mElementInfos) {
                if (notInSkip(typeInfo))
                    file << "		createComponent<"
                         << typeInfo->mFullName << ">,\n";
            }
        }

        file << R"(
	}; 
}

#    define ACC 0

)";

        for (CollectorInfo *collector : *reg) {
            GuardGuard g(file, collector->mBinary);
            file << "constexpr size_t CollectorBaseIndex_"
                 << collector->mBaseInfo->mTypeName << "_"
                 << collector->mBinary->mName << " = ACC;\n";
            size_t i = 0;
            for (const TypeInfo *typeInfo : collector->mElementInfos) {
                if (notInSkip(typeInfo)) {
                    while (typeInfo) {
                        file << R"(template <>
size_t component_index<)"
                             << typeInfo->mFullName
                             << ">() { return CollectorBaseIndex_"
                             << collector->mBaseInfo->mTypeName << "_"
                             << collector->mBinary->mName << " + " << i
                             << "; }\n";
                        typeInfo = typeInfo->mDecayType;
                    }
                    ++i;
                }
            }
            file << "#        undef ACC\n"
                 << "#        define ACC CollectorBaseIndex_"
                 << collector->mBaseInfo->mTypeName << "_"
                 << collector->mBinary->mName << " + " << i << "\n";
        }

        file << "\n#    undef ACC\n\n";
    }

    file << "}\n";
}

MODULES_EXPORT std::map<std::string, ComponentRegistryBase *> &
registryRegistry()
{
    static std::map<std::string, ComponentRegistryBase *> dummy;
    return dummy;
}

} // namespace Engine

#endif