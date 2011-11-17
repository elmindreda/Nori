
#extension GL_EXT_geometry_shader4 : require

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 vPosition[];
in vec3 vNormal[];

out vec3 gPosition;
out vec3 gNormal;
out vec3 gBarycentric;

// This geometry shader doesn't actually alter geometry,
// but is passes the barycentric coordinates to the
// fragment shader, which allows for cool wireframe stuff.

void main()
{
  gPosition = vPosition[0];
  gNormal = vNormal[0];
  gBarycentric = vec3(1, 0, 0);
  gl_Position = gl_in[0].gl_Position;
  EmitVertex();

  gPosition = vPosition[1];
  gNormal = vNormal[1];
  gBarycentric = vec3(0, 1, 0);
  gl_Position = gl_in[1].gl_Position;
  EmitVertex();

  gPosition = vPosition[2];
  gNormal = vNormal[2];
  gBarycentric = vec3(0, 0, 1);
  gl_Position = gl_in[2].gl_Position;
  EmitVertex();

  EndPrimitive();
}

