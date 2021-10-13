#pragma once

#include "resourcebase.h"
#include "resourceloadercollector.h"

#include "Modules/threading/globalstorage.h"

#include "handle.h"

#include "resourceloaderbase.h"

#include "Modules/uniquecomponent/uniquecomponent.h"
#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/keyvalue/virtualscope.h"

#include "Interfaces/filesystem/filewatcher.h"

namespace Engine {
namespace Resources {

    MADGINE_RESOURCES_EXPORT ResourceLoaderBase &getLoaderByIndex(size_t i);
    MADGINE_RESOURCES_EXPORT void waitForIOThread();

    template <typename Interface>
    struct ResourceDataInfoBase {
        ResourceDataInfoBase(typename Interface::ResourceType *res)
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

        typename Interface::ResourceType *resource() const
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

        void setPersistent(bool b) {
            mPersistent = b;
        }

    private:
        typename Interface::ResourceType *mResource;
        std::atomic<uint32_t> mRefCount = 0;
        bool mPersistent = false;
    };

    template <typename Interface>
    struct ResourceDataInfo : ResourceDataInfoBase<Interface> {
        using ResourceDataInfoBase<Interface>::ResourceDataInfoBase;
    };

    template <typename Loader>
    struct ResourceData {

        ResourceData(typename Loader::ResourceType *res)
            : mInfo(res)
        {
        }

        typename Loader::ResourceDataInfo mInfo;
        typename Loader::Data mData;
    };

    template <typename Loader>
    struct ResourceType : Loader::Interface::ResourceType {

        ResourceType(const std::string &name, const Filesystem::Path &path, typename Loader::Ctor ctor = {}, typename Loader::Dtor dtor = {})
            : Loader::Interface::ResourceType(name, path)
            , mCtor(ctor ? std::move(ctor) : [](Loader *loader, typename Loader::Data &data, typename Loader::ResourceDataInfo &info, Filesystem::FileEventType event) { return TupleUnpacker::invoke(&Loader::loadImpl, loader, data, info, event); })
            , mDtor(dtor ? std::move(dtor) : [](Loader *loader, typename Loader::Data &data, typename Loader::ResourceDataInfo &info) { loader->unloadImpl(data, info); })
        {
        }

        typename Loader::HandleType loadData()
        {
            return Loader::load(this);
        }

        typename Loader::Data *dataPtr()
        {
            return Loader::getDataPtr(loadData());
        }

        typename Loader::Ctor mCtor;
        typename Loader::Dtor mDtor;

        typename Loader::Storage::template container_type<typename container_traits<typename Loader::DataContainer>::position_handle> mHolder;
    };

    template <typename T, typename _Data, typename _Container = std::list<Placeholder<0>>, typename _Storage = Threading::GlobalStorage, typename _Base = ResourceLoaderCollector::Base>
    struct ResourceLoaderInterface : _Base {
        using Base = _Base;
        using Data = _Data;
        using Container = _Container;
        using Storage = _Storage;

        struct ResourceType;

        using ResourceDataInfo = ResourceDataInfo<T>;

        using DataContainer = typename replace<Container>::template type<ResourceData<T>>;

        using HandleType = Handle<T, typename container_traits<DataContainer>::handle>;
        using OriginalHandleType = HandleType;

        using Ctor = std::function<bool(T *, Data &, ResourceDataInfo &, Filesystem::FileEventType event)>;
        using Dtor = std::function<void(T *, Data &, ResourceDataInfo &)>;

        struct ResourceType : ResourceLoaderBase::ResourceType {

            using ResourceLoaderBase::ResourceType::ResourceType;

            HandleType loadData()
            {
                return T::load(this);
            }

            Data *dataPtr()
            {
                return T::getDataPtr(loadData());
            }

            using traits = container_traits<DataContainer>;

            typename Storage::template container_type<typename container_traits<DataContainer>::handle> mData;
        };

        template <typename C>
        static Ctor toCtor(C &&ctor)
        {
            if constexpr (std::is_same_v<std::remove_reference_t<C>, Ctor>) {
                return std::forward<C>(ctor);
            } else {
                return [ctor { std::forward<C>(ctor) }](T *loader, Data &data, ResourceDataInfo &info, Filesystem::FileEventType event) mutable {
                    return TupleUnpacker::invoke(ctor, loader, data, info, event);
                };
            }
        }

        template <typename D>
        static Dtor toDtor(D &&dtor)
        {
            if constexpr (std::is_same_v<std::remove_reference_t<D>, Dtor>) {
                return std::forward<D>(dtor);
            } else {
                return [dtor { std::forward<D>(dtor) }](T *loader, Data &data, ResourceDataInfo &info) mutable {
                    return TupleUnpacker::invoke(dtor, loader, data, info);
                };
            }
        }

