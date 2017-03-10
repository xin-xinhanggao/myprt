#include "prt_shader.h"
#include "texture.h"



Prt::Prt()
{
	modelshader.load("/Users/apple/Desktop/myprt/shader/model.vs", "/Users/apple/Desktop/myprt/shader/model.frag");
	boxshader.load("/Users/apple/Desktop/myprt/shader/skybox.vs", "/Users/apple/Desktop/myprt/shader/skybox.frag");
	ourModel.loadModel("/Users/apple/Desktop/myprt/obj/nanosuit/nanosuit.obj");
}


void Prt::prepare()
{
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
}

void Prt::render(Camera camera)
{
	// Draw scene as normal
    modelshader.Use();
    glm::mat4 model;
    model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); // Translate it down a bit so it's at the center of the scene
    model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// It's a bit too big for our scene, so scale it down
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth/(float)screenHeight, 0.1f, 100.0f);
    glUniformMatrix4fv(glGetUniformLocation(modelshader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(modelshader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(modelshader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    ourModel.Draw(modelshader);
    
    // Draw skybox as last
    glDepthFunc(GL_LEQUAL);  // Change depth function so depth test passes when values are equal to depth buffer's content
    boxshader.Use();
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
    glDepthFunc(GL_LESS); // Set depth function back to default
}
