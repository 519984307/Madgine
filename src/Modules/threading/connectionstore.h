#pragma once

namespace Engine {
namespace Threading {

    struct MODULES_EXPORT ConnectionStore {    
        template <typename Con, typename... Args>
        static std::shared_ptr<Con> create(std::shared_ptr<ConnectionBase> *prev, Args &&... args)
        {

            return std::static_pointer_cast<Con>(
                make_shared_connection(
                    std::make_unique<Con>(prev, std::forward<Args>(args)...)));
        }

        template <typename Con, typename... Args>
        std::weak_ptr<Con> emplace_front(Args &&... args)
        {
            std::shared_ptr<Con> ptr = create<Con>(&mBegin, std::forward<Args>(args)...);
            mBegin = ptr;
            return ptr;
        }

        ConnectionStore();

        void clear();

        static ConnectionStore &globalStore();

    private:
        static std::shared_ptr<ConnectionBase> make_shared_connection(std::unique_ptr<ConnectionBase> &&conn);

        std::shared_ptr<ConnectionBase> mBegin;
    };

}
}
