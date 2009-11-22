uniform sampler2D   R5_texture0;    // Diffuse (RGB) + Glow (A)
uniform sampler2D   R5_texture1;    // Normal (RGB) + Specular (A)

varying vec2 _texCoord;
varying vec3 _normal;
varying vec3 _tangent;

void main()
{
    // Diffuse map
    vec4 diffuseMap = texture2D(R5_texture0, _texCoord);
    vec4 normalMap 	= texture2D(R5_texture1, _texCoord);

    // Normal map
    vec3 tangent = normalize(_tangent);
    vec3 normal  = normalize(_normal);
    mat3 TBN     = mat3(tangent, cross(normal, tangent), normal);
    normal       = TBN * normalize(normalMap.rgb * 2.0 - 1.0);

    // Encode the values
    gl_FragData[0] = vec4(gl_FrontMaterial.diffuse.rgb * diffuseMap.rgb, gl_FrontMaterial.diffuse.a);
    gl_FragData[1] = vec4(gl_FrontMaterial.specular.rgb * normalMap.a, gl_FrontMaterial.emission.a + diffuseMap.a);
    gl_FragData[2] = vec4(normalize(normal) * 0.5 + 0.5, gl_FrontMaterial.specular.a);
}