#pragma once

#if defined(Im3D_EXPORTS)
#    define MADGINE_IM3D_EXPORT DLL_EXPORT
#else
#    define MADGINE_IM3D_EXPORT DLL_IMPORT
#endif

#include "Modules/math/matrix4.h"
#include "Modules/math/ray.h"
#include "Modules/math/vector2i.h"
#include "render/vertex.h"

#include "im3d_parameters.h"

namespace Engine {

enum Im3DMeshType {
    IM3D_POINTS,
    IM3D_LINES,
    IM3D_TRIANGLES,
    IM3D_MESHTYPE_COUNT
};

typedef uintptr_t Im3DNativeMesh;
typedef uintptr_t Im3DTextureId;

typedef uint32_t Im3DID;
typedef int Im3DBoundingObjectFlags;

enum Im3DBoundingObjectFlags_ {
    Im3DBoundingObjectFlags_ShowOutline = 1 << 0,
    Im3DBoundingObjectFlags_ShowOnHover = 1 << 1
};

struct Im3DFont {
    Im3DTextureId *mTexture;
    Vector2i mTextureSize;

    Render::Glyph *mGlyphs;
};

struct Im3DIO {
    Ray mNextFrameMouseRay;

    Im3DFont (*mFetchFont)(const char *) = nullptr;
    void (*mReleaseFont)(Im3DFont &) = nullptr;
};

namespace Im3D {

    struct Im3DContext;
    struct Im3DObject;

    MADGINE_IM3D_EXPORT void CreateContext();
    MADGINE_IM3D_EXPORT void DestroyContext();
    MADGINE_IM3D_EXPORT Im3DContext *GetCurrentContext();

    MADGINE_IM3D_EXPORT Im3DIO &GetIO();

    MADGINE_IM3D_EXPORT void NewFrame();

    MADGINE_IM3D_EXPORT void PushID(const void *ptr);
    MADGINE_IM3D_EXPORT void PopID();

    MADGINE_IM3D_EXPORT void Mesh(Im3DMeshType type, const Render::Vertex *vertices, size_t vertexCount, const MeshParameters &param = {}, const unsigned short *indices = nullptr, size_t indexCount = 0);
    MADGINE_IM3D_EXPORT void Mesh(Im3DMeshType type, Render::RenderPassFlags flags, const Render::Vertex2 *vertices, size_t vertexCount, const MeshParameters &param = {}, const unsigned short *indices = nullptr, size_t indexCount = 0, Im3DTextureId texId = 0);

    MADGINE_IM3D_EXPORT void NativeMesh(Im3DNativeMesh mesh, const AABB &bb, const Matrix4 &transform = Matrix4::IDENTITY);

    MADGINE_IM3D_EXPORT void Text(const char *text, const TextParameters &param);

    MADGINE_IM3D_EXPORT void Line(const Vector3 &a, const Vector3 &b, const LineParameters &param = {});
    MADGINE_IM3D_EXPORT void Arrow(float radius, const Vector3 &a, const Vector3 &b, const Parameters &param = {});
    MADGINE_IM3D_EXPORT void Arrow3D(Im3DMeshType type, float radius, const Vector3 &a, const Vector3 &b, const Parameters &param = {});
    MADGINE_IM3D_EXPORT void Sphere(const Vector3 &center, float radius, const SphereParameters &param = {});

    MADGINE_IM3D_EXPORT bool BoundingSphere(const char *name, Im3DBoundingObjectFlags flags = 0, size_t priority = 1);
    MADGINE_IM3D_EXPORT bool BoundingSphere(Im3DID id, Im3DBoundingObjectFlags flags = 0, size_t priority = 1);
    MADGINE_IM3D_EXPORT bool BoundingSphere(const char *name, const AABB &bb, const Matrix4 &transform = Matrix4::IDENTITY, Im3DBoundingObjectFlags flags = 0, size_t priority = 1);
    MADGINE_IM3D_EXPORT bool BoundingSphere(Im3DID id, const AABB &bb, const Matrix4 &transform = Matrix4::IDENTITY, Im3DBoundingObjectFlags flags = 0, size_t priority = 1);

    MADGINE_IM3D_EXPORT bool BoundingBox(const char *name, Im3DBoundingObjectFlags flags = 0, size_t priority = 1);
    MADGINE_IM3D_EXPORT bool BoundingBox(Im3DID id, Im3DBoundingObjectFlags flags = 0, size_t priority = 1);
    MADGINE_IM3D_EXPORT bool BoundingBox(const char *name, const AABB &bb, const Matrix4 &transform = Matrix4::IDENTITY, Im3DBoundingObjectFlags flags = 0, size_t priority = 1);
    MADGINE_IM3D_EXPORT bool BoundingBox(Im3DID id, const AABB &bb, const Matrix4 &transform = Matrix4::IDENTITY, Im3DBoundingObjectFlags flags = 0, size_t priority = 1);

    MADGINE_IM3D_EXPORT bool BoundingObject(Im3DID id, float distance = 0.0f, Im3DBoundingObjectFlags flags = 0, size_t priority = 1);

    MADGINE_IM3D_EXPORT bool IsObjectHovered();
    MADGINE_IM3D_EXPORT bool IsAnyObjectHovered();
}

}