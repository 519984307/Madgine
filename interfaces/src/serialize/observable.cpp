#include "interfaceslib.h"

#include "observable.h"
#include "streams/bufferedstream.h"
#include "toplevelserializableunit.h"

namespace Engine {
		namespace Serialize {

			Observable::Observable() :
				mUnit(SerializableUnitBase::findParent(this)),
				mIndex(mUnit->addObservable(this))
			{
			}

			std::set<BufferedOutStream*, CompareStreamId> Observable::getMasterActionMessageTargets(const std::set<ParticipantId> &targets) const
			{
				std::set<BufferedOutStream*, CompareStreamId> result = mUnit->getMasterMessageTargets();
				if (targets.empty()) {
					for (BufferedOutStream *out : result) {
						out->beginMessage(mUnit, ACTION);
						*out << mIndex;
					}
				}
				else {
					auto it1 = result.begin();
					auto it2 = targets.begin();
					while (it1 != result.end() && it2 != targets.end()) {
						BufferedOutStream *out = *it1;
						while (*it2 < out->id()) {
							LOG_WARNING("Specific Target not in MessageTargetList!");
							++it2;
						}
						if (*it2 == out->id()) {
							out->beginMessage(mUnit, ACTION);
							*out << mIndex;
							++it2;
							++it1;
						}
						else {
							it1 = result.erase(it1);
						}
					}
					result.erase(it1, result.end());
				}
				
				return result;
			}

			BufferedOutStream * Observable::getSlaveActionMessageTarget() const
			{
				BufferedOutStream *out = mUnit->getSlaveMessageTarget();
				out->beginMessage(mUnit, REQUEST);
				*out << mIndex;
				return out;
			}

			void Observable::beginActionResponseMessage(BufferedOutStream * stream)
			{
				stream->beginMessage(mUnit, ACTION);
				*stream << mIndex;
			}

			ParticipantId Observable::id()
			{
				return isMaster() ? mUnit->topLevel()->getLocalMasterParticipantId() : mUnit->topLevel()->getSlaveParticipantId();
			}

			bool Observable::isMaster() const
			{
				return !mUnit->isActive() || !mUnit->topLevel() || mUnit->topLevel()->isMaster();
			}

			SerializableUnitBase *Observable::parent() {
				return mUnit;
			}
		
			
		}
}
