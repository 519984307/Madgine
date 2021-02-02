#pragma once

#include "imconfig.h"

#include "Modules/keyvalue/typedscopeptr.h"
#include "Modules/keyvalue/valuetype.h"

namespace ImGui {

typedef int ImGuiDragDropFlags;
typedef int ImGuiTreeNodeFlags;

struct IMGUI_API ValueTypeDrawer {
    bool draw(Engine::TypedScopePtr &scope);
    bool draw(const Engine::TypedScopePtr &scope);
    bool draw(Engine::OwnedScopePtr &scope);
    bool draw(const Engine::OwnedScopePtr &scope);
    bool draw(bool &b);
    bool draw(const bool &b);
    bool draw(Engine::CoWString &s);
    bool draw(const Engine::CoWString &s);
    bool draw(int &i);
    bool draw(const int &i);
    bool draw(uint64_t &i);
    bool draw(const uint64_t &i);
    bool draw(float &f);
    bool draw(const float &f);
    bool draw(Engine::Matrix3 &m);
    bool draw(const Engine::Matrix3 &m);
    bool draw(Engine::Matrix3 *m);
    bool draw(const Engine::Matrix3 *m);
    bool draw(Engine::Matrix4 &m);
    bool draw(const Engine::Matrix4 &m);
    bool draw(Engine::Matrix4 *m);
    bool draw(const Engine::Matrix4 *m);
    bool draw(Engine::Vector2 &v);
    bool draw(const Engine::Vector2 &v);
    bool draw(Engine::Vector3 &v);
    bool draw(const Engine::Vector3 &v);
    bool draw(Engine::Vector4 &v);
    bool draw(const Engine::Vector4 &v);
    bool draw(Engine::KeyValueVirtualRange &range);
    bool draw(const Engine::KeyValueVirtualRange &range);
    bool draw(Engine::ApiFunction &m);
    bool draw(const Engine::ApiFunction &m);
    bool draw(Engine::BoundApiFunction &m);
    bool draw(const Engine::BoundApiFunction &m);
    bool draw(std::monostate &);
    bool draw(const std::monostate &);
    bool draw(Engine::Quaternion &q);
    bool draw(const Engine::Quaternion &q);
    bool draw(Engine::ObjectPtr &o);
    bool draw(const Engine::ObjectPtr &o);
    bool draw(Engine::Filesystem::Path &p);
    bool draw(const Engine::Filesystem::Path &p);

