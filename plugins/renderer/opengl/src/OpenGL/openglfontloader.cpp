#include "opengllib.h"

#include "openglfontloader.h"

#include "openglfontdata.h"

#include "Modules/keyvalue/metatable_impl.h"
#include "Modules/reflection/classname.h"

#include "Modules/math/atlas2.h"
#include "Modules/math/vector2i.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

#undef INFINITE
#include "msdfgen.h"

#include "core/edge-coloring.h"

namespace msdfgen {

struct FtContext {
    Point2 position;
    Shape *shape;
    Contour *contour;
};

static Point2 ftPoint2(const FT_Vector &vector)
{
    return Point2(vector.x / 64., vector.y / 64.);
}

static int ftMoveTo(const FT_Vector *to, void *user)
{
    FtContext *context = reinterpret_cast<FtContext *>(user);
    context->contour = &context->shape->addContour();
    context->position = ftPoint2(*to);
    return 0;
}

static int ftLineTo(const FT_Vector *to, void *user)
{
    FtContext *context = reinterpret_cast<FtContext *>(user);
    context->contour->addEdge(new LinearSegment(context->position, ftPoint2(*to)));
    context->position = ftPoint2(*to);
    return 0;
}

static int ftConicTo(const FT_Vector *control, const FT_Vector *to, void *user)
{
    FtContext *context = reinterpret_cast<FtContext *>(user);
    context->contour->addEdge(new QuadraticSegment(context->position, ftPoint2(*control), ftPoint2(*to)));
    context->position = ftPoint2(*to);
    return 0;
}

static int ftCubicTo(const FT_Vector *control1, const FT_Vector *control2, const FT_Vector *to, void *user)
{
    FtContext *context = reinterpret_cast<FtContext *>(user);
    context->contour->addEdge(new CubicSegment(context->position, ftPoint2(*control1), ftPoint2(*control2), ftPoint2(*to)));
    context->position = ftPoint2(*to);
    return 0;
}


}

namespace Engine {
namespace Render {

    OpenGLFontLoader::OpenGLFontLoader()
        : ResourceLoader({ ".ttf" })
    {
    }

