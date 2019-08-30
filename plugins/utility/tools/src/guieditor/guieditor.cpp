#include "../toolslib.h"

#include "guieditor.h"

#include "imgui/imgui.h"
#include "../renderer/imguiroot.h"

#include "imgui/imguiaddons.h"

#include "Modules/keyvalue/metatable_impl.h"
#include "Modules/math/bounds.h"
#include "Modules/reflection/classname.h"
#include "Modules/serialize/serializetable_impl.h"

#include "Madgine/gui/guisystem.h"
#include "Madgine/gui/widgets/toplevelwindow.h"
#include "Madgine/gui/widgets/widget.h"

#include "Interfaces/window/windowapi.h"

UNIQUECOMPONENT(Engine::Tools::GuiEditor);

namespace Engine {
namespace Tools {

    GuiEditor::GuiEditor(ImGuiRoot &root)
        : Tool<GuiEditor>(root)
        , mWindow(root.window())
    {
    }

    bool GuiEditor::init()
    {
        return ToolBase::init();
    }

    void GuiEditor::render()
    {
        GUI::Widget *hoveredWidget = nullptr;
        if (mHierarchyVisible)
            renderHierarchy(&hoveredWidget);
        renderSelection(hoveredWidget);
    }

    void GuiEditor::renderMenu()
    {
        if (ImGui::BeginMenu("Layouts")) {
            for (GUI::Widget *w : mWindow.widgets()) {
                if (ImGui::MenuItem(w->key(), nullptr, w->isVisible())) {
                    mWindow.swapCurrentRoot(w);
                }
            }
            ImGui::EndMenu();
        }

        if (mVisible) {

            if (ImGui::BeginMenu("GuiEditor")) {

                ImGui::MenuItem("Hierarchy", nullptr, &mHierarchyVisible);

                ImGui::EndMenu();
            }
        }
    }

    void GuiEditor::update()
    {

        ToolBase::update();
    }

    const char *GuiEditor::key() const
    {
        return "GuiEditor";
    }

