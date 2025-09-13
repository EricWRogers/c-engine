#pragma once
#include <Canis/Scene.hpp>
#include <Canis/OpenGL.hpp>
#include <Canis/Data/GLTexture.hpp>
#include <Canis/Camera2D.hpp>
#include <Canis/Data/Glyph.hpp>
#include <Canis/System.hpp>

namespace Canis
{
    class Shader;

    class RenderBatch
    {
    public:
        RenderBatch(unsigned int Offset, unsigned int NumVertices, unsigned int Texture) : offset(Offset),
                                                                         numVertices(NumVertices), texture(Texture) {}
        unsigned int offset;
        unsigned int numVertices;
        unsigned int texture;
    };

    class SpriteRenderer2DSystem : public System
    {
    public:
        GlyphSortType glyphSortType = GlyphSortType::FRONT_TO_BACK;
        std::vector<Glyph *> glyphs;
        std::vector<SpriteVertex> vertices = {};
        std::vector<unsigned int> indices = {};
        std::vector<RenderBatch> spriteRenderBatch;
        Shader *spriteShader;
        Camera2D camera2D;

        unsigned int vbo = 0;
        unsigned int vao = 0;
        unsigned int ebo = 0;
        unsigned int glyphsCurrentIndex = 0;
        unsigned int glyphsMaxIndex = 0;

        static bool CompareFrontToBack(Glyph *a, Glyph *b) { return (a->depth < b->depth); }
        static bool CompareBackToFront(Glyph *a, Glyph *b) { return (a->depth > b->depth); }
        static bool CompareTexture(Glyph *a, Glyph *b) { return (a->textureId < b->textureId); }

        SpriteRenderer2DSystem() : System() { m_name = type_name<SpriteRenderer2DSystem>(); }

        ~SpriteRenderer2DSystem();

        void SortGlyphs();

        void CreateRenderBatches();

        void Begin(GlyphSortType sortType);

        void End();

        void DrawUI(const Vector4 &destRect, const Vector4 &uvRect, const GLTexture &texture, float depth, const Color &color, const float &angle, const Vector2 &origin, const Vector2 &rotationOriginOffset);

        void Draw(const Vector4 &destRect, const Vector4 &uvRect, const GLTexture &texture, const float &depth, const Color &color, const float &angle, const Vector2 &origin);

        void SpriteRenderBatch(bool use2DCamera);

        void CreateVertexArray();

        void SetSort(GlyphSortType _sortType);

        void Create();

        void Ready() {}

        void Update();
    private:
        float m_time = 0.0f;
    };
} // end of Canis namespace