#pragma once

#include <imgui/imgui.h>
#include <Engine/SceneGraph.h>

struct Inspector
{
    static ImGuiChildFlags flags;
    static ImVec2 windowSize;

    template <typename T>
    static void drawContent(const std::shared_ptr<T> &node, bool draw_children = true);
    template <typename T>
    static void draw(const std::shared_ptr<T> &node);
    static void drawNode(const std::shared_ptr<Node> &node);
    static void drawChildren(const std::shared_ptr<Node> &node)
    {
        if (ImGui::BeginChild((node->name + "_Children").c_str(), ImVec2(0, 0), flags))
        {
            ImGui::Text("Children");
            for (const auto &child : node->children)
            {
                drawNode(child);
            }
        }
        ImGui::EndChild();
    }
};

inline ImGuiChildFlags Inspector::flags = ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY;
inline ImVec2 Inspector::windowSize = ImVec2(0, 0);

template <typename T>
inline void Inspector::drawContent(const std::shared_ptr<T> &node, bool draw_children)
{
    throw;
}
template <typename T>
inline void Inspector::draw(const std::shared_ptr<T> &node)
{
    using namespace ImGui;
    Separator();
    if (CollapsingHeader((node->name + "_Node").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Indent();
        if (ImGui::BeginChild(node->name.c_str(), windowSize, flags))
        {
            drawContent<T>(node);
        }
        ImGui::EndChild();
        Unindent();
    }
}

template <>
inline void Inspector::drawContent<Node>(const std::shared_ptr<Node> &node, bool draw_children)
{
    if (draw_children)
        drawChildren(node);
}

template <>
inline void Inspector::drawContent<Transformable>(const std::shared_ptr<Transformable> &node, bool draw_children)
{
    using namespace ImGui;
    node->transform.transformChanged |= InputFloat3("position", &node->transform.position.x);
#ifdef GLM_FORCE_QUAT_DATA_WXYZ
    node->transform.transformChanged |= InputFloat4("rotation", &node->transform.rotation.w);
#else
    node->transform.transformChanged |= InputFloat4("rotation", &node->transform.rotation.x);
#endif

    glm::vec3 eulerAngles = glm::eulerAngles(node->transform.rotation);
    eulerAngles = glm::degrees(eulerAngles);
    node->transform.transformChanged |= InputFloat3("euler angles", &eulerAngles.x);
    eulerAngles = glm::radians(eulerAngles);
    node->transform.rotation = glm::quat(eulerAngles);

    node->transform.transformChanged |= InputFloat3("scale", &node->transform.scale.x);

    node->updateTransform();

    if (draw_children)
        drawChildren(node);
}

#include <imgui/imgui_stdlib.h>

template <>
inline void Inspector::drawContent<Renderable>(const std::shared_ptr<Renderable> &node, bool draw_children)
{
    using namespace ImGui;
    drawContent<Transformable>(std::reinterpret_pointer_cast<Transformable>(node), false);
    std::string name_buff = node->render_name;
    if (InputText("name", &name_buff, ImGuiInputTextFlags_EnterReturnsTrue))
        node->render_name = name_buff;
    Checkbox("visible", &node->visible);
    InputInt("forced shader", &node->forced_shader);
    if (draw_children)
        drawChildren(node);
}

template <>
inline void Inspector::drawContent<SwitchNode>(const std::shared_ptr<SwitchNode> &node, bool draw_children)
{
    using namespace ImGui;
    InputInt("active child", &node->active_child);
    if (draw_children)
        drawChildren(node);
}

// template <>
// inline void Inspector::draw<SceneGraph::CameraNode>(const std::shared_ptr<Node> &node)
// {
//     using namespace ImGui;
//     Separator();
//     if (CollapsingHeader((std::to_string(node->id) + "_CameraNode").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
//     {
//         Indent();
//         if (BeginChild(std::to_string(node->id).c_str(), windowSize, flags))
//         {
//             Text("Projection Type: %s", (node->projectionType == SceneGraph::CameraNode::ProjectionType::Ortho ? "orthographic" : "perspective"));
//             InputFloat("near  ", &node->near); SameLine(); InputFloat("far", &node->far);
//             if (node->projectionType == SceneGraph::CameraNode::ProjectionType::Ortho)
//             {
//                 InputFloat("left  ", &node->ortho.left); SameLine(); InputFloat("right", &node->ortho.right);
//                 InputFloat("bottom", &node->ortho.bottom); SameLine(); InputFloat("top", &node->ortho.top);
//             }
//             else
//             {
//                 InputFloat("aspect", &node->perspective.aspect); SameLine(); InputFloat("FOV", &node->perspective.fov);
//             }
//             Text("Viewport");
//             Indent();
//             InputInt("x    ", (int*)&node->viewport.x, 0, 0); SameLine(); InputInt("y     ", (int*)&node->viewport.y, 0, 0);
//             InputInt("width", (int*)&node->viewport.width, 0, 0); SameLine(); InputInt("height", (int*)&node->viewport.height, 0, 0);
//             Unindent();

//             node->calcProjection();
//             node->calcView();
//             node->calcFrustum();

//             drawChildren(node);
//         }
//         EndChild();
//         Unindent();
//     }
// }

// template <>
// inline void Inspector::draw<SceneGraph::MouseLookNode>(const std::shared_ptr<Node> &node)
// {
//     using namespace ImGui;
//     Separator();
//     if (CollapsingHeader((std::to_string(node->id) + "_MouseLookNode").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
//     {
//         Indent();
//         if (BeginChild(std::to_string(node->id).c_str(), windowSize, flags))
//         {
//             InputFloat("sensitivity", &node->sensitivity);
//             InputFloat("Min X rotation", &node->x_limit_min); SameLine(); InputFloat("Max X rotation", &node->x_limit_max);
//             InputFloat3("position", &node->transformation.position.x);
// #ifdef GLM_FORCE_QUAT_DATA_WXYZ
//             InputFloat4("rotation", &node->transformation.rotation.w);
// #else
//             InputFloat4("rotation", &node->transformation.rotation.x);
// #endif

//             glm::vec3 eulerAngles = glm::eulerAngles(node->transformation.rotation);
//             eulerAngles = glm::degrees(eulerAngles);
//             InputFloat3("euler angles", &eulerAngles.x);
//             eulerAngles = glm::radians(eulerAngles);
//             node->transformation.rotation = glm::quat(eulerAngles);

//             InputFloat3("scale", &node->transformation.scale.x);

//             if (node->transformation.isDirty())
//                 node->transformation.computeModelMatrix();
//             drawChildren(node);
//         }
//         EndChild();
//         Unindent();
//     }
// }

//

inline void Inspector::drawNode(const std::shared_ptr<Node> &_node)
{

    if (const auto &node = std::dynamic_pointer_cast<Renderable>(_node))
    {
        draw<Renderable>(node);
    }
    else if (const auto &node = std::dynamic_pointer_cast<Transformable>(_node))
    {
        draw<Transformable>(node);
    }
    else if (const auto &node = std::dynamic_pointer_cast<SwitchNode>(_node))
    {
        draw<SwitchNode>(node);
    }
    else
    {
        draw<Node>(_node);
    }
}