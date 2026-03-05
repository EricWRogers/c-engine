#include <Canis/App.hpp>
#include <Canis/Entity.hpp>
#include <Canis/Scene.hpp>
#include <Canis/Window.hpp>
#include <Canis/AssetManager.hpp>
#include <Canis/Debug.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>

namespace Canis {

void Entity::Destroy() {
    scene->Destroy(id);
}

void RectTransform::EditorInspectorDraw()
{
    std::string nameOfType = "RectTransform";
    ImGui::Text("%s", nameOfType.c_str());
    ImGui::InputFloat2("position", &position.x, "%.3f");
    ImGui::InputFloat2("size", &size.x, "%.3f");
    ImGui::InputFloat2("scale", &scale.x);
    ImGui::InputFloat2("originOffset", &originOffset.x, "%.3f");
    ImGui::InputFloat("depth", &depth);
    // let user work with degrees
    float degrees = RAD2DEG * rotation;
    ImGui::InputFloat("rotation", &degrees);
    rotation = DEG2RAD * degrees;
}

void Sprite2D::EditorInspectorDraw()
{
    std::string nameOfType = "Sprite2D";
    ImGui::Text("%s", nameOfType.c_str());
    ImGui::ColorEdit4("color", &color.r);
    ImGui::InputFloat4("uv", &uv.x, "%.3f");
}

void Text::EditorInspectorDraw()
{
    std::string nameOfType = "Text";
    ImGui::Text("%s", nameOfType.c_str());
    ImGui::InputText("text", &text);
    ImGui::InputInt("font asset id", &assetId);
}

void Transform3D::EditorInspectorDraw()
{
    std::string nameOfType = "Transform3D";
    ImGui::Text("%s", nameOfType.c_str());
    ImGui::InputFloat3("position", &position.x, "%.3f");

    Vector3 degrees = rotation * RAD2DEG;
    ImGui::InputFloat3("rotation", &degrees.x, "%.3f");
    rotation = degrees * DEG2RAD;

    ImGui::InputFloat3("scale", &scale.x, "%.3f");
}

void Camera3D::EditorInspectorDraw()
{
    std::string nameOfType = "Camera3D";
    ImGui::Text("%s", nameOfType.c_str());
    ImGui::Checkbox("primary", &primary);
    ImGui::InputFloat("fovDegrees", &fovDegrees);
    ImGui::InputFloat("nearClip", &nearClip);
    ImGui::InputFloat("farClip", &farClip);
}

void Model3D::EditorInspectorDraw()
{
    std::string nameOfType = "Model3D";
    ImGui::Text("%s", nameOfType.c_str());
    ImGui::InputInt("modelId", &modelId);
    ImGui::ColorEdit4("color", &color.r);
}

void ModelAnimation3D::EditorInspectorDraw()
{
    std::string nameOfType = "ModelAnimation3D";
    ImGui::Text("%s", nameOfType.c_str());
    ImGui::Checkbox("playAnimation", &playAnimation);
    ImGui::Checkbox("loop", &loop);
    ImGui::InputFloat("animationSpeed", &animationSpeed);
    ImGui::InputFloat("animationTime", &animationTime);
    ImGui::InputInt("animationIndex", &animationIndex);
}

Camera2D::Camera2D(Canis::Entity &_entity)
    : Canis::ScriptableEntity(_entity), m_position(0.0f, 0.0f), m_scale(8.0f),
      m_needsMatrixUpdate(true), m_screenWidth(500), m_screenHeight(500) {
    m_cameraMatrix.Identity();
    m_view.Identity();
    m_projection.Identity();
}

Camera2D::~Camera2D() {}

void Camera2D::Create() {
    m_screenWidth = entity.scene->GetWindow().GetScreenWidth();
    m_screenHeight = entity.scene->GetWindow().GetScreenHeight();
    m_projection.Orthographic(0.0f, (float)m_screenWidth, 0.0f,
                              (float)m_screenHeight, 0.0f, 100.0f);
    SetPosition(Vector2(0.0f)); // Vector2((float)m_screenWidth / 2,
                                // (float)m_screenHeight / 2));
    SetScale(1.0f);
}

void Camera2D::Destroy() {
    Debug::Log("DestroyCamera");
}

void Camera2D::Update(float _dt) {}

void Camera2D::EditorInspectorDraw()
{
    std::string nameOfType = "Camera2D";
    ImGui::Text("%s", nameOfType.c_str());

    Vector2 lastPosition = GetPosition();
    float lastScale = GetScale();

    ImGui::InputFloat2("position", &lastPosition.x, "%.3f");
    ImGui::InputFloat("scale", &lastScale);

    if (lastPosition != GetPosition())
        SetPosition(lastPosition);

    if (lastScale != GetScale())
        SetScale(lastScale);
}

void Camera2D::UpdateMatrix()
{
    m_view.Identity();
    m_view.Translate(Vector3(-m_position.x + m_screenWidth / 2,
                                -m_position.y + m_screenHeight / 2, 0.0f));
    m_view.Scale(Vector3(m_scale, m_scale, 0.0f));

    m_cameraMatrix = m_projection * m_view;

    m_needsMatrixUpdate = false;
}

void SpriteAnimation::Play(std::string _path)
{
    speed = 1.0f;
    id = AssetManager::LoadSpriteAnimation(_path);
    index = 0;
    redraw = true;
}
} // namespace Canis
