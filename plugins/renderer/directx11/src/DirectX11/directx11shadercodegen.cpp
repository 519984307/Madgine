#include "directx11lib.h"

#include "directx11shadercodegen.h"

#include "codegen/codegen_shader.h"

#include "Meta/math/vector4.h"

namespace Engine {
namespace Render {
    namespace DirectX11ShaderCodeGen {

        static std::map<ValueTypeDesc, std::string_view> sTypeMap {
            { toValueTypeDesc<Vector4>(), "float4" },
            { toValueTypeDesc<Vector3>(), "float3" },
            { toValueTypeDesc<Vector2>(), "float2" },
            { toValueTypeDesc<Vector4i>(), "int4" },
            { toValueTypeDesc<Vector3i>(), "int3" },
            { toValueTypeDesc<Vector2i>(), "int2" },
            { toValueTypeDesc<Matrix4>(), "float4x4" }
        };

        std::string generateIncludeGuard(const Engine::BitArray<64> &conditionals, const std::vector<std::string> &conditionalTokenList)
        {
            if (conditionals == Engine::BitArray<64> {})
                return "";

            std::string result = "#if ";

            bool first = true;

            for (size_t i = 0; i < 64; ++i) {
                if (conditionals[i]) {
                    if (first)
                        first = false;
                    else
                        result += " && ";
                    result += "defined(" + conditionalTokenList[i] + ")";
                }
            }

            return result + "\n";
        }

        void generateType(std::ostream &stream, const CodeGen::Type &type)
        {
            std::visit(overloaded {
                           [&](const ValueTypeDesc &type) {
                               stream << sTypeMap.at(type);
                           },
                           [&](CodeGen::Struct *structInfo) {
                               stream << structInfo->mName;
                           } },
                type);
        }

        void generate(std::ostream &stream, const CodeGen::Statement &statement);

        void generate(std::ostream &stream, const Vector4 &v)
        {
            stream << "float4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
        }

        void generate(std::ostream &stream, const Vector3 &v)
        {
            stream << "float4(" << v.x << ", " << v.y << ", " << v.z << ")";
        }

        void generate(std::ostream &stream, const Void &v)
        {
            throw 0;
        }

        void generate(std::ostream &stream, const std::string_view &v)
        {
            stream << "\"" << v << "\"";
        }

        void generate(std::ostream &stream, const float &f)
        {
            stream << f;
        }

        template <typename T>
        void generate(std::ostream &stream, const T &v)
        {
            throw 0;
        }

        void generate(std::ostream &stream, const CodeGen::Assignment &statement)
        {
            stream << statement.mVariableName << " = ";
            generate(stream, *statement.mStatement);
        }

        void generate(std::ostream &stream, const CodeGen::Return &statement)
        {
            stream << "return ";
            generate(stream, *statement.mStatement);
        }

        void generate(std::ostream &stream, const CodeGen::VariableRead &statement)
        {
            stream << statement.mVariableName;
        }

        void generate(std::ostream &stream, const CodeGen::VariableDefinition &def)
        {
            generateType(stream, def.mVariable.mType);
            stream << " " << def.mVariable.mName;
            if (!def.mDefaultValue.is<std::monostate>()) {
                stream << " = ";
                def.mDefaultValue.visit([&](const auto &part) { generate(stream, part); });
            }
        }

        void generate(std::ostream &stream, const CodeGen::MemberAccess &access)
        {
            generate(stream, *access.mStatement);
            stream << "." << access.mMemberName;
        }

        void generate(std::ostream &stream, const CodeGen::Namespace &ns)
        {
            throw 0;
        }

        void generate(std::ostream &stream, const CodeGen::CustomCodeBlock &block)
        {
            throw 0;
        }

        void generate(std::ostream &stream, const CodeGen::ArithOperation &op)
        {
            const char *opCode;
            switch (op.mType) {
            case CodeGen::ArithOperation::ADD:
                opCode = " + ";
                break;
            case CodeGen::ArithOperation::MUL:
                stream << "mul(";
                opCode = ", ";
                break;
            default:
                throw 0;
            }

            bool first = true;
            for (const CodeGen::Statement &statement : op.mOperands) {
                if (first)
                    first = false;
                else
                    stream << opCode;
                generate(stream, statement);
            }

            switch (op.mType) {
            case CodeGen::ArithOperation::MUL:
                stream << ")";
                break;
            default:
                break;
            }
        }

        void generate(std::ostream &stream, const CodeGen::Constructor &ctor)
        {
            generateType(stream, ctor.mType);
            stream << "(";
            bool first = true;
            for (const CodeGen::Statement &statement : ctor.mArguments) {
                if (first)
                    first = false;
                else
                    stream << ",";
                generate(stream, statement);
            }
            stream << ")";
        }

        void generate(std::ostream &stream, const CodeGen::Constant &c)
        {
            c.mValue.visit([&](const auto &part) { generate(stream, part); });
        }

        void generate(std::ostream &stream, const CodeGen::Statement &statement)
        {
            std::visit([&](const auto &part) { generate(stream, part); }, statement);
        }

