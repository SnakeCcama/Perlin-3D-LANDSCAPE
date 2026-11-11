#include <KHR/khrplatform.h>
#include <cstdlib>
#include <glm/detail/qualifier.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <iostream>
#include <random>
#include <algorithm>

#include "includes/gl.h"
#include "includes/gl.c"
#include "includes/sheader.hpp"
#include "includes/cam.hpp"

#define STB_IMAGE_IMPLEMENTATION

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp> 
#include <vector>
#include <cmath>

int width = 600, height = 500;
float deltaTime, lastFrame;
int size = 256; 
int useWireframe = 0;
int displayGrayscale = 0;

Camera camera(glm::vec3(0.0f, 350.0f, 0.0f),
              glm::vec3(0.0f, 1.0f, 0.0f),
              -90.0f, -89.0f);


float lastX = width / 2.0f;
float lastY = height / 2.0f;
bool firstMouse = true;



std::vector <float> vertices = {

    1.0f,  1.0f, 0.0f,      1.0f, 1.0f, // top right
    1.0f, -1.0f, 0.0f,      1.0f, 0.0f,// bottom right
    -1.0f, -1.0f, 0.0f,     0.0f, 0.0f,// bottom left
    -1.0f,  1.0f, 0.0f,     0.0f, 1.0f // top left 

};

