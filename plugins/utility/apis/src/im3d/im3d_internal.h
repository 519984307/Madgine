#pragma once

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "Modules/math/sphere.h"

namespace Engine {
namespace Im3D {

    constexpr size_t MAX_PRIORITY_COUNT = 3;

    struct Im3DObject {
        Im3DObject(const char *name);

        std::string mName;
        ImU32 mID;

        Matrix4 mTransform;
        Matrix4 mInverseTransform;

        Sphere mBounds;
    };

    struct ImObjectTempData {
        Im3DObject *mLastObject;
    };

    struct Im3DContext {

        Im3DIO mIO;

        std::vector<Render::Vertex> mTriangleVertices;
        std::vector<unsigned int> mTriangleIndices;

		std::map<Im3DNativeMesh, std::vector<Matrix4>> mNativeMeshes;

        std::vector<std::unique_ptr<Im3DObject>> mObjects;
        ImGuiStorage mObjectsById;

        Ray mMouseRay;
        Im3DObject *mHoveredObject;
        Im3DObject *mNextHoveredObject;
        float mNextHoveredDistance;
        size_t mNextHoveredPriority;

        ImObjectTempData mTemp;
    };

    MADGINE_APIS_EXPORT Im3DObject *FindObjectByID(ImGuiID id);

    MADGINE_APIS_EXPORT Im3DObject *FindObjectByName(const char *name);

}
}