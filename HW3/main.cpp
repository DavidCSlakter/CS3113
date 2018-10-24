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
#include <cmath>
#include <vector>
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

using namespace std;

enum GameMode {TITLE_SCREEN, GAME_LEVEL, GAME_OVER, GAME_WON};


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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    stbi_image_free(image);
    return retTexture;
}

 void DrawText(ShaderProgram &program, int fontTexture, std::string text, float size, float spacing, float xPos, float yPos) {
    float character_size = 1.0/16.0f;
    vector<float> vertexData;
    vector<float> texCoordData;
    for(int i=0; i < text.size(); i++) {
        int spriteIndex = (int)text[i];
        float texture_x = (float)(spriteIndex % 16) / 16.0f;
        float texture_y = (float)(spriteIndex / 16) / 16.0f;
        vertexData.insert(vertexData.end(), {
            ((size+spacing) * i) + (-0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
        });
        texCoordData.insert(texCoordData.end(), {
            texture_x, texture_y,
            texture_x, texture_y + character_size,
            texture_x + character_size, texture_y,
            texture_x + character_size, texture_y + character_size,
            texture_x + character_size, texture_y,
            texture_x, texture_y + character_size,
        });
    }
    
    //draw the text on the screen
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    float* v = vertexData.data();
    float* t = texCoordData.data();
    
    for (int i = 1;  i < text.size() * 2; i++){
        glm::mat4 modelMatrix = glm::mat4(1.0f);
   
        modelMatrix = glm::translate(modelMatrix, glm::vec3(xPos, yPos, 0.0f));
    
        program.SetModelMatrix(modelMatrix);
    
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, v);
        glEnableVertexAttribArray(program.positionAttribute);
    
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, t);
        glEnableVertexAttribArray(program.texCoordAttribute);
    
        glDrawArrays(GL_TRIANGLES, 0, 6);
    
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        v += 6;
        t += 6;
    }
}

void Setup(){
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
    #ifdef _WINDOWS
    glewInit();
    #endif
    
    glViewport(0, 0, 640, 360);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}



class Entity{
 public:
    int textureID;
    float xPos;
    float yPos;
    float xVelocity;
    float yVelocity;
    float height;
    float width;
    float u;
    float v;
    float timeAlive = 0.0f;
    float scaleFactor;
 
    Entity(unsigned int texture, float xPos, float yPos, float xVel, float yVel, float h, float w, float u, float v, float scaleFactor) : textureID(texture), xPos(xPos), yPos(yPos), xVelocity(xVel), yVelocity(yVel), height(h), width(w), u(u), v(v), scaleFactor(scaleFactor) {}
 
    void Draw(ShaderProgram &program){
        #define DEFAULT_VERTICES {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5,-0.5, -0.5, 0.5, 0.5, -0.5, 0.5}
        
        glBindTexture(GL_TEXTURE_2D, textureID);

        glm::mat4 modelMatrix = glm::mat4(1.0f);
   
        modelMatrix = glm::translate(modelMatrix, glm::vec3(xPos, yPos, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(scaleFactor*width, scaleFactor*height, 1.0f));
    
        program.SetModelMatrix(modelMatrix);

        float vertices[] = DEFAULT_VERTICES;
    
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);

        float textCoords[] = {u, v + height, u + width, v + height, u + width, v, u, v + height, u + width, v, u, v};
    
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, textCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
    
    
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
    
    }
};

class GameState {
    public:
     Entity ship;
     vector<Entity> entities;
     vector<Entity> bullets;
     int score;
     float lastSpacePress;
    
     GameState(unsigned int invaderSheet): ship(invaderSheet, 0.0, -0.8, 0.5, 0.0, 0.15, 0.15, 0.25, 0.85, 1.5), score(0), lastSpacePress(0.0){
        
        entities.push_back(ship);
        for (int i = 0; i < 20; i++){
            Entity Invader = Entity(invaderSheet, -1.0 + (i/4)*0.4, 0.6 -
            (i % 4)*0.3, 0.3, 0.0, 0.15, 0.20, 0.02, 0.02, 1.5);
            entities.push_back(Invader);
        }
     }
};


void shootBullet(vector<Entity> &bullets, unsigned int bulletTex, float shipXPos) {
    Entity newBullet = Entity(bulletTex, shipXPos, -0.75, 0.0, 1.0, 0.08, 0.015, 0.0, 1.0, 1.5);
    bullets.push_back(newBullet);
}

bool shouldRemoveBullet(Entity bullet) {
    if(bullet.timeAlive > 2.0) {
        return true;
    } else {
        return false;
    }
}