        const char *guessSemantic(std::string_view name, bool rasterized)
        {
            if (name.find("pos") != std::string_view::npos)
                return rasterized ? "SV_POSITION" : "POSITION";
            else if (name.find("col") != std::string_view::npos)
                return "COLOR";
            else if (StringUtil::toLower(name).find("uv") != std::string::npos)
                return "TEXCOORD";
            else if (StringUtil::toLower(name).find("normal") != std::string::npos)
                return "NORMAL";
            else if (StringUtil::toLower(name).find("bone") != std::string::npos)
                return "BONEINDICES";
            else if (StringUtil::toLower(name).find("weights") != std::string::npos)
                return "WEIGHTS";
            else
                throw 0;
        }

        const char *guessDefault(std::string_view name)
        {
            if (name.find("pos2") != std::string_view::npos)
                return "float2(0,0)";
            else if (name.find("pos") != std::string_view::npos)
                return "float3(0,0,0)";
            else if (name.find("col") != std::string_view::npos)
                return "float4(1,1,1,1)";
            else if (StringUtil::toLower(name).find("uv") != std::string::npos)
                return "float2(0,0)";
            else if (StringUtil::toLower(name).find("normal") != std::string::npos)
                return "float3(0,0)";
            else if (StringUtil::toLower(name).find("bone") != std::string::npos)
                return "int4(0,0,0,0)";
            else if (StringUtil::toLower(name).find("weights") != std::string::npos)
                return "float4(0,0,0,0)";
            else
                throw 0;
        }

        void generate(std::ostream &stream, const CodeGen::ShaderFile &file, uint32_t index)
        {
            for (const std::pair<const std::string, CodeGen::Struct> &structInfo : file.mStructs) {

                if (structInfo.first == "VertexData") {
                    if (index == 0) {
                        stream << "struct VertexDataIn {\n";
                        for (const CodeGen::Variable &arg : structInfo.second.mVariables) {
                            stream << generateIncludeGuard(arg.mConditionals, file.mConditionalTokenList);
                            stream << "\t";
                            generateType(stream, arg.mType);
                            stream << " " << arg.mName << " : " << guessSemantic(arg.mName, false) << ";\n";
                            if (arg.mConditionals != Engine::BitArray<64> {})
                                stream << "#endif\n";
                        }
                        stream << "};\n";
                    } else {
                        continue;
                    }
                }

                bool isBuffer = !structInfo.second.mAnnotations.empty() && StringUtil::startsWith(structInfo.second.mAnnotations.front(), "buffer");
                if (isBuffer) {
                    std::string_view number = std::string_view { structInfo.second.mAnnotations.front() }.substr(strlen("buffer"));
                    uint32_t bufferIndex = std::atoi(number.data());
                    stream << "cbuffer " << structInfo.second.mName << " : register(b" << bufferIndex << ")";
                } else
                    stream << "struct " << structInfo.second.mName;
                stream << "{\n";

                for (const CodeGen::Variable &arg : structInfo.second.mVariables) {
                    stream << "\t";
                    for (const std::string &annotation : arg.mAnnotations) {

                        stream << annotation << " ";
                    }
                    generateType(stream, arg.mType);
                    stream << " " << arg.mName;
                    if (structInfo.second.mName == "RasterizerData" && !structInfo.second.mAnnotations.empty() && structInfo.second.mAnnotations.front() == "semantic")
                        stream << " : " << guessSemantic(arg.mName, true);
                    stream << ";\n";
                }
                stream << "};\n\n";
            }

            const std::vector<CodeGen::Function> &functions = *std::next(file.mInstances.begin(), index);
            for (const CodeGen::Function &function : functions) {
                std::string_view name = function.mName == "main" ? "mainImpl" : std::string_view { function.mName };
                generateType(stream, function.mReturnType);
                stream << " " << name << "(";
                bool first = true;
                for (const CodeGen::Variable &arg : function.mArguments) {
                    generateType(stream, arg.mType);
                    stream << " " << arg.mName;
                    if (first)
                        first = false;
                    else
                        stream << ",";
                }
                stream << ")";

                stream << "{\n";

                for (const CodeGen::Statement &statement : function.mStatements) {
                    stream << "\t";
                    generate(stream, statement);
                    stream << ";\n";
                }

                stream << "}\n\n";
            }

            if (index == 0) {
                stream << "RasterizerData main(VertexDataIn dataIn){\n";
                stream << "\tVertexData IN;\n";
                for (const CodeGen::Variable &inVar : file.mStructs.at("VertexData").mVariables) {
                    stream << generateIncludeGuard(inVar.mConditionals, file.mConditionalTokenList);
                    stream << "\tIN." << inVar.mName << " = dataIn." << inVar.mName << ";\n";
                    if (inVar.mConditionals != Engine::BitArray<64> {}) {
                        stream << "#else\n";
                        stream << "\tIN." << inVar.mName << " = " << guessDefault(inVar.mName) << ";\n";
                        stream << "#endif\n";
                    }
                }
                stream << "\treturn mainImpl(IN);\n";
                stream << "}\n\n";
            } else {
                stream << "float4 main(RasterizerData IN) : SV_TARGET {\n";
                stream << "\treturn mainImpl(IN);\n";
                stream << "}\n\n";
            }

        }

    }

}
}