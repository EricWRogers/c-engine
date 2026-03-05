#include <Canis/ECS/Systems/ModelAnimation3DSystem.hpp>

#include <unordered_set>

#include <Canis/AssetManager.hpp>
#include <Canis/Entity.hpp>
#include <Canis/Scene.hpp>
#include <Canis/Time.hpp>

namespace Canis
{
    void ModelAnimation3DSystem::Update()
    {
        const float deltaTime = Time::DeltaTime();
        std::unordered_set<i32> updatedModelIds = {};

        for (Entity *entity : scene->GetEntities())
        {
            if (entity == nullptr || !entity->active)
                continue;

            Model3D *modelRenderer = entity->GetScript<Model3D>();
            if (modelRenderer == nullptr || modelRenderer->modelId < 0)
                continue;

            if (updatedModelIds.contains(modelRenderer->modelId))
                continue;

            ModelAsset *model = AssetManager::GetModel(modelRenderer->modelId);
            if (model == nullptr)
                continue;

            const i32 animationCount = model->GetAnimationCount();
            if (animationCount <= 0)
            {
                model->ResetPose();
                updatedModelIds.insert(modelRenderer->modelId);
                continue;
            }

            Clamp(modelRenderer->animationIndex, 0, animationCount - 1);
            const float animationDuration = model->GetAnimationDuration(modelRenderer->animationIndex);

            if (modelRenderer->playAnimation && animationDuration > 0.0f)
            {
                modelRenderer->animationTime += deltaTime * modelRenderer->animationSpeed;

                if (modelRenderer->loop)
                {
                    while (modelRenderer->animationTime < 0.0f)
                        modelRenderer->animationTime += animationDuration;

                    while (modelRenderer->animationTime >= animationDuration)
                        modelRenderer->animationTime -= animationDuration;
                }
                else
                {
                    Clamp(modelRenderer->animationTime, 0.0f, animationDuration);
                }
            }

            model->UpdateAnimation(modelRenderer->animationIndex, modelRenderer->animationTime);
            updatedModelIds.insert(modelRenderer->modelId);
        }
    }
} // end of Canis namespace
