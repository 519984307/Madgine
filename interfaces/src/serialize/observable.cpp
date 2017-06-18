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

			std::list<BufferedOutStream*> Observable::getMasterActionMessageTargets(const std::list<ParticipantId> &targets) const
			{
				std::list<BufferedOutStream*> result = mUnit->getMasterMessageTargets(true, targets);
				for (BufferedOutStream *out : result) {
					*out << mIndex;
				}
				return result;
			}

			void Observable::writeMasterActionMessageHeader(BufferedOutStream & out) const
			{
				mUnit->writeMasterMessageHeader(out, true);
				out << mIndex;
			}

			BufferedOutStream * Observable::getSlaveActionMessageTarget() const
			{
				BufferedOutStream *out = mUnit->getSlaveMessageTarget();
				*out << mIndex;
				return out;
			}

			ParticipantId Observable::id()
			{
				return isMaster() ? mUnit->topLevel()->getLocalMasterParticipantId() : mUnit->topLevel()->getSlaveParticipantId();
			}

			bool Observable::isMaster() const
			{
				return !mUnit->topLevel() || mUnit->topLevel()->isMaster();
			}

			SerializableUnitBase *Observable::parent() {
				return mUnit;
			}
		
			
		}
}