        using Base::Base;

        static T &getSingleton()
        {
            return static_cast<T &>(getLoaderByIndex(component_index<T>()));
        }
    };

    template <typename T, typename _Data, typename _Base = ResourceLoaderInterface<T, _Data>>
    struct ResourceLoaderImpl : _Base {

        using Interface = _Base;
        using Base = _Base;
        using Data = _Data;

        using ResourceDataInfo = typename Interface::ResourceDataInfo;
        using ResourceType = ResourceType<T>;

        using DataContainer = typename replace<typename Base::Container>::template type<ResourceData<T>>;

        static_assert(!container_traits<DataContainer>::remove_invalidates_handles);

        using HandleType = Handle<T, typename container_traits<DataContainer>::handle>;

        using Ctor = std::function<bool(T *, Data &, ResourceDataInfo &, Filesystem::FileEventType event)>;
        using Dtor = std::function<void(T *, Data &, ResourceDataInfo &)>;

        using Base::Base;

        static T &getSingleton()
        {
            return static_cast<T &>(getLoaderByIndex(component_index<T>()));
        }

        static HandleType load(std::string_view name, T *loader = nullptr)
        {
            if (name.empty())
                return {};
            if (!loader)
                loader = &getSingleton();
            ResourceType *res = get(name, loader);
            if (!res) {
                LOG_ERROR("No resource '" << name << "' available!");
                return {};
            }
            return load(res, Filesystem::FileEventType::FILE_CREATED, loader);
        }

        static ResourceType *get(std::string_view name, T *loader = nullptr)
        {
            if (!loader)
                loader = &getSingleton();

            waitForIOThread();

            auto it = loader->mResources.find(name);
            if (it != loader->mResources.end())
                return &it->second;
            else
                return nullptr;
        }

        static ResourceDataInfo *getInfo(const HandleType &handle, T *loader = nullptr)
        {
            if (!handle)
                return nullptr;
            if constexpr (container_traits<DataContainer>::has_dependent_handle) {
                if (!loader)
                    loader = &getSingleton();
                return &(*loader->mData)[handle.mData].mInfo;
            } else {
                return &handle.mData->mInfo;
            }
        }

        template <typename C = Ctor, typename D = Dtor>
        static ResourceType *getOrCreateManual(std::string_view name, const Filesystem::Path &path = {}, C &&ctor = {}, D &&dtor = {}, T *loader = nullptr)
        {
            if (!loader)
                loader = &getSingleton();
            if (name == ResourceBase::sUnnamed) {
                return new ResourceType(ResourceBase::sUnnamed, path, Interface::toCtor(std::forward<C>(ctor)),
                    Interface::toDtor(std::forward<D>(dtor)));
            } else {
                return &loader->mResources.try_emplace(
                                              std::string { name }, std::string { name }, path, Interface::toCtor(std::forward<C>(ctor)),
                                              Interface::toDtor(std::forward<D>(dtor)))
                            .first->second;
            }
        }

        static HandleType create(ResourceType *resource, Filesystem::FileEventType event = Filesystem::FileEventType::FILE_CREATED, T *loader = nullptr)
        {
            HandleType handle { (typename container_traits<DataContainer>::handle) * resource->mData };
            assert(event == Filesystem::FileEventType::FILE_CREATED || handle);
            if (!handle || event != Filesystem::FileEventType::FILE_CREATED) {
                if (event == Filesystem::FileEventType::FILE_CREATED || !loader->mSettings.mInplaceReload) {
                    if (!loader)
                        loader = &getSingleton();
                    typename container_traits<DataContainer>::iterator it = container_traits<DataContainer>::emplace(*loader->mData, loader->mData->end(), resource);
                    *resource->mHolder = container_traits<DataContainer>::toPositionHandle(*loader->mData, it);
                    handle = container_traits<DataContainer>::toHandle(*loader->mData, *resource->mHolder);
                    *resource->mData = (decltype(*resource->mData))handle.mData;
                }
            }
            return handle;
        }

        static HandleType load(ResourceType *resource, Filesystem::FileEventType event = Filesystem::FileEventType::FILE_CREATED, T *loader = nullptr)
        {
            HandleType handle { (typename container_traits<DataContainer>::handle) * resource->mData };

            if (!handle || event != Filesystem::FileEventType::FILE_CREATED) {
                if (!loader)
                    loader = &getSingleton();
                handle = create(resource, event, loader);
                resource->mCtor(loader, *getDataPtr(handle, loader), *getInfo(handle, loader), event);
            }

            return handle;
        }

