#include "libinclude.h"

#include "configset.h"

#include "Os\os.h"
#include "UI\UIManager.h"

#include "Scripting\Datatypes\Serialize\serializestream.h"

namespace Engine {
namespace App {

const bool ConfigSet::defaultFullScreen = false;
const bool ConfigSet::defaultVSync = false;
const int ConfigSet::defaultResolutionWidth = 1024;
const int ConfigSet::defaultResolutionHeight = 768;
const int ConfigSet::defaultColorDepth = 32;
const std::string ConfigSet::defaultRenderSystem = "OpenGL Rendering Subsystem";
const std::string ConfigSet::defaultLanguage = "en";

const std::string ConfigSet::yesString = "Yes";
const std::string ConfigSet::noString = "No";

const std::string ConfigSet::fullScreenProperty = "Full Screen";
const std::string ConfigSet::vSyncProperty = "VSync";
const std::string ConfigSet::heightProperty = "Height";
const std::string ConfigSet::widthProperty = "Width";
const std::string ConfigSet::colorDepthProperty = "Colour Depth";
const std::string ConfigSet::renderSystemProperty = "RenderSystem";
const std::string ConfigSet::videoModeProperty = "Video Mode";
const std::string ConfigSet::languageProperty = "Language";


ConfigSet::ConfigSet(Ogre::Root *root, const std::string &configFileName) :
    mConfigFileName(configFileName)
{
    load();

    mRenderSystem = root->getRenderSystemByName(getRenderSystem());
    if (!mRenderSystem || !(mRenderSystem->getName() == getRenderSystem()))
        throw 0;
    mRenderSystem->setConfigOption(fullScreenProperty,
                                   getFullscreen() ? yesString : noString);
    mRenderSystem->setConfigOption(vSyncProperty,
                                   getVSync() ? yesString : noString);
    mRenderSystem->setConfigOption(videoModeProperty,
                                   getResolutionAndColorDepthString());

    root->setRenderSystem(mRenderSystem);

}

bool ConfigSet::getFullscreen()
{
    return mSettings[fullScreenProperty].asBool(defaultFullScreen);
}

bool ConfigSet::getVSync()
{
    return mSettings[vSyncProperty].asBool(defaultFullScreen);
}

int ConfigSet::getResolutionWidth()
{
    return mSettings[widthProperty].asInt(defaultResolutionWidth);
}

int ConfigSet::getResolutionHeight()
{
    return mSettings[heightProperty].asInt(defaultResolutionHeight);
}

int ConfigSet::getColorDepth()
{
    return mSettings[colorDepthProperty].asInt(defaultColorDepth);
}

std::string ConfigSet::getLanguage()
{
    return mSettings[languageProperty].asString(defaultLanguage);
}

std::string ConfigSet::getRenderSystem()
{
    return mSettings[renderSystemProperty].asString(defaultRenderSystem);
}

std::string ConfigSet::getResolutionAndColorDepthString()
{
    return getResolutionString() + " @ " + std::to_string(getColorDepth()) + "-bit";
}

std::string ConfigSet::getResolutionString()
{
    return std::to_string(getResolutionWidth()) + " x " + std::to_string(getResolutionHeight());
}

void ConfigSet::setFullscreen(bool b)
{
    mSettings[fullScreenProperty] = b;
}

void ConfigSet::setResolutionWidth(int i)
{
    mSettings[widthProperty] = i;
}

void ConfigSet::setResolutionHeight(int i)
{
    mSettings[heightProperty] = i;
}

void ConfigSet::setResolutionString(const std::string &s)
{
    std::stringstream ss(s);
    int width, height;
    ss >> width >> std::string() >> height;
    setResolutionWidth(width);
    setResolutionHeight(height);
}

void ConfigSet::setColorDepth(int i)
{
    mSettings[colorDepthProperty] = i;
}

void ConfigSet::setLanguage(const std::string &language)
{
    mSettings[languageProperty] = language;
    mTranslation.setCurrentLanguage(language);
    //UI::UIManager::getSingleton().retranslate();
}

void ConfigSet::applyLanguage()
{
    setLanguage(getLanguage());
}

void ConfigSet::updateWindow()
{
    UI::UIManager::getSingleton().setWindowProperties(getFullscreen(), getResolutionWidth(),
                          getResolutionHeight());
}

void ConfigSet::save()
{
    std::ofstream ofs(mConfigFileName, std::ios::binary);
    Scripting::Serialize::SerializeOutStream of(ofs);
    of << mSettings;
    ofs.close();

}

void ConfigSet::load()
{
    std::ifstream f(mConfigFileName, std::ios::binary);
    if (f.good()) {
        Scripting::Serialize::SerializeInStream ifs(f);
        ifs >> mSettings;
    }
    f.close();

}

Ogre::StringVector &ConfigSet::getPossibleVideoModes()
{
    return mRenderSystem->getConfigOptions()[videoModeProperty].possibleValues;
}

Ogre::StringVector &ConfigSet::getPossibleColorDepths()
{
    return mRenderSystem->getConfigOptions()[colorDepthProperty].possibleValues;
}

}
}
