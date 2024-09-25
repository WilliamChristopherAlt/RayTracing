#version 430 core
out vec4 FragColor;

in vec3 fragPos;

uniform sampler2D tex;

uniform sampler2D texIndexTest;

void main()
{     
    vec2 uv = fragPos.xy * 0.5f + 0.5f;        
    vec3 texCol = texture(tex, uv).rgb;      
    FragColor = vec4(texCol, 1.0);
}