#pragma once

namespace Engine {
namespace Database {

std::string MADGINE_EXPORT tr(const std::string &key);

class TranslationUnit : public Ogre::ScriptLoader,
    public Ogre::Singleton<TranslationUnit> {
public:
    TranslationUnit();
    ~TranslationUnit();

    std::string getMessage(const std::string &key) const;

    std::list<std::string> availableLanguages();

    std::string currentLanguageName();

    const std::string &resolveLanguageName(const std::string &name);

    void setCurrentLanguage(const std::string &name);

    const Ogre::StringVector &getScriptPatterns() const;
    float getLoadingOrder() const;

    void parseScript(Ogre::DataStreamPtr &stream, const Ogre::String &);


private:

    float mLoadOrder;
    Ogre::StringVector mScriptPatterns;

    Ogre::ResourceGroupManager *mRsgm;

    std::map<std::string, std::map<std::string, std::string>> mLanguages;

    std::map<std::string, std::string> *mCurrentLanguage;
    std::string mCurrentLanguageName;

    static const std::string sExtension;
};

} // namespace Database
}