    std::shared_ptr<OpenGLFontData> OpenGLFontLoader::loadImpl(ResourceType *res)
    {

        FT_Library ft;
        if (FT_Init_FreeType(&ft)) {
            LOG_ERROR("FREETYPE: Could not init FreeType Library");
            return {};
        }

        FT_Face face;
        if (FT_New_Face(ft, res->path().c_str(), 0, &face)) {
            LOG_ERROR("FREETYPE: Failed to load font");
        }

        FT_Set_Pixel_Sizes(face, 0, 64);

        std::shared_ptr<OpenGLFontData> data = std::make_shared<OpenGLFontData>();

        std::vector<Vector2i> sizes;
        sizes.resize(128);
        std::vector<Vector2i> extendedSizes;
        extendedSizes.resize(128);

        for (GLubyte c = 0; c < 128; c++) {
            // Load character glyph
            if (FT_Load_Char(face, c, FT_LOAD_DEFAULT)) {
                LOG_ERROR("FREETYTPE: Failed to load Glyph");
                sizes[c] = { 0, 0 };
                extendedSizes[c] = { 0, 0 };
                continue;
            }
            sizes[c] = { static_cast<int>(face->glyph->bitmap.width) + 2, static_cast<int>(face->glyph->bitmap.rows) + 2 };
            extendedSizes[c] = { static_cast<int>(face->glyph->bitmap.width) + 3, static_cast<int>(face->glyph->bitmap.rows) + 3 };
        }

        constexpr int UNIT_SIZE = 256;

        Atlas2 atlas({ UNIT_SIZE, UNIT_SIZE });
        atlas.addBin({ 0, 0 });

        int areaSize = 1;

        auto expand = [&]() {
            for (int i = 0; i < areaSize; ++i) {
                for (int j = 0; j < areaSize; ++j) {
                    atlas.addBin({ j * UNIT_SIZE, (areaSize + i) * UNIT_SIZE });
                    atlas.addBin({ (areaSize + j) * UNIT_SIZE, i * UNIT_SIZE });
                    atlas.addBin({ (areaSize + j) * UNIT_SIZE, (areaSize + i) * UNIT_SIZE });
                }
            }
            areaSize *= 2;
        };

        std::vector<Atlas2::Entry> entries = atlas.insert(
            extendedSizes, expand, true);

        data->mTextureSize = { areaSize * UNIT_SIZE,
            areaSize * UNIT_SIZE };
        data->mTexture.setData(data->mTextureSize, nullptr);

        for (GLubyte c = 0; c < 128; c++) {

            // Load character glyph
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                LOG_ERROR("FREETYTPE: Failed to load Glyph");
                continue;
            }

            std::unique_ptr<Vector3[]> test = std::make_unique<Vector3[]>(sizes[c].x * sizes[c].y);

            msdfgen::BitmapRef<float, 3> bm { test[0].ptr(), sizes[c].x, sizes[c].y };

            msdfgen::Shape shape;
            shape.inverseYAxis = true;

            msdfgen::FtContext context = {};
            context.shape = &shape;
            FT_Outline_Funcs ftFunctions;
            ftFunctions.move_to = &msdfgen::ftMoveTo;
            ftFunctions.line_to = &msdfgen::ftLineTo;
            ftFunctions.conic_to = &msdfgen::ftConicTo;
            ftFunctions.cubic_to = &msdfgen::ftCubicTo;
            ftFunctions.shift = 0;
            ftFunctions.delta = 0;
            FT_Outline_Decompose(&face->glyph->outline, &ftFunctions, &context);

			msdfgen::edgeColoringSimple(shape, 3);
            msdfgen::generateMSDF(bm, shape, 4.0, { 1, 1 }, { static_cast<double>(-face->glyph->bitmap_left + 1),static_cast<double>(sizes[c].y - face->glyph->bitmap_top - 1) });

            data->mGlyphs[c].mSize = sizes[c];
            data->mGlyphs[c].mUV = entries[c].mArea.mTopLeft;
            data->mGlyphs[c].mFlipped = entries[c].mFlipped;
            data->mGlyphs[c].mAdvance = face->glyph->advance.x;
            data->mGlyphs[c].mBearingY = face->glyph->bitmap_top - 1;

            Vector2i size = sizes[c];
            Vector2i pos = entries[c].mArea.mTopLeft;
            if (entries[c].mFlipped)
                std::swap(size.x, size.y);
            pos = { pos.x, areaSize * UNIT_SIZE - pos.y - size.y };

            std::unique_ptr<Vector4[]> colors = std::make_unique<Vector4[]>(size.x * size.y);
            for (int y = 0; y < size.y; ++y) {
                for (int x = 0; x < size.x; ++x) {
                    int index = (size.y - y - 1) * size.x + x;
                    int sourceIndex = entries[c].mFlipped ? x * size.y + y : y * size.x + x;

                    colors[index] = /*Vector4 { 1, 1, 1, 1 } * (face->glyph->bitmap.buffer[sourceIndex] / 255.0f)*/
                        Vector4 { test[sourceIndex], 1 };
                }
            }

            data->mTexture.setSubData(pos, size, colors.get());
        }

        FT_Done_Face(face);
        FT_Done_FreeType(ft);

        return data;
    }

}
}

UNIQUECOMPONENT(Engine::Render::OpenGLFontLoader);

METATABLE_BEGIN(Engine::Render::OpenGLFontLoader)
MEMBER(mResources)
METATABLE_END(Engine::Render::OpenGLFontLoader)

METATABLE_BEGIN(Engine::Render::OpenGLFontLoader::ResourceType)
METATABLE_END(Engine::Render::OpenGLFontLoader::ResourceType)

RegisterType(Engine::Render::OpenGLFontLoader);