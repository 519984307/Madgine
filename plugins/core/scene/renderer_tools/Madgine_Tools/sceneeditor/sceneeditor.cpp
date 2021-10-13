#include "../scenerenderertoolslib.h"

#include "sceneeditor.h"

#include "Madgine_Tools/imgui/clientimroot.h"
#include "imgui/imgui.h"
#include "imgui/imguiaddons.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine/app/application.h"
#include "Madgine/scene/entity/entitycomponentcollector.h"
#include "Madgine/scene/scenemanager.h"

#include "Madgine/scene/entity/components/animation.h"
#include "Madgine/scene/entity/components/mesh.h"
#include "Madgine/scene/entity/components/skeleton.h"
#include "Madgine/scene/entity/components/transform.h"

#include "inspector/inspector.h"

#include "im3d/im3d.h"

#include "Madgine/resources/resourcemanager.h"

#include "Meta/math/boundingbox.h"

#include "Madgine/window/mainwindow.h"

#include "Madgine/scene/entity/entity.h"

#include "Interfaces/input/inputevents.h"

#include "serialize/memory/memorylib.h"
#include "serialize/memory/memorymanager.h"


#include "Meta/serialize/formatter/xmlformatter.h"

#include "serialize/filesystem/filesystemlib.h"
#include "serialize/filesystem/filemanager.h"


#include "Meta/serialize/formatter/safebinaryformatter.h"

#include "imgui/imgui_internal.h"


UNIQUECOMPONENT(Engine::Tools::SceneEditor);

