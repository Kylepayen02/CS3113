/**
* Author: Kyle Payen
* Assignment: Simple 2D Scene
* Date due: 2023-06-11, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define GL_GLEXT_PROTOTYPES 1
#define LOG(argument) std::cout << argument << '\n'

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"


const int WINDOW_WIDTH = 640,
          WINDOW_HEIGHT = 480;

const float BG_RED = 1.0f,
            BG_BLUE = 0.0f,
            BG_GREEN = 0.643f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;


const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL = 0;
const GLint TEXTURE_BORDER = 0;

const char BLACK_MAGE_SPRITE_FILEPATH[] = "textures/black_mage.png";
const char CACTAR_SPRITE_FILEPATH[] = "textures/cactar.png";

SDL_Window* display_window;
bool game_is_running = true;
bool is_growing = true;

ShaderProgram program;
glm::mat4 view_matrix, character_matrix, projection_matrix, trans_matrix,
          cactar_matrix;

float previous_ticks = 0.0f;

GLuint black_mage_texture_id, cactar_texture_id;

float x_character = 0.0f;
float y_character = 0.0f;
float rot_angle = 0.0f;
const float shrink_factor = 0.999f;


void initialise();
void process_input();
void update();
void render();
void shutdown();

GLuint load_texture(const char* filepath);
void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id);


int main(int argc, char* argv[])
{
    initialise();
    
    while (game_is_running)
    {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}


GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    //Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    //Releasing our file from memory and returning our texture id
    stbi_image_free(image);
    
    return textureID;
}

void initialise()
{
    
    SDL_Init(SDL_INIT_VIDEO);
    
    
    display_window = SDL_CreateWindow("2d Scene",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(display_window);
    SDL_GL_MakeCurrent(display_window, context);
    
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    program.Load(V_SHADER_PATH, F_SHADER_PATH);
    
    character_matrix = glm::mat4(1.0f);
    cactar_matrix = glm::mat4(1.0f);
    view_matrix = glm::mat4(1.0f);
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    
    program.SetProjectionMatrix(projection_matrix);
    program.SetViewMatrix(view_matrix);
    
    glUseProgram(program.programID);
    
    glClearColor(BG_RED, BG_GREEN, BG_BLUE, BG_OPACITY);
    
    black_mage_texture_id = load_texture(BLACK_MAGE_SPRITE_FILEPATH);
    cactar_texture_id = load_texture(CACTAR_SPRITE_FILEPATH);
    
    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    
    SDL_Event event;
    
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
        {
            game_is_running = false;
        }
    }
    
}

void update()
{
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - previous_ticks;
    previous_ticks = ticks;
    
    x_character += 0.01f * delta_time;
    y_character += 0.01f * delta_time;
    rot_angle += 120.0 * delta_time;
    
    cactar_matrix = glm::mat4(1.0f);
    
    character_matrix = glm::translate(character_matrix, glm::vec3(x_character, y_character, 0.0f));
    character_matrix = glm::scale(character_matrix, glm::vec3(shrink_factor, shrink_factor, 0.0f));
    
    cactar_matrix = glm::rotate(cactar_matrix, glm::radians(rot_angle), glm::vec3(0.0f, 0.0f, 1.0f));
    cactar_matrix = glm::translate(cactar_matrix, glm::vec3(-3.0f, 0.05f, 0.0f));
    
    
}

void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id)
{
    program.SetModelMatrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Vertices
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };
    
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(program.texCoordAttribute);
    

    draw_object(character_matrix, black_mage_texture_id);
    draw_object(cactar_matrix, cactar_texture_id);
    
    
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
    
    SDL_GL_SwapWindow(display_window);
}

void shutdown()
{
    SDL_Quit();
}


