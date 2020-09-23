#include "../scenerenderertoolslib.h"

#include "sceneeditor.h"

#include "Madgine_Tools/imgui/clientimroot.h"
#include "imgui/imgui.h"
#include "imgui/imguiaddons.h"

#include "Modules/keyvalue/metatable_impl.h"
#include "Modules/serialize/serializetable_impl.h"

#include "Madgine/app/application.h"
#include "Madgine/scene/entity/entitycomponentcollector.h"
#include "Madgine/scene/scenemanager.h"

#include "Madgine/scene/entity/components/animation.h"
#include "Madgine/scene/entity/components/mesh.h"
#include "Madgine/scene/entity/components/skeleton.h"
#include "Madgine/scene/entity/components/transform.h"

#include "inspector/inspector.h"

#include "im3d/im3d.h"

#include "Modules/resources/resourcemanager.h"

#include "Modules/math/boundingbox.h"

#include "Madgine/window/mainwindow.h"

#include "Madgine/scene/entity/entity.h"

namespace Engine {
namespace Tools {

    SceneEditor::SceneEditor(ImRoot &root)
        : Tool<SceneEditor>(root)
        , mWindow(static_cast<const ClientImRoot &>(root).window())
    {
    }

    bool SceneEditor::init()
    {
        mSceneMgr = &App::Application::getSingleton().getGlobalAPIComponent<Scene::SceneManager>();
        mInspector = &mRoot.getTool<Inspector>();
        mSceneViews.emplace_back(this, mWindow.getRenderer());

        return ToolBase::init();
    }

    void SceneEditor::finalize()
    {
        mSceneViews.clear();

        ToolBase::finalize();
    }

    void SceneEditor::render()
    {
        Engine::Threading::DataLock lock(mSceneMgr->mutex(), Engine::Threading::AccessMode::READ);
        if (mHierarchyVisible)
            renderHierarchy();
        if (mSettingsVisible)
            renderSettings();
        renderSelection();
        for (SceneView &sceneView : mSceneViews) {
            sceneView.render();
        }
    }

    void SceneEditor::renderMenu()
    {
        if (mVisible) {

            if (ImGui::BeginMenu("SceneEditor")) {

                ImGui::MenuItem("Hierarchy", nullptr, &mHierarchyVisible);

                ImGui::MenuItem("Settings", nullptr, &mSettingsVisible);

                ImGui::EndMenu();
            }
        }
    }

    std::string_view SceneEditor::key() const
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

    void SceneEditor::deselect()
    {
        mSelectedEntity = {};
        mSelectedCamera = nullptr;
        mHoveredAxis = -1;
    }

    void SceneEditor::select(Render::Camera *camera)
    {
        mSelectedEntity = {};
        mSelectedCamera = camera;
        mHoveredAxis = -1;
    }

    void SceneEditor::select(const Scene::Entity::EntityPtr &entity)
    {
        mSelectedEntity = entity;
        mSelectedCamera = nullptr;
        mHoveredAxis = -1;
    }

    Scene::SceneManager &SceneEditor::sceneMgr()
    {
        return *mSceneMgr;
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

            if (ImGui::Button("+ New Entity")) {
                select(mSceneMgr->createEntity());
            }

            ImGui::Separator();

            if (ImGui::BeginChild("EntityList")) {

                updateEntityCache();

                for (EntityNode &entity : mEntityCache)
                    renderHierarchyEntity(entity);

                ImGui::EndChild();
            }
        }
        ImGui::End();
    }

    void SceneEditor::renderSettings()
    {
        if (ImGui::Begin("SceneEditor - Settings", &mSettingsVisible)) {

            ImGui::ValueTypeDrawer { "Bone-Forward" }.draw(mBoneForward);
            ImGui::DragFloat("Default Bone Length", &mDefaultBoneLength);
            ImGui::Checkbox("Show Bone Names", &mShowBoneNames);
        }
        ImGui::End();
    }

    void SceneEditor::renderHierarchyEntity(EntityNode &node)
    {

        const char *name = node.mEntity->key().c_str();
        if (!name[0])
            name = "<unnamed>";

        bool hovered = mSelectedEntity == node.mEntity;

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
        if (hovered)
            flags |= ImGuiTreeNodeFlags_Selected;

        if (node.mChildren.empty())
            flags |= ImGuiTreeNodeFlags_Leaf;

        bool open = ImGui::TreeNodeEx(name, flags);

        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
            select(node.mEntity);
        }