METATABLE_BEGIN_BASE(Engine::Tools::SceneEditor, Engine::Tools::ToolBase)
READONLY_PROPERTY(Views, views)
READONLY_PROPERTY(CurrentScene, currentSceneFile)
METATABLE_END(Engine::Tools::SceneEditor)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::SceneEditor, Engine::Tools::ToolBase)
FIELD(mHierarchyVisible)
FIELD(mToolbarVisible)
ENCAPSULATED_FIELD(mCurrentSceneFile, currentSceneFile, openScene)
SERIALIZETABLE_END(Engine::Tools::SceneEditor)

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

        mSceneMgr->pause();
        mMode = STOP;

        return ToolBase::init();
    }

    void SceneEditor::finalize()
    {
        mSceneViews.clear();

        ToolBase::finalize();
    }

    void SceneEditor::render()
    {
        Engine::Threading::DataLock lock(mSceneMgr->mutex(), Engine::Threading::AccessMode::WRITE);
        if (mHierarchyVisible)
            renderHierarchy();
        if (mSettingsVisible)
            renderSettings();
        if (mToolbarVisible)
            renderToolbar();
        renderSelection();
        for (SceneView &sceneView : mSceneViews) {
            sceneView.render();
        }
        handleInputs();
    }

    void SceneEditor::renderMenu()
    {
        if (mVisible) {

            bool openOpenScenePopup = false;
            bool openSaveScenePopup = false;

            if (ImGui::BeginMenu("SceneEditor")) {

                bool isStopped = mMode == STOP;

                if (ImGui::MenuItem("Open", nullptr, nullptr, isStopped))
                    openOpenScenePopup = true;
                if (ImGui::MenuItem("Save", nullptr, nullptr, isStopped)) {
                    if (mCurrentSceneFile.empty())
                        openSaveScenePopup = true;
                    else
                        saveScene(mCurrentSceneFile);
                }
                if (ImGui::MenuItem("Save As...", nullptr, nullptr, isStopped))
                    openSaveScenePopup = true;

                ImGui::Separator();

                if (ImGui::BeginMenu("Views")) {
                    ImGui::MenuItem("Hierarchy", nullptr, &mHierarchyVisible);

                    ImGui::MenuItem("Settings", nullptr, &mSettingsVisible);

                    ImGui::MenuItem("Toolbar", nullptr, &mToolbarVisible);

                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }

            if (openOpenScenePopup)
                ImGui::OpenPopup("openScene");
            if (openSaveScenePopup)
                ImGui::OpenPopup("saveScene");

            renderPopups();
        }
    }

    std::string_view SceneEditor::key() const
    {
        return "SceneEditor";
    }

    const Filesystem::Path &SceneEditor::currentSceneFile() const
    {
        return mCurrentSceneFile;
    }

    int SceneEditor::hoveredAxis() const
    {
        return mHoveredAxis;
    }

    const Scene::Entity::EntityComponentPtr<Scene::Entity::Transform> &SceneEditor::hoveredTransform() const
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

    bool SceneEditor::render3DCursor() const
    {
        return mRender3DCursor;
    }

    void SceneEditor::play()
    {
        if (mMode == PLAY)
            return;

        if (mMode == STOP) {
            Memory::MemoryManager mgr { "Tmp" };
            Serialize::SerializeOutStream out = mgr.openWrite(mStartBuffer, std::make_unique<Serialize::SafeBinaryFormatter>());
            mSceneMgr->writeState(out);
        }

        mSceneMgr->unpause();
        mMode = PLAY;
    }

    void SceneEditor::pause()
    {
        if (mMode != PLAY)
            return;

        mSceneMgr->pause();
        mMode = PAUSE;
    }

    void SceneEditor::stop()
    {
        if (mMode == STOP)
            return;

        if (mMode == PLAY)
            mSceneMgr->pause();
        mMode = STOP;

        Memory::MemoryManager mgr { "Tmp" };
        Serialize::SerializeInStream in = mgr.openRead(mStartBuffer, std::make_unique<Serialize::SafeBinaryFormatter>());
        mSceneMgr->readState(in, nullptr, {}, Serialize::StateTransmissionFlags_ApplyMap);
    }

    void SceneEditor::openScene(const Filesystem::Path &p)
    {
        if (p.empty())
            return;

        mCurrentSceneFile = p;

        Engine::Threading::DataLock lock(mSceneMgr->mutex(), Engine::Threading::AccessMode::WRITE);

        Filesystem::FileManager mgr { "Scene" };
        Serialize::SerializeInStream in = mgr.openRead(mCurrentSceneFile, std::make_unique<Serialize::XMLFormatter>());
        mSceneMgr->readState(in, "Scene", {}, Serialize::StateTransmissionFlags_ApplyMap);
    }

    void SceneEditor::saveScene(const Filesystem::Path &p)
    {
        mCurrentSceneFile = p;

        Engine::Threading::DataLock lock(mSceneMgr->mutex(), Engine::Threading::AccessMode::READ);

        Filesystem::FileManager mgr { "Scene" };
        Serialize::SerializeOutStream out = mgr.openWrite(mCurrentSceneFile, std::make_unique<Serialize::XMLFormatter>());
        mSceneMgr->writeState(out, "Scene");
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
            ImGui::Checkbox("Render 3D-Cursor", &mRender3DCursor);
        }
        ImGui::End();
    }

    void SceneEditor::renderToolbar()
    {
        if (ImGui::Begin("SceneEditor - Toolbar", &mToolbarVisible, ImGuiWindowFlags_NoTitleBar)) {

            auto pre = [](bool b) { if (b) ImGui::PushDisabled(); };
            auto post = [](bool b) { if (b) ImGui::PopDisabled(); };

            bool b = mMode == PLAY;
            pre(b);
            if (ImGui::Button(mSceneMgr->isPaused() && b ? "Halted" : "Play")) {
                play();
            }
            post(b);

            ImGui::SameLine(0, 0);

            b = mMode == PAUSE || mMode == STOP;
            pre(b);
            if (ImGui::Button("Pause")) {
                pause();
            }
            post(b);

            ImGui::SameLine(0, 0);

            b = mMode == STOP;
            pre(b);
            if (ImGui::Button("Stop")) {
                stop();
            }
            post(b);
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

        ImGui::DraggableValueTypeSource(name, node.mEntity);

        Scene::Entity::EntityComponentPtr<Scene::Entity::Transform> transform = node.mEntity->getComponent<Engine::Scene::Entity::Transform>();
        if (transform) {
            if (ImGui::BeginDragDropTarget()) {
                Scene::Entity::EntityPtr *newChild;
                if (ImGui::AcceptDraggableValueType(newChild, nullptr, [](Scene::Entity::EntityPtr *child) { return (*child)->hasComponent<Scene::Entity::Transform>(); })) {
                    Scene::Entity::EntityComponentPtr<Engine::Scene::Entity::Transform> childTransform = (*newChild)->getComponent<Engine::Scene::Entity::Transform>();
                    assert(childTransform);
                    childTransform->setParent(transform);
                }
                ImGui::EndDragDropTarget();
            }

            Matrix4 transformM = transform->worldMatrix();
            AABB bb = { { -0.2f, -0.2f, -0.2f }, { 0.2f, 0.2f, 0.2f } };
            if (node.mEntity->hasComponent<Scene::Entity::Mesh>() && node.mEntity->getComponent<Scene::Entity::Mesh>()->data())
                bb = node.mEntity->getComponent<Scene::Entity::Mesh>()->aabb();

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

    void SceneEditor::eraseNode(EntityNode &node)
    {
        for (EntityNode &child : node.mChildren)
            eraseNode(child);
        mEntityMapping.erase(node.mEntity);
    }

    void SceneEditor::renderEntity(Scene::Entity::EntityPtr &entity)
    {
        bool found = false;
        for (const Scene::Entity::EntityPtr &e : mSceneMgr->entities()) {
            if (e == entity) {
                found = true;
                break;
            }
        }
        if (!found) {
            entity = {};
            return;
        }

        ImGui::InputText("Name", &entity->mName);

        for (const Scene::Entity::EntityComponentPtr<Scene::Entity::EntityComponentBase> &component : entity->components()) {
            std::string name = std::string { component.name() };
            bool open = ImGui::TreeNode(name.c_str());
            //TODO
            //ImGui::DraggableValueTypeSource(name.c_str(), this, ValueType { Scene::Entity::EntityComponentPtr<Scene::Entity::EntityComponentBase> { component } });
            if (open) {
                mInspector->drawMembers(component.getTyped());
                ImGui::TreePop();
            }
        }

        if (ImGui::Button("+ Add Component")) {
            ImGui::OpenPopup("add_component_popup");
        }

        if (ImGui::BeginPopup("add_component_popup")) {
            for (const std::pair<const std::string_view, size_t> &componentDesc : Scene::Entity::EntityComponentRegistry::sComponentsByName()) {
                if (!entity->hasComponent(componentDesc.first)) {
                    if (ImGui::Selectable(componentDesc.first.data())) {
                        entity->addComponent(componentDesc.first);
                        if (componentDesc.first == "Transform") {
                            entity->getComponent<Scene::Entity::Transform>()->setPosition({ 0, 0, 0 });
                        }
                        ImGui::CloseCurrentPopup();
                    }
                }
            }
            ImGui::EndPopup();
        }

        if (Engine::Scene::Entity::EntityComponentPtr<Scene::Entity::Transform> t = entity->getComponent<Scene::Entity::Transform>()) {
            constexpr Vector4 colors[] = {
                { 0.5f, 0, 0, 0.7f },
                { 0, 0.5f, 0, 0.7f },
                { 0, 0, 0.5f, 0.7f }
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
            mHoveredTransform = {};

            Vector3 pos = (t->worldMatrix() * Vector4::UNIT_W).xyz();

            for (size_t i = 0; i < 3; ++i) {
                Im3D::Arrow3D(IM3D_TRIANGLES, 0.1f, pos, pos + offsets[i], colors[i]);
                if (Im3D::BoundingBox(labels[i], 0, 2)) {
                    mHoveredAxis = i;
                    mHoveredTransform = t;
                }
            }

            if (Scene::Entity::Skeleton *s = entity->getComponent<Scene::Entity::Skeleton>()) {
                if (Render::SkeletonDescriptor *skeleton = s->data()) {
                    for (size_t i = 0; i < skeleton->mBones.size(); ++i) {
                        const Engine::Render::Bone &bone = skeleton->mBones[i];

                        Matrix4 m = s->matrices()[i] * bone.mOffsetMatrix.Inverse() * skeleton->mMatrix.Inverse();
                        Matrix4 world = t->worldMatrix();

                        if (mShowBoneNames)
                            Im3D::Text(bone.mName.c_str(), Im3D::TextParameters { world * m, 2.0f });

                        Vector4 start = world * m * Vector4::UNIT_W;
                        Vector4 end;

                        if (bone.mFirstChild) {
                            Matrix4 m_child = s->matrices()[bone.mFirstChild] * skeleton->mBones[bone.mFirstChild].mOffsetMatrix.Inverse() * skeleton->mMatrix.Inverse();
                            end = world * m_child * Vector4::UNIT_W;
                        } else {
                            end = world * m * skeleton->mMatrix * (mBoneForward * mDefaultBoneLength) + (1.0f - mBoneForward.w) * start;
                        }
                        float length = (end - start).xyz().length();
                        Im3D::Arrow3D(IM3D_LINES, 0.1f * length, start.xyz(), end.xyz());
                    }
                }
            }
        }
    }

    void SceneEditor::renderCamera(Render::Camera *camera)
    {
        mInspector->drawMembers(camera);
    }

    void SceneEditor::renderPopups()
    {
        ImGui::SetNextWindowSize({ 500, 400 }, ImGuiCond_FirstUseEver);
        if (ImGui::BeginPopup("openScene")) {
            bool accepted;
            if (ImGui::FilePicker(&mFilepickerCache, &mFilepickerSelectionCache, accepted, false)) {
                if (accepted) {
                    openScene(mFilepickerSelectionCache);
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::SetNextWindowSize({ 500, 400 }, ImGuiCond_FirstUseEver);
        if (ImGui::BeginPopup("saveScene")) {
            bool accepted;
            if (ImGui::FilePicker(&mFilepickerCache, &mFilepickerSelectionCache, accepted, true)) {
                if (accepted) {
                    saveScene(mFilepickerSelectionCache);
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    void SceneEditor::handleInputs()
    {
        if (ImGui::IsKeyPressed(Input::Key::Delete)) {
            if (mSelectedEntity)
                mSelectedEntity->remove();
        }
    }

    void SceneEditor::updateEntityCache()
    {
        //Update + Remove deleted Entities
        mEntityCache.remove_if([this](EntityNode &node) { return updateEntityCache(node); });

        //Add missing Entities
        for (Scene::Entity::EntityPtr entity : mSceneMgr->entities()) {
            if (!mEntityMapping.count(entity))
                createEntityMapping(std::move(entity));
        }
    }

    bool SceneEditor::updateEntityCache(EntityNode &node, const Scene::Entity::EntityPtr &parent)
    {
        if (node.mEntity.isDead() || (!node.mEntity->hasComponent<Scene::Entity::Transform>() && parent) || (node.mEntity->hasComponent<Scene::Entity::Transform>() && ((parent && parent->getComponent<Scene::Entity::Transform>() != node.mEntity->getComponent<Scene::Entity::Transform>()->parent()) || (!parent && node.mEntity->getComponent<Scene::Entity::Transform>()->parent())))) {
            eraseNode(node);
            return true;
        }
        node.mChildren.remove_if([&](EntityNode &childNode) { return updateEntityCache(childNode, node.mEntity); });
        return false;
    }

    void SceneEditor::createEntityMapping(Scene::Entity::EntityPtr e)
    {
        Scene::Entity::EntityComponentPtr<Scene::Entity::Transform> transform = e->getComponent<Scene::Entity::Transform>();

        Scene::Entity::EntityPtr parent;

        if (transform) {
            Scene::Entity::Transform *parentTransform = transform->parent();
            if (parentTransform) {
                for (Scene::Entity::EntityPtr p : mSceneMgr->entities()) {
                    if (p->getComponent<Scene::Entity::Transform>() == parentTransform) {
                        parent = p;
                        break;
                    }
                }
            }
        }

        if (parent) {
            if (!mEntityMapping.count(parent))
                createEntityMapping(parent);
        }

        std::list<EntityNode> &container = parent ? mEntityMapping[parent]->mChildren : mEntityCache;

        container.push_back({ std::move(e) });
        mEntityMapping[e] = &container.back();
    }

}
}
