#include <Canis/ECS/Systems/MeshRenderer3DSystem.hpp>

#include <Canis/AssetManager.hpp>
#include <Canis/Entity.hpp>
#include <Canis/OpenGL.hpp>
#include <Canis/Scene.hpp>
#include <Canis/Shader.hpp>
#include <Canis/Window.hpp>

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

    void MeshRenderer3DSystem::Update()
    {
        if (m_shader == nullptr)
            return;

        Camera3D *camera = nullptr;
        Transform3D *cameraTransform = nullptr;

        for (Entity *entity : scene->GetEntities())
        {
            if (entity == nullptr || !entity->active)
                continue;

            Camera3D *candidateCamera = entity->GetScript<Camera3D>();
            Transform3D *candidateTransform = entity->GetScript<Transform3D>();
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

        Matrix4 projection;
        projection.Identity();
        const float aspect = (window->GetScreenHeight() > 0)
            ? (static_cast<float>(window->GetScreenWidth()) / static_cast<float>(window->GetScreenHeight()))
            : 1.0f;
        projection.Perspective(DEG2RAD * camera->fovDegrees, aspect, camera->nearClip, camera->farClip);

        const Vector3 eye = cameraTransform->GetGlobalPosition();
        const Vector3 target = eye + cameraTransform->GetForward();
        const Vector3 up = cameraTransform->GetUp();
        const Matrix4 view = LookAt(eye, target, up);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        m_shader->Use();
        m_shader->SetMat4("P", projection);
        m_shader->SetMat4("V", view);

        for (Entity *entity : scene->GetEntities())
        {
            if (entity == nullptr || !entity->active)
                continue;

            Transform3D *transform = entity->GetScript<Transform3D>();
            Model3D *modelRenderer = entity->GetScript<Model3D>();
            if (transform == nullptr || modelRenderer == nullptr || modelRenderer->modelId < 0)
                continue;

            ModelAsset *model = AssetManager::GetModel(modelRenderer->modelId);
            if (model == nullptr)
                continue;

            const ModelAsset::Pose3D *pose = nullptr;
            if (ModelAnimation3D *animation = entity->GetScript<ModelAnimation3D>())
            {
                if (animation->poseModelId == modelRenderer->modelId)
                    pose = &animation->pose;
            }

            m_shader->SetVec4("baseColor", modelRenderer->color);
            model->Draw(*m_shader, transform->GetModelMatrix(), pose);
        }

        m_shader->UnUse();

        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
    }
} // end of Canis namespace
