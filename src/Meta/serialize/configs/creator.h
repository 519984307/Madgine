#pragma once

#include "../formatter.h"
#include "../streams/serializestream.h"
#include "Generic/callerhierarchy.h"
#include "Generic/functor.h"
#include "Generic/makeowning.h"

namespace Engine {
namespace Serialize {

    struct CreatorCategory;

    template <typename KeyCreator, typename ValueCreator>
    struct KeyValueCreator : private KeyCreator, private ValueCreator {

        using Category = CreatorCategory;

        static const constexpr bool controlled = false;

        using ArgsTuple = std::tuple<std::piecewise_construct_t, typename KeyCreator::ArgsTuple, typename ValueCreator::ArgsTuple>;

        using T = std::pair<typename KeyCreator::T, typename ValueCreator::T>;

        static void writeItem(SerializeOutStream &out, const T &t)
        {
            out.format().beginCompound(out, "Item");
            writeCreationData(out, t);
            write<typename KeyCreator::T>(out, t.first, "Key");
            write<typename ValueCreator::T>(out, t.second, "Value");
            out.format().endCompound(out, "Item");
        }

        template <typename Op>
        static StreamResult readItem(SerializeInStream &in, Op &op, typename container_traits<Op>::emplace_return &it, const typename container_traits<Op>::const_iterator &where)
        {
            STREAM_PROPAGATE_ERROR(in.format().beginCompound(in, nullptr));
            ArgsTuple tuple;
            STREAM_PROPAGATE_ERROR(readCreationData(in, tuple));
            it = TupleUnpacker::invokeExpand(LIFT(container_traits<Op>::emplace), op, where, std::move(tuple));
            assert(container_traits<Op>::was_emplace_successful(it));
            STREAM_PROPAGATE_ERROR(read<typename KeyCreator::T>(in, it.first->first, "Key"));
            STREAM_PROPAGATE_ERROR(read<typename ValueCreator::T>(in, it.first->second, "Value"));
            return in.format().endCompound(in, nullptr);
        }

        static void writeCreationData(SerializeOutStream &out, const T &t)
        {
            KeyCreator::writeCreationData(out, t.first, "Key");
            ValueCreator::writeCreationData(out, t.second, "Value");
        }

        static StreamResult readCreationData(SerializeInStream &in, ArgsTuple &tuple)
        {
            STREAM_PROPAGATE_ERROR(KeyCreator::readCreationData(in, std::get<1>(tuple)));
            return ValueCreator::readCreationData(in, std::get<2>(tuple));
        }

        template <typename Op>
        static void clear(Op &op)
        {
            op.clear();
        }
    };

    template <typename _T>
    struct DefaultCreator {
        using Category = CreatorCategory;

        static const constexpr bool controlled = false;

        using T = _T;

        using ArgsTuple = std::conditional_t<std::is_const_v<T>, std::tuple<std::remove_const_t<T>>, std::tuple<>>;

        static StreamResult readCreationData(SerializeInStream &in, ArgsTuple &result)
        {
            if constexpr (std::is_const_v<T>) {
                return read<std::remove_const_t<T>>(in, std::get<0>(result), nullptr);
            } else {
                return {};
            }
        }

        static void writeCreationData(SerializeOutStream &out, const T &t, const char *name = "Item")
        {
            if constexpr (std::is_const_v<T>) {
                write<std::remove_const_t<T>>(out, t, name);
            }
        }

        static void writeItem(SerializeOutStream &out, const T &t)
        {
            writeCreationData(out, t);
            write<T>(out, t, "Item");
        }

        template <typename Op>
        static StreamResult readItem(SerializeInStream &in, Op &op, typename container_traits<Op>::emplace_return &it, const typename container_traits<Op>::const_iterator &where)
        {
            ArgsTuple tuple;
            STREAM_PROPAGATE_ERROR(readCreationData(in, tuple));
            it = TupleUnpacker::invokeExpand(LIFT(container_traits<Op>::emplace), op, where, std::move(tuple));
            STREAM_PROPAGATE_ERROR(read<T>(in, *it, "Item"));
            assert(container_traits<Op>::was_emplace_successful(it));
            return {};
        }

