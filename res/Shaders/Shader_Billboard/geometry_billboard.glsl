#version 450

layout(points) in;
layout(triangle_strip, max_vertices = 6) out;

layout(location = 0) in vec3 g_midPoint[1];

layout(location = 0) out vec2 f_uv;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform int lockHorizontal;

vec4 getCorner(mat4 inv_view, vec2 offset)
{
    return inv_view * model * vec4(vec3(offset, 0.0), 0.0);
}

void main()
{
    mat3 rot = mat3(view);
    rot[0] = vec3(1.0, 0.0, 0.0);
    rot[2] = vec3(0.0, 0.0, 1.0);
    mat4 newView = mat4(vec4(0.0), vec4(0.0), vec4(0.0), vec4(0.0));
    newView[0] = vec4(rot[0], view[0][3]);
    newView[1] = vec4(rot[1], view[1][3]);
    newView[2] = vec4(rot[2], view[2][3]);
    newView[3] = view[3];

    mat4 inv_view = mat4(1.0);

    if (lockHorizontal == 0)
    {
        inv_view = inverse(view);
        newView = view;
    }

    gl_Position = projection * newView * ((vec4(g_midPoint[0], 1.0)) + getCorner(inv_view, vec2(-1.0, -1.0)));
    f_uv = vec2(0.0, 0.0);
    EmitVertex();

    gl_Position = projection * newView * ((vec4(g_midPoint[0], 1.0)) + getCorner(inv_view, vec2(1.0, -1.0)));
    f_uv = vec2(1.0, 0.0);
    EmitVertex();

    gl_Position = projection * newView * ((vec4(g_midPoint[0], 1.0)) + getCorner(inv_view, vec2(1.0, 1.0)));
    f_uv = vec2(1.0, 1.0);
    EmitVertex();

    EndPrimitive();

    gl_Position = projection * newView * ((vec4(g_midPoint[0], 1.0)) + getCorner(inv_view, vec2(1.0, 1.0)));
    f_uv = vec2(1.0, 1.0);
    EmitVertex();

    gl_Position = projection * newView * ((vec4(g_midPoint[0], 1.0)) + getCorner(inv_view, vec2(-1.0, 1.0)));
    f_uv = vec2(0.0, 1.0);
    EmitVertex();

    gl_Position = projection * newView * ((vec4(g_midPoint[0], 1.0)) + getCorner(inv_view, vec2(-1.0, -1.0)));
    f_uv = vec2(0.0, 0.0);
    EmitVertex();

    EndPrimitive();
}