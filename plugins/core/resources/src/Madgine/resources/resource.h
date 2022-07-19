#pragma once

namespace Engine {
namespace Resources {

    template <typename Interface>
    struct ResourceDataInfoBase {
        ResourceDataInfoBase(typename Interface::Resource *res)
            : mResource(res)
        {
        }

        ResourceDataInfoBase(ResourceDataInfoBase &&other) noexcept
            : mResource(other.mResource)
            , mRefCount(other.mRefCount.exchange(0))
        {
        }

        ResourceDataInfoBase &operator=(ResourceDataInfoBase &&other) noexcept
        {
            mResource = other.mResource;
            mRefCount = other.mRefCount.exchange(0);
            return *this;
        }

        typename Interface::Resource *resource() const
        {
            return mResource;
        }

        void incRef()
        {
            ++mRefCount;
        }

        bool decRef()
        {
            uint32_t oldCount = mRefCount--;
            assert(oldCount > 0);
            return oldCount > 1 || mPersistent;
        }

        void setPersistent(bool b)
        {
            mPersistent = b;
        }

        void setLoadingTask(Threading::TaskFuture<bool> task)
        {
            assert(!mLoadingTask.valid());
            assert(!mUnloadingTask.valid() || mUnloadingTask.is_ready());
            mUnloadingTask.reset();
            mLoadingTask = task;
        }

        void setUnloadingTask(Threading::TaskFuture<void> task)
        {
            assert(!mUnloadingTask.valid());
            assert(mLoadingTask.is_ready());
            mLoadingTask.reset();
            mUnloadingTask = task;
        }

        bool verify()
        {
            return mLoadingTask.valid() && mLoadingTask.is_ready() && mLoadingTask && !mUnloadingTask.valid();
        }

        Threading::TaskFuture<bool> loadingTask()
        {
            return mLoadingTask;
        }

        Threading::TaskFuture<void> unloadingTask()
        {
            return mUnloadingTask;
        }

    private:
        typename Interface::Resource *mResource;
        std::atomic<uint32_t> mRefCount = 0;
        bool mPersistent = false;
        Threading::TaskFuture<bool> mLoadingTask;
        Threading::TaskFuture<void> mUnloadingTask;
    };

    template <typename Interface>
    struct ResourceDataInfo : ResourceDataInfoBase<Interface> {
        using ResourceDataInfoBase<Interface>::ResourceDataInfoBase;
    };

    template <typename Loader>
    struct ResourceData {

        ResourceData(typename Loader::Resource *res)
            : mInfo(res)
        {
        }

        typename Loader::Data *verified(bool verified)
        {
            if (verified && !mInfo.verify())
                return nullptr;
            return std::addressof(mData);
        }

        typename Loader::ResourceDataInfo mInfo;
        typename Loader::Data mData;
    };

    template <typename Loader>
    struct Resource : Loader::Interface::Resource {

        Resource(const std::string &name, const Filesystem::Path &path, typename Loader::Ctor ctor = {})
            : Loader::Interface::Resource(name, path)
            , mCtor(ctor ? std::move(ctor) : Loader::Interface::template toCtor<Loader>(&Loader::loadImpl))
        {
        }

        typename Loader::Handle loadData()
        {
            return Loader::load(this);
        }

        Threading::TaskFuture<void> forceUnload()
        {
            return Loader::unload(this);
        }

        typename Loader::Data *dataPtr()
        {
            return Loader::getDataPtr(loadData());
        }

        typename Loader::Ctor mCtor;

        typename Loader::Storage::template container_type<typename container_traits<typename Loader::DataContainer>::position_handle> mHolder;
    };

}
}