        static void unload(const HandleType &handle, T *loader = nullptr)
        {
            assert(handle);
            if (!handle.info()->decRef()) {
                if (!loader)
                    loader = &getSingleton();
                ResourceType *resource = handle.resource();
                resource->mDtor(loader, *getDataPtr(handle, loader), *getInfo(handle, loader));

                typename container_traits<DataContainer>::iterator it = container_traits<DataContainer>::toIterator(*loader->mData, *resource->mHolder);
                loader->mData->erase(it);
                *resource->mData = {};
                *resource->mHolder = {};

                //TODO: Check for multi-storage data for unnamed resources
                if (resource->name() == ResourceBase::sUnnamed)
                    delete resource;
            }
        }

        template <typename C = Ctor, typename D = Dtor>
        static HandleType loadManual(std::string_view name, const Filesystem::Path &path = {}, C &&ctor = {}, D &&dtor = {}, T *loader = nullptr)
        {
            if (!loader)
                loader = &getSingleton();
            return load(getOrCreateManual(
                            name, path, std::forward<C>(ctor), std::forward<D>(dtor),
                            loader),
                Filesystem::FileEventType::FILE_CREATED, loader);
        }

        /*static Data *getDataPtr(ResourceType *resource, T *loader = nullptr)
        {
            HandleType handle { (typename container_traits<DataContainer>::handle) * resource->mData };
            return getDataPtr(handle, loader);
        }*/

        static Data *getDataPtr(const HandleType &handle, T *loader = nullptr)
        {
            if (!handle)
                return nullptr;
            if constexpr (container_traits<DataContainer>::has_dependent_handle) {
                if (!loader)
                    loader = &getSingleton();
                return &(*loader->mData)[handle.mData].mData;
            } else {
                return &handle.mData->mData;
            }
        }

        //bool load(Data &data, ResourceType *res) = 0;
        //void unload(Data &data) = 0;
        std::pair<ResourceBase *, bool> addResource(const Filesystem::Path &path, std::string_view name = {}) override
        {
            std::string actualName { name.empty() ? path.stem() : name };
            auto pib = mResources.try_emplace(actualName, actualName, path);

            if (pib.second)
                this->resourceAdded(&pib.first->second);

            return std::make_pair(&pib.first->second, pib.second);
        }

        void updateResourceData(ResourceBase *resource) override
        {
            if (static_cast<T *>(this)->mSettings.mAutoReload) {
                ResourceType *res = static_cast<ResourceType *>(resource);
                if (*res->mData)
                    load(res, Filesystem::FileEventType::FILE_MODIFIED);
            }
        }

        typename std::map<std::string, ResourceType>::iterator begin()
        {
            return mResources.begin();
        }

        typename std::map<std::string, ResourceType>::iterator end()
        {
            return mResources.end();
        }

        virtual std::vector<std::pair<std::string_view, TypedScopePtr>> resources() override
        {
            std::vector<std::pair<std::string_view, TypedScopePtr>> result;
            std::transform(mResources.begin(), mResources.end(), std::back_inserter(result), [](std::pair<const std::string, ResourceType> &p) {
                return std::make_pair(std::string_view { p.first }, &p.second);
            });
            return result;
        }

        virtual std::vector<const MetaTable *> resourceTypes() const override
        {
            std::vector<const MetaTable *> result = Base::resourceTypes();
            result.push_back(table<decayed_t<ResourceType>>);
            return result;
        }

        std::map<std::string, ResourceType, std::less<>> mResources;
        std::vector<ResourceType> mUnnamedResources;

        typename Base::Storage::template container_type<DataContainer> mData;
    };

    template <typename T, typename _Data, typename Container = std::list<Placeholder<0>>, typename Storage = Threading::GlobalStorage>
    struct ResourceLoader : ResourceLoaderComponent<T, VirtualScope<T, ResourceLoaderImpl<T, _Data, ResourceLoaderInterface<T, _Data, Container, Storage>>>> {

        using ResourceLoaderComponent<T, VirtualScope<T, ResourceLoaderImpl<T, _Data, ResourceLoaderInterface<T, _Data, Container, Storage>>>>::ResourceLoaderComponent;
    };

    template <typename T, typename _Data, typename _Container = std::list<Placeholder<0>>, typename _Storage = Threading::GlobalStorage>
    struct VirtualResourceLoaderBase : ResourceLoaderVirtualBase<T, ResourceLoaderInterface<T, _Data, _Container, _Storage>> {

        using Base = ResourceLoaderInterface<T, _Data, _Container, _Storage>;

        using ResourceLoaderVirtualBase<T, ResourceLoaderInterface<T, _Data, _Container, _Storage>>::ResourceLoaderVirtualBase;

        static typename Base::HandleType load(std::string_view name, T *loader = nullptr)
        {
            if (!loader)
                loader = &Base::getSingleton();
            return loader->loadVImpl(name);
        }

