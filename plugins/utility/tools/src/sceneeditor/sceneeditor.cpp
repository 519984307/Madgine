#include "../toolslib.h"

#include "sceneeditor.h"

#include "../renderer/imguiroot.h"
#include "imgui/imgui.h"
#include "imgui/imguiaddons.h"

#include "Modules/keyvalue/metatable_impl.h"
#include "Modules/reflection/classname.h"
#include "Modules/serialize/serializetable_impl.h"

#include "Madgine/app/application.h"
#include "Madgine/scene/entity/entitycomponentcollector.h"
#include "Madgine/scene/scenemanager.h"

#include "Madgine/scene/entity/components/mesh.h"
#include "Madgine/scene/entity/components/transform.h"

#include "../inspector/inspector.h"

#include "im3d/im3d.h"

namespace Engine {
namespace Tools {

    SceneEditor::SceneEditor(ImGuiRoot &root)
        : Tool<SceneEditor>(root)
        , mWindow(root.window())
    {
    }

    bool SceneEditor::init()
    {
        mSceneMgr = &app().getGlobalAPIComponent<Scene::SceneManager>();
        mInspector = &mRoot.getTool<Inspector>();
        mSceneViews.emplace_back(this, mWindow.getRenderer(), mRoot.manager());
        return ToolBase::init();
    }

    void SceneEditor::render()
    {
        if (mHierarchyVisible)
            renderHierarchy();
        renderSelection();
        for (SceneView &sceneView : mSceneViews) {
            sceneView.render();
        }
    }

    void SceneEditor::renderMenu()
    {
        if (ImGui::BeginMenu("Scene")) {

            ImGui::EndMenu();
        }

        if (mVisible) {

            if (ImGui::BeginMenu("SceneEditor")) {

                ImGui::MenuItem("Hierarchy", nullptr, &mHierarchyVisible);

                ImGui::EndMenu();
            }
        }
    }

    void SceneEditor::update()
    {
        for (SceneView &sceneView : mSceneViews) {
            auto &c = toPointer(mSceneMgr->entities());
            sceneView.camera().setVisibleEntities({ c.begin(), c.end() });
        }

        ToolBase::update();
    }

    const char *SceneEditor::key() const
    {
        return "SceneEditor";
    }

    int SceneEditor::hoveredAxis() const
    {
        return mHoveredAxis;
    }

    Scene::Entity::Transform *SceneEditor::hoveredTransform() const
    {
        return mHoveredTransform;
    }

    void SceneEditor::renderSelection()
    {
        if (ImGui::Begin("SceneEditor", &mVisible)) {
            if (mSelectedEntity)
                renderEntity(mSelectedEntity);
            if (mSelectedCamera)
                renderCamera(mSelectedCamera);
        }
        ImGui::End();
    }

    void SceneEditor::renderHierarchy()
    {
        if (ImGui::Begin("SceneEditor - Hierarchy", &mHierarchyVisible)) {

            for (Entity::Entity &e : mSceneMgr->entities()) {
                const char *name = e.key();
                if (!name[0])
                    name = "<unnamed>";
                if (ImGui::Selectable(name, mSelectedEntity == &e)) {
                    mSelectedEntity = &e;
                    mSelectedCamera = nullptr;
                }
                ImGui::DraggableValueTypeSource(name, this, ValueType { e });
            }

            if (ImGui::Button("+ New Entity")) {
                mSelectedEntity = mSceneMgr->createEntity();
                mSelectedCamera = nullptr;
            }

            ImGui::Separator();

            for (Scene::Camera &camera : mSceneMgr->cameras()) {
                std::string name = camera.getName();
                if (name.empty())
                    name = "<unnamed camera>";
                if (ImGui::Selectable(name.c_str(), mSelectedCamera == &camera)) {
                    mSelectedCamera = &camera;
                    mSelectedEntity = nullptr;
                }
                ImGui::DraggableValueTypeSource(name, this, ValueType { camera });
            }

            if (ImGui::Button("+ New Camera")) {
                mSelectedCamera = mSceneMgr->createCamera();
                mSelectedEntity = nullptr;
            }
        }
        ImGui::End();
    }

