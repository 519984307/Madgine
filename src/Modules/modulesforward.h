#pragma once

namespace Engine {
struct ValueType;

struct InvScopePtr;

enum KeyValueValueFlags : uint8_t;

struct KeyValueVirtualIteratorBase;

struct ScopeBase;
struct MetaTable;
struct ScopeIterator;
struct Accessor;
struct TypedScopePtr;

struct ObjectInstance;
struct ObjectPtr;
struct ObjectFieldAccessor;

struct IndexHolder;

struct UniqueComponentCollectorManager;
template <typename T, typename _Collector, typename _Base = typename _Collector::Base>
struct UniqueComponent;
template <typename T, typename _Collector, typename Base = typename _Collector::Base>
struct VirtualUniqueComponentBase;

struct ComponentRegistryBase;

struct InStream;
struct OutStream;

template <typename RefT>
struct VirtualIterator;

using KeyValueVirtualIterator = VirtualIterator<std::pair<ValueType, ValueType>>;

using ArgumentList = std::vector<ValueType>;

struct ApiMethod;

struct Any;

namespace CLI {
    struct CLICore;
    struct ParameterBase;
}

namespace Debug {

    struct TraceBack;
    struct StackTraceIterator;

    namespace Memory {
        struct StatsMemoryResource;
        struct MemoryTracker;
    }

    namespace Profiler {
        struct Profiler;
        struct ProfilerThread;
    }
}

namespace Ini {
    struct IniFile;
    struct IniSection;
}

namespace Serialize {
    struct SerializeInStream;
    struct SerializeOutStream;
    struct SerializableUnitBase;
    struct BufferedInOutStream;
    struct BufferedInStream;
    struct BufferedOutStream;
    struct TopLevelSerializableUnitBase;
    struct SyncableBase;
    struct SerializableBase;
    struct FileBuffer;
    struct MessageHeader;
    struct SerializeManager;
    struct SyncManager;
    struct SerializeStreambuf;

	struct Serializer;

    struct Formatter;

    struct CompareStreamId;

    typedef size_t ParticipantId;

    struct buffered_streambuf;

    struct noparent_deleter;

    struct SerializeTable;
    using SerializableUnitMap = std::map<size_t, SerializableUnitBase *>;

    enum MessageType {
        STATE,
        ACTION,
        REQUEST
    };

    enum Command {
        INITIAL_STATE_DONE,
        STREAM_EOF
    };
}

namespace Network {
    struct NetworkManager;
}

namespace Plugins {
    struct PluginManager;
    struct Plugin;
    struct PluginSection;
    struct PluginListener;

    struct BinaryInfo;
}

namespace Util {

    struct Log;

    struct LogListener;

    struct Process;
    struct StandardLog;
}

namespace Threading {
    struct ConnectionBase;
    template <typename T, typename... _Ty>
    struct ConnectionInstance;
    struct TaskQueue;
}

namespace Window {
    struct WindowEventListener;
    struct Window;
}

namespace Filesystem {
    struct Path;
    struct FileQuery;
    struct FileQueryState;

    struct SharedLibraryQuery;
    struct SharedLibraryQueryState;
}

namespace Threading {
    struct WorkGroup;
    struct WorkGroupHandle;
    struct Scheduler;

    struct FrameListener;
    struct FrameLoop;

    struct DataMutex;
}

namespace Font {

    struct Font;
    struct Glyph;

}

namespace Render {
    /*struct Vertex;
    struct Vertex2;
    struct Vertex3;*/
    typedef int RenderPassFlags;
}

namespace Resources {
    struct ResourceManager;
    template <typename Data, typename Container, typename Storage>
    struct Resource;
    template <typename T, typename Data, typename Container, typename Storage>
    struct ResourceLoader;
    template <typename T, typename _Data, typename Container, typename Storage, typename Base>
    struct ResourceLoaderImpl;
    struct ResourceBase;
}

struct Vector2;
struct Vector3;

struct Vector2i;
struct Vector3i;

struct Rect2i;

struct Matrix3;
struct Matrix4;

struct TypeInfo;

struct Ray;
struct Sphere;
struct Plane;
struct AABB;
struct BoundingBox;

struct MadgineObject;
}
