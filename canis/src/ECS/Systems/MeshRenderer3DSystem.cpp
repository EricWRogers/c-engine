#include <Canis/ECS/Systems/MeshRenderer3DSystem.hpp>

#include <Canis/AssetManager.hpp>
#include <Canis/Entity.hpp>
#include <Canis/OpenGL.hpp>
#include <Canis/Scene.hpp>
#include <Canis/Shader.hpp>
#include <Canis/Window.hpp>
#include <Canis/App.hpp>

namespace Canis
{
    void MeshRenderer3DSystem::Create()
    {
        int id = AssetManager::LoadShader("assets/shaders/model3d");
        Shader *shader = AssetManager::Get<ShaderAsset>(id)->GetShader();

        if (!shader->IsLinked())
        {
            shader->AddAttribute("vertexPosition");
            shader->AddAttribute("vertexNormal");
            shader->AddAttribute("vertexUV");
            shader->Link();
        }

        m_shader = shader;
    }

    void MeshRenderer3DSystem::Ready()
    {
        u64 cameraMask = 0;
        u64 renderablesMask = 0;

        if (scene != nullptr && scene->app != nullptr)
        {
            if (ScriptConf *cameraConf = scene->app->GetScriptConf(Camera3D::ScriptName))
                cameraMask |= cameraConf->componentMask;

            if (ScriptConf *transformConf = scene->app->GetScriptConf(Transform3D::ScriptName))
            {
                cameraMask |= transformConf->componentMask;
                renderablesMask |= transformConf->componentMask;
            }

            if (ScriptConf *modelConf = scene->app->GetScriptConf(Model3D::ScriptName))
                renderablesMask |= modelConf->componentMask;

            if (ScriptConf *materialConf = scene->app->GetScriptConf(Material::ScriptName))
                renderablesMask |= materialConf->componentMask;
        }

        scene->InitECSView(m_cameraView, cameraMask);
        scene->InitECSView(m_renderablesView, renderablesMask);
    }

    void MeshRenderer3DSystem::Update()
    {
        if (m_shader == nullptr)
            return;

        Matrix4 projection = Matrix4(1.0f);
        Matrix4 view = Matrix4(1.0f);

        if (scene->HasEditorCamera3DOverride())
        {
            projection = scene->GetEditorCamera3DProjection();
            view = scene->GetEditorCamera3DView();
        }
        else
        {
            Camera3D *camera = nullptr;
            Transform3D *cameraTransform = nullptr;

            scene->UpdateECSView(m_cameraView);
            for (u32 entityId : m_cameraView.entities)
            {
                Entity *entity = scene->GetEntity(static_cast<int>(entityId));
                if (entity == nullptr || !entity->active)
                    continue;

                Camera3D *candidateCamera = CANIS_GET_SCRIPT(entity, Camera3D);
                Transform3D *candidateTransform = CANIS_GET_SCRIPT(entity, Transform3D);
                if (candidateCamera == nullptr || candidateTransform == nullptr)
                    continue;

                if (candidateCamera->primary)
                {
                    camera = candidateCamera;
                    cameraTransform = candidateTransform;
                    break;
                }

                if (camera == nullptr)
                {
                    camera = candidateCamera;
                    cameraTransform = candidateTransform;
                }
            }

            if (camera == nullptr || cameraTransform == nullptr)
                return;

            const float aspect = (window->GetScreenHeight() > 0)
                ? (static_cast<float>(window->GetScreenWidth()) / static_cast<float>(window->GetScreenHeight()))
                : 1.0f;
            projection = glm::perspective(DEG2RAD * camera->fovDegrees, aspect, camera->nearClip, camera->farClip);

            const Vector3 eye = cameraTransform->GetGlobalPosition();
            const Vector3 target = eye + cameraTransform->GetForward();
            const Vector3 up = cameraTransform->GetUp();
            view = glm::lookAt(eye, target, up);
        }

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        Shader *currentShader = nullptr;

        scene->UpdateECSView(m_renderablesView);
        for (u32 entityId : m_renderablesView.entities)
        {
            Entity *entity = scene->GetEntity(static_cast<int>(entityId));
            if (entity == nullptr || !entity->active)
                continue;

            Transform3D *transform = CANIS_GET_SCRIPT(entity, Transform3D);
            Model3D *modelRenderer = CANIS_GET_SCRIPT(entity, Model3D);
            if (transform == nullptr || modelRenderer == nullptr || modelRenderer->modelId < 0)
                continue;

            ModelAsset *model = AssetManager::GetModel(modelRenderer->modelId);
            if (model == nullptr)
                continue;

            MaterialAsset *materialAsset = nullptr;
            Material *material = CANIS_GET_SCRIPT(entity, Material);
            if (material != nullptr && material->materialId >= 0)
                materialAsset = AssetManager::GetMaterial(material->materialId);

            Shader *activeShader = m_shader;
            if (materialAsset != nullptr && materialAsset->shaderId >= 0)
            {
                if (ShaderAsset *shaderAsset = AssetManager::Get<ShaderAsset>(materialAsset->shaderId))
                {
                    if (!shaderAsset->GetShader()->IsLinked())
                        shaderAsset->GetShader()->Link();
                    activeShader = shaderAsset->GetShader();
                }
            }

            if (currentShader != activeShader)
            {
                if (currentShader != nullptr)
                    currentShader->UnUse();

                currentShader = activeShader;
                currentShader->Use();
                currentShader->SetMat4("P", projection);
                currentShader->SetMat4("V", view);
            }

            const ModelAsset::Pose3D *pose = nullptr;
            if (ModelAnimation3D *animation = CANIS_GET_SCRIPT(entity, ModelAnimation3D))
            {
                if (animation->poseModelId == modelRenderer->modelId)
                    pose = &animation->pose;
            }

            Color baseColor = modelRenderer->color;
            i32 overrideTextureId = -1;

            glDisable(GL_CULL_FACE);
            if (materialAsset != nullptr)
            {
                if ((materialAsset->info & MATERIAL_HAS_COLOR) != 0u)
                    baseColor *= materialAsset->color;

                if (materialAsset->albedoId >= 0)
                    overrideTextureId = materialAsset->albedoId;

                if ((materialAsset->info & MATERIAL_BACK_FACE_CULLING) != 0u)
                {
                    glEnable(GL_CULL_FACE);
                    glCullFace(GL_BACK);
                }
                else if ((materialAsset->info & MATERIAL_FRONT_FACE_CULLING) != 0u)
                {
                    glEnable(GL_CULL_FACE);
                    glCullFace(GL_FRONT);
                }

                materialAsset->materialFields.Use(*currentShader);
            }

            if (material != nullptr)
                baseColor *= material->color;

            currentShader->SetVec4("baseColor", baseColor);
            model->Draw(*currentShader, transform->GetModelMatrix(), pose, overrideTextureId);
        }

        if (currentShader != nullptr)
            currentShader->UnUse();

        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
    }
} // end of Canis namespace
