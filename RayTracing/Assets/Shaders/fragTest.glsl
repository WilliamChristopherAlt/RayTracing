#version 430 core

in vec3 fragPos;

uniform sampler2D textureSampler;

out vec4 FragColor;

void main()
{
    FragColor = texture(textureSampler, fragPos.xy / 2 + vec2(0.5f));
}
