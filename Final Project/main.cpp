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
#include <SDL_mixer.h>
#include <cmath>
#include <vector>
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#define screenHeight 1.779f
#endif

SDL_Window* displayWindow;
using namespace std;

enum GameMode {START_SCREEN, GAME_ON, GAME_OVER};

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

struct vec2 {
    float x, y;
    vec2(float x, float y): x(x), y(y) {}
};

class Entity {
    public:
    vec2 position;
    vec2 velocity;
    unsigned int TextureID;
    float scaleFactor;
    float width;
    float height;
    float timeSinceLastFlap;
    
    Entity(unsigned int tex , vec2 pos, float sf = 1.0f, vec2 vel = vec2(0.0f, 0.0f)): velocity(vel), TextureID(tex), position(pos), scaleFactor(sf), timeSinceLastFlap(0.0f){}
    
    void Draw(ShaderProgram &program){
    
        glBindTexture(GL_TEXTURE_2D, TextureID);
        #define DEFAULT_VERTICES {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5,-0.5, -0.5, 0.5, 0.5, -0.5, 0.5}

        glm::mat4 modelMatrix = glm::mat4(1.0f);
   
        modelMatrix = glm::translate(modelMatrix, glm::vec3(position.x, position.y, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(scaleFactor*0.6, scaleFactor*0.5, 1.0f));
        width = scaleFactor*0.6;
        height = scaleFactor*0.5;
    
        program.SetModelMatrix(modelMatrix);

        float vertices[] = DEFAULT_VERTICES;
    
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);

        float textCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
    
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, textCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
    
    
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
    
    }
    
    bool didCollideWith(Entity &otherEntity){
        if (abs(otherEntity.position.x - position.x) - (width + otherEntity.width)/2.0 < 0 && (abs(otherEntity.position.y - position.y) - (height + otherEntity.height)/2.0 < 0)){
            return true;
        }
        return false;
    }
    
};

class GameState{
    public:
    Entity plane;
    vector<Entity> boxes;
    vector<Entity> birds;
    int score;
    float timeTillNextBox;
    float timeTillNextBird;
    
    GameState(unsigned int planeTex, unsigned int boxTex): score(0), plane(planeTex, vec2(0.0, -0.8), 0.8), timeTillNextBox(0.0f), timeTillNextBird(10.0f) { }
    
};

void Setup(){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 375, 667, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
    glClearColor(0.47, 0.85, 1.0, 1.0);
    
    #ifdef _WINDOWS
    glewInit();
    #endif
    
    glViewport(0, 0, 375, 667);
    
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096 );
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

void Update(float elapsed, Entity &plane, vector<Entity> &boxes, vector<Entity> &birds, unsigned int &bird1, unsigned int &bird2, unsigned int   &birdR1, unsigned int &birdR2){
    

    plane.position.x += elapsed * plane.velocity.x;
    for (Entity &box: boxes){
        box.position.y += elapsed * box.velocity.y;
    }
    for (Entity &bird: birds){
        bird.timeSinceLastFlap += elapsed;
        bird.position.y += elapsed * bird.velocity.y;
        bird.position.x += elapsed * bird.velocity.x;
        //switch direction of bird if it hits edges of screen
        if(bird.position.x >= 0.95){
            bird.velocity.x = -bird.velocity.x;
            bird.TextureID = birdR1;
        }
        else if(bird.position.x <= -0.95){
            bird.velocity.x = -bird.velocity.x;
            bird.TextureID = bird1;
        }
        if (bird.timeSinceLastFlap > 0.5f && bird.velocity.x > 0.0f){
            if (bird.TextureID == bird1){
                bird.TextureID = bird2;
            }
            else{
                bird.TextureID = bird1;
            }
            bird.timeSinceLastFlap = 0.0;
        }
        else if (bird.timeSinceLastFlap > 0.5f && bird.velocity.x < 0.0f){
            if (bird.TextureID == birdR1){
                bird.TextureID = birdR2;
            }
            else{
                bird.TextureID = birdR1;
            }
            bird.timeSinceLastFlap = 0.0;
        }
    }
}

