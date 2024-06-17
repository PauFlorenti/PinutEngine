
struct Light
{
    vec3 color;
    float intensity;
    vec3 position;
    int type;
    float radius;
};

struct PerFrameData
{
    mat4 view;
    mat4 projection;
    //vec3 cameraPosition;
    //float padding;

    //Light directionalLight;
    // Light lights[10];
};
