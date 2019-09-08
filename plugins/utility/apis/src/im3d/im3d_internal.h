#pragma once

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "Modules/math/boundingbox.h"
#include "Modules/math/sphere.h"

namespace Engine {
namespace Im3D {

    constexpr size_t MAX_PRIORITY_COUNT = 3;

    struct Im3DObject {
        Im3DObject(Im3DID id);

        std::string mName;
        ImU32 mID;
    };

    struct ImObjectTempData {
        Im3DObject *mLastObject;
        AABB mLastAABB;
        Matrix4 mLastTransform;
    };

    struct Im3DContext {

        Im3DIO mIO;

        struct RenderData {
            std::vector<Render::Vertex> mVertices[IM3D_MESHTYPE_COUNT];
            std::vector<unsigned int> mIndices[IM3D_MESHTYPE_COUNT];
            size_t mVertexBase[IM3D_MESHTYPE_COUNT];
            std::vector<Render::Vertex2> mVertices2[IM3D_MESHTYPE_COUNT];
            std::vector<unsigned int> mIndices2[IM3D_MESHTYPE_COUNT];
            size_t mVertexBase2[IM3D_MESHTYPE_COUNT];
        };
        std::map<Im3DTextureId, RenderData> mRenderData;

        std::map<Im3DNativeMesh, std::vector<Matrix4>> mNativeMeshes;

        std::vector<std::unique_ptr<Im3DObject>> mObjects;
        ImGuiStorage mObjectsById;
        std::vector<Im3DID> mIDStack;

        Ray mMouseRay;
        Im3DObject *mHoveredObject;
        Im3DObject *mNextHoveredObject;
        float mNextHoveredDistance;
        size_t mNextHoveredPriority;

        ImObjectTempData mTemp;
    };

    MADGINE_APIS_EXPORT Im3DObject *FindObjectByID(Im3DID id);

    MADGINE_APIS_EXPORT Im3DObject *FindObjectByName(const char *name);

    MADGINE_APIS_EXPORT Im3DID GetID(const char *name);
    MADGINE_APIS_EXPORT Im3DID GetID(const void *ptr);

}
}