    const char *mName;
    bool mMinified;
};

struct ValueTypePayload {
    std::string mName;
    Engine::TypedScopePtr mSender;
    Engine::ValueType mValue;
    mutable std::string mStatusMessage;
};


IMGUI_API bool showDebugData();

IMGUI_API void Text(const std::string &s);
IMGUI_API void Text(const std::string_view &s);
IMGUI_API bool InputText(const char *label, std::string *s);
IMGUI_API bool InputText(const char *label, Engine::CoWString *s);

IMGUI_API void BeginValueType(Engine::ExtendedValueTypeDesc type, const char *name = "");
IMGUI_API bool EndValueType(Engine::ValueType *v, Engine::ExtendedValueTypeDesc type);

IMGUI_API void PushDisabled();
IMGUI_API void PopDisabled();

IMGUI_API void BeginTreeArrow(const void *label, ImGuiTreeNodeFlags flags = 0);
IMGUI_API bool EndTreeArrow(bool *opened = nullptr);
IMGUI_API void BeginSpanningTreeNode(const void *id, const char *label, ImGuiTreeNodeFlags flags = 0);
IMGUI_API bool EndSpanningTreeNode();

IMGUI_API void Duration(std::chrono::nanoseconds dur);
IMGUI_API void RightAlignDuration(std::chrono::nanoseconds dur);

IMGUI_API void RightAlignText(const char *s, ...);

IMGUI_API bool DragMatrix3(const char *label, Engine::Matrix3 *m, float v_speed, bool *enabled = nullptr);
IMGUI_API bool DragMatrix3(const char *label, Engine::Matrix3 *m, float *v_speeds, bool *enabled = nullptr);
IMGUI_API bool DragMatrix4(const char *label, Engine::Matrix4 *m, float v_speed, bool *enabled = nullptr);
IMGUI_API bool DragMatrix4(const char *label, Engine::Matrix4 *m, float *v_speeds, bool *enabled = nullptr);

IMGUI_API bool MethodPicker(const char *label, const std::vector<std::pair<std::string, Engine::BoundApiFunction>> &methods, Engine::BoundApiFunction *m, std::string *currentName, std::string *filter = nullptr, int expectedArgumentCount = -1);

IMGUI_API void DraggableValueTypeSource(const std::string &name, Engine::TypedScopePtr scope, const Engine::ValueType &value, ImGuiDragDropFlags flags = 0);
IMGUI_API const ValueTypePayload *GetValuetypePayload();
IMGUI_API bool AcceptDraggableValueType(const ValueTypePayload **payloadPointer = nullptr);
template <typename T, typename Validator = bool (*)(const T &)>
bool AcceptDraggableValueType(
    T &result, const ValueTypePayload **payloadPointer = nullptr, Validator &&validate = [](const T &t) { return true; })
{
    const ValueTypePayload *payload = GetValuetypePayload();
    if (payload) {
        if (payload->mValue.is<T>()) {
            if (validate(payload->mValue.as<T>()) && AcceptDraggableValueType(payloadPointer)) {
                result = payload->mValue.as<T>();
                return true;
            } else {
                if (showDebugData())
                    payload->mStatusMessage = "Payload does not validate";
                return false;
            }
        }

        if constexpr (std::is_base_of_v<Engine::ScopeBase, T>) {
            if (payload->mValue.is<T *>()) {
                if (validate(*payload->mValue.as<T *>()) && AcceptDraggableValueType(payloadPointer)) {
                    result = *payload->mValue.as<T *>();
                    return true;
                } else {
                    if (showDebugData())
                        payload->mStatusMessage = "Payload does not validate";
                    return false;
                }
            }
        }

        if (showDebugData())
            payload->mStatusMessage = "Payload incompatible with target type: "s + typeid(T).name();
    }
    return false;
}
template <typename T, typename Validator = bool (*)(const T *)>
bool AcceptDraggableValueType(
    T *&result, const ValueTypePayload **payloadPointer = nullptr, Validator &&validate = [](const T *t) { return true; })
{
    const ValueTypePayload *payload = GetValuetypePayload();
    if (payload) {
        if (payload->mValue.is<T>()) {
            if (validate(&payload->mValue.as<T>()) && AcceptDraggableValueType(payloadPointer)) {
                result = &payload->mValue.as<T>();
                return true;
            } else {
                if (showDebugData())
                    payload->mStatusMessage = "Payload does not validate";
            }
        } else if (payload->mValue.is<T *>()) {
            if (validate(payload->mValue.as<T *>()) && AcceptDraggableValueType(payloadPointer)) {
                result = payload->mValue.as<T *>();
                return true;
            } else {
                if (showDebugData())
                    payload->mStatusMessage = "Payload does not validate";
            }
        } else {
            if (showDebugData())
                payload->mStatusMessage = "Payload incompatible with target type: "s + typeid(T).name();
        }
    }
    return false;
}
template <typename Validator = bool (*)(const Engine::ValueType &)>
bool AcceptDraggableValueType(
    Engine::ValueType &result, Engine::ExtendedValueTypeDesc type, const ValueTypePayload **payloadPointer = nullptr, Validator &&validate = [](const Engine::ValueType &t) { return true; })
{
    const ValueTypePayload *payload = GetValuetypePayload();
    if (payload) {
        if (validate(payload->mValue) && type.canAccept(payload->mValue.type()) && AcceptDraggableValueType(payloadPointer)) {
            result = payload->mValue;
            return true;
        }
    }
    return false;
}
IMGUI_API bool IsDraggableValueTypeBeingAccepted(const ValueTypePayload **payloadPointer = nullptr);
template <typename T, typename Validator = bool (*)(const T &)>
bool IsDraggableValueTypeBeingAccepted(
    T &result, const ValueTypePayload **payloadPointer = nullptr, Validator &&validate = [](const T &t) { return true; })
{
    const ValueTypePayload *payload = GetValuetypePayload();
    if (payload) {
        if (payload->mValue.is<T>()) {
            if (validate(payload->mValue.as<T>()) && IsDraggableValueTypeBeingAccepted(payloadPointer)) {
                result = payload->mValue.as<T>();
                return true;
            } else {
                if (showDebugData())
                    payload->mStatusMessage = "Payload does not validate";
            }
        } else {
            if (showDebugData())
                payload->mStatusMessage = "Payload incompatible with target type: "s + typeid(T).name();
        }
    }
    return false;
}

IMGUI_API bool DirectoryPicker(Engine::Filesystem::Path *path, Engine::Filesystem::Path *selection, bool &accepted);
IMGUI_API bool FilePicker(Engine::Filesystem::Path *path, Engine::Filesystem::Path *selection, bool &accepted, bool allowNewFile = false);
}
