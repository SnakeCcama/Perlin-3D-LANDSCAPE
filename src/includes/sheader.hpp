#pragma once
#include "gl.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class Shader
{
    public:

    //Shader program ID
    unsigned int ID;

    //string objects to recieve shader source code
    std::string vsCode, fsCode, tcCode, teCode;

    /*Constructor*/
    Shader (const char* vsPath, const char* fsPath, const char* tcPath, const char* tePath)
    {
        
        //ifstream objects to read from source file
        std::ifstream vsFile, fsFile, tcFile, teFile;

        //stringstream objects to recieve the source code
        std::stringstream vsStream, fsStream, tcStream, teStream;
      

        //set exceptions
        vsFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fsFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        tcFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        teFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try
        {
            
            //open source code
            vsFile.open(vsPath);
            fsFile.open(fsPath);
            tcFile.open(tcPath);
            teFile.open(tePath);


            //write to stringstream
            vsStream << vsFile.rdbuf();
            fsStream << fsFile.rdbuf();
            tcStream << tcFile.rdbuf();
            teStream << teFile.rdbuf();

            vsFile.close();
            fsFile.close();
            tcFile.close();
            teFile.close();

            //assign to string objects
            vsCode = vsStream.str();
            fsCode = fsStream.str();
            tcCode = tcStream.str();
            teCode = teStream.str();


        } catch (...)

        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        }

        //add source code as c-style string
        const char* vShCode = vsCode.c_str();
        const char* fShCode = fsCode.c_str();
        const char* tcShCode = tcCode.c_str();
        const char* teShCode = teCode.c_str();


        //shaders
        unsigned int vert, frag, tessC, tessE;
        int success;
        char infoLog[512];

        /*Vertex*/
        vert = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vert, 1, &vShCode, NULL);
        glCompileShader(vert);
        checkCompileErrors(vert, "VERTEX");

        /*Fragment*/
        frag = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(frag, 1, &fShCode, NULL);
        glCompileShader(frag);
        checkCompileErrors(frag, "FRAGMENT");

        /*Tessellation Control*/
        tessC = glCreateShader(GL_TESS_CONTROL_SHADER);
        glShaderSource(tessC, 1, &tcShCode, NULL);
        glCompileShader(tessC);
        checkCompileErrors(frag, "TESS_CONTROL");

        /*Tessellation Evaluation*/
        tessE = glCreateShader(GL_TESS_EVALUATION_SHADER);
        glShaderSource(tessE, 1, & teShCode, NULL);
        glCompileShader(tessE);
        checkCompileErrors(tessE, "TESS_CONTROL");

        
        /*Shader Program*/
        ID= glCreateProgram();
        glAttachShader(ID, vert);
        glAttachShader(ID, frag);
        glAttachShader(ID, tessC);
        glAttachShader(ID, tessE);
        
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        glDeleteShader(vert);
        glDeleteShader(frag);

    }

    void Use()
    {
        glUseProgram(ID);
    }

    /*uniform setter functions (depends on value type you want to set)
    these member functions are const-qualified (barred from modifying variables)*/


    void setBool(const std::string &name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }


    void setInt(const std::string &name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value );
    }


    void setFloat(const std::string &name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }


    void setVec3(const std::string &name, const glm::vec3 &vec) const
    {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &vec[0]);
    }


    void setMat4(const std::string &name, const glm::mat4 &mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }


    void setthisMat4(const std::string &name, const glm::mat4 &mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    }


    void cleanUp()
    {
        glDeleteProgram(ID);
    }


private:

    void checkCompileErrors(GLuint shader, std::string type)
    {
        GLint success;
        GLchar infoLog[1024];
        if(type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if(!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout <<"ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n-- --------------------------------------------------- -- " << std::endl;
                
            }

        }

        else 
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if(!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;               
            }

        }  

    }
    

};

