#include <Canis/ECS/Systems/SpriteRenderer2DSystem.hpp>

#include <vector>
#include <algorithm>

#include <Canis/Math.hpp>
#include <Canis/Time.hpp>
#include <Canis/Entity.hpp>
#include <Canis/Scene.hpp>
#include <Canis/Shader.hpp>
#include <Canis/Window.hpp>
#include <Canis/AssetHandle.hpp>
#include <Canis/AssetManager.hpp>

#include <Canis/OpenGL.hpp>

namespace Canis
{
    SpriteRenderer2DSystem::~SpriteRenderer2DSystem()
    {
        for (int i = 0; i < glyphs.size(); i++)
            delete glyphs[i];

        glyphs.clear();
    }

    void SpriteRenderer2DSystem::SortGlyphs()
    {
        switch (glyphSortType)
        {
        case GlyphSortType::BACK_TO_FRONT:
            std::stable_sort(glyphs.begin(), glyphs.end(), CompareFrontToBack);
            break;
        case GlyphSortType::FRONT_TO_BACK:
            std::stable_sort(glyphs.begin(), glyphs.end(), CompareBackToFront);
            break;
        case GlyphSortType::TEXTURE:
            std::stable_sort(glyphs.begin(), glyphs.end(), CompareTexture);
            break;
        default:
            break;
        }
    }