    void GuiEditor::renderSelection(GUI::Widget *hoveredWidget)
    {
        constexpr float borderSize = 10.0f;

        if (ImGui::Begin("GuiEditor", &mVisible)) {

            ImDrawList *background = ImGui::GetBackgroundDrawList(ImGui::GetMainViewport());

            auto [screenPos, screenSize] = mWindow.getAvailableScreenSpace();

            Vector3 windowPos = {
                static_cast<float>(mWindow.window()->renderX()), static_cast<float>(mWindow.window()->renderY()), 0.0f
            };

            ImGuiIO &io = ImGui::GetIO();

            Vector2 mouse = ImGui::GetMousePos();
            Vector2 dragDistance = mouse - io.MouseClickedPos[0];

            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
                screenPos += windowPos;

            bool acceptHover = (hoveredWidget != nullptr || !io.WantCaptureMouse) && (!mSelected || !mSelected->widget()->containsPoint(mouse, screenSize, screenPos, borderSize));

            if (mSelected) {
                GUI::Widget *selectedWidget = mSelected->widget();

                Vector3 absoluteSize = selectedWidget->getAbsoluteSize() * screenSize;
                Vector3 absolutePos = selectedWidget->getAbsolutePosition() * screenSize + screenPos;

                Math::Bounds bounds(absolutePos.x, absolutePos.y + absoluteSize.y, absolutePos.x + absoluteSize.x, absolutePos.y);

                background->AddRect(bounds.topLeft(), bounds.bottomRight(), IM_COL32(255, 255, 255, 255));

                ImU32 resizeColor = IM_COL32(0, 255, 255, 255);
                if (io.KeyShift) {
                    resizeColor = IM_COL32(0, 255, 0, 255);
                }

                if (!io.WantCaptureMouse) {

                    bool rightBorder = false, leftBorder = false, topBorder = false, bottomBorder = false;

                    if (!mDragging && selectedWidget->containsPoint(mouse, screenSize, screenPos, borderSize)) {
                        leftBorder = abs(mouse.x - bounds.left()) < borderSize;
                        rightBorder = abs(mouse.x - bounds.right()) < borderSize;
                        topBorder = abs(mouse.y - bounds.top()) < borderSize;
                        bottomBorder = abs(mouse.y - bounds.bottom()) < borderSize;

                        if (mSelected->aspectRatio()) {
                            if (topBorder || leftBorder) {
                                leftBorder = !rightBorder;
                                topBorder = !bottomBorder;
                            }
                            if (bottomBorder || rightBorder) {
                                rightBorder = !leftBorder;
                                bottomBorder = !topBorder;
                            }
                        }

                        acceptHover &= (!rightBorder && !leftBorder && !topBorder && !bottomBorder);
                    }

                    constexpr float thickness = 4.0f;

                    //TODO
                    if (leftBorder || mDraggingLeft)
                        background->AddLine(bounds.topLeft(), bounds.bottomLeft(), resizeColor, thickness);
                    if (rightBorder || mDraggingRight)
                        background->AddLine(bounds.topRight(), bounds.bottomRight(), resizeColor, thickness);
                    if (topBorder || mDraggingTop)
                        background->AddLine(bounds.topLeft(), bounds.topRight(), resizeColor, thickness);
                    if (bottomBorder || mDraggingBottom)
                        background->AddLine(bounds.bottomLeft(), bounds.bottomRight(), resizeColor, thickness);

                    if (io.MouseClicked[0] && selectedWidget->containsPoint(mouse, screenSize, screenPos, borderSize)) {
                        mMouseDown = true;
                        mDraggingLeft = leftBorder;
                        mDraggingRight = rightBorder;
                        mDraggingTop = topBorder;
                        mDraggingBottom = bottomBorder;
                    }

                    if (mMouseDown && dragDistance.length() >= io.MouseDragThreshold && !mDragging) {
                        mSelected->saveGeometry();
                        mDragging = true;
                    }
                }
            }

            if (!hoveredWidget)
                hoveredWidget = mWindow.hoveredWidget();
            WidgetSettings *hoveredSettings = nullptr;

            if (acceptHover) {
                if (hoveredWidget) {
                    hoveredSettings = static_cast<WidgetSettings *>(hoveredWidget->userData());
                    if (!hoveredSettings) {
                        hoveredSettings = &mSettings.emplace_back(hoveredWidget);
                    }

                    if (!mDragging) {
                        Vector3 size = hoveredWidget->getAbsoluteSize() * screenSize;
                        Vector3 pos = hoveredWidget->getAbsolutePosition() * screenSize + screenPos;

                        Math::Bounds bounds(pos.x, pos.y + size.y, pos.x + size.x, pos.y);

                        background->AddRect(bounds.topLeft(), bounds.bottomRight(), IM_COL32(127, 127, 127, 255));
                    }
                }
                if (io.MouseClicked[0] && !mDragging) {
                    mSelected = hoveredSettings;
                }
            }

            if (mSelected) {

                enum ResizeMode {
                    RELATIVE,
                    ABSOLUTE
                };

                ResizeMode resizeMode = RELATIVE;
                if (io.KeyShift) {
                    resizeMode = ABSOLUTE;
                }

                if (mDragging) {

                    auto [pos, size] = mSelected->savedGeometry();

                    Matrix3 parentSize = mSelected->widget()->getParent() ? mSelected->widget()->getParent()->getAbsoluteSize() : Matrix3::IDENTITY;

                    Vector2 relDragDistance = dragDistance / (parentSize * screenSize).xy();

                    Matrix3 dragDistanceSize;

                    switch (resizeMode) {
                    case RELATIVE:
                        dragDistanceSize = Matrix3 {
                            relDragDistance.x, 0, 0,
                            0, relDragDistance.y, 0,
                            0, 0, 0
                        };
                        break;
                    case ABSOLUTE:
                        dragDistanceSize = Matrix3 {
                            0, 0, dragDistance.x / parentSize[2][2],
                            0, 0, dragDistance.y / parentSize[2][2],
                            0, 0, 0
                        };
                        break;
                    }

                    if (!mDraggingLeft && !mDraggingRight && !mDraggingTop && !mDraggingBottom) {
                        pos += dragDistanceSize;
                    } else {
                        Matrix3 dragDistancePos { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
                        if (!mDraggingLeft && !mDraggingRight) {
                            dragDistanceSize[0][0] = 0.0f;
                            dragDistanceSize[0][2] = 0.0f;
                            dragDistancePos[0][0] = 0.0f;
                            dragDistancePos[0][2] = 0.0f;
                        } else if (mDraggingLeft) {
                            dragDistancePos[0][0] = dragDistanceSize[0][0];
                            dragDistancePos[0][2] = dragDistanceSize[0][2];
							dragDistanceSize[0][0] *= -1.0f;                            
							dragDistanceSize[0][2] *= -1.0f;                            
                        }
                        if (!mDraggingTop && !mDraggingBottom) {
                            dragDistanceSize[1][1] = 0.0f;
                            dragDistanceSize[1][2] = 0.0f;
                            dragDistancePos[1][1] = 0.0f;
                            dragDistancePos[1][2] = 0.0f;
                        } else if (mDraggingBottom) {
                            dragDistancePos[1][1] = dragDistanceSize[1][1];
                            dragDistancePos[1][2] = dragDistanceSize[1][2];
                            dragDistanceSize[1][1] *= -1.0f;
                            dragDistanceSize[1][2] *= -1.0f;
                        }

                        pos += dragDistancePos;
                        size += dragDistanceSize;
                    }

                    mSelected->setSize(size);
                    mSelected->setPos(pos);

                    if (io.MouseReleased[0]) {
                        mSelected->applyGeometry();
                        mDragging = false;
                    }
                }

                if (io.MouseReleased[0]) {
                    mMouseDown = false;
                    mDraggingLeft = false;
                    mDraggingRight = false;
                    mDraggingTop = false;
                    mDraggingBottom = false;
                }
            }

            if (mSelected) {
                ImGui::Text("Selected");
                mSelected->render();
            }
            /*if (hoveredSettings && !mDragging && hoveredSettings != mSelected) {
                ImGui::Text("Hovered");
                hoveredSettings->render();
            }*/

            //io.WantCaptureMouse = true;
        }
        ImGui::End();
    }

    void GuiEditor::listWidgets(GUI::Widget *w, GUI::Widget **hoveredWidget)
    {
        for (GUI::Widget *child : w->children()) {

            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
            if (child->children().empty())
                flags |= ImGuiTreeNodeFlags_Leaf;
            if (mSelected && mSelected->widget() == child)
                flags |= ImGuiTreeNodeFlags_Selected;

            if (ImGui::TreeNodeEx(child->getName().c_str(), flags)) {
                listWidgets(child, hoveredWidget);
                ImGui::TreePop();
            }
            if (hoveredWidget && !*hoveredWidget) {
                if (ImGui::IsItemHovered()) {
                    *hoveredWidget = child;
                }
            }
        }
    }

    void GuiEditor::renderHierarchy(GUI::Widget **hoveredWidget)
    {
        if (ImGui::Begin("GuiEditor - Hierarchy", &mHierarchyVisible)) {
            GUI::Widget *root = mWindow.currentRoot();
            if (root) {
                listWidgets(root, hoveredWidget);
            } else {
                ImGui::Text("%s", "Please select a root window under 'Layout' in the menu bar.");
            }
        }
        ImGui::End();
    }

}
}

METATABLE_BEGIN(Engine::Tools::GuiEditor)
METATABLE_END(Engine::Tools::GuiEditor)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::GuiEditor, Engine::Tools::ToolBase)
FIELD(mHierarchyVisible)
SERIALIZETABLE_END(Engine::Tools::GuiEditor)

RegisterType(Engine::Tools::GuiEditor);