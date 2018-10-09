#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#include "ShaderProgram.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <cmath>
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

void Setup(){
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
    #ifdef _WINDOWS
    glewInit();
    #endif
    
    glViewport(0, 0, 640, 360);
}

void Render(const float &rightPaddleY, const float &leftPaddleY, const float &ballXpos, const float &ballYpos){
    #define DEFAULT_VERTICES {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5,-0.5, -0.5, 0.5, 0.5, -0.5, 0.5}

    ShaderProgram program;
    program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    glUseProgram(program.programID);
    
    //draw the ball in the center
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    
    projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(ballXpos, ballYpos, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f, 0.1f, 1.0f));
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    program.SetModelMatrix(modelMatrix);

    float vertices[] = DEFAULT_VERTICES;

    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    //draw the paddles
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(1.6f, rightPaddleY, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.08f, 0.4f, 1.0f));
    program.SetModelMatrix(modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-1.6f, leftPaddleY, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.08f, 0.4f, 1.0f));
    program.SetModelMatrix(modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    //draw top and bottom bars
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(5.0f, 0.1f, 1.0f));
    program.SetModelMatrix(modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -1.0f, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(5.0f, 0.1f, 1.0f));
    program.SetModelMatrix(modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    
    glDisableVertexAttribArray(program.positionAttribute);
    SDL_GL_SwapWindow(displayWindow);
}

int main(int argc, char *argv[])
{
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    Setup();
    
    SDL_Event event;
    bool done = false;
    float lastFrameTicks = 0.0f;
    float rightYPos = 0.0f;
    float leftYpos = 0.0f;
    
    const float MAX_Y = 0.75f;
    const float MIN_Y = -0.75f;
    
    float xVelocity = 1.0f;
    float yVelocity = 0.0f;
    float xPos = 0.0f;
    float yPos = 0.0f;
    

    while (!done) {
        
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        //moves right paddle
        if (keys[SDL_SCANCODE_UP] && rightYPos < MAX_Y) {
            rightYPos += elapsed * 1.0f;
        }
        else if (keys[SDL_SCANCODE_DOWN] && rightYPos > MIN_Y){
            rightYPos -= elapsed * 1.0f;
        }
        
        //moves left paddle
        if(keys[SDL_SCANCODE_W] && leftYpos < MAX_Y) {
            leftYpos += elapsed * 1.0f;
        }
        else if (keys[SDL_SCANCODE_S] && leftYpos > MIN_Y){
            leftYpos -= elapsed * 1.0f;
        }
        //determines if paddle came in contact with the ball
        if (xPos > 1.5f && xPos < 1.55f && (abs(yPos - rightYPos) - 0.25f) < 0){
            xVelocity = -xVelocity;
            
            if(yVelocity == 0.0f){
                yVelocity = 0.5f;
            }
        }
        else if (xPos < -1.5f && xPos > -1.55f && (abs(yPos - leftYpos) - 0.25f) < 0){
            xVelocity = -xVelocity;
        }
        
        //ball left the screen
        else if (xPos > 2.0f || xPos < -2.0f){
            xPos = 0.0f;
            yPos = 0.0f;
            rightYPos = 0.0f;
            leftYpos = 0.0f;
            yVelocity = 0.0f;
        }
    
        //ball hits top or bot bumper
        if(yPos > 0.9f || yPos < -0.9f){
            yVelocity = -yVelocity;
        }
        
        xPos += elapsed * xVelocity;
        yPos += elapsed * yVelocity;
        
        Render(rightYPos, leftYpos, xPos, yPos);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    
    SDL_Quit();
    return 0;
}
