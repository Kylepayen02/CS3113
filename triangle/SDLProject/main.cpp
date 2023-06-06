// Used to supress the OPENGL error msgs
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1

// Header Files for importing necessary libraries
#include <iostream>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"   // 4x4 matrix
#include "glm/gtc/matrix_transform.hpp" // Matrix transformation
#include "ShaderProgram.h"
#include <OpenGL/gl.h>


#ifdef _WINDOWS
#include <GL/glew.h>
#endif


// width and height for the window
const int WINDOW_WIDTH  = 640,
          WINDOW_HEIGHT = 480;

// Background color for the window
const float BG_RED     = 0.1922f,
            BG_BLUE    = 0.549f,
            BG_GREEN   = 0.9059f,
            BG_OPACITY = 1.0f;

// Our viewport-or our "camera's position and dimensions
const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

// The color of our triangle
const float TRIANGLE_RED = 1.0,
            TRIANGLE_BLUE = 0.4,
            TRIANGLE_GREEN = 0.4,
            TRIANGLE_OPACITY = 1.0;

const char V_SHADER_PATH[] = "shaders/vertex.glsl";
const char F_SHADER_PATH[] = "shaders/fragment.glsl";

// Instantiating Window object
// g_display_window is where our game will be displayed
SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_program;

glm::mat4 g_view_matrix;    // Defines the position (location and orientation) of the camera
glm::mat4 g_model_matrix;   // Defines every translation, rotation, and/or scaling applied to an object
glm::mat4 g_projection_matrix; // Defines the characteristics of your camera, such as clip panes, field of view, projection method, etc.

void initialize();
void process_input();
void update();
void render();
void shutdown();

int main(int argc, char* argv[])
{
    initialize();
    
    while (g_game_is_running){
        process_input();
        update();
        render();
    }
    shutdown();
    return 0;
}

void initialize(){
    
    // Here we are initialize OpenGL
    SDL_Init(SDL_INIT_VIDEO);
    
    // Initializing our display window object
    g_display_window = SDL_CreateWindow("Hello, Triangle!",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        WINDOW_WIDTH, WINDOW_HEIGHT,
                                        SDL_WINDOW_OPENGL);
    
    // Initializing OpenGL context for an OpenGL window
    // The context in OpenGL stores all of the states associated with this instance of OpenGL
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    
    // And now we make the context the current one we are working in
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);    // Initialize our camera
    g_program.Load(V_SHADER_PATH, F_SHADER_PATH);                           // Load up shaders
    
    // Initialize our view model, and projection matrices
    g_view_matrix       = glm::mat4(1.0f);
    g_model_matrix      = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_program.SetViewMatrix(g_view_matrix);
    g_program.SetProjectionMatrix(g_projection_matrix);
    
    g_program.SetColor(TRIANGLE_RED, TRIANGLE_BLUE, TRIANGLE_GREEN, TRIANGLE_OPACITY);
    
    glUseProgram(g_program.programID);
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
}

void process_input(){
    SDL_Event event;
    while (SDL_PollEvent(&event)){
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE){
            g_game_is_running = false;
        }
    }
}

void update(){
    /*
     No updates yet so this will remain empty for now
     */
}

void render(){
    //Step 1 - Clear the color
    glClear(GL_COLOR_BUFFER_BIT);
    
    //step 2 - Now we set our model matrix
    g_program.SetModelMatrix(g_model_matrix);
    
    //Step 3 - set up triangle vertices and draw them
    float vertices[] =
    {
        0.5f, -0.5f,     //(x1,y1)
        0.0f, 0.5f,     //(x2,y2)
       -0.5f, -0.5f     //(x3, y3)
    };
    
    glVertexAttribPointer(g_program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_program.positionAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(g_program.positionAttribute);
    
    //step 4 - swap the previous frame to the current frame
    SDL_GL_SwapWindow(g_display_window);
    
}

void shutdown(){
    SDL_Quit();
}
