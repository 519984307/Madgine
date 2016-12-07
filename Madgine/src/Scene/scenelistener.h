#pragma once


namespace Engine {
namespace Scene {

class MADGINE_EXPORT SceneListener {
public:
    SceneListener();
    ~SceneListener();

    virtual void onSceneLoad();
    virtual void onSceneClear();
    virtual void beforeSceneClear();

    SceneManager *sceneMgr() const;

private:
    SceneManager *mSceneMgr;
};

}
}


