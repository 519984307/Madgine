#pragma once

#include "valuetype.h"


namespace Engine {

	template<bool...>
	struct bool_pack;

	template<bool... values>
	struct all_of
		: std::is_same<bool_pack<values..., true>, bool_pack<true, values...>> {};

	template<bool... values>
	const constexpr bool all_of_v = all_of<values...>::value;

	template <class _T>
	struct Caster {
		static auto cast(const ValueType &v) {
			return v.as<std::remove_const_t<std::remove_reference_t<_T>>>();
		}
	};

	template <>
	struct MADGINE_EXPORT Caster<Scripting::List*> {
		static Scripting::List* cast(const ValueType &v);
	};

	template <>
	struct MADGINE_EXPORT Caster<Scene::Entity::Entity*> {
		static Scene::Entity::Entity* cast(const ValueType &v);
	};

	
	struct TupleSerializer {

		template <class... Args, class _ = std::enable_if_t<all_of<ValueType::isValueType<Args>::value...>::value>>
		static void readTuple(std::tuple<Args...> &tuple, Serialize::SerializeInStream &in) {
			readTuple(tuple, in, std::make_index_sequence<sizeof...(Args)>());
		}

		template <class Arg, class... Args, size_t... S>
		static void readTuple(std::tuple<Arg, Args...> &tuple, Serialize::SerializeInStream &in, std::index_sequence<S...>) {
			using expander = int[];
			(void)expander {
				(void(in >> std::get<S>(tuple)), 0)...
			};
		}

		static void readTuple(std::tuple<> &tuple, Serialize::SerializeInStream &in, std::index_sequence<>) {
		}

		template <class... Args, class _ = std::enable_if_t<all_of<ValueType::isValueType<Args>::value...>::value>>
		static void writeTuple(const std::tuple<Args...> &tuple, Serialize::SerializeOutStream &out) {
			writeTuple(tuple, out, std::make_index_sequence<sizeof...(Args)>());
		}

		template <class Arg, class... Args, size_t... S>
		static void writeTuple(const std::tuple<Arg, Args...> &tuple, Serialize::SerializeOutStream &out, std::index_sequence<S...>) {
			using expander = int[];
			(void)expander {
				(void(out << std::get<S>(tuple)), 0)...
			};
		}

		static void writeTuple(const std::tuple<> &tuple, Serialize::SerializeOutStream &out, std::index_sequence<>) {
		}

	};

	template <class... _Ty>
	struct TupleUnpacker {

		template <class R, class T, class... Args, class Tuple = std::tuple<Args...>>
		static R call(T *t, R(T::*f)(_Ty..., Args...), _Ty&&... _ty, Tuple&& args) {
			return unpack(t, f, std::forward<_Ty>(_ty)..., std::forward<Tuple>(args), std::make_index_sequence<sizeof...(Args)>());
		}

		template <class R, class T, class... Args, size_t... S, class Tuple = std::tuple<Args...>>
		static R unpack(T *t, R(T::*f)(_Ty..., Args...), _Ty&&... _ty, Tuple&& args, std::index_sequence<S...>) {
			return (t->*f)(std::forward<_Ty>(_ty)..., std::get<S>(std::forward<Tuple>(args))...);
		}
		
	};




}



