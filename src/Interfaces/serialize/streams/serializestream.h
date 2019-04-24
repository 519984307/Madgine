#pragma once

#include "debugging/streamdebugging.h"
#include "../serializeexception.h"
#include "../../generic/templates.h"
#include "../../streams/streams.h"

namespace Engine
{
	namespace Serialize
	{

		class EOLType
		{
		public:
			constexpr bool operator==(const EOLType&) const { return true; }
		};

		constexpr const int SERIALIZE_MAGIC_NUMBER = 0x12345678;

		using SerializePrimitives = type_pack<bool, size_t, int, float, SerializableUnitBase*, EOLType, std::string, Vector2, Vector3, InvScopePtr>;

		template <class T, class = void>
		struct PrimitiveTypeIndex : type_pack_index<SerializePrimitives, T> {};

		template <class T>
		struct PrimitiveTypeIndex<T, std::enable_if_t<std::is_enum_v<T>>> : PrimitiveTypeIndex<int>{};

		template <class T>
		const constexpr size_t PrimitiveTypeIndex_v = PrimitiveTypeIndex<T>::value;

		template <class T, class = void>
		struct PrimitiveTypesContain : type_pack_contains<SerializePrimitives, T>{};

		template <class T>
		struct PrimitiveTypesContain<T, std::enable_if_t<std::is_enum_v<T>>> : PrimitiveTypesContain<int> {};

		template <class T>
		const constexpr bool PrimitiveTypesContain_v = PrimitiveTypesContain<T>::value;

		struct INTERFACES_EXPORT SerializeStreambuf : std::basic_streambuf<char>
		{
		public:
			SerializeStreambuf(SerializeManager& mgr, ParticipantId id);
			virtual ~SerializeStreambuf() = default;

			void setManager(SerializeManager &mgr);
			SerializeManager& manager();
			bool isMaster();

			ParticipantId id() const;
			void setId(ParticipantId id);

		private:			
			SerializeManager *mManager;
			ParticipantId mId;
		};

		struct INTERFACES_EXPORT SerializeInStream : InStream
		{
		public:
			SerializeInStream(std::unique_ptr<SerializeStreambuf> &&buffer);
			SerializeInStream(SerializeInStream &&other);

			template <class T, typename = std::enable_if_t<PrimitiveTypesContain_v<T>>>
			SerializeInStream& operator >>(T& t)
			{
				int type;
				read(type);
				if (type != SERIALIZE_MAGIC_NUMBER + PrimitiveTypeIndex_v<T>)
					throw SerializeException(Database::Exceptions::unknownSerializationType);
				read(t);
				mLog.log(t);
				return *this;
			}

			SerializeInStream& operator >>(ValueType& result);

			template <class T, typename V = std::enable_if_t<std::is_base_of<SerializableUnitBase, T>::value>>
			SerializeInStream& operator >>(T*& p)
			{
				SerializableUnitBase* unit;
				*this >> unit;
				p = dynamic_cast<T*>(unit);
				if (unit && !p)
					throw 0;
				return *this;
			}

			SerializeInStream& operator >>(SerializableUnitBase*& p);

			SerializeInStream& operator >>(Serializable& s);

			SerializeInStream& operator >>(std::string &s);

			template <class T>
			bool loopRead(T& val)
			{
				bool result = loopRead();
				if (result)
					*this >> val;
				return result;
			}

			bool loopRead();

			void readRaw(void* buffer, size_t size);

			void logReadHeader(const MessageHeader& header, const std::string& object);

			void setManager(SerializeManager &mgr) const;

			SerializeManager &manager() const;

			void setId(ParticipantId id);

			ParticipantId id() const;

			bool isMaster();

		protected:
			SerializeInStream(SerializeStreambuf *buffer);

			template <class T>
			void read(T& t)
			{
				read(&t, sizeof(T));
			}

			void read(void* buffer, size_t size);

			SerializableUnitBase* convertPtr(size_t ptr);

			SerializeStreambuf &buffer() const;

		protected:
			Debugging::StreamLog mLog;
		};

		struct INTERFACES_EXPORT SerializeOutStream : OutStream
		{
		public:
			SerializeOutStream(std::unique_ptr<SerializeStreambuf> &&buffer);
			SerializeOutStream(SerializeOutStream &&other);
			
			ParticipantId id() const;

			SerializeOutStream& operator<<(const ValueType& v);

			template <class T, typename = std::enable_if_t<PrimitiveTypesContain_v<T>>>
			SerializeOutStream& operator<<(const T& t)
			{
				write<int>(SERIALIZE_MAGIC_NUMBER + PrimitiveTypeIndex_v<T>);
				write(t);
				mLog.log(t);
				return *this;
			}

			SerializeOutStream& operator<<(SerializableUnitBase* p);

			SerializeOutStream& operator<<(const Serializable& s);

			SerializeOutStream& operator<<(const std::string &s);

			void writeRaw(const void* buffer, size_t size);

			SerializeManager &manager() const;

			bool isMaster();

		protected:
			pos_type tell() const;
			void seek(pos_type p);

			template <class T>
			void write(const T& t)
			{
				writeData(&t, sizeof(T));
			}

			void writeData(const void *data, size_t count);

			SerializeStreambuf &buffer() const;

		protected:
			Debugging::StreamLog mLog;
		};
	}
} // namespace Scripting
