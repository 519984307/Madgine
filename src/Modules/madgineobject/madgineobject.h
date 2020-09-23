#pragma once

#include "../generic/derive.h"
#include "../keyvalueutil/virtualscopebase.h"
#include "objectstate.h"

namespace Engine {

DERIVE_FUNCTION(parent);

struct DependencyInitException {
};

struct MadgineObjectState {

    bool isInitialized() const
    {
        return mState == ObjectState::INITIALIZED || mState == ObjectState::MARKED_INITIALIZED;
    }

    void checkInitState() const
    {
        if (mState == ObjectState::UNINITIALIZED)
            throw DependencyInitException {};
    }

        
    template <typename U>
    U &getChild(U &u, bool init = true)
    {
        if (init) {
            checkInitState();
            u.callInit();
        }
        return static_cast<U &>(u.getSelf(init));
    }


protected:
    ObjectState getState() const
    {
        return mState;
    }

    void checkDependency() const
    {
        switch (mState) {
        case ObjectState::UNINITIALIZED:
            throw DependencyInitException {};
        case ObjectState::INITIALIZING:
            LOG_WARNING("Possible circular dependency! Consider using markInitialized!");
            break;
        default:
            break;
        }
    }
    void markInitialized()
    {
        assert(mState == ObjectState::INITIALIZING);
        mState = ObjectState::MARKED_INITIALIZED;
    }


    ObjectState mState = ObjectState::UNINITIALIZED;
};

template <typename T>
struct MadgineObject : MadgineObjectState {
    bool callInit()
    {
        if (mState == ObjectState::UNINITIALIZED) {
            mState = ObjectState::INITIALIZING;
            bool result;
            mName = typeid(static_cast<T&>(*this)).name();
            try {
                LOG("Initializing: " << mName << "...");
                result = static_cast<T *>(this)->init();
            } catch (const DependencyInitException &) {
                result = false;
            }
            if (result)
                mState = ObjectState::INITIALIZED;
            else if (mState == ObjectState::INITIALIZING)
                mState = ObjectState::UNINITIALIZED;
            else if (mState == ObjectState::MARKED_INITIALIZED) {
                mState = ObjectState::INITIALIZED;
                callFinalize();
            } else
                std::terminate();
            LOG((isInitialized() ? "Success" : "Failure"));
        }
        return isInitialized();
    }
    bool callInit(int &count)
    {
        bool notInit = mState == ObjectState::UNINITIALIZED;
        bool result = callInit();
        if (notInit && mState == ObjectState::INITIALIZED)
            mOrder = ++count;
        return result;
    }

    void callFinalize()
    {
        if (mState == ObjectState::INITIALIZED) {
            static_cast<T *>(this)->finalize();
            mState = ObjectState::UNINITIALIZED;
        }
        assert(mState == ObjectState::UNINITIALIZED);
    }
    void callFinalize(int order)
    {
        if (mOrder == order)
            callFinalize();
    }

    static constexpr bool hasParent()
    {
        return has_function_parent_v<T>;
    }

    auto getParent() const
    {
        if constexpr (hasParent())
            return static_cast<const T *>(this)->parent();
        else
            return nullptr;
    }

    T &getSelf(bool init = true)
    {
        if (init) {
            checkDependency();
        }
        return static_cast<T &>(*this);
    }

protected:
    MadgineObject()
        : mOrder(-1)
    {
    }

    ~MadgineObject()
    {
        if (mState != ObjectState::UNINITIALIZED) {
            LOG_WARNING("Deleting still initialized Object: " << mName);
        }
    }

private:
    int mOrder;
    std::string_view mName;
};

}