int main(int argc, char *argv[])
{
    
    Setup();
    GLuint planeTex = LoadTexture(RESOURCE_FOLDER"Plane.png");
    GLuint crateTex = LoadTexture(RESOURCE_FOLDER"crate.png");
    GLuint cloudTex1 = LoadTexture(RESOURCE_FOLDER"cloud.png");
    GLuint cloudTex2 = LoadTexture(RESOURCE_FOLDER"cloud2.png");
    GLuint bird1Tex = LoadTexture(RESOURCE_FOLDER"bird.png");
    GLuint bird2Tex = LoadTexture(RESOURCE_FOLDER"bird2.png");
    GLuint bird1RTex = LoadTexture(RESOURCE_FOLDER"birdR1.png");
    GLuint bird2RTex = LoadTexture(RESOURCE_FOLDER"birdR2.png");
    GLuint fontTex = LoadTexture(RESOURCE_FOLDER"font1.png");
    GLuint explosionTex = LoadTexture(RESOURCE_FOLDER"explosion.png");
    GLuint rightArrowTex = LoadTexture(RESOURCE_FOLDER"arrowRight.png");
    GLuint leftArrowTex = LoadTexture(RESOURCE_FOLDER"arrowLeft.png");
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    GameMode mode = START_SCREEN;
    GameState state = GameState(planeTex, crateTex);
    
    ShaderProgram program;
    program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    glUseProgram(program.programID);
    
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    
    projectionMatrix = glm::ortho(-1.0f, 1.0f, -screenHeight, screenHeight, -1.0f, 1.0f);
    
    program.SetViewMatrix(viewMatrix);

    program.SetProjectionMatrix(projectionMatrix);
    
    Entity arrowRight = Entity(rightArrowTex, vec2(0.5, -0.8), 0.3);
    Entity arrowLeft = Entity(leftArrowTex, vec2(-0.5, -0.8), 0.3);
    Entity cloud1 = Entity(cloudTex1, vec2(-0.7, 1.0));
    Entity cloud2 = Entity(cloudTex2, vec2(0.5, -0.3));
    
    Mix_Music *backgroundMusic;
    backgroundMusic = Mix_LoadMUS(RESOURCE_FOLDER"music.mp3");
    Mix_PlayMusic(backgroundMusic, -1);
    
    Mix_Chunk *crashSound;
    crashSound = Mix_LoadWAV(RESOURCE_FOLDER"Explosion.wav");
    
    SDL_Event event;
    bool done = false;
    float lastframeTicks = 0.0;
    float elapsedAn = 0.0;
    bool isDrawn = false;
    while (!done) {
        
        float ticks = (float)SDL_GetTicks()/1000.0;
        float elapsed = ticks - lastframeTicks;
        lastframeTicks = ticks;
        
        
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN){
                if(event.key.keysym.scancode == SDL_SCANCODE_ESCAPE && (mode == GAME_OVER || mode == START_SCREEN)){
                    SDL_Quit();
                    done = true;
                    Mix_FreeMusic(backgroundMusic);
                    Mix_FreeChunk(crashSound);
                }
            }
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
                Mix_FreeMusic(backgroundMusic);
                Mix_FreeChunk(crashSound);
            }
        }
        //process events;2
        if (keys[SDL_SCANCODE_LEFT]){
            if (mode == START_SCREEN){
                mode = GAME_ON;
            }
            state.plane.velocity.x = -1.5;
        }
        else if (keys[SDL_SCANCODE_RIGHT]){
            if (mode == START_SCREEN){
                mode = GAME_ON;
            }
        
            state.plane.velocity.x = 1.5f;
        }
        else{
            state.plane.velocity.x = 0.0f;
        }
        
        if (mode == GAME_OVER && keys[SDL_SCANCODE_R]){
            mode = GAME_ON;
            state = GameState(planeTex, crateTex);
            Mix_ResumeMusic();
        }
        
        glClear(GL_COLOR_BUFFER_BIT);
        
        state.plane.Draw(program);
        cloud1.Draw(program);
        cloud2.Draw(program);
        
        switch (mode) {
        case START_SCREEN:
            if(elapsedAn > 0.5){
                isDrawn = !isDrawn;
                elapsedAn = 0.0;
            }
            else{
                elapsedAn += elapsed;
            }
            
            DrawText(program, fontTex, "Plane", 0.35, -0.11, -0.45, 1.4);
            DrawText(program, fontTex, "Glider", 0.35, -0.11, -0.55, 1.0);
        
            DrawText(program, fontTex, "(move left or right to start)", 0.1, -0.045, -0.7, -1.3);
            
            if (isDrawn){
                arrowLeft.Draw(program);
                arrowRight.Draw(program);
            }
        
        break;
        
        case GAME_ON:
            state.timeTillNextBox -= elapsed;
            state.timeTillNextBird -= elapsed;
            
             if (state.timeTillNextBox <= 0.0f){
                //spawn box
                float randomX = (float)(rand() % 200 - 100)/100.0;
                Entity box = Entity(crateTex, vec2(randomX, screenHeight), 1.0f, vec2(0.0, -0.7));
                state.boxes.push_back(box);
                state.timeTillNextBox = 2.0f;
            }
            
            if(state.timeTillNextBird <= 0.0f){
                Entity bird = Entity(bird1Tex, vec2(0.0, screenHeight), 0.7f, vec2(0.3, -0.4));
                state.birds.push_back(bird);
                state.timeTillNextBird = 6.0f;
            }
        
            Update(elapsed, state.plane, state.boxes, state.birds, bird1Tex, bird2Tex, bird1RTex, bird2RTex);
            
             if(state.plane.position.x > 1.05f){
                state.plane.position.x = -1.05f;
             }
            else if(state.plane.position.x < -1.05f){
                state.plane.position.x = 1.05f;
            }
            
            for (Entity &box: state.boxes){
                if (box.position.y < -screenHeight - 0.2){
                    state.boxes.erase(state.boxes.begin());
                    state.score += 1;
                }
                if (state.plane.didCollideWith(box)){
                    mode = GAME_OVER;
                    state.plane.TextureID = explosionTex;
                    Mix_PlayChannel(1, crashSound, 0);
                }
                box.Draw(program);
                
            }
            for (Entity &bird: state.birds){
                if (bird.position.y < -screenHeight - 0.2){
                    state.birds.erase(state.birds.begin());
                }
                if (state.plane.didCollideWith(bird)){
                    mode = GAME_OVER;
                    state.plane.TextureID = explosionTex;
                    Mix_PlayChannel(1, crashSound, 0);
                }
                bird.Draw(program);
                
            }
            
            DrawText(program, fontTex, to_string(state.score), 0.35f, -0.11f, 0.0f, 1.5f);
            
        break;
        
        case GAME_OVER:
        
            for (Entity &box: state.boxes){
                box.Draw(program);
            }
            for (Entity &bird: state.birds){
                bird.Draw(program);
            }
            DrawText(program, fontTex, "Game Over", 0.2f, -0.05f, -0.6f, 0.6f);
            DrawText(program, fontTex, "press R to play again", 0.1f, -0.02f, -0.8f, 0.0f);
            DrawText(program, fontTex, "or press esc to exit", 0.1f, -0.01f, -0.8f, -0.2f);
            DrawText(program, fontTex, to_string(state.score), 0.35f, -0.11f, 0.0f, 1.5f);
            
            Mix_PauseMusic();
            
        break;
        default:
        break;
        }

        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
