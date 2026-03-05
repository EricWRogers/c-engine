#include <Canis/ECS/Systems/ModelAnimation3DSystem.hpp>

#include <Canis/AssetManager.hpp>
#include <Canis/Entity.hpp>
#include <Canis/Scene.hpp>
#include <Canis/Time.hpp>

namespace Canis
{
    void ModelAnimation3DSystem::Update()
    {
        const float deltaTime = Time::DeltaTime();

        for (Entity *entity : scene->GetEntities())
        {
            if (entity == nullptr || !entity->active)
                continue;

            Model3D *modelRenderer = entity->GetScript<Model3D>();
            ModelAnimation3D *modelAnimation = entity->GetScript<ModelAnimation3D>();
            if (modelRenderer == nullptr || modelAnimation == nullptr || modelRenderer->modelId < 0)
                continue;

            ModelAsset *model = AssetManager::GetModel(modelRenderer->modelId);
            if (model == nullptr)
                continue;

            if (modelAnimation->poseModelId != modelRenderer->modelId)
            {
                modelAnimation->poseModelId = modelRenderer->modelId;
                model->ResetPose(modelAnimation->pose);
            }

            const i32 animationCount = model->GetAnimationCount();
            if (animationCount <= 0)
            {
                model->ResetPose(modelAnimation->pose);
                continue;
            }

            Clamp(modelAnimation->animationIndex, 0, animationCount - 1);
            const float animationDuration = model->GetAnimationDuration(modelAnimation->animationIndex);

            if (modelAnimation->playAnimation && animationDuration > 0.0f)
            {
                modelAnimation->animationTime += deltaTime * modelAnimation->animationSpeed;

                if (modelAnimation->loop)
                {
                    while (modelAnimation->animationTime < 0.0f)
                        modelAnimation->animationTime += animationDuration;

                    while (modelAnimation->animationTime >= animationDuration)
                        modelAnimation->animationTime -= animationDuration;
                }
                else
                {
                    Clamp(modelAnimation->animationTime, 0.0f, animationDuration);
                }
            }

            if (!model->UpdateAnimation(modelAnimation->pose, modelAnimation->animationIndex, modelAnimation->animationTime))
                model->ResetPose(modelAnimation->pose);
        }
    }
} // end of Canis namespace
