#pragma once

#include "toolscollector.h"

#include "toolbase.h"

#include "sceneview.h"

#include "Madgine/scene/entity/entityptr.h"

#include "Madgine/scene/entity/entitycomponentptr.h"

namespace Engine {
namespace Tools {

    struct SceneEditor : Tool<SceneEditor> {

        SERIALIZABLEUNIT(SceneEditor);

        SceneEditor(ImRoot &root);
        SceneEditor(const SceneEditor &) = delete;

        virtual bool init() override;
        virtual void finalize() override;

        virtual void render() override;
        virtual void renderMenu() override;

        std::string_view key() const override;

        const Filesystem::Path &currentSceneFile() const;

        std::vector<SceneView> &views()
        {
            return mSceneViews;
        }

        int hoveredAxis() const;
        const Engine::Scene::Entity::EntityComponentPtr<Scene::Entity::Transform> &hoveredTransform() const;

        void deselect();
        void select(Render::Camera *camera);
        void select(const Scene::Entity::EntityPtr &entity);

        Scene::SceneManager &sceneMgr();

        bool render3DCursor() const;

        void play();
        void pause();
        void stop();

        void openScene(const Filesystem::Path &p);
        void saveScene(const Filesystem::Path &p);

    private:
        void renderSelection();
        void renderHierarchy();
        void renderSettings();
        void renderToolbar();
        void renderEntity(Scene::Entity::EntityPtr &entity);
        void renderCamera(Render::Camera *camera);
        void renderPopups();

        void handleInputs();

    private:
        Window::MainWindow &mWindow;

        std::vector<SceneView> mSceneViews;

        Inspector *mInspector;
        Scene::SceneManager *mSceneMgr;

        Scene::Entity::EntityPtr mSelectedEntity;
        Render::Camera *mSelectedCamera = nullptr;

        bool mHierarchyVisible = false;
        bool mSettingsVisible = false;
        bool mToolbarVisible = false;

        enum { PLAY,
            STOP,
            PAUSE } mMode;

        //Save/Load
        std::vector<char> mStartBuffer;

        Filesystem::Path mFilepickerCache;
        Filesystem::Path mFilepickerSelectionCache;
        Filesystem::Path mCurrentSceneFile;

        //Entity-Cache
        struct EntityNode {
            Scene::Entity::EntityPtr mEntity;
            std::list<EntityNode> mChildren;
        };
        std::list<EntityNode> mEntityCache;
        std::map<Scene::Entity::EntityPtr, EntityNode *> mEntityMapping;

        void updateEntityCache();
        bool updateEntityCache(EntityNode &node, const Scene::Entity::EntityPtr &parent = {});
        void createEntityMapping(Scene::Entity::EntityPtr e);
        void renderHierarchyEntity(EntityNode &entity);
        void eraseNode(EntityNode &node);

        int mHoveredAxis = -1;
        Engine::Scene::Entity::EntityComponentPtr<Scene::Entity::Transform> mHoveredTransform;


        //Settings
        Vector4 mBoneForward = { 1, 0, 0, 0 };
        float mDefaultBoneLength = 1.0f;
        bool mShowBoneNames = true;
        bool mRender3DCursor = false;
    };

}
}

RegisterType(Engine::Tools::SceneEditor);