    void SceneEditor::renderEntity(Scene::Entity::Entity *entity)
    {
        ImGui::InputText("Name", &entity->mName);

        for (Entity::EntityComponentBase *component : entity->components()) {
            bool open = ImGui::TreeNode(component->key());
            ImGui::DraggableValueTypeSource(component->key(), this, ValueType { component });
            if (open) {
                mInspector->draw({ component, Entity::EntityComponentCollector::getComponentType(component->key()) });
                ImGui::TreePop();
            }
        }

        if (ImGui::Button("+ Add Component")) {
            ImGui::OpenPopup("add_component_popup");
        }

        if (ImGui::BeginPopup("add_component_popup")) {
            for (const std::string &componentName : Entity::EntityComponentCollector::registeredComponentNames()) {
                if (!entity->hasComponent(componentName)) {
                    if (ImGui::Selectable(componentName.c_str())) {
                        entity->addComponent(componentName);
                        if (componentName == "Transform") {
                            entity->getComponent<Entity::Transform>()->setPosition({ 0, 0, 0 });
                        }
                        if (componentName == "Mesh") {
                            entity->getComponent<Entity::Mesh>()->setName("mage");
                        }
                        ImGui::CloseCurrentPopup();
                    }
                }
            }
            ImGui::EndPopup();
        }

        if (Entity::Transform *t = entity->getComponent<Entity::Transform>()) {
            Vector4 colors[] = {
                { 0.5f, 0, 0, 0.5f },
                { 0, 0.5f, 0, 0.5f },
                { 0, 0, 0.5f, 0.5f }
            };
            Matrix4 transforms[] = {
                Matrix4::IDENTITY,
                { 0, 1, 0, 0,
                    1, 0, 0, 0,
                    0, 0, 1, 0,
                    0, 0, 0, 1 },
                { 0, 0, 1, 0,
                    0, 1, 0, 0,
                    1, 0, 0, 0,
                    0, 0, 0, 1 }
            };

            const unsigned int indices[] = {
                0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 1, 1, 2, 5, 2, 3, 5, 3, 4, 5, 4, 1, 5
            };

            const char *labels[] = {
                "x-move",
                "y-move",
                "z-move"
            };

            mHoveredAxis = -1;
            mHoveredTransform = nullptr;

            for (size_t i = 0; i < 3; ++i) {
                const Render::Vertex vertices[]
                    = { { { 0, 0, 0 }, colors[i], { -1, 0, 0 } },
                          { { 0.1, 0.1, -0.1 }, colors[i], { 0, 1, -1 } },
                          { { 0.1, 0.1, 0.1 }, colors[i], { 0, 1, 1 } },
                          { { 0.1, -0.1, 0.1 }, colors[i], { 0, -1, 1 } },
                          { { 0.1, -0.1, -0.1 }, colors[i], { 0, -1, -1 } },
                          { { 1, 0, 0 }, colors[i], { 1, 0, 0 } } };

                Im3D::Mesh(labels[i], vertices, 6, t->matrix() * transforms[i], indices, 24);

                if (Im3D::IsObjectHovered()) {
                    mHoveredAxis = i;
                    mHoveredTransform = t;
                }
            }
        }
    }

    void SceneEditor::renderCamera(Scene::Camera *camera)
    {
        mInspector->draw(camera);
    }

}
}

UNIQUECOMPONENT(Engine::Tools::SceneEditor);

METATABLE_BEGIN(Engine::Tools::SceneEditor)
READONLY_PROPERTY(Views, views)
METATABLE_END(Engine::Tools::SceneEditor)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::SceneEditor, Engine::Tools::ToolBase)
FIELD(mHierarchyVisible)
SERIALIZETABLE_END(Engine::Tools::SceneEditor)

RegisterType(Engine::Tools::SceneEditor);