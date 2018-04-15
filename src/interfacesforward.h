#pragma once

namespace Engine
{
	class ValueType;

	class InvScopePtr;

	class KeyValueIterator;
	class KeyValueMapList;

	namespace Serialize
	{
		class Stream;
		class SerializeInStream;
		class SerializeOutStream;
		class SerializableUnitBase;
		class BufferedInOutStream;
		class BufferedInStream;
		class BufferedOutStream;
		class TopLevelSerializableUnitBase;
		class Observable;
		class Serializable;
		class FileBuffer;
		struct MessageHeader;
		class SerializeManager;

		struct CompareStreamId;

		typedef size_t ParticipantId;

		class buffered_streambuf;

		struct noparent_deleter;

		template <typename T>
		using noparent_unique_ptr = std::unique_ptr<T, noparent_deleter>;

		using SerializableUnitMap = std::map<size_t, SerializableUnitBase*>;

		enum StreamError
		{
			NO_ERROR = 0,
			WOULD_BLOCK,
			ALREADY_CONNECTED,
			TIMEOUT,
			NO_SERVER,
			NO_CONNECTION,
			CONNECTION_REFUSED,
			UNKNOWN_ERROR
		};

		enum MessageType
		{
			STATE,
			ACTION,
			REQUEST
		};

		enum Command
		{
			INITIAL_STATE_DONE,
			STREAM_EOF
		};
	}

	namespace Scripting
	{
		class ScriptingManager;

		class ScopeBase;
		class GlobalScopeBase;
		class GlobalAPIComponentBase;

		class LuaState;
		class LuaTable;
		class LuaTableIterator;

		struct Mapper;

		class ArgumentList;
		typedef std::map<std::string, ValueType> Stack;

		typedef ValueType (*ApiMethod)(ScopeBase*, const ArgumentList&);

		namespace Parsing
		{
			class ScriptParser;
		}
	}


	namespace Network
	{
		class NetworkManager;
		class NetworkStream;
	}


	namespace Hierarchy
	{
		class any_ptr;
	}


	namespace Util
	{
		
		class Log;

		struct TraceBack;

		class LogListener;

		class Process;
		class StandardLog;
	}

	namespace SignalSlot
	{
		class ConnectionBase;
		template <class... _Ty>
		class ConnectionInstance;
		class ConnectionManager;
	}
}

struct lua_State;
struct luaL_Reg;