        Scene::Entity::EntityComponentPtr<Scene::Entity::Transform> transform = node.mEntity.getComponent<Engine::Scene::Entity::Transform>();
        if (transform) {
            ImGui::DraggableValueTypeSource(name, this, ValueType { *node.mEntity });
            if (ImGui::BeginDragDropTarget()) {
                Scene::Entity::Entity *newChild;
                if (ImGui::AcceptDraggableValueType(newChild)) {
                    Scene::Entity::EntityPtr child = mSceneMgr->toEntityPtr(newChild);
                    Scene::Entity::EntityComponentPtr<Engine::Scene::Entity::Transform> childTransform = child.getComponent<Engine::Scene::Entity::Transform>();
                    assert(childTransform);
                    childTransform->setParent(transform);

                }
                ImGui::EndDragDropTarget();
            }

            Matrix4 transformM = transform->worldMatrix(mSceneMgr->entityComponentList<Engine::Scene::Entity::Transform>());
            AABB bb = { { -0.2f, -0.2f, -0.2f }, { 0.2f, 0.2f, 0.2f } };
            if (node.mEntity->hasComponent<Scene::Entity::Mesh>() && node.mEntity.getComponent<Scene::Entity::Mesh>()->data())
                bb = node.mEntity.getComponent<Scene::Entity::Mesh>()->aabb();

            Im3DBoundingObjectFlags flags = Im3DBoundingObjectFlags_ShowOnHover;
            if (hovered)
                flags |= Im3DBoundingObjectFlags_ShowOutline;

            if (Im3D::BoundingBox(name, bb, transformM, flags)) {
                if (ImGui::IsMouseClicked(0)) {
                    select(node.mEntity);
                }
                hovered = true;
            }
        }
        if (open) {
            for (EntityNode &node : node.mChildren)
                renderHierarchyEntity(node);
            ImGui::TreePop();
        }
    }

    void SceneEditor::renderEntity(Scene::Entity::EntityPtr &entity)
    {
        bool found = false;
        for (Scene::Entity::Entity &e : mSceneMgr->entities()) {
            if (&e == entity) {
                found = true;
                break;
            }
        }
        if (!found) {
            entity = {};
            return;
        }

        ImGui::InputText("Name", &entity->mName);

        for (Scene::Entity::EntityComponentBase *component : entity->components()) {
            std::string name = std::string { component->key() };
            bool open = ImGui::TreeNode(name.c_str());
            ImGui::DraggableValueTypeSource(name.c_str(), this, ValueType { component });
            if (open) {
                mInspector->draw(component);
                ImGui::TreePop();
            }
        }

        if (ImGui::Button("+ Add Component")) {
            ImGui::OpenPopup("add_component_popup");
        }

        if (ImGui::BeginPopup("add_component_popup")) {
            for (const std::pair<const std::string_view, IndexRef> &componentDesc : Scene::Entity::sComponentsByName()) {
                if (componentDesc.second.isValid() && !entity->hasComponent(componentDesc.first)) {
                    if (ImGui::Selectable(componentDesc.first.data())) {
                        entity.addComponent(componentDesc.first);
                        if (componentDesc.first == "Transform") {
                            entity.getComponent<Scene::Entity::Transform>()->setPosition({ 0, 0, 0 });
                        }
                        ImGui::CloseCurrentPopup();
                    }
                }
            }
            ImGui::EndPopup();
        }

        if (Scene::Entity::Transform *t = entity.getComponent<Scene::Entity::Transform>()) {
            constexpr Vector4 colors[] = {
                { 0.5f, 0, 0, 0.5f },
                { 0, 0.5f, 0, 0.5f },
                { 0, 0, 0.5f, 0.5f }
            };
            constexpr Vector3 offsets[] = {
                { 1, 0, 0 },
                { 0, 1, 0 },
                { 0, 0, 1 }
            };

            const char *labels[] = {
                "x-move",
                "y-move",
                "z-move"
            };

            mHoveredAxis = -1;
            mHoveredTransform = nullptr;

            Vector3 pos = (t->worldMatrix(entity.sceneMgr()->entityComponentList<Scene::Entity::Transform>()) * Vector4::UNIT_W).xyz();

            for (size_t i = 0; i < 3; ++i) {
                Im3D::Arrow(IM3D_TRIANGLES, 0.1f, pos, pos + offsets[i], colors[i]);
                if (Im3D::BoundingBox(labels[i], 0, 2)) {
                    mHoveredAxis = i;
                    mHoveredTransform = t;
                }
            }

            if (Scene::Entity::Skeleton *s = entity.getComponent<Scene::Entity::Skeleton>()) {
                if (Render::SkeletonDescriptor *skeleton = s->data()) {
                    for (size_t i = 0; i < skeleton->mBones.size(); ++i) {
                        const Engine::Render::Bone &bone = skeleton->mBones[i];

                        Matrix4 m = s->matrices()[i] * bone.mOffsetMatrix.Inverse() * skeleton->mMatrix.Inverse();
                        Matrix4 world = t->worldMatrix(entity.sceneMgr()->entityComponentList<Scene::Entity::Transform>());

                        if (mShowBoneNames)
                            Im3D::Text(bone.mName.c_str(), world * m, 2.0f);

                        Vector4 start = world * m * Vector4::UNIT_W;
                        Vector4 end;

                        if (bone.mFirstChild != std::numeric_limits<uint32_t>::max()) {
                            Matrix4 m_child = s->matrices()[bone.mFirstChild] * skeleton->mBones[bone.mFirstChild].mOffsetMatrix.Inverse() * skeleton->mMatrix.Inverse();
                            end = world * m_child * Vector4::UNIT_W;
                        } else {
                            end = world * m * skeleton->mMatrix * (mBoneForward * mDefaultBoneLength) + (1.0f - mBoneForward.w) * start;
                        }
                        float length = (end - start).xyz().length();
                        Im3D::Arrow(IM3D_LINES, 0.1f * length, start.xyz(), end.xyz());
                    }
                }
            }
        }
    }

    void SceneEditor::renderCamera(Render::Camera *camera)
    {
        mInspector->draw(camera);
    }

    void SceneEditor::updateEntityCache()
    {
        //Update + Remove deleted Entities
        mEntityCache.erase(std::remove_if(mEntityCache.begin(), mEntityCache.end(), [this](EntityNode &node) { return updateEntityCache(node); }), mEntityCache.end());

        for (EntityNode &node : mEntityCache)
            iterateMapping(node);

        //Add missing Entities
        size_t size = mSceneMgr->entities().size();
        mEntityMapping.resize(size);
        for (size_t i = 0; i < size; ++i) {
            if (!mEntityMapping[i])
                createEntityMapping(i);
        }
    }

    bool SceneEditor::updateEntityCache(EntityNode &node, const Scene::Entity::EntityPtr &parent)
    {
        uint32_t oldIndex = node.mEntity.update();
        if (!node.mEntity || 
            (!node.mEntity.hasComponent<Scene::Entity::Transform>() && parent) || 
            (node.mEntity.hasComponent<Scene::Entity::Transform>() && 
                ((parent && parent.getComponent<Scene::Entity::Transform>() != node.mEntity.getComponent<Scene::Entity::Transform>()->parent()) ||
                    (!parent && node.mEntity.getComponent<Scene::Entity::Transform>()->parent().mIndex)))) {
            mEntityMapping[oldIndex] = nullptr;
            return true;
        } else if (node.mEntity.it().index() != oldIndex) {
            mEntityMapping[oldIndex] = nullptr;
        }
        node.mChildren.erase(std::remove_if(node.mChildren.begin(), node.mChildren.end(), [&](EntityNode &childNode) { return updateEntityCache(childNode, node.mEntity); }), node.mChildren.end());
        return false;
    }

    void SceneEditor::createEntityMapping(size_t index)
    {
        Scene::Entity::EntityPtr e = mSceneMgr->entities().begin() + index;

        Scene::Entity::EntityComponentPtr<Scene::Entity::Transform> transform = e.getComponent<Scene::Entity::Transform>();

        Scene::Entity::EntityPtr parent;

        if (transform) {
            const Scene::Entity::EntityComponentHandle<Scene::Entity::Transform> &parentTransform = transform->parent();
            if (parentTransform.mIndex) {
                for (parent = mSceneMgr->entities().begin(); parent != mSceneMgr->entities().end(); ++parent) {
                    if (parent.getComponent<Scene::Entity::Transform>() == parentTransform)
                        break;
                }
            }
        }

        if (parent) {
            uint32_t parentIndex = parent.it().index();
            if (!mEntityMapping[parentIndex])
                createEntityMapping(parentIndex);
        }

        std::list<EntityNode> &container = parent ? mEntityMapping[parent.it().index()]->mChildren : mEntityCache;

        container.push_back({ std::move(e) });
        mEntityMapping[index] = &container.back();
    }

    void SceneEditor::iterateMapping(EntityNode &node)
    {
        mEntityMapping[node.mEntity.it().index()] = &node;
        for (EntityNode &node : node.mChildren)
            iterateMapping(node);
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
