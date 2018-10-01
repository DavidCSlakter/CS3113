#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "ShaderProgram.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

GLuint LoadTexture(const char *filePath) {
    int w,h,comp;
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    if(image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(image);
    return retTexture;
}
int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, 1280, 720);
    ShaderProgram programTextured;
    programTextured.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    ShaderProgram programUntextured;
    programUntextured.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    
    GLuint alienTexture = LoadTexture(RESOURCE_FOLDER"alien.png");
    GLuint planeTexture = LoadTexture(RESOURCE_FOLDER"plane.png");
    GLuint alertTexture = LoadTexture(RESOURCE_FOLDER"alert-icon.png");
    
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    
    projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    
    
    glUseProgram(programUntextured.programID);
    glUseProgram(programTextured.programID);
    
    
    glClearColor(0.7f, 0.2f, 0.4f, 1.0f); //sets background color
 
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    float lastframeticks = 0.0f;
    float angle = 0.0f;
    
    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        glClear(GL_COLOR_BUFFER_BIT); //start every frame by clearing the screen
    
        float ticks = (float)SDL_GetTicks()/1000;
        float elapsed = ticks - lastframeticks;
        lastframeticks = ticks;
        
         modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.4f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f, 0.17f, 1.0f));
        programUntextured.SetModelMatrix(modelMatrix);
        programUntextured.SetProjectionMatrix(projectionMatrix);
        programUntextured.SetViewMatrix(viewMatrix);
        
        float vertices4[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5,-0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(programUntextured.positionAttribute, 2, GL_FLOAT, false, 0, vertices4);
        glEnableVertexAttribArray(programUntextured.positionAttribute);
        
        float texCoords4[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(programUntextured.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords4);
        glEnableVertexAttribArray(programUntextured.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        
        
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.5f, 0.0f)); //translates 0.5 units up
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.25f, 0.25f, 1.0f)); //scales by 1/4
        programTextured.SetModelMatrix(modelMatrix);
        programTextured.SetProjectionMatrix(projectionMatrix);
        programTextured.SetViewMatrix(viewMatrix);
        
        glBindTexture(GL_TEXTURE_2D, alienTexture);
        
        float vertices[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5,-0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(programTextured.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(programTextured.positionAttribute);
        
        float texCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(programTextured.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(programTextured.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.25f, 0.25f, 1.0f)); //
        programTextured.SetModelMatrix(modelMatrix);
        programTextured.SetProjectionMatrix(projectionMatrix);
        programTextured.SetViewMatrix(viewMatrix);
        
        
        glBindTexture(GL_TEXTURE_2D, planeTexture);
        
        float vertices2[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5,-0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(programTextured.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
        glEnableVertexAttribArray(programTextured.positionAttribute);
        
        float texCoords2[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(programTextured.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords2);
        glEnableVertexAttribArray(programTextured.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        angle += 3.14f*elapsed;
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.2f, 0.6f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f, 0.1f, 1.0f));
        modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0.0f, 0.0f, 1.0f));
        programTextured.SetModelMatrix(modelMatrix);
        programTextured.SetProjectionMatrix(projectionMatrix);
        programTextured.SetViewMatrix(viewMatrix);
        
        glBindTexture(GL_TEXTURE_2D, alertTexture);
        
        float vertices3[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5,-0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(programTextured.positionAttribute, 2, GL_FLOAT, false, 0, vertices3);
        glEnableVertexAttribArray(programTextured.positionAttribute);
            
        float texCoords3[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(programTextured.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords3);
        glEnableVertexAttribArray(programTextured.texCoordAttribute);
            
        glDrawArrays(GL_TRIANGLES, 0, 6);
            

        glDisableVertexAttribArray(programTextured.positionAttribute);
        glDisableVertexAttribArray(programTextured.texCoordAttribute);
        glDisableVertexAttribArray(programUntextured.texCoordAttribute);
        glDisableVertexAttribArray(programUntextured.positionAttribute);
        SDL_GL_SwapWindow(displayWindow);
    

    }
    SDL_Quit();
    return 0;
}





