#include "../toolslib.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "inspector.h"

#include "imgui/imguiaddons.h"

#include "Madgine/resources/resourcemanager.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Meta/keyvalue/scopeiterator.h"

#include "Meta/keyvalue/keyvaluepair.h"

#include "functiontool.h"

#include "Madgine/base/keyvalueregistry.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine/resources/resourceloaderbase.h"

#include "../renderer/imroot.h"

UNIQUECOMPONENT(Engine::Tools::Inspector);

METATABLE_BEGIN_BASE(Engine::Tools::Inspector, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::Inspector)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::Inspector, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::Inspector)

namespace Engine {
namespace Tools {

    Inspector::Inspector(ImRoot &root)
        : Tool<Inspector>(root)
    {
        addObjectSuggestion<FunctionTable>([]() {
            std::vector<std::pair<std::string_view, TypedScopePtr>> result;
            const FunctionTable *table = sFunctionList();
            while (table) {
                result.emplace_back(table->mName, const_cast<FunctionTable *>(table));
                table = table->mNext;
            }
            return result;
        });
    }

    Inspector::~Inspector()
    {
    }

    void Inspector::render()
    {
        if (ImGui::Begin("Inspector", &mVisible)) {
            ImGui::SetWindowDockingDir(mRoot.dockSpaceId(), ImGuiDir_Left, 0.2f, false, ImGuiCond_FirstUseEver);

            auto drawList = [this](const std::map<std::string_view, TypedScopePtr> &items) {
                for (const std::pair<const std::string_view, TypedScopePtr> &p : items) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    if (ImGui::TreeNode(p.first.data())) {
                        drawMembers(p.second, {});
                        ImGui::TreePop();
                    }
                }
            };

            if (ImGui::BeginTable("table", 2, ImGuiTableFlags_Resizable)) {
                drawList(KeyValueRegistry::globals());
                drawList(KeyValueRegistry::workgroupLocals());
                ImGui::EndTable();
            }
        }
        ImGui::End();
    }

    bool Inspector::drawRemainingMembers(TypedScopePtr scope, std::set<std::string> &drawn)
    {
        bool changed = false;

        for (ScopeIterator it = scope.begin(); it != scope.end(); ++it) {
            if (drawn.count(it->key()) == 0) {
                ImGui::TableNextRow();
                changed |= drawMember(scope, it);
                drawn.insert(it->key());
            }
        }

        return changed;
    }

    bool Inspector::drawMember(TypedScopePtr parent, const ScopeIterator &it, tinyxml2::XMLElement *element)
    {
        ValueType value;
        if (streq(it->key(), "__proxy")) {
            it->value(value);
            return drawMembers(value.as<TypedScopePtr>(), {});
        }

        std::string_view id = it->key();
        bool editable = it->isEditable();
        bool generic = it->isGeneric();

        it->value(value);
        std::pair<bool, bool> modified = drawValue(id, value, editable, generic, element);

        if (modified.first || (modified.second && !value.isReference()))
            *it = value;
        return modified.first || modified.second;
    }

    std::pair<bool, bool> Inspector::drawValue(std::string_view id, ValueType &value, bool editable, bool generic, tinyxml2::XMLElement *element)
    {
        if (generic)
            ImGui::BeginValueType({ ExtendedValueTypeEnum::GenericType }, id.data());

        std::pair<bool, bool> modified = value.visit(overloaded { [&](TypedScopePtr &scope) {
                                                                     return drawValue(id, scope, editable, element);
                                                                 },
            [&](OwnedScopePtr scope) {
                return drawValue(id, scope, editable, element);
            },
            [&](KeyValueVirtualSequenceRange &range) {
                return std::make_pair(false, drawValue(id, range, editable, element));
            },
            [&](KeyValueVirtualAssociativeRange &range) {
                return std::make_pair(false, drawValue(id, range, editable, element));
            },
            [&](BoundApiFunction &function) {
                drawValue(id, function, editable, element);
                return std::make_pair(false, false);
            },
            [&](auto &other) {
                if (!editable)
                    ImGui::BeginDisabled();
                ImGui::Indent();
                std::pair<bool, bool> result = std::make_pair(ImGui::ValueTypeDrawer { id.data(), false }.draw(other), false);
                ImGui::Unindent();
                if (!editable)
                    ImGui::EndDisabled();
                return result;
            } });

        if (generic)
            modified.first |= ImGui::EndValueType(&value, { ExtendedValueTypeEnum::GenericType });

        return modified;
    }

