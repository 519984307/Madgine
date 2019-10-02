#pragma once

#include "../../generic/tupleunpacker.h"
#include "../streams/bufferedstream.h"
#include "../syncable.h"
#include "tupleserialize.h"
#include "offset.h"

namespace Engine {
namespace Serialize {
    namespace __actionpolicy__impl__ {
        template <bool executeOnMasterOnly, bool callByMasterOnly>
        struct _ActionPolicy {
            static constexpr bool sExecuteOnMasterOnly = executeOnMasterOnly;
            static constexpr bool sCallByMasterOnly = callByMasterOnly;
        };

        template <auto f, typename PtrOffset, class Config, class R, class T, class... _Ty>
        class ActionImpl : public Syncable<PtrOffset> {
        public:
            ActionImpl()
            {
            }

            void operator()(_Ty... args, const std::set<ParticipantId> &targets = {})
            {
                tryCall(this->participantId(), targets, args...);
            }

            void readAction(SerializeInStream &in)
            {
                std::tuple<std::remove_const_t<std::remove_reference_t<_Ty>>...> args;
                in >> args;
                TupleUnpacker::invokeExpand(&ActionImpl::call, this, std::set<ParticipantId> {}, std::move(args));
            }

            void readRequest(BufferedInOutStream &in)
            {
                if (!Config::sCallByMasterOnly) {
                    std::tuple<std::remove_const_t<std::remove_reference_t<_Ty>>...> args;
                    in >> args;
                    TupleUnpacker::invokeExpand(&ActionImpl::tryCall, this, in.id(), std::set<ParticipantId> {}, std::move(args));
                }
            }

            void setVerify(std::function<bool(ParticipantId, _Ty...)> verify)
            {
                mVerify = verify;
            }

        protected:
            bool verify(ParticipantId id, _Ty... args)
            {
                return !mVerify || mVerify(id, args...);
            }

        private:
            void call(const std::set<ParticipantId> &targets, _Ty... args)
            {
                if (!Config::sExecuteOnMasterOnly) {
                    for (BufferedOutStream *out : this->getMasterActionMessageTargets(targets)) {
                        *out << std::forward_as_tuple(args...);
                        out->endMessage();
                    }
                } else {
                    assert(targets.empty());
                }

                (PtrOffset::parent(this)->*f)(args...);
            }

            void tryCall(ParticipantId id, const std::set<ParticipantId> &targets, _Ty... args)
            {
                if (verify(id, args...)) {
                    if (this->isMaster()) {
                        call(targets, args...);
                    } else {
                        if (!Config::sCallByMasterOnly && targets.empty()) {
                            BufferedOutStream *out = this->getSlaveActionMessageTarget();
                            (*out << ... << args);
                            out->endMessage();
                        } else {
                            throw 0;
                        }
                    }
                }
            }

        private:
            std::function<bool(ParticipantId, _Ty...)> mVerify;
        };
    }

    struct ActionPolicy {
        //using masterOnly = _ActionPolicy<true, true>;
        using request = __actionpolicy__impl__::_ActionPolicy<true, false>;
        using broadcast = __actionpolicy__impl__::_ActionPolicy<false, false>;
        using notification = __actionpolicy__impl__::_ActionPolicy<false, true>;
    };

    /*template <typename F, F f, class C>
		using Action = typename MemberFunctionCapture<__actionpolicy__impl__::ActionImpl, F, f, C>::type;*/

    template <typename PtrOffset, auto f, class C>
    class Action : public MemberFunctionCapture<__actionpolicy__impl__::ActionImpl, f, PtrOffset, C>::type {
        using MemberFunctionCapture<__actionpolicy__impl__::ActionImpl, f, PtrOffset, C>::type::type;
    };

#define ACTION(Name, ...)                                                                                 \
    ::Engine::Serialize::Action<::Engine::Serialize::SyncableOffsetPtr<Self, __LINE__>, __VA_ARGS__> Name; \
    DEFINE_SYNCABLE_OFFSET(Name)

}
}