        template <typename Op>
        static void clear(Op &op)
        {
            op.clear();
        }
    };

    namespace __serialize_impl__ {

        struct DefaultClear {
            template <typename T, typename Op>
            void operator()(T &&t, Op &op)
            {
                op.clear();
            }
            template <typename Op>
            void operator()(Op &op)
            {
                op.clear();
            }
        };

        template <auto reader, typename WriteFunctor, typename ClearFunctor, typename R, typename, typename... _Ty>
        struct _CustomCreator : WriteFunctor {

            using Category = CreatorCategory;

            static const constexpr bool controlled = false;

            using ArgsTuple = std::tuple<std::remove_const_t<std::remove_reference_t<_Ty>>...>;

            static StreamResult readCreationData(SerializeInStream &in, R &result)
            {
                STREAM_PROPAGATE_ERROR(in.format().beginExtended(in, "Item", sizeof...(_Ty)));
                ArgsTuple tuple;
                typename WriteFunctor::FNames names;
                size_t index = 0;
                STREAM_PROPAGATE_ERROR(TupleUnpacker::accumulate(
                    tuple, [&](auto &e, StreamResult r) {
                        STREAM_PROPAGATE_ERROR(std::move(r));
                        return read(in, e, names(index++));
                    },
                    StreamResult {}));
                result = TupleUnpacker::invokeExpand(reader, std::move(tuple));
                return {};
            }

            template <typename Op>
            static StreamResult readItem(SerializeInStream &in, Op &op, typename container_traits<Op>::emplace_return &it, const typename container_traits<Op>::const_iterator &where, const CallerHierarchyBasePtr &hierarchy)
            {
                R tuple;
                STREAM_PROPAGATE_ERROR(readCreationData(in, tuple));
                if constexpr (std::is_const_v<typename container_traits<Op>::value_type>) {

                    std::remove_const_t<typename container_traits<Op>::value_type> temp = TupleUnpacker::constructFromTuple<std::remove_const_t<typename container_traits<Op>::value_type>>(std::move(tuple));
                    STREAM_PROPAGATE_ERROR(read(in, temp, "Item", hierarchy));
                    it = container_traits<Op>::emplace(op, where, std::move(temp));
                } else {
                    it = TupleUnpacker::invokeExpand(LIFT(container_traits<Op>::emplace), op, where, std::move(tuple));
                    STREAM_PROPAGATE_ERROR(read(in, *it, "Item", hierarchy));
                }
                assert(container_traits<Op>::was_emplace_successful(it));
                return {};
            }

            template <typename Op>
            static void clear(Op &op)
            {
                ClearFunctor {}(op);
            }
        };

        template <auto writer, typename _FNames, typename R, typename, typename Arg>
        struct _CustomCreatorWriter {

            using FNames = _FNames;

            static void writeCreationData(SerializeOutStream &out, Arg arg, const CallerHierarchyBasePtr &hierarchy)
            {
                out.format().beginExtended(out, "Item", std::tuple_size_v<R>);
                R tuple = writer(arg);
                size_t index = 0;
                TupleUnpacker::forEach(tuple, [&](auto &e) { write(out, e, FNames {}(index++)); });
            }

            static void writeItem(SerializeOutStream &out, Arg arg, const CallerHierarchyBasePtr &hierarchy)
            {
                writeCreationData(out, arg, hierarchy);
                write(out, arg, "Item", hierarchy);
            }
        };

        template <auto reader, typename WriteFunctor, typename ClearFunctor, typename R, typename T, typename... _Ty>
        struct _ParentCreator : WriteFunctor {

            using Category = CreatorCategory;

            static const constexpr bool controlled = false;

            using ArgsTuple = std::tuple<MakeOwning_t<std::remove_const_t<std::remove_reference_t<_Ty>>>...>;

