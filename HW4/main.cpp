#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#define STB_IMAGE_IMPLEMENTATION
#include "FlareMap.h"
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
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
#endif

SDL_Window* displayWindow;

using namespace std;

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

struct Vec2 {
    float x, y;
    Vec2(float x, float y): x(x), y(y) {}
};

float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}

class Entity{

public:

    bool isTouchingGround;
    bool collidedRight;
    bool collidedLeft;
    bool collected;

    Vec2 position;
    Vec2 velocity;
    Vec2 acceleration;
    
    float height = 0.056;
    float width = 0.03;
    
    float u;
    float v;
    
    Entity(Vec2 pos, float u, float v): u(u), v(v), position(pos),  velocity(0, 0), acceleration(0, 0), collected(false){}
    
    bool isInContact(Entity &otherEntity){

        float yDiff = fabs(position.y - otherEntity.position.y) - 0.2;
        float xDiff = fabs(position.x - otherEntity.position.x) - 0.1;
        if(yDiff < 0.0f && xDiff < 0.0f){
            return true;
        }
        else{
            return false;
        }
    }
    
    void Draw(ShaderProgram &program){
        #define DEFAULT_VERTICES {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5,-0.5, -0.5, 0.5, 0.5, -0.5, 0.5}

        glm::mat4 modelMatrix = glm::mat4(1.0f);
   
        modelMatrix = glm::translate(modelMatrix, glm::vec3(position.x, position.y, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.3, 0.3, 1.0f));
    
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

void Setup(){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Platformer Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
    #ifdef _WINDOWS
    glewInit();
    #endif
    
    glViewport(0, 0, 640, 360);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}
void Update(float elapsed, Entity &player){
    player.velocity.x += player.acceleration.x * elapsed;
    player.velocity.y += player.acceleration.y * elapsed;
    player.position.x += player.velocity.x * elapsed;
    player.position.y += player.velocity.y * elapsed;
    player.velocity.x = lerp(player.velocity.x, 0.0f, elapsed * 2.0);
    player.velocity.y = lerp(player.velocity.y, 0.0f, elapsed * 2.0);
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
    
    program.SetViewMatrix(viewMatrix);

    program.SetProjectionMatrix(projectionMatrix);
    
    float lastFrameTicks = 0.0f;
    float accumulator = 0.0f;
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    GLuint spriteSheet = LoadTexture(RESOURCE_FOLDER"spritesheet.png");
    Entity player  = Entity(Vec2(2.3, -2.5), 0.635, 0.01);
    Entity coin1 = Entity(Vec2(2.8, -2.6), 0.6, 0.13);
    Entity coin2 = Entity(Vec2(4.7, -2.6), 0.6, 0.13);
    Entity coin3 = Entity(Vec2(3.8, -2.4), 0.6, 0.13);
    FlareMap map;
    vector<Entity> tiles;
    
    map.Load(RESOURCE_FOLDER"TileMap3.txt");
    for(int x=0; x < map.mapWidth; x++) {
        for(int y=0; y < map.mapHeight; y++) {
         // check map.mapData[y][x] for tile index
          int tileIndex = map.mapData[y][x];
          float u, v;
          int a = tileIndex % 30;
          int b = tileIndex/30;
          u = (float)a/30 + 1.0/372.0;
          v = (float)b/16 + 3.0/372.0;
          Entity tile = Entity(Vec2(x*0.3, -y*0.3), u, v);
          tiles.push_back(tile);
        }
        
    }
    
    SDL_Event event;
    bool done = false;
    
    while (!done) {
    
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
            else if(event.type == SDL_KEYDOWN) {
                if(event.key.keysym.scancode == SDL_SCANCODE_SPACE && player.isTouchingGround) {
                    player.velocity.y = 2.0;
                }
            }
        }
        
        //process events
        if (keys[SDL_SCANCODE_RIGHT] && player.position.x < 5.7f && !player.collidedRight) {
            player.velocity.x = 0.5;
        }
        else if(keys[SDL_SCANCODE_LEFT] && player.position.x > 0.3f && !player.collidedLeft){
            player.velocity.x = -0.5;
        }
        else {
            player.velocity.x = 0.0;
        }
      
        //update with fixed timestep
        elapsed += accumulator;
        if(elapsed < FIXED_TIMESTEP) {
            accumulator = elapsed;
            continue;
        }
        while(elapsed >= FIXED_TIMESTEP) {
            Update(FIXED_TIMESTEP, player);
            elapsed -= FIXED_TIMESTEP;
        }
        accumulator = elapsed;
        
        //rendering
        glClear(GL_COLOR_BUFFER_BIT);
        
        glBindTexture(GL_TEXTURE_2D, spriteSheet);
        
        
        for (Entity tile: tiles){
            tile.Draw(program);
        }
        
        player.collidedRight = false;
        player.collidedLeft = false;
        
        int playerBottomLeftX = abs(player.position.x - 0.0145)/0.3;
        int playerBottomLeftY = abs(player.position.y - 0.31)/0.3;
        
        int playerBottomRightX = abs(player.position.x + 0.25)/0.3;
        int playerBottomRightY = abs(player.position.y - 0.31)/0.3;
        
        int playerMidRightX = (player.position.x + 0.3001)/0.3;
        int playerMidLeftX = (player.position.x - 0.0145001)/0.3;
        int playerMidY = (player.position.y - 0.01)/-0.3;
        int playerMidY2 = (player.position.y - 0.3)/-0.3;
        
        
        if (map.mapData[playerMidY][playerMidRightX] == 122 || map.mapData[playerMidY][playerMidRightX] == 152 || map.mapData[playerMidY2][playerMidRightX] == 122){
            player.collidedRight = true;
        }
        if (map.mapData[playerMidY][playerMidLeftX] == 122 || map.mapData[playerMidY][playerMidLeftX] == 152 || map.mapData[playerMidY2][playerMidLeftX] == 122){
            player.collidedLeft = true;
        }
        
        
        if (map.mapData[playerBottomLeftY][playerBottomLeftX] == 122 || map.mapData[playerBottomRightY][playerBottomRightX] == 122){
            player.acceleration.y = 0.0;
            player.velocity.y = 0.0;
            
            player.position.y = player.position.y + 0.0001;
            player.isTouchingGround = true;
        }
        else {
            player.acceleration.y = -2.5;
            player.isTouchingGround = false;
        }
        
        //player has fallen
        if (player.position.y < -4.5f){
            player.position.y = -2.5f;
            player.position.x = 2.3f;
        }
        
        
        if(coin1.collected == false){
            coin1.Draw(program);
        }
        if(player.isInContact(coin1)){
            coin1.collected = true;
        }
        if(coin2.collected == false){
            coin2.Draw(program);
        }
        if(player.isInContact(coin2)){
            coin2.collected = true;
        }
        if(coin3.collected == false){
            coin3.Draw(program);
        }
        if(player.isInContact(coin3)){
            coin3.collected = true;
        }
        
        
        player.Draw(program);
        
        
        
        if(player.position.x > 2.0f && player.position.x < 4.0f){
        
            viewMatrix = glm::mat4(1.0f);
            viewMatrix = glm::translate(viewMatrix, glm::vec3(-player.position.x, -player.position.y, 1.0f));
            program.SetViewMatrix(viewMatrix);
        }
        
        

        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
