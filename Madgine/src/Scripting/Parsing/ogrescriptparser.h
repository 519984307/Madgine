#pragma once

#include "Scripting\Parsing\scriptparser.h"

namespace Engine {
namespace Scripting {
namespace Parsing {

class OgreScriptParser : public Ogre::ScriptLoader, public Ogre::GeneralAllocatedObject,
    public ScriptParser {
public:
    OgreScriptParser();
	OgreScriptParser(const OgreScriptParser &) = delete;
    virtual ~OgreScriptParser();

	void operator=(const OgreScriptParser &) = delete;

    const Ogre::StringVector &getScriptPatterns() const;
    float getLoadingOrder() const;

    virtual void parseScript(Ogre::DataStreamPtr &stream, const Ogre::String &group) override;

protected:
	

private:

    float mLoadOrder;
    Ogre::StringVector mScriptPatterns;

    Ogre::ResourceGroupManager *mRsgm;


};

}
}
}