            static StreamResult readCreationData(SerializeInStream &in, std::optional<R> &result, const CallerHierarchy<T *> &parent)
            {
                assert(parent.mData);
                STREAM_PROPAGATE_ERROR(in.format().beginExtended(in, "Item", sizeof...(_Ty)));
                ArgsTuple tuple;
                typename WriteFunctor::FNames names;
                size_t index = 0;
                STREAM_PROPAGATE_ERROR(TupleUnpacker::accumulate(
                    tuple, [&](auto &e, StreamResult r) {
                        STREAM_PROPAGATE_ERROR(std::move(r));
                        return read(in, e, names(index++));
                    },
                    StreamResult {}));
                result.emplace(TupleUnpacker::invokeExpand(reader, parent.mData, std::move(tuple)));
                return {};
            }

            template <typename Op>
            static StreamResult readItem(SerializeInStream &in, Op &op, typename container_traits<Op>::emplace_return &it, const typename container_traits<Op>::const_iterator &where, const CallerHierarchy<T *> &parent)
            {
                std::optional<R> tuple;
                STREAM_PROPAGATE_ERROR(readCreationData(in, tuple, parent));
                if constexpr (std::is_const_v<typename container_traits<Op>::value_type>) {
                    std::remove_const_t<typename container_traits<Op>::value_type> temp = TupleUnpacker::constructFromTuple<std::remove_const_t<typename container_traits<Op>::value_type>>(std::move(*tuple));
                    STREAM_PROPAGATE_ERROR(read(in, temp, "Item", CallerHierarchyPtr { parent }));
                    it = container_traits<Op>::emplace(op, where, std::move(temp));
                } else {
                    it = TupleUnpacker::invokeExpand(LIFT(container_traits<Op>::emplace), op, where, std::move(*tuple));
                    STREAM_PROPAGATE_ERROR(read(in, *it, "Item", CallerHierarchyPtr { parent }));
                }
                assert(container_traits<Op>::was_emplace_successful(it));
                return {};
            }

            template <typename Op>
            static void clear(Op &op, const CallerHierarchy<T *> &parent)
            {
                ClearFunctor {}(parent.mData, op);
            }
        };

        template <auto writer, typename _FNames, typename R, typename T, typename Arg>
        struct _ParentCreatorWriter {

            using FNames = _FNames;

            static void writeCreationData(SerializeOutStream &out, Arg arg, const CallerHierarchy<const T *> &parent)
            {
                assert(parent.mData);
                out.format().beginExtended(out, "Item", std::tuple_size_v<R>);
                R tuple = (parent.mData->*writer)(arg);
                size_t index = 0;
                TupleUnpacker::forEach(tuple, [&](auto &e) { write(out, e, FNames {}(index++)); });
            }

            static void writeItem(SerializeOutStream &out, Arg arg, const CallerHierarchy<const T *> &parent)
            {
                writeCreationData(out, arg, parent);
                write(out, arg, "Item", CallerHierarchyPtr { parent });
            }
        };
    }

    template <auto names, auto reader, auto writer, auto clear = nullptr>
    using ParentCreator = typename FunctionCapture<__serialize_impl__::_ParentCreator, reader, typename FunctionCapture<__serialize_impl__::_ParentCreatorWriter, writer, Functor<names>>::type, std::conditional_t<std::is_same_v<decltype(clear), std::nullptr_t>, __serialize_impl__::DefaultClear, UnpackingMemberFunctor<clear>>>::type;

    template <auto names, auto reader, auto writer, auto clear = nullptr>
    using CustomCreator = typename FunctionCapture<__serialize_impl__::_CustomCreator, reader, typename FunctionCapture<__serialize_impl__::_CustomCreatorWriter, writer, Functor<names>>::type, std::conditional_t<std::is_same_v<decltype(clear), std::nullptr_t>, __serialize_impl__::DefaultClear, Functor<clear>>>::type;
}
}
