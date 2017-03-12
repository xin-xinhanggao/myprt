#ifndef PRT_ALGORITHM_H
#define PRT_ALGORITHM_H

#include "Shader.h"
#include "Model.h"
#include "Camera.h"
#include "texture.h"
#include "lamptexture.h"
#include "bvh_tree.h"

#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class Prt{
private:
    int lmax = 5;
    const static int samps = 100;

    std::vector<float> phi, theta;
    std::vector<float> p_coeff;
    std::vector<float> k_coeff;
    std::vector<float> y_coeff[samps];
    std::vector<float> cos_coeff;
    std::vector<float> sin_coeff;
    std::vector<glm::vec3> light_coeff, transfer_coeff;

	GLuint screenWidth = 800, screenHeight = 600;
	Shader modelshader;
	Shader boxshader;
	Model ourModel;
    BvhTree bvhTree;
    LampTexture* lamptexture;

    GLfloat envmap[12] = {
        -1.f, -1.f, 0.f,
        -1.f, 1.f, 0.f,
        1.f, -1.f, 0.f,
        1.f, 1.f, 0.f
    };

    GLuint indices[6] = { 
        0, 2, 1, 
        1, 2, 3  
    };

	GLfloat skyboxVertices[108] = {
        // Positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
  
        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
  
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
   
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
  
        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,
  
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    

	GLuint skyboxVAO, cubemapTexture, skyboxVBO;
    GLuint vertexArrayID, vertexBufferPositionID, vertexBufferIndicesID;
	std::vector<const GLchar*> faces;

    int p(int l,int m) 
    {
        return l * l + l + m;
    }

    void generate_sample_angles() 
    {
        int sqrtSamps = (int)sqrtf(1.f * samps);
        for (int i = 0; i < sqrtSamps; ++i)
            for (int j = 0; j < sqrtSamps; ++j) 
            {
                float x = (1.f * rand() / RAND_MAX + 1.f * i) / float(sqrtSamps);
                float y = (1.f * rand() / RAND_MAX + 1.f * j) / float(sqrtSamps);
                theta.push_back(2.f * acos(sqrt(1.f - x)));
                phi.push_back(y * 2.f * M_PI);
            }
    }

    void calc_sh_coeff(float theta_, float phi_, std::vector<float>& y_coeff) 
    {
        for (int j = 0; j < (lmax + 1) * (lmax + 1); ++j) {
            y_coeff.push_back(0.f);
        }
        float x = cos(theta_);

        // step 1
        p_coeff[p(0, 0)] = 1.f;
        p_coeff[p(1, 0)] = x;
        for (int l = 2; l <= lmax; ++l)
            p_coeff[p(l, 0)] =
                ((2.f * l - 1) * x * p_coeff[p(l - 1, 0)] -
                (1.f * l - 1) * p_coeff[p(l - 2, 0)]) / float(l);
        // step 2
        float neg = -1.f;
        float dfact = 1.f;
        float xroot = sqrtf(std::max(0.f, 1.f - x * x));
        float xpow = xroot;
        for (int l = 1; l <= lmax; ++l) {
            p_coeff[p(l, l)] = neg * dfact * xpow;
            neg *= -1.f;
            dfact *= 2.f * l + 1.f;
            xpow *= xroot;
        }
        // step 3
        for (int l = 2; l <= lmax; ++l)
            p_coeff[p(l, l - 1)] =
                x * (2.f * l - 1.f) * p_coeff[p(l - 1, l - 1)];
        // step 4
        for (int l = 3; l <= lmax; ++l)
            for (int m = 1; m <= l - 2; ++m)
                p_coeff[p(l, m)] =
                    ((2.f * (l - 1.f) + 1.f) * x * p_coeff[p(l - 1, m)] -
                    (1.f * l - 1.f + m) * p_coeff[p(l - 2, m)]) / (l - m);
        // step 5
        for (int l = 0; l <= lmax; ++l)
            for (int m = -l; m <= l; ++m) {
                float k = (2.f * l + 1.f) / 4.f / M_PI;
                for (int i = l - abs(m) + 1; i <= l + abs(m); ++i)
                    k /= float(i);
                k = sqrtf(k);
                k_coeff[p(l, m)] = k;
            }
        // step 6
        sin_coeff[0] = 0;
        cos_coeff[0] = 1;
        sin_coeff[1] = sin(phi_);
        cos_coeff[1] = cos(phi_);
        for (int l = 2; l <= lmax; ++l) {
            sin_coeff[l] = sin_coeff[l - 1] * cos_coeff[1] + cos_coeff[l - 1] * sin_coeff[1];
            cos_coeff[l] = cos_coeff[l - 1] * cos_coeff[1] - sin_coeff[l - 1] * sin_coeff[1];
        }
        // step 7
        for (int l = 0; l <= lmax; ++l) {
            for (int m = -l; m < 0; ++m) {
                y_coeff[p(l, m)] = sqrtf(2.f) * k_coeff[p(l, m)] * p_coeff[p(l, -m)] * sin_coeff[-m];
            }
            y_coeff[p(l, 0)] = k_coeff[p(l, 0)] * p_coeff[p(l, 0)];
            for (int m = 1; m <= l; ++m) {
                y_coeff[p(l, m)] = sqrtf(2.f) * k_coeff[p(l, m)] * p_coeff[p(l, m)] * cos_coeff[m];
            }
        }
    }

    void generate_sh()
    {
        for (int i = 0; i < (lmax + 1) * (lmax + 1); ++i) {
            p_coeff.push_back(0.f);
            k_coeff.push_back(0.f);
            cos_coeff.push_back(0.f);
            sin_coeff.push_back(0.f);
            light_coeff.push_back(glm::vec3());
            transfer_coeff.push_back(glm::vec3());
        }

        for (int i = 0; i < samps; ++i) {
            calc_sh_coeff(theta[i],phi[i],y_coeff[i]);
        }
    }

    glm::vec3 calc_diffuse_color(Vertex vertex)
    {
        int lmaxlmax = (lmax + 1) * (lmax + 1);
        for (int j = 0; j < lmaxlmax; ++j) {
            light_coeff[j] = glm::vec3(0, 0, 0);
            transfer_coeff[j] = glm::vec3(0, 0, 0);
        }

        for (int j = 0; j < samps; ++j) {
            glm::vec3 color_value = glm::vec3(5.0,5.0,5.0) * lamptexture->get_color(phi[j],theta[j]);
            // calc light
            for (int k = 0; k < lmaxlmax; ++k) {
                light_coeff[k] += color_value * y_coeff[j][k];
            }
            // calc transfer
            //glm::vec3 pos(model.positionData[i], model.positionData[i + 1], model.positionData[i + 2]);
            glm::vec3 dir(sin(theta[j]) * cos(phi[j]), sin(theta[j]) * sin(phi[j]), cos(theta[j]));
            //if (1)//!bvhTree.ray_intersect_with_mesh(BvhTree::Ray(pos, dir)))
            if(!bvhTree.ray_intersect_with_mesh(BvhTree::Ray(vertex.Position,dir)))
            {
                float cos_value = glm::dot(
                    vertex.Normal,
                    glm::vec3(sin(theta[j]) * cos(phi[j]), sin(theta[j]) * sin(phi[j]), cos(theta[j]))
                );
                
                for (int k = 0; k < lmaxlmax; ++k) {
                    float single = std::max(cos_value, 0.f) * y_coeff[j][k];
                    transfer_coeff[k] += glm::vec3(single, single, single);
                }
            }
        }
        glm::vec3 color(0, 0, 0);
        for (int j = 0; j < lmaxlmax; ++j) {
            light_coeff[j] *= (4.f * M_PI) / float(samps);
            transfer_coeff[j] *= (4.f * M_PI) / float(samps);
            color += light_coeff[j] * transfer_coeff[j] / float(M_PI);
        }

        return color;
    }

public:
	Prt()
    {
        generate_sample_angles();
        generate_sh();
        
        lamptexture = new LampTexture("/Users/apple/Desktop/myprt/texture/Lamp.jpg");
        modelshader.load("/Users/apple/Desktop/myprt/shader/model.vs", "/Users/apple/Desktop/myprt/shader/model.frag");
        boxshader.load("/Users/apple/Desktop/myprt/shader/envmap.vs", "/Users/apple/Desktop/myprt/shader/envmap.frag");
        //boxshader.load("/Users/apple/Desktop/myprt/shader/skybox.vs", "/Users/apple/Desktop/myprt/shader/skybox.frag");
        ourModel.loadModel("/Users/apple/Desktop/myprt/obj/nanosuit/nanosuit.obj");
        bvhTree.load(ourModel);
    }
    

	void prepare()
    {
        /*
        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);
        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        glBindVertexArray(0);
        
        faces.push_back("/Users/apple/Desktop/myprt/texture/skybox/right.jpg");
        faces.push_back("/Users/apple/Desktop/myprt/texture/skybox/left.jpg");
        faces.push_back("/Users/apple/Desktop/myprt/texture/skybox/top.jpg");
        faces.push_back("/Users/apple/Desktop/myprt/texture/skybox/bottom.jpg");
        faces.push_back("/Users/apple/Desktop/myprt/texture/skybox/back.jpg");
        faces.push_back("/Users/apple/Desktop/myprt/texture/skybox/front.jpg");
        cubemapTexture = loadCubemap(faces);
        */
        glGenTextures(1, &cubemapTexture);
        glBindTexture(GL_TEXTURE_2D, cubemapTexture);
        int width, height;
        unsigned char* image = SOIL_load_image("/Users/apple/Desktop/myprt/texture/Lamp.jpg", &width, &height, 0, SOIL_LOAD_RGB);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        glGenVertexArrays(1, &vertexArrayID);
        glBindVertexArray(vertexArrayID);

        glGenBuffers(1, &vertexBufferPositionID);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferPositionID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(envmap), envmap, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &vertexBufferIndicesID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBufferIndicesID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        glBindVertexArray(0);

        for(int i = 0; i < ourModel.meshes.size(); i++)
        {
            Mesh& mesh = ourModel.meshes[i];
            for(int j = 0; j < mesh.vertices.size(); j++)
            {
                mesh.vertices[j].Prtcolor = calc_diffuse_color(mesh.vertices[j]);
            }
            mesh.setup();
        }
    }

	void render(Camera camera)
    {
        // Draw scene as normal
        modelshader.Use();
        glm::mat4 model;
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        //model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); // Translate it down a bit so it's at the center of the scene
        //model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// It's a bit too big for our scene, so scale it down
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth/(float)screenHeight, 0.1f, 100.0f);
        glUniformMatrix4fv(glGetUniformLocation(modelshader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(modelshader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(modelshader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        ourModel.Draw(modelshader);
        
        
        // Draw skybox as last
        glDepthFunc(GL_LEQUAL);  // Change depth function so depth test passes when values are equal to depth buffer's content
        boxshader.Use();
        /*
        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));	// Remove any translation component of the view matrix
        glUniformMatrix4fv(glGetUniformLocation(boxshader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(boxshader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(boxshader.Program, "skybox"), 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        */
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cubemapTexture);
        glBindVertexArray(vertexArrayID);
        GLuint panoramaID = glGetUniformLocation(boxshader.Program, "panorama");
        glUniform1i(panoramaID, 0);

        //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBufferIndicesID);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // Set depth function back to default
    }

};


#endif
