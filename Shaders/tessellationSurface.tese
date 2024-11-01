#version 460 core

layout (isolines) in;

uniform mat4 proj;
uniform int segmentCount;
uniform int segmentIdx;
uniform int division;
uniform bool otherAxis;
uniform bool bspline;
uniform bool gregory;

void main()
{
    float u, v;

    if (!otherAxis)
	{
		u = (gl_TessCoord.x * (1 + segmentIdx)) / segmentCount;
		v = gl_TessCoord.y * division / (division - 1.f);
	}
	else
	{
        u = gl_TessCoord.y * division / (division - 1.f);
        // BUG, FIX MAYBE
        // u = ((gl_TessCoord.y * division / (division - 1.f)) * (1 + segmentIdx)) / segmentCount;
		v = gl_TessCoord.x;
	}

    float minU = (1.0 - u);
    float minV = (1.0 - v);

    vec3 result;

    vec3 F0 = (u * gl_in[6].gl_Position.xyz + v * gl_in[5].gl_Position.xyz) / (u + v);
    vec3 F1 = (minU * gl_in[7].gl_Position.xyz + v * gl_in[8].gl_Position.xyz) / (minU + v);
    vec3 F2 = (minU * gl_in[13].gl_Position.xyz + minV * gl_in[14].gl_Position.xyz) / (minU + minV);
    vec3 F3 = (u * gl_in[12].gl_Position.xyz + minV * gl_in[11].gl_Position.xyz) / (u + minV);

    vec3 p0 = gregory   ? gl_in[0].gl_Position.xyz  : gl_in[0].gl_Position.xyz;
    vec3 p1 = gregory   ? gl_in[4].gl_Position.xyz  : gl_in[1].gl_Position.xyz;
    vec3 p2 = gregory   ? gl_in[10].gl_Position.xyz : gl_in[2].gl_Position.xyz;
    vec3 p3 = gregory   ? gl_in[16].gl_Position.xyz : gl_in[3].gl_Position.xyz;

    vec3 p4 = gregory   ? gl_in[1].gl_Position.xyz  : gl_in[4].gl_Position.xyz;
    vec3 p5 = gregory   ? F0                        : gl_in[5].gl_Position.xyz;
    vec3 p6 = gregory   ? F3                        : gl_in[6].gl_Position.xyz;
    vec3 p7 = gregory   ? gl_in[17].gl_Position.xyz : gl_in[7].gl_Position.xyz;

    vec3 p8 = gregory   ? gl_in[2].gl_Position.xyz  : gl_in[8].gl_Position.xyz;
    vec3 p9 = gregory   ? F1                        : gl_in[9].gl_Position.xyz;
    vec3 p10 = gregory  ? F2                        : gl_in[10].gl_Position.xyz;
    vec3 p11 = gregory  ? gl_in[18].gl_Position.xyz : gl_in[11].gl_Position.xyz;

    vec3 p12 = gregory  ? gl_in[3].gl_Position.xyz  : gl_in[12].gl_Position.xyz;
    vec3 p13 = gregory  ? gl_in[9].gl_Position.xyz  : gl_in[13].gl_Position.xyz;
    vec3 p14 = gregory  ? gl_in[15].gl_Position.xyz : gl_in[14].gl_Position.xyz;
    vec3 p15 = gregory  ? gl_in[19].gl_Position.xyz : gl_in[15].gl_Position.xyz;

    vec4 W_u, W_v;

    W_u[3] = u * u * u;
    W_u[2] = bspline ? minU * (u + 1) * (u + 1) + u * (2 - u) * (u + 1) + (3 - u) * u * u : 3.0 * minU * u * u;
    W_u[1] = bspline ? minU * minU * (u + 2) + minU * (u + 1) * (2 - u) + u * (2 - u) * (2 - u) : 3.0 * minU * minU * u;
    W_u[0] = minU * minU * minU;

    W_v[3] = v * v * v;
    W_v[2] = bspline ? minV * (v + 1) * (v + 1) + v * (2 - v) * (v + 1) + (3 - v) * v * v : 3.0 * minV * v * v;
    W_v[1] = bspline ? minV * minV * (v + 2) + minV * (v + 1) * (2 - v) + v * (2 - v) * (2 - v) : 3.0 * minV * minV * v;
    W_v[0] = minV * minV * minV;

    if (bspline)
    {
        W_u /= 6.0;
        W_v /= 6.0;
    }

    result = W_u[0] * W_v[0] * p0 +
             W_u[1] * W_v[0] * p1 + 
             W_u[2] * W_v[0] * p2 + 
             W_u[3] * W_v[0] * p3 +
			 W_u[0] * W_v[1] * p4 + 
             W_u[1] * W_v[1] * p5 + 
             W_u[2] * W_v[1] * p6 + 
             W_u[3] * W_v[1] * p7 +
			 W_u[0] * W_v[2] * p8 + 
             W_u[1] * W_v[2] * p9 + 
             W_u[2] * W_v[2] * p10 + 
             W_u[3] * W_v[2] * p11 +
			 W_u[0] * W_v[3] * p12 + 
             W_u[1] * W_v[3] * p13 + 
             W_u[2] * W_v[3] * p14 + 
             W_u[3] * W_v[3] * p15;

    gl_Position = proj * vec4(result, 1.0);
}