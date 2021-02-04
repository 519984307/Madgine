#include "../moduleslib.h"

#if ENABLE_PLUGINS

#    include "uniquecomponentregistry.h"

#    include "../plugins/binaryinfo.h"

#    include "Interfaces/util/stringutil.h"

#    include "Interfaces/filesystem/api.h"

#include "codegen/codegen_cpp.h"

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
    Filesystem::Path p = pStr;
    if (p.isRelative())
        p = Filesystem::Path { BINARY_OUT_DIR } / p;
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

void skipUniqueComponentOnExport(const TypeInfo *t)
{
    sSkip().push_back(t);
}

void exportStaticComponentHeader(const Filesystem::Path &outFile, bool hasTools)
{
    LOG("Exporting uniquecomponent configuration source file '" << outFile << "'");
    LOG_WARNING("TODO: Export in alphabetical order to prevent changes in the version control.");

    std::set<const Plugins::BinaryInfo *> binaries;

    auto notInSkip = [&](const TypeInfo *v) {
        return std::find_if(sSkip().begin(), sSkip().end(), [=](const TypeInfo *v2) {
            return strcmp(v->mFullName, v2->mFullName) == 0;
        }) == sSkip().end();
    };

    auto skipBinary = [&](const Plugins::BinaryInfo *bin) {
        return !hasTools && StringUtil::endsWith(bin->mName, "Tools");
    };

    for (auto &[name, reg] : registryRegistry()) {
        if (skipBinary(reg->mBinary))
            continue;
        binaries.insert(reg->mBinary);

        for (CollectorInfoBase *collector : *reg) {
            if (!skipBinary(collector->mBinary))
                binaries.insert(collector->mBinary);
        }
    }

    CodeGen::CppFile file;

    file.include(0, "Modules/moduleslib.h");
    file.include(0, "Modules/uniquecomponent/uniquecomponentregistry.h");



    for (const Plugins::BinaryInfo *bin : binaries) {
        if (strlen(bin->mPrecompiledHeaderPath)) {
            file.beginCondition("BUILD_"s + bin->mName);
            file.include(0, bin->mPrecompiledHeaderPath);
            file.endCondition("BUILD_"s + bin->mName);
        }
    }

    for (auto &[name, reg] : registryRegistry()) {
        const Plugins::BinaryInfo *bin = reg->mBinary;
        if (skipBinary(bin))
            continue;
        file.beginCondition("BUILD_"s + bin->mName);
        file.include(1, fixInclude(reg->type_info()->mHeaderPath, bin));

        for (CollectorInfoBase *collector : *reg) {
            if (skipBinary(collector->mBinary))
                continue;
            for (const std::vector<const TypeInfo *> &typeInfos : collector->mElementInfos) {
                const TypeInfo *typeInfo = typeInfos.front();
                if (notInSkip(typeInfo)) {
                    file.beginCondition("BUILD_"s + collector->mBinary->mName);
                    file.include(1, fixInclude(typeInfo->mHeaderPath, collector->mBinary));
                    file.endCondition("BUILD_"s + collector->mBinary->mName);
                }
            }
        }
        file.endCondition("BUILD_"s + bin->mName);
    }

    file.beginNamespace("Engine");

    for (auto &[name, reg] : registryRegistry()) {
        if (skipBinary(reg->mBinary))
            continue;
        file.beginCondition("BUILD_"s + reg->mBinary->mName);
        
        file << R"(template <>
std::vector<)"
             << name << "::F> " << name
             << R"(::sComponents()
{
	return {
)";

        for (CollectorInfoBase *collector : *reg) {
            if (skipBinary(collector->mBinary))
                continue;
            file.beginCondition("BUILD_"s + collector->mBinary->mName);
            for (const std::vector<const TypeInfo *> &typeInfos : collector->mElementInfos) {
                const TypeInfo *typeInfo = typeInfos.front();
                if (notInSkip(typeInfo))
                    file << "		createComponent<"
                         << typeInfo->mFullName << ">,\n";
            }
            file.endCondition("BUILD_"s + collector->mBinary->mName);
        }

        file << R"(
	}; 
}

#    define ACC 0

)";

        for (CollectorInfoBase *collector : *reg) {
            if (skipBinary(collector->mBinary))
                continue;
            file.beginCondition("BUILD_"s + collector->mBinary->mName);            
            file << "constexpr size_t CollectorBaseIndex_"
                 << collector->mBaseInfo->mTypeName << "_"
                 << collector->mBinary->mName << " = ACC;\n";
            size_t i = 0;
            for (const std::vector<const TypeInfo *> &typeInfos : collector->mElementInfos) {
                if (notInSkip(typeInfos.front())) {
                    for (const TypeInfo *typeInfo : typeInfos) {
                        while (typeInfo) {
                            file << R"(template <>
size_t component_index<)"
                                 << typeInfo->mFullName
                                 << ">() { return CollectorBaseIndex_"
                                 << collector->mBaseInfo->mTypeName << "_"
                                 << collector->mBinary->mName << " + " << i
                                 << "; }\n";
                            typeInfo = typeInfo->mDecayType ? typeInfo->mDecayType : nullptr;
                        }
                    }
                    ++i;
                }
            }
            file << "#        undef ACC\n"
                 << "#        define ACC CollectorBaseIndex_"
                 << collector->mBaseInfo->mTypeName << "_"
                 << collector->mBinary->mName << " + " << i << "\n";
            file.endCondition("BUILD_"s + collector->mBinary->mName);
        }

        file << "\n#    undef ACC\n\n";

        file.endCondition("BUILD_"s + reg->mBinary->mName);
    }

    file.endNamespace();

    std::ofstream stream(outFile.str());
    assert(stream);

    file.generate(stream);
}

MODULES_EXPORT std::map<std::string, ComponentRegistryBase *> &
registryRegistry()
{
    static std::map<std::string, ComponentRegistryBase *> dummy;
    return dummy;
}

} // namespace Engine

#endif