    std::pair<bool, bool> Inspector::drawValue(std::string_view id, TypedScopePtr &scope, bool editable, tinyxml2::XMLElement *element)
    {
        bool modified = false;
        bool changed = false;

        auto it = mObjectSuggestionsByType.find(scope.mType);
        bool hasSuggestions = editable && it != mObjectSuggestionsByType.end();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        bool open = false;
        if (scope)
            open = ImGui::TreeNode(id.data());
        else {
            ImGui::Indent();
            ImGui::Text(id);
            ImGui::Unindent();
        }

        ImGui::TableNextColumn();

        if (hasSuggestions) {
            ImGui::PushID(id.data());
            ImGui::PushItemWidth(-1.0f);
            if (ImGui::BeginCombo("##suggestions", scope.name().c_str())) {
                for (std::pair<std::string_view, TypedScopePtr> p : it->second()) {
                    if (ImGui::Selectable(p.first.data())) {
                        scope = p.second;
                        modified = true;
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
            ImGui::PopID();
        }

        ImGui::DraggableValueTypeSource(id, scope, ImGuiDragDropFlags_SourceAllowNullID);
        if (editable && ImGui::BeginDragDropTarget()) {
            if (ImGui::AcceptDraggableValueType(scope, nullptr, [&](const TypedScopePtr &ptr) {
                    return ptr.mType->isDerivedFrom(scope.mType);
                })) {
                modified = true;
            }
            /*OwnedScopePtr dummy;
            if (ImGui::AcceptDraggableValueType(dummy, nullptr, [&](const OwnedScopePtr &ptr) {
                    return ptr.type()->isDerivedFrom(scope.mType);
                })) {
                scope = dummy;
                modified = true;
            }*/
            ImGui::EndDragDropTarget();
        }

        if (open) {
            changed |= drawMembers(scope, {});
            ImGui::TreePop();
        }
        return std::make_pair(modified, changed);
    }

    std::pair<bool, bool> Inspector::drawValue(std::string_view id, OwnedScopePtr &scope, bool editable, tinyxml2::XMLElement *element)
    {
        TypedScopePtr ptr = scope.get();
        return drawValue(id, ptr, editable, element);
    }

    bool Inspector::drawValue(std::string_view id, KeyValueVirtualSequenceRange &range, bool editable, tinyxml2::XMLElement *element)
    {
        ImGui::TableNextColumn();

        bool changed = false;
        bool b = ImGui::TreeNodeEx(id.data());
        ImGui::DraggableValueTypeSource(id, range);

        ImGui::TableNextColumn();

        if (b) {
            size_t i = 0;
            for (auto vValue : range) {
                ImGui::TableNextRow();
                ValueType value = vValue;
                std::string key = std::to_string(i);
                if (value.is<TypedScopePtr>()) {
                    key = "[" + std::to_string(i) + "] " + value.as<TypedScopePtr>().name() + "##" + key;
                } else if (value.is<OwnedScopePtr>()) {
                    key = "[" + std::to_string(i) + "] " + value.as<OwnedScopePtr>().name() + "##" + key;
                }
                std::pair<bool, bool> result = drawValue(key, value, /*editable && */ vValue.isEditable(), false, element);
                if (result.first)
                    vValue = value;
                changed |= result.second;
                ++i;
            }
            ImGui::TreePop();
        }
        return changed;
    }

    bool Inspector::drawValue(std::string_view id, KeyValueVirtualAssociativeRange &range, bool editable, tinyxml2::XMLElement *element)
    {
        ImGui::TableNextColumn();

        bool changed = false;
        bool b = ImGui::TreeNodeEx(id.data());
        ImGui::DraggableValueTypeSource(id, range);

        ImGui::TableNextColumn();

        if (b) {
            size_t i = 0;
            for (auto [vKey, vValue] : range) {
                ImGui::TableNextRow();
                ValueType value = vValue;
                std::string key = vKey.toShortString() /* + "##" + std::to_string(i)*/;
                std::pair<bool, bool> result = drawValue(key, value, /*editable && */ vValue.isEditable(), false, element);
                if (result.first)
                    vValue = value;
                changed |= result.second;
                ++i;
            }
            ImGui::TreePop();
        }
        return changed;
    }

    void Inspector::drawValue(std::string_view id, BoundApiFunction &function, bool editable, tinyxml2::XMLElement *element)
    {
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        std::string extended = "-> " + std::string { id };
        if (ImGui::Button(extended.c_str())) {
            getTool<FunctionTool>().setCurrentFunction(id, function);
        }
        ImGui::DraggableValueTypeSource(id, function);
    }

    bool Inspector::drawMembers(TypedScopePtr scope, std::set<std::string> drawn)
    {
        assert(scope);

        bool changed = drawRemainingMembers(scope, drawn);

        auto it2 = mPreviews.find(scope.mType);
        if (it2 != mPreviews.end()) {
            it2->second(scope);
        }
        return changed;
    }

    std::string_view Inspector::key() const
    {
        return "Inspector";
    }

    void Inspector::addObjectSuggestion(const MetaTable *type, std::function<std::vector<std::pair<std::string_view, TypedScopePtr>>()> getter)
    {
        mObjectSuggestionsByType[type] = getter;
    }

    bool Inspector::hasObjectSuggestion(const MetaTable *type) const
    {
        return mObjectSuggestionsByType.contains(type);
    }

    void Inspector::addPreviewDefinition(const MetaTable *type, std::function<void(TypedScopePtr)> preview)
    {
        mPreviews[type] = preview;
    }

}
}