    void SpriteRenderer2DSystem::CreateRenderBatches()
    {
        int gSize = glyphs.size();
        if (indices.size() < gSize * 6)
        {
            indices.resize(gSize * 6);
            int ci = 0;
            int cv = 0;
            int size = gSize * 6;
            while (ci < size)
            {
                cv += 4;
                indices[ci++] = cv - 4;
                indices[ci++] = cv - 3;
                indices[ci++] = cv - 1;
                indices[ci++] = cv - 3;
                indices[ci++] = cv - 2;
                indices[ci++] = cv - 1;
            }
        }

        if (vertices.size() < gSize * 4)
            vertices.resize(gSize * 4);

        if (gSize == 0)
            return;

        int offset = 0;
        int cv = 0; // current vertex
        spriteRenderBatch.emplace_back(offset, 4, glyphs[0]->textureId);

        vertices[cv++] = glyphs[0]->topRight;
        vertices[cv++] = glyphs[0]->bottomRight;
        vertices[cv++] = glyphs[0]->bottomLeft;
        vertices[cv++] = glyphs[0]->topLeft;

        offset += 4;
        for (int cg = 1; cg < gSize; cg++)
        {
            if (glyphs[cg]->textureId != glyphs[cg - 1]->textureId)
            {
                spriteRenderBatch.emplace_back(offset, 4, glyphs[cg]->textureId);
            }
            else
            {
                spriteRenderBatch.back().numVertices += 4;
            }

            vertices[cv++] = glyphs[cg]->topRight;
            vertices[cv++] = glyphs[cg]->bottomRight;
            vertices[cv++] = glyphs[cg]->bottomLeft;
            vertices[cv++] = glyphs[cg]->topLeft;

            offset += 4;
        }

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, gSize * 4 * sizeof(SpriteVertex), nullptr, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, gSize * 4 * sizeof(SpriteVertex), vertices.data());

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, gSize * 6 * sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, gSize * 6 * sizeof(unsigned int), indices.data());

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void SpriteRenderer2DSystem::Begin(GlyphSortType sortType)
    {
        glyphSortType = sortType;
        spriteRenderBatch.clear();
        glyphsCurrentIndex = 0;
    }

    void SpriteRenderer2DSystem::End()
    {
        if (glyphsCurrentIndex < glyphs.size())
        {
            for (int i = glyphsCurrentIndex; i < glyphs.size(); i++)
                delete glyphs[i];

            glyphs.resize(glyphsCurrentIndex);
        }

        SortGlyphs();
        CreateRenderBatches();
    }

    void SpriteRenderer2DSystem::DrawUI(const Vector4 &destRect, const Vector4 &uvRect, const GLTexture &texture, float depth, const Color &color, const float &angle, const Vector2 &origin, const Vector2 &rotationOriginOffset)
    {

        Glyph *newGlyph;

        if (glyphsCurrentIndex < glyphs.size())
        {
            newGlyph = glyphs[glyphsCurrentIndex];
        }
        else
        {
            newGlyph = new Glyph;
            glyphs.push_back(newGlyph);
        }

        Vector2 halfDims(destRect.z / 2.0f, destRect.w / 2.0f);

        Vector2 topLeft(origin.x, origin.y + destRect.w);
        Vector2 bottomLeft(origin.x, origin.y);
        Vector2 bottomRight(origin.x + destRect.z, origin.y);
        Vector2 topRight(origin.x + destRect.z, origin.y + destRect.w);

         /*

         Vector2 topLeft(origin.x, origin.y + destRect.w);
        Vector2 bottomLeft(origin.x, origin.y);
        Vector2 bottomRight(origin.x + destRect.z, origin.y);
        Vector2 topRight(origin.x + destRect.z, origin.y + destRect.w);
        
        newGlyph->topLeft.position.x = destRect.x;
        newGlyph->topLeft.position.y = destRect.y + destRect.w;
        newGlyph->topLeft.position.z = depth;
        newGlyph->topLeft.color = color;
        newGlyph->topLeft.uv = Vector2(uvRect.x, uvRect.y + uvRect.w);

        newGlyph->bottomLeft.position.x = destRect.x;
        newGlyph->bottomLeft.position.y = destRect.y;
        newGlyph->bottomLeft.position.z = depth;
        newGlyph->bottomLeft.color = color;
        newGlyph->bottomLeft.uv = Vector2(uvRect.x, uvRect.y);

        newGlyph->bottomRight.position.x = destRect.x + destRect.z;
        newGlyph->bottomRight.position.y = destRect.y;
        newGlyph->bottomRight.position.z = depth;
        newGlyph->bottomRight.color = color;
        newGlyph->bottomRight.uv = Vector2(uvRect.x + uvRect.z, uvRect.y);

        newGlyph->topRight.position.x = destRect.x + destRect.z;
        newGlyph->topRight.position.y = destRect.y + destRect.w;
        newGlyph->topRight.position.z = depth;
        newGlyph->topRight.color = color;
        newGlyph->topRight.uv = Vector2(uvRect.x + uvRect.z, uvRect.y + uvRect.w);*/

        if (angle != 0.0f)
        {
            RotatePointAroundPivot(topLeft, rotationOriginOffset, angle);
            RotatePointAroundPivot(bottomLeft, rotationOriginOffset, angle);
            RotatePointAroundPivot(bottomRight, rotationOriginOffset, angle);
            RotatePointAroundPivot(topRight, rotationOriginOffset, angle);
        }

        newGlyph->textureId = texture.id;
        newGlyph->depth = depth;
        newGlyph->angle = angle;

        newGlyph->topLeft.position.x = topLeft.x + destRect.x;
        newGlyph->topLeft.position.y = topLeft.y + destRect.y;
        newGlyph->topLeft.position.z = depth;
        newGlyph->topLeft.color = color;
        newGlyph->topLeft.uv = Vector2(uvRect.x, uvRect.y + uvRect.w);

        newGlyph->bottomLeft.position.x = bottomLeft.x + destRect.x;
        newGlyph->bottomLeft.position.y = bottomLeft.y + destRect.y;
        newGlyph->bottomLeft.position.z = depth;
        newGlyph->bottomLeft.color = color;
        newGlyph->bottomLeft.uv = Vector2(uvRect.x, uvRect.y);

        newGlyph->bottomRight.position.x = bottomRight.x + destRect.x;
        newGlyph->bottomRight.position.y = bottomRight.y + destRect.y;
        newGlyph->bottomRight.position.z = depth;
        newGlyph->bottomRight.color = color;
        newGlyph->bottomRight.uv = Vector2(uvRect.x + uvRect.z, uvRect.y);

        newGlyph->topRight.position.x = topRight.x + destRect.x;
        newGlyph->topRight.position.y = topRight.y + destRect.y;
        newGlyph->topRight.position.z = depth;
        newGlyph->topRight.color = color;
        newGlyph->topRight.uv = Vector2(uvRect.x + uvRect.z, uvRect.y + uvRect.w);

       

        glyphsCurrentIndex++;
    }

    void SpriteRenderer2DSystem::Draw(const Vector4 &destRect, const Vector4 &uvRect, const GLTexture &texture, const float &depth, const Color &color, const float &angle, const Vector2 &origin)
    {
        Glyph *newGlyph;

        if (glyphsCurrentIndex < glyphs.size())
        {
            newGlyph = glyphs[glyphsCurrentIndex];
        }
        else
        {
            newGlyph = new Glyph;
            glyphs.push_back(newGlyph);
        }

        newGlyph->textureId = texture.id;
        newGlyph->depth = depth;
        newGlyph->angle = angle;

        Vector2 halfDims(destRect.z / 2.0f, destRect.w / 2.0f);

        Vector2 topLeft(-halfDims.x + origin.x, halfDims.y + origin.y);
        Vector2 bottomLeft(-halfDims.x + origin.x, -halfDims.y + origin.y);
        Vector2 bottomRight(halfDims.x + origin.x, -halfDims.y + origin.y);
        Vector2 topRight(halfDims.x + origin.x, halfDims.y + origin.y);

        if (angle != 0.0f)
        {
            float cAngle = cos(angle);
            float sAngle = sin(angle);
            RotatePoint(topLeft, cAngle, sAngle);
            RotatePoint(bottomLeft, cAngle, sAngle);
            RotatePoint(bottomRight, cAngle, sAngle);
            RotatePoint(topRight, cAngle, sAngle);
        }

        // Glyph

        // newGlyph->topLeft.position = Vector3(topLeft.x + destRect.x, topLeft.y + destRect.y, depth);
        newGlyph->topLeft.position.x = topLeft.x + destRect.x;
        newGlyph->topLeft.position.y = topLeft.y + destRect.y;
        newGlyph->topLeft.position.z = depth;
        newGlyph->topLeft.color = color;
        newGlyph->topLeft.uv.x = uvRect.x;
        newGlyph->topLeft.uv.y = uvRect.y + uvRect.w;

        // newGlyph->bottomLeft.position = Vector3(bottomLeft.x + destRect.x, bottomLeft.y + destRect.y, depth);
        newGlyph->bottomLeft.position.x = bottomLeft.x + destRect.x;
        newGlyph->bottomLeft.position.y = bottomLeft.y + destRect.y;
        newGlyph->bottomLeft.position.z = depth;
        newGlyph->bottomLeft.color = color;
        // newGlyph->bottomLeft.uv = Vector2(uvRect.x, uvRect.y);
        newGlyph->bottomLeft.uv.x = uvRect.x;
        newGlyph->bottomLeft.uv.y = uvRect.y;

        // newGlyph->bottomRight.position = Vector3(bottomRight.x + destRect.x, bottomRight.y + destRect.y, depth);
        newGlyph->bottomRight.position.x = bottomRight.x + destRect.x;
        newGlyph->bottomRight.position.y = bottomRight.y + destRect.y;
        newGlyph->bottomRight.position.z = depth;
        newGlyph->bottomRight.color = color;
        // newGlyph->bottomRight.uv = Vector2(uvRect.x + uvRect.z, uvRect.y);
        newGlyph->bottomRight.uv.x = uvRect.x + uvRect.z;
        newGlyph->bottomRight.uv.y = uvRect.y;

        // newGlyph->topRight.position = Vector3(topRight.x + destRect.x, topRight.y + destRect.y, depth);
        newGlyph->topRight.position.x = topRight.x + destRect.x;
        newGlyph->topRight.position.y = topRight.y + destRect.y;
        newGlyph->topRight.position.z = depth;
        newGlyph->topRight.color = color;
        // newGlyph->topRight.uv = Vector2(uvRect.x + uvRect.z, uvRect.y + uvRect.w);
        newGlyph->topRight.uv.x = uvRect.x + uvRect.z;
        newGlyph->topRight.uv.y = uvRect.y + uvRect.w;

        glyphsCurrentIndex++;
    }

    void SpriteRenderer2DSystem::SpriteRenderBatch(bool use2DCamera)
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);
        Debug::Log("spriteShader %p", spriteShader);
        spriteShader->Use();
        spriteShader->SetFloat("TIME", m_time);
        glBindVertexArray(vao);

        Matrix4 projection;
        projection.Identity();



        if (use2DCamera) {
            projection = camera2D->GetCameraMatrix();
        } else {
            projection.Orthographic(0.0f, static_cast<float>(window->GetScreenWidth()), 0.0f, static_cast<float>(window->GetScreenHeight()), 0.0f, 100.0f);
        }

        spriteShader->SetMat4("P", projection);

        for (int i = 0; i < spriteRenderBatch.size(); i++)
        {
            glBindTexture(GL_TEXTURE_2D, spriteRenderBatch[i].texture);

            glDrawElements(GL_TRIANGLES, (spriteRenderBatch[i].numVertices / 4) * 6, GL_UNSIGNED_INT, (void *)((spriteRenderBatch[i].offset / 4) * 6 * sizeof(unsigned int))); // spriteRenderBatch[i].offset, spriteRenderBatch[i].numVertices);
        }

        glBindVertexArray(0);
        spriteShader->UnUse();
    }

    void SpriteRenderer2DSystem::CreateVertexArray()
    {
        if (vao == 0)
            glGenVertexArrays(1, &vao);

        glBindVertexArray(vao);

        if (vbo == 0)
            glGenBuffers(1, &vbo);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        if (ebo == 0)
            glGenBuffers(1, &ebo);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (void *)0);
        // color
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (void *)(3 * sizeof(float)));
        // uv
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (void *)(7 * sizeof(float)));

        glBindVertexArray(0);
    }

    void SpriteRenderer2DSystem::SetSort(GlyphSortType _sortType)
    {
        glyphSortType = _sortType;
    }

    void SpriteRenderer2DSystem::Create()
    {
        int id = AssetManager::LoadShader("assets/shaders/sprite");
        Canis::Shader *shader = AssetManager::Get<Canis::ShaderAsset>(id)->GetShader();

        if (!shader->IsLinked())
        {
            Debug::Log("Link");
            shader->AddAttribute("vertexPosition");
            shader->AddAttribute("vertexColor");
            shader->AddAttribute("vertexUV");

            shader->Link();
        }

        spriteShader = shader;

        CreateVertexArray();
    }

    void SpriteRenderer2DSystem::Update()
    {
        m_time += Canis::Time::DeltaTime();
        
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthFunc(GL_ALWAYS);

        Begin(glyphSortType);

        bool cameraFound = false;

        std::vector<Entity*>& entities = scene->GetEntities();

        for (Entity* entity : entities)
        {
            Camera2D* camera = entity->GetScript<Camera2D>();

            if (camera == nullptr)
                continue;

            camera2D = camera;
            cameraFound = true;
        }

        // Draw
        Vector2 positionAnchor = Vector2(0.0f);
        float halfWidth = window->GetScreenWidth() / 2;
        float halfHeight = window->GetScreenHeight() / 2;
        Vector2 camPos;

        if (cameraFound)
            camPos = camera2D->GetPosition();
        else
            camPos = Vector2(0.0f);
        
        /*Vector2 anchorTable[] = {
            GetAnchor(Canis::RectAnchor::TOPLEFT, (float)window->GetScreenWidth(), (float)window->GetScreenHeight()),
            GetAnchor(Canis::RectAnchor::TOPCENTER, (float)window->GetScreenWidth(), (float)window->GetScreenHeight()),
            GetAnchor(Canis::RectAnchor::TOPRIGHT, (float)window->GetScreenWidth(), (float)window->GetScreenHeight()),
            GetAnchor(Canis::RectAnchor::CENTERLEFT, (float)window->GetScreenWidth(), (float)window->GetScreenHeight()),
            GetAnchor(Canis::RectAnchor::CENTER, (float)window->GetScreenWidth(), (float)window->GetScreenHeight()),
            GetAnchor(Canis::RectAnchor::CENTERRIGHT, (float)window->GetScreenWidth(), (float)window->GetScreenHeight()),
            GetAnchor(Canis::RectAnchor::BOTTOMLEFT, (float)window->GetScreenWidth(), (float)window->GetScreenHeight()),
            GetAnchor(Canis::RectAnchor::BOTTOMCENTER, (float)window->GetScreenWidth(), (float)window->GetScreenHeight()),
            GetAnchor(Canis::RectAnchor::BOTTOMRIGHT, (float)window->GetScreenWidth(), (float)window->GetScreenHeight())};
        */
        
        Vector2 p;
        Vector2 s;


        for (Entity* entity : entities)
        {
            Sprite2D* sprite = entity->GetScript<Sprite2D>();

            if (sprite == nullptr)
                continue;
            
            p = sprite->position;// + anchorTable[rect_transform.anchor];
            s.x = sprite->size.x + halfWidth;
            s.y = sprite->size.y + halfHeight;
            if (p.x > camPos.x - s.x &&
                p.x < camPos.x + s.x &&
                p.y > camPos.y - s.y &&
                p.y < camPos.y + s.y &&
                entity->active)
            {
                /*Draw(
                    Vector4(rect_transform.position.x + anchorTable[rect_transform.anchor].x, rect_transform.position.y + anchorTable[rect_transform.anchor].y, rect_transform.size.x, rect_transform.size.y),
                    sprite->uv,
                    sprite.textureHandle.texture,
                    rect_transform.depth,
                    sprite->color,
                    rect_transform.rotation,
                    rect_transform.originOffset);*/
                Draw(
                    Vector4(sprite->position.x, sprite->position.y, sprite->size.x, sprite->size.y),
                    sprite->uv,
                    sprite->textureHandle.texture,
                    sprite->depth,
                    sprite->color,
                    sprite->rotation,
                    sprite->originOffset);
            }
        }

        End();
        SpriteRenderBatch(cameraFound);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }
} // end of Canis namespace