int main(int argc, char *argv[])
{
    Setup();
   
    ShaderProgram program;
    program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    glUseProgram(program.programID);
    
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    
    projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    
    float lastFrameTicks = 0.0f;
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    GameMode mode = TITLE_SCREEN;
    
    
    GLuint InvaderSheet = LoadTexture(RESOURCE_FOLDER"InvadersSheet.png");
    GLuint PixelFont = LoadTexture(RESOURCE_FOLDER"pixel_font.png");
    GLint bulletTex = LoadTexture(RESOURCE_FOLDER"Bullet.png");
    

    Entity titleImage = Entity(InvaderSheet, 0.0, 0.0, 0.0, 0.0, 0.15, 0.2, 0.75, 0.65, 5);
    
    GameState state = GameState(InvaderSheet);
    
    SDL_Event event;
    bool done = false;
    while (!done) {
        
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        state.lastSpacePress += elapsed;
        
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
            else if(event.type == SDL_KEYDOWN) {
                if(event.key.keysym.scancode == SDL_SCANCODE_SPACE && state.lastSpacePress > 1.0f) {

                        shootBullet(state.bullets, bulletTex, state.entities[0].xPos);
                        state.lastSpacePress = 0.0f;
                    }
                }
            
            else if(event.key.keysym.scancode == SDL_SCANCODE_S){
                mode = GAME_LEVEL;
                state = GameState(InvaderSheet);
            }
        }
        
     
        if (keys[SDL_SCANCODE_RIGHT] && state.entities[0].xPos < 1.6f) {
            state.entities[0].xPos += elapsed * 0.5f;
        }
        else if (keys[SDL_SCANCODE_LEFT] && state.entities[0].xPos > -1.6f){
            state.entities[0].xPos -= elapsed * 0.5f;
        }
        
        
        //Rendering
        glClear(GL_COLOR_BUFFER_BIT);
        
        switch(mode){
            case TITLE_SCREEN:
                titleImage.Draw(program);
                DrawText(program, PixelFont, "press s to start", 0.08, 0.02, -0.65, -0.8);
                break;
            case GAME_LEVEL:
                state.entities[0].Draw(program);
        
        
                //update invaders' position
                if (state.entities.size() == 1){
                    mode = GAME_WON;
                }
                else{
                    for (size_t i = 1; i < state.entities.size(); i++){
                        if (state.entities[i].xPos > 1.7){
                            for(size_t j = 1; j < state.entities.size(); j++){
                                state.entities[j].xVelocity = -state.entities[j].xVelocity;
                                state.entities[j].xPos -= 0.01;
                                state.entities[j].yPos -= 0.1;
                            }
                        }
                        else if (state.entities[i].xPos < -1.7){
                            for(size_t j = 1; j < state.entities.size(); j++){
                                state.entities[j].xVelocity = -state.entities[j].xVelocity;
                                state.entities[j].xPos += 0.01;
                                state.entities[j].yPos -= 0.1;
                            }
                        }
                        state.entities[i].xPos += elapsed * state.entities[i].xVelocity;
                        state.entities[i].Draw(program);
                        if (state.entities[i].yPos <= state.ship.yPos){
                            mode = GAME_OVER;
                        }
                    }
                }
            
                for (size_t i = 0; i < state.bullets.size(); i++){
                    state.bullets[i].yPos += elapsed * state.bullets[i].yVelocity;
                    state.bullets[i].timeAlive += elapsed;
                    state.bullets[i].Draw(program);
                   state.bullets.erase(remove_if(state.bullets.begin(), state.bullets.end(), shouldRemoveBullet), state.bullets.end());
                }
                
                
                //check collisions between bullets and invaders
                for (size_t z = 0; z < state.bullets.size(); z++){
                    for (size_t w = 1; w < state.entities.size(); w++){
                        if ((abs(state.entities[w].xPos - state.bullets[z].xPos) - 0.1075 < 0) && (abs(state.entities[w].yPos - state.bullets[z].yPos) - 0.115 < 0)){
                            state.score += 10;
                            state.entities.erase(state.entities.begin() + w);
                            state.bullets.erase(state.bullets.begin() + z);
                        }
                    }
                }
                
                DrawText(program, PixelFont, "Score: " + to_string(state.score), 0.15, 0.02, -1.6, 0.9);
                
                break;
                
                case GAME_OVER:
                
                DrawText(program, PixelFont, "GAME OVER", 0.15, 0.02, -0.5, 0.0);
                
                break;
                
                case GAME_WON:
                
                DrawText(program, PixelFont, "YOU WON!", 0.15, 0.02, -0.5, 0.0);
                
                break;
            }
        
                SDL_GL_SwapWindow(displayWindow);
    }
    SDL_Quit();
    return 0;
}