        static typename Base::HandleType load(typename Base::ResourceType *resource, T *loader = nullptr)
        {
            if (!loader)
                loader = &Base::getSingleton();
            return loader->loadVImpl(resource);
        }

        static void unload(const typename Base::HandleType &handle, T *loader = nullptr)
        {
            if (!loader)
                loader = &Base::getSingleton();
            return loader->unloadVImpl(handle);
        }

        template <typename C = typename Base::Ctor, typename D = typename Base::Dtor>
        static typename Base::HandleType loadManual(std::string_view name, const Filesystem::Path &path = {}, C &&ctor = {}, D &&dtor = {}, T *loader = nullptr)
        {
            if (!loader)
                loader = &Base::getSingleton();
            return loader->loadManualVImpl(
                name, path, Base::toCtor(std::forward<C>(ctor)),
                Base::toDtor(std::forward<D>(dtor)));
        }

        /*static typename Base::Data *getDataPtr(typename Base::ResourceType *resource, T *loader = nullptr)
        {
            typename Base::HandleType handle { (typename container_traits<typename Base::DataContainer>::handle) * resource->mData };
            return getDataPtr(handle, loader);
        }*/

        static typename Base::Data *getDataPtr(const typename Base::HandleType &handle, T *loader = nullptr)
        {
            if (!handle)
                return nullptr;
            if constexpr (container_traits<typename Base::DataContainer>::has_dependent_handle) {
                if (!loader)
                    loader = &Base::getSingleton();
                return loader->getDataPtrVImpl(handle);
            } else {
                return &handle.mData->mData;
            }
        }

        static typename Base::ResourceDataInfo *getInfo(const typename Base::HandleType &handle, T *loader = nullptr)
        {
            if (!handle)
                return nullptr;
            if constexpr (container_traits<typename Base::DataContainer>::has_dependent_handle) {
                if (!loader)
                    loader = &Base::getSingleton();
                return loader->getInfoVImpl(handle);
            } else {
                return &handle.mData->mInfo;
            }
        }

        virtual std::vector<const MetaTable *> resourceTypes() const override
        {
            std::vector<const MetaTable *> result = ResourceLoaderBase::resourceTypes();
            result.push_back(table<typename Base::ResourceType>);
            return result;
        }

        virtual typename Base::HandleType loadManualVImpl(std::string_view name, const Filesystem::Path &path = {}, typename Base::Ctor ctor = {}, typename Base::Dtor dtor = {}) = 0;
        virtual typename Base::HandleType loadVImpl(std::string_view name) = 0;
        virtual typename Base::HandleType loadVImpl(typename Base::ResourceType *resource) = 0;
        virtual void unloadVImpl(const typename Base::HandleType &handle) = 0;
        virtual typename Base::Data *getDataPtrVImpl(const typename Base::HandleType &handle) = 0;
        virtual typename Base::ResourceDataInfo *getInfoVImpl(const typename Base::HandleType &handle) = 0;
    };

    template <typename T, typename _Data, typename _Base>
    struct VirtualResourceLoaderImpl : VirtualUniqueComponentImpl<T, VirtualScope<T, ResourceLoaderImpl<T, _Data, _Base>>, _Base> {

        using Data = _Data;
        using Base = _Base;

        using Self = VirtualUniqueComponentImpl<T, VirtualScope<T, ResourceLoaderImpl<T, Data, Base>>, _Base>;

        using Self::Self;

        virtual typename Base::OriginalHandleType loadManualVImpl(std::string_view name, const Filesystem::Path &path = {}, typename Base::Ctor ctor = {}, typename Base::Dtor dtor = {}) override
        {
            return Self::loadManual(name, path, std::move(ctor), std::move(dtor), static_cast<T *>(this));
        }
        virtual typename Base::OriginalHandleType loadVImpl(std::string_view name) override
        {
            return Self::load(name, static_cast<T *>(this));
        }
        virtual typename Base::OriginalHandleType loadVImpl(typename Base::ResourceType *resource) override
        {
            return Self::load(static_cast<typename Self::ResourceType *>(resource), Filesystem::FileEventType::FILE_CREATED, static_cast<T *>(this));
        }
        virtual void unloadVImpl(const typename Base::OriginalHandleType &handle) override
        {
            Self::unload(handle, static_cast<T *>(this));
        }
        virtual typename Base::Data *getDataPtrVImpl(const typename Base::OriginalHandleType &handle) override
        {
            return Self::getDataPtr(handle, static_cast<T *>(this));
        }
        virtual typename Base::ResourceDataInfo *getInfoVImpl(const typename Base::OriginalHandleType &handle) override
        {
            return Self::getInfo(handle, static_cast<T *>(this));
        }
    };
}
}