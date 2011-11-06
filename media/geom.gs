#extension GL_EXT_geometry_shader4 : require

layout(triangles) in;
layout(triangle_strip, max_vertices = 24) out;

in vec3 viewpos_geom[];
in vec3 normal_geom[];

out vec3 viewpos;
out vec3 normal;

void main(void)
{
	// Emit primitives, slightly translated
	for(int i = 0; i < gl_VerticesIn; i++)
	{
		gl_Position = gl_PositionIn[i];
		gl_Position.x -= 1.0;
		viewpos = viewpos_geom[i];
		normal = normal_geom[i];
		EmitVertex();
	}
	EndPrimitive();

	// Generate additional copy, next to the original
	for(int i = 0; i < gl_VerticesIn; i++)
	{
		gl_Position = gl_PositionIn[i];
		gl_Position.x += 1.0;
		viewpos = viewpos_geom[i];
		normal = normal_geom[i];
		EmitVertex();
	}
	EndPrimitive();
}

