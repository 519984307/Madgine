#pragma once

namespace Engine {

struct MODULES_EXPORT ApiFunction {    

    bool operator==(const ApiFunction &other) const
    {
        return mTable == other.mTable;
    }

    operator bool() const
    {
        return mTable != nullptr;
    }

    void operator()(ValueType &retVal, const ArgumentList &args) const;

    size_t argumentsCount(bool excludeThis = false) const;
    bool isMemberFunction() const;
    
    const FunctionTable *mTable = nullptr;
};

}