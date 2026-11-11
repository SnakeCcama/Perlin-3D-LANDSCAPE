#version 420 core


out vec4 fragColor;
in float Height;


void main()
{   
    float h = (Height + 16)/32.0f;

    //height thresholds

    float tDeepShallow = 0.32; 
    float tWaterEarth = 0.33;
    float tEarthGrass = 0.40;
    float tGrassVeg = 0.65;
    float tVegForest = 0.88; 
    float tForSnow = 0.98;

    //transition softness
    float blendWidth = 0.09;

    //colors
    vec3 cDeepWater = vec3(0.271, 0.404, 0.537);
    vec3 cShallowWater = vec3(0.514, 0.773, 0.745);
    vec3 cEarthSoil = vec3(0.867, 0.745, 0.663);
    vec3 cHighForest = vec3(0.639, 0.694, 0.541);
    vec3 cVegetation = vec3(0.345, 0.506, 0.341);
    vec3 cGrass = vec3(0.227, 0.353, 0.251);
    vec3 cSnow = vec3(1.000, 0.945, 0.902);

    //Bring it all together
    vec3 finalColor = cDeepWater;

    //Water -> shallow water
    float blend1 = (h - tDeepShallow)/blendWidth;
    blend1 = clamp(blend1, 0.0, 1.0); 
    finalColor = mix(finalColor, cShallowWater, blend1);


    //Shallow -> earth
    float blend2 = (h - tWaterEarth)/blendWidth;
    blend2 = clamp(blend2, 0.0, 1.0);
    finalColor = mix(finalColor, cEarthSoil, blend2);


    //earth -> grass 
    float blend4 = (h - tEarthGrass)/blendWidth;
    blend4 = clamp(blend4, 0.0, 1.0);
    finalColor = mix(finalColor, cGrass, blend4);


    //grass -> Veg 
    float blend5 = (h - tGrassVeg)/blendWidth;
    blend5 = clamp(blend5, 0.0, 1.0);
    finalColor = mix(finalColor, cVegetation, blend5);


    //veg -> forest 
    float blend6 = (h - tVegForest)/blendWidth;
    blend6 = clamp(blend6, 0.0, 1.0);
    finalColor = mix(finalColor, cHighForest, blend6); 

    //forest -> Snow
    float blend3 = (h - tForSnow)/blendWidth;
    blend3 = clamp(blend3, 0.0, 1.0);
    finalColor = mix(finalColor, cSnow, blend3);  



    vec3 rColor = vec3(h);
    fragColor = vec4(finalColor, 1.0);
}