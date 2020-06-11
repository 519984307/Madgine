#pragma once

#include "../generic/functor.h"

#include "../generic/type_pack.h"

#include "streams/serializestream.h"

namespace Engine {
namespace Serialize {

    template <typename FirstCreator, typename SecondCreator>
    struct PairCreator : private FirstCreator, private SecondCreator {

        static const constexpr bool controlled = false;

        using ArgsTuple = std::tuple<std::piecewise_construct_t, typename FirstCreator::ArgsTuple, typename SecondCreator::ArgsTuple>;

        static auto readCreationData(SerializeInStream &in)
        {            
            auto &&first = FirstCreator::readCreationData(in);
            return std::make_tuple(std::piecewise_construct, first, SecondCreator::readCreationData(in));
        }
    };

    template <typename T>
    struct DefaultCreator {

        static const constexpr bool controlled = false;

        using ArgsTuple = std::tuple<>;

        static ArgsTuple readCreationData(SerializeInStream &in)
        {
            return {};
        }

        static void writeCreationData(SerializeOutStream &out, const T &t)
        {
        }

        static void writeItem(SerializeOutStream &out, const T &t)
        {
            writeCreationData(out, t);
            write(out, t, "Item");
        }

        template <typename C, typename Op>
        static typename container_traits<C>::emplace_return readItem(SerializeInStream &in, C &c, const typename C::const_iterator &where, Op &op)
        {

            typename container_traits<C>::emplace_return it;
            if constexpr (std::is_const_v<typename C::value_type>) {
                std::remove_const_t<typename C::value_type> temp = TupleUnpacker::constructFromTuple<std::remove_const_t<typename C::value_type>>(readCreationData(in));
                read(in, temp, "Item");
                it = op.emplace(c, where, std::move(temp));
            } else {
                it = TupleUnpacker::invokeExpand(LIFT_MEMBER(emplace), &op, where, readCreationData(in));
                read(in, *it, "Item");
            }
            assert(container_traits<C>::was_emplace_successful(it));
            return it;
        }

        template <typename Op>
        static void clear(Op &op)
        {
            op.clear();
        }
    };

    /*namespace __creationhelper__impl__{
			template <typename R, typename T, typename... _Ty>
			struct _CustomCreator
			{
				using ArgsTuple = R;

				void setCreator(const std::function<R(_Ty ...)>& f)
				{
					mCallback = f;
				}

				R readCreationData(SerializeInStream& in)
				{
					std::tuple<std::remove_const_t<std::remove_reference_t<_Ty>>...> tuple;
					in >> tuple;
					return TupleUnpacker::invokeExpand(mCallback, std::move(tuple));
				}

			private:
				std::function<R(_Ty ...)> mCallback;
			};
		}

		template <typename F>
		using CustomCreator = typename CallableTraits<F>::template instance<__creationhelper__impl__::_CustomCreator>::type;*/

    namespace __creationhelper__impl__ {
        template <auto reader, typename WriteFunctor, typename R, typename T, typename... _Ty>
        struct _ParentCreator : WriteFunctor {

            static const constexpr bool controlled = false;

            using ArgsTuple = R;

            static R readCreationData(SerializeInStream &in, T *parent)
            {
                assert(parent);
                in.format().beginExtended(in, "Item", sizeof...(_Ty));
                std::tuple<std::remove_const_t<std::remove_reference_t<_Ty>>...> tuple;
                TupleUnpacker::forEach(tuple, [&](auto &e) { in >> e; });
                return TupleUnpacker::invokeExpand(reader, parent, std::move(tuple));
            }

            template <typename C, typename Op>
            static typename container_traits<C>::emplace_return readItem(SerializeInStream &in, C &c, const typename C::iterator &where, Op &op, T *parent)
            {
                typename container_traits<C>::emplace_return it;
                if constexpr (std::is_const_v<typename C::value_type>) {
                    std::remove_const_t<typename C::value_type> temp = TupleUnpacker::constructFromTuple<std::remove_const_t<typename C::value_type>>(readCreationData(in, parent));
                    read(in, temp, "Item");
                    it = op.emplace(where, std::move(temp));
                } else {
                    it = TupleUnpacker::invokeExpand(LIFT_MEMBER(emplace), &op, where, readCreationData(in, parent));
                    read(in, *it, "Item");
                }
                assert(container_traits<C>::was_emplace_successful(it));
                return it;
            }

            template <typename Op>
            static void clear(Op &op)
            {
                op.clear();
            }
        };

        template <auto writer, typename R, typename T, typename... _Ty>
        struct _ParentCreatorWriter {

            static void writeCreationData(SerializeOutStream &out, _Ty... t, T *parent)
            {
                assert(parent);
                out.format().beginExtended(out, "Item", std::tuple_size_v<R>);
                R tuple = (parent->*writer)(t...);
                TupleUnpacker::forEach(tuple, [&](auto &e) { write(out, e.second, e.first); });
            }

            static void writeItem(SerializeOutStream &out, _Ty... t, T *parent)
            {
                writeCreationData(out, t..., parent);
                write(out, t..., "Item");
            }
        };
    }

    template <auto reader, auto writer>
    using ParentCreator = typename MemberFunctionCapture<__creationhelper__impl__::_ParentCreator, reader, typename MemberFunctionCapture<__creationhelper__impl__::_ParentCreatorWriter, writer>::type>::type;
}
}
