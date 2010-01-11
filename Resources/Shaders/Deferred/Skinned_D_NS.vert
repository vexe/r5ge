attribute vec3 R5_tangent;

varying vec2 _texCoord;
varying vec3 _normal;
varying vec3 _tangent;

void main()
{
    vec4 vertex  = gl_Vertex;
	vec3 normal  = gl_Normal;
	vec3 tangent = R5_tangent;

    // R5_IMPLEMENT_SKINNING vertex normal tangent
    // R5_IMPLEMENT_INSTANCING vertex normal tangent

    _texCoord   = (gl_TextureMatrix[0] * gl_MultiTexCoord0).xy;
    _normal     = gl_NormalMatrix * normal;
    _tangent    = gl_NormalMatrix * tangent;
}