#version 460 core

layout (vertices = 28) out;

uniform mat4 proj;
uniform int cpCount;
uniform ivec2 resolution;
uniform int division;

float CalcTessLevel();

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    gl_TessLevelOuter[0] = division;
    gl_TessLevelOuter[1] = CalcTessLevel();
}


float CalcTessLevel()
{
	vec4 p0 = proj * gl_in[0].gl_Position;
	p0 /= p0.w;

	float x_min = p0.x;
	float x_max = p0.x;
	float y_min = p0.y;
	float y_max = p0.y;

	for (int i = 1; i < cpCount; i++)
	{
		vec4 clipped = proj * gl_in[i].gl_Position;
		clipped /= clipped.w;

		if (!isnan(clipped.x) && !isinf(clipped.x))
		{
			if (x_min > clipped.x)
				x_min = clipped.x;
			if (x_max < clipped.x)
				x_max = clipped.x;
		}
		if (!isnan(clipped.y) && !isinf(clipped.y))
		{
			if (y_min > clipped.y)
				y_min = clipped.y;
			if (y_max < clipped.y)
				y_max = clipped.y;
		}
	}

	float clipbox_width = x_max - x_min;
	float clipbox_height = y_max - y_min;

	float width_scale = clipbox_width / 2.0;
	float height_scale = clipbox_height / 2.0;
	
	return max(width_scale * resolution.x, height_scale * resolution.y);
}