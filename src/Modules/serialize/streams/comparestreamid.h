#pragma once

namespace Engine {
namespace Serialize {

struct MODULES_EXPORT CompareStreamId {
    bool operator()(ParticipantId first,
                    const BufferedInOutStream &second) const;
    bool operator()(const BufferedInOutStream &first,
					ParticipantId second) const;
    bool operator()(SerializeInStream *first,
                                      SerializeInStream *second) const;
    bool operator()(SerializeOutStream *first,
                    SerializeOutStream *second) const;
    bool operator()(BufferedInStream *first, BufferedInStream *second) const;
    bool operator()(BufferedOutStream *first, BufferedOutStream *second) const;
    bool operator()(const BufferedInOutStream &first,
                    const BufferedOutStream &second) const;

    using is_transparent = void;
};
} // namespace Serialize
} // namespace Engine