std::vector <int> indices = 
{
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle   
};


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window, int flyCam)
{
    
    
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
            
    
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int modifiers)
{
    if(action == GLFW_PRESS)
    {
        switch(key)
        {
            case GLFW_KEY_SPACE:
                useWireframe = 1 - useWireframe;
                break;
            case GLFW_KEY_G:
                displayGrayscale = 1 - displayGrayscale;
                break;
            default:
                break;
        }
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}



void noiseGen(float (*canvas)[256], int size) 
{

    
    std::random_device rd;

    std::mt19937 gen(rd());

    std::uniform_real_distribution<float> distrib(0.0f, 1.0f);

    for (int i= 0; i < size; i++)
    {
        for(int j= 0; j < size; j++)
        {
            canvas[i][j] = distrib(gen);
        }
    }

    
}


int rows = 256;
int cols = 256;

//setup

float normalize(float value, float min, float max)
{
    return 2.0f * (value - min) / (max - min) - 1.0f;
}


float smoothingfunc(float t)
{
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

float lerp(float a, float b, float t)
{
    return a + t * (b-a);
}

#define B 0x100

float perlinNoise(float x, float y, float z = 0.0f)
{
    
    
    //setup normalized random gradients for each point 


    static glm::vec3 gradients[B + B + 2];
    static int permutation[B + B + 2];
    static bool initialized = false;

    if (!initialized)
    {
    
        for (int i = 0; i< B; i++)
        {
            permutation[i] = i;
            gradients[i] = glm::sphericalRand(1.0f);
        }

        for (int i = 0, j = 0; i < B ; i ++)
        { 
            int k = permutation[i];
            permutation[i] = permutation[j = random() % B];
            permutation[j] = k;
        }


        for (int i = 0; i < B + 2; i ++)
        {
            permutation[B + i] = permutation[i];
            gradients[B+ i] = gradients[i];
        }

        initialized = true; 
    }


    //perlin maths:: 
    
    //find grid cell coordinates surrounding point p
    
    int X0 = (int)std::floor(x) & 255;
    int Y0 = (int)std::floor(y) & 255;
    int Z0 = (int)std::floor(z) & 255; 
    
    int X1 = X0 + 1;
    int Y1 = Y0 + 1;
    int Z1 = Z0 + 1;

    //relative position of p inside unit cell
    float xp = x - std::floor(x);
    float yp = y - std::floor(y);
    float zp = z - std::floor(z);
    
    //vector from each corner to sample point
    
    //front
    glm::vec3 d00 = glm::vec3(xp, yp,zp);           //Bottom-Left
    glm::vec3 d10 = glm::vec3(xp - 1.0f, yp, zp);           // Bottom-Right
    glm::vec3 d01 = glm::vec3(xp, yp - 1.0f, zp);           // Top-Left
    glm::vec3 d11 = glm::vec3(xp - 1.0f, yp - 1.0f, zp);    // Top-Right 
    
    //back
    glm::vec3 e00 = glm::vec3(xp, yp, zp -1.0f); //bottom left
    glm::vec3 e10 = glm::vec3(xp - 1.0f, yp, zp - 1.0f); //bottom right
    glm::vec3 e01 = glm::vec3(xp, yp - 1.0f, zp -1.0f); //top left
    glm::vec3 e11 = glm::vec3(xp - 1.0f, yp - 1.0f, zp - 1.0f); //top-right

    //hash each corner

    int p00 = permutation[permutation[X0]  + Y0];
    int p10 = permutation[permutation[X1] + Y0 ];
    int p01 = permutation[permutation[X0]  + Y1];
    int p11 = permutation[permutation[X1] + Y1 ];

    //frontface
    int gi00 = permutation[p00 + Z0];
    int gi10 = permutation[p10 + Z0];
    int gi01 = permutation[p01 + Z0];
    int gi11 = permutation[p11 + Z0];

    //backface
    int hi00 = permutation[p00 + Z1];
    int hi10 = permutation[p10 + Z1];
    int hi01 = permutation[p01 + Z1];
    int hi11 = permutation[p11 + Z1];


    //fetch gradient from precomputed table
    
    //frontface
    glm::vec3 g00 = gradients[gi00];
    glm::vec3 g10 = gradients[gi10];
    glm::vec3 g01 = gradients[gi01];
    glm::vec3 g11 = gradients[gi11];
    
    //backface
    glm::vec3 h00 = gradients[hi00];
    glm::vec3 h10 = gradients[hi10];
    glm::vec3 h01 = gradients[hi01];
    glm::vec3 h11 = gradients[hi11];

    //dot product of gradient (randomly assigned) and vector at/from each corner

    //frontface
    float s = glm::dot(g00, d00);
    float t = glm::dot(g10, d10);
    float u = glm::dot(g01, d01);
    float v = glm::dot(g11, d11);
    
    //backsface
    float w0 = glm::dot(h00, e00);
    float x0 = glm::dot(h10, e10);
    float y0 = glm::dot(h01, e01);
    float z0 = glm::dot(h11, e11);



    //smoothing func for point p
    float fx = smoothingfunc(xp);
    float fy = smoothingfunc(yp);
    float fz = smoothingfunc(zp);

    //frontface
    float nx0 = lerp(s, t, fx); // Blend bottom edge (s to t)
    float nx1 = lerp(u, v, fx); // Blend top edge (u to v)
    
    //backface
    float nx2 = lerp(w0, x0, fx);
    float nx3 = lerp(y0, z0, fx);
    
    //lerp between them
    float nx4 = lerp(nx0, nx1, fy);
    float nx5 = lerp(nx2, nx3, fy);

    return lerp(nx4, nx5, fz); 

}

int makeTexture (float(*canvas)[256],int size)
{
    GLuint texID;
    glGenTextures (1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    //set wrapping params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //set texture filtering params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    void* heightmap_data = canvas;
    
    
    glTexImage2D(
     GL_TEXTURE_2D,
     0,
     GL_R32F,
     size,
     size, 
     0,
     GL_RED,
     GL_FLOAT,
     heightmap_data
    );

    glGenerateMipmap(GL_TEXTURE_2D);

    return texID;
}

std::vector<float> makeMesh (float (*canvas)[256], int size)
{
 

    //vector generation
    std::vector<float> meshVertices;
    float yScale = 256.0f/256.0f, yShift = 16.0f;
    
    for(unsigned int i = 0; i < size; i++ )
    {
        for(unsigned int j =0; j < size; j++)
        {
            meshVertices.push_back((-size/2.0f + i)/(size/2.0f));
            meshVertices.push_back(canvas[i][j]);
            meshVertices.push_back((-size/2.0f + j)/(size/2.0f));

        }
    }

    return meshVertices;

}

std::vector<unsigned int> genIndices(int size)
{
    
    std::vector<unsigned int> meshIndices;
    
    for(unsigned int i = 0; i< size -1; i++)
    {
        for(unsigned int j = 0; j <  size; j ++)

        {
            for(unsigned int k = 0; k < 2; k++)
            {
                meshIndices.push_back(j + size * (i + k));
            }
        }
    }

    return meshIndices;

}


std::vector<float> testVectors( int rez, int size)
{
    
    std::vector<float> meshVertices;
    for(unsigned i = 0; i <= rez-1; i++)
    {
	    for(unsigned j = 0; j <= rez-1; j++)
	    {
	    	meshVertices.push_back(-size/2.0f + size*i/(float)rez); // v.x
	    	meshVertices.push_back(0.0f); // v.y
	    	meshVertices.push_back(-size/2.0f + size*j/(float)rez); // v.z
	    	meshVertices.push_back(i / (float)rez); // u
	    	meshVertices.push_back(j / (float)rez); // v

	    	meshVertices.push_back(-size/2.0f + size*(i+1)/(float)rez); // v.x
	    	meshVertices.push_back(0.0f); // v.y
	    	meshVertices.push_back(-size/2.0f + size*j/(float)rez); // v.z
	    	meshVertices.push_back((i+1) / (float)rez); // u
	    	meshVertices.push_back(j / (float)rez); // v

	    	meshVertices.push_back(-size/2.0f + size*i/(float)rez); // v.x
	    	meshVertices.push_back(0.0f); // v.y
	    	meshVertices.push_back(-size/2.0f + size*(j+1)/(float)rez); // v.z
	    	meshVertices.push_back(i / (float)rez); // u
	    	meshVertices.push_back((j+1) / (float)rez); // v

	    	meshVertices.push_back(-size/2.0f + size*(i+1)/(float)rez); // v.x
	    	meshVertices.push_back(0.0f); // v.y
	    	meshVertices.push_back(-size/2.0f + size*(j+1)/(float)rez); // v.z
	    	meshVertices.push_back((i+1) / (float)rez); // u
	    	meshVertices.push_back((j+1) / (float)rez); // v
	    }
    }

    return meshVertices;
}

void doOctaves(int octaves, float(*canvas)[256])
{

    float baseFrequency = 0.02f;
    float persistence = 0.5f;

    float offsetX = 256.0f;
    float offsetY = 256.0f;
    float offsetZ = 0.0f;

    

    

    for (int y =0 ; y < 256; ++y)
    {
        for (int x = 0; x < 256; ++x)
        {
            
            float total = 0.0f;
            float maxValue = 0.0f;
            float frequency = baseFrequency;
            float amplitude = 1.0f;

            for (int o = 0; o < octaves; ++o)
            {
                float sampleX = (x * frequency) + offsetX;
                float sampleY = (y * frequency) + offsetY;
                float sampleZ = offsetZ;
                
                float rawNoise = perlinNoise(sampleX, sampleY, sampleZ);
                float n = std::clamp((rawNoise + 1.0f) * 0.5f, 0.0f, 1.0f);
                
                total += n * amplitude;
                maxValue += amplitude;
                
                amplitude *= persistence;
                frequency *= 2.0f;
            }
            
            /*ternary expression to ensure we only divide by maxValue if it's greater than 0*/

            canvas[y][x] = maxValue > 0.0f ? total / maxValue : 0.0f; 

        }
    }
    

}


int main ()
{  

    std::cout << "Lights on" << std::endl;

    #if defined(__linux__)
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    #endif

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, "PerlinNoise", NULL, NULL);
    
    if(window == NULL)
    {
        std::cout << "Window creation failed" << std::endl;
        glfwTerminate();
        return -1; 
    }

    glfwMakeContextCurrent(window);
    
    if(!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
    {
        std::cout <<"Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, width, height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);



    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);


    
    float canvas[256][256];
    int rez = 20;
    float speed = 1.0f;



    /*
    for (int y =0 ; y < 256; ++y)
    {
        for (int x = 0; x < 256; ++x)
        {
            float sampleX = (x * frequency) + offsetX;
            float sampleY = (y * frequency) + offsetY;
            float sampleZ = offsetZ;



            float rawNoise = perlinNoise(sampleX, sampleY, sampleZ);
            canvas[y][x] = std::clamp((rawNoise + 1.0f) * 0.5f, 0.0f, 1.0f);

        }
    }*/

    doOctaves(3, canvas);
    
    
    std::vector <float> meshVertices = testVectors(20, 256);
    std::cout << "meshVertices.size() = " << meshVertices.size() << "\n";
    std::cout << "expected vertices = " << 4 * rez * rez << "\n";
    std::cout << "Loaded " << rez*rez << " patches of 4 control points each" << std::endl;




    //std::vector <float> meshVertices = makeMesh(canvas, 256);
    //std::vector <unsigned int> meshIndices = genIndices(256);
    
    const unsigned int NUM_STRIPS = size - 1;
    const unsigned int NUM_VERTICES_PER_STRIP = size * 2; 

    //virtual array object
    GLuint vaoId;
    glGenVertexArrays(1, &vaoId);
    glBindVertexArray(vaoId);


    //virtual buffer object
    GLuint vboId;
    glGenBuffers(1, &vboId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBufferData(GL_ARRAY_BUFFER, meshVertices.size() * sizeof(float), &meshVertices[0], GL_STATIC_DRAW);


    /*
    //index buffer object
    GLuint iboId;
    glGenBuffers(1, &iboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshIndices.size() * sizeof(unsigned int), meshIndices.data(), GL_STATIC_DRAW);
    */

    //how to read data in VBO
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(0);    
    glEnableVertexAttribArray(1);

    glPatchParameteri(GL_PATCH_VERTICES, 4); // Set the number of control points per patch

    
    
    Shader myShader("/home/magsgl/Desktop/OpenGl/perlNoise/src/shaders/shader0.vert","/home/magsgl/Desktop/OpenGl/perlNoise/src/shaders/shader0.frag",
    "/home/magsgl/Desktop/OpenGl/perlNoise/src/shaders/shader0.tcs", "/home/magsgl/Desktop/OpenGl/perlNoise/src/shaders/shader0.tes");
    
    myShader.Use();
    
    unsigned int myHeightMap = makeTexture(canvas, size);
    myShader.setInt("heightMap", 0);
    lastFrame = glfwGetTime();

    int flyCam;

    

    while (!glfwWindowShouldClose(window))
    {
        
       
        
        processInput(window, flyCam);

        if (useWireframe)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, myHeightMap);

        
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
    
        myShader.Use();

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100000.0f);
        glm::mat4 view = camera.GetViewMatrix();

        myShader.setMat4("projection", projection);
        myShader.setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        myShader.setMat4("model", model);

        glBindVertexArray(vaoId);
        glDrawArrays(GL_PATCHES, 0, 4*rez*rez);

        /*

        for(unsigned int strip = 0; strip < NUM_STRIPS; ++ strip)
        {
            glDrawElements(GL_LINE_STRIP, NUM_VERTICES_PER_STRIP, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * NUM_VERTICES_PER_STRIP * strip));
        } */


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0; 
}