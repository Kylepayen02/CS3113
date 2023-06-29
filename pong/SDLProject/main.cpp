/**
* Author: Kyle Payen
* Assignment: Pong Clone
* Date due: 2023-06-23, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define LOG(argument) std::cout << argument << '\n'
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include <cmath>

const int WINDOW_WIDTH  = 640,
          WINDOW_HEIGHT = 480;

const float BG_RED     = 0.0f,
            BG_BLUE    = 0.0f,
            BG_GREEN   = 0.0f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X      = 0,
          VIEWPORT_Y      = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const float MAX_Y  =   3.0f,
            MIN_Y  =  -3.0f,
            MAX_X  =   5.0f,
            MIN_X  =  -5.0f;


float RAND_X  =  rand() % 5 + (-3),
      RAND_Y  =  rand() % 5 + (-3);


const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

/**------------------------SPRITES---------------------------------**/

const char LEFT_PADDLE_SPRITE[] = "textures/paddle.png";
const char RIGHT_PADDLE_SPRITE[] = "textures/paddle.png";
const char BALL_SPRITE[] = "textures/ball.png";



/**------------------------INITIAL POSITION---------------------------------**/
const glm::vec3 LEFT_PADDLE_INIT_POS = glm::vec3(-4.74f, 0.0f, 0.0f);
const glm::vec3 RIGHT_PADDLE_INIT_POS = glm::vec3(4.78f, 0.0f, 0.0f);
const glm::vec3 BALL_INIT_POS = glm::vec3(0.0f, 0.0f, 0.0f);

const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL  = 0,
            TEXTURE_BORDER   = 0;

const float MILLISECONDS_IN_SECOND = 1000.0;

SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_pong_program;

/**------------------------TEXTURE IDS---------------------------------**/
GLuint        g_left_paddle_texture_id;
GLuint        g_right_paddle_texture_id;
GLuint        g_ball_texture_id;


glm::mat4 g_view_matrix,
          g_projection_matrix;

glm::mat4 g_left_paddle_model_matrix,
          g_right_paddle_model_matrix,
          g_ball_model_matrix;

float g_previous_ticks  = 0.0f;

float x = 0.0;

/**------------------------MOTION VECTORS--------------------------------**/
glm::vec3 g_left_paddle_movement = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_left_paddle_position = glm::vec3(0.0f, 0.0f, 0.0f);
float g_left_paddle_speed = 4.0f;

glm::vec3 g_right_paddle_movement = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_right_paddle_position = glm::vec3(0.0f, 0.0f, 0.0f);
float g_right_paddle_speed = 4.0f;

glm::vec3 g_ball_movement = glm::vec3(RAND_X, RAND_Y, 0.0f);
glm::vec3 g_ball_position = glm::vec3(0.0f, 0.0f, 0.0f);
float g_ball_speed = 1.5f;

GLuint load_texture(const char* filepath);
void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id);
void initialise();
void process_input();
void update();
void render();
void shutdown();

int main(int argc, char* argv[])
{
    initialise();
    
    while (g_game_is_running)
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
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    stbi_image_free(image);
    
    return textureID;
}

void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id)
{
    g_pong_program.SetModelMatrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("PONG",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        WINDOW_WIDTH, WINDOW_HEIGHT,
                                        SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    g_pong_program.Load(V_SHADER_PATH, F_SHADER_PATH);
    
    /**-------------------------RIGHT PADDLE MATRIX---------------------------------**/
    g_right_paddle_model_matrix = glm::mat4(1.0f);
    g_right_paddle_model_matrix = glm::translate(g_right_paddle_model_matrix, RIGHT_PADDLE_INIT_POS);
    
    /**-------------------------LEFT PADDLE MODEL MATRIX---------------------------------**/
    g_left_paddle_model_matrix = glm::mat4(1.0f);
    g_left_paddle_model_matrix = glm::translate(g_left_paddle_model_matrix, LEFT_PADDLE_INIT_POS);
    
    /**-------------------------BALL MODEL MATRIX---------------------------------**/
    g_ball_model_matrix = glm::mat4(1.0f);
    g_ball_model_matrix = glm::translate(g_ball_model_matrix, BALL_INIT_POS);
    
    g_pong_program.SetProjectionMatrix(g_projection_matrix);
    g_pong_program.SetViewMatrix(g_view_matrix);
    
    glUseProgram(g_pong_program.programID);
    
    /**------------------------LOADING TEXTURES---------------------------------**/
    g_left_paddle_texture_id = load_texture(LEFT_PADDLE_SPRITE);
    g_right_paddle_texture_id = load_texture(RIGHT_PADDLE_SPRITE);
    g_ball_texture_id = load_texture(BALL_SPRITE);

    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
}

void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_game_is_running = !g_game_is_running;
                break;
                
        }
    }
    
    /**------------------------RIGHT PADDLE KEYBINDING---------------------------------**/
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);
    
    if (key_state[SDL_SCANCODE_UP]) {
        g_right_paddle_movement.y = 1.0f;
    }
    if (key_state[SDL_SCANCODE_DOWN]) {
        g_right_paddle_movement.y = -1.0f;
    }
    
    /**------------------------LEFT PADDLE KEYBINDING---------------------------------**/
    if (key_state[SDL_SCANCODE_W]) {
        g_left_paddle_movement.y = 1.0f;
    }
    if (key_state[SDL_SCANCODE_S]) {
        g_left_paddle_movement.y = -1.0f;
    }
    
    /**------------------------NORMALIZATION---------------------------------**/
    if (glm::length(g_right_paddle_movement) > 1.0f)
    {
        g_right_paddle_movement = glm::normalize(g_right_paddle_movement);
    }
    
    if (glm::length(g_left_paddle_movement) > 1.0f)
    {
        g_left_paddle_movement = glm::normalize(g_left_paddle_movement);
    }
    
    if (glm::length(g_ball_movement) > 1.0f)
    {
        g_ball_movement = glm::normalize(g_ball_movement);
    }
    
}

void update()
{
    
    /** ———— DELTA TIME CALCULATIONS ———— **/
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    /** ———— COLLISION DETECTION ———— **/
    float paddle_collision_factor = 0.035;
    float ball_collision_factor = 0.1;
    float ball_paddle_collision_factor = 0.65f;
    
    // RIGHT PADDLE
    float right_pad_top_y_distance = fabs(g_right_paddle_position.y - MAX_Y) - (1.0f * paddle_collision_factor);
    float right_pad_bottom_y_distance = fabs(g_right_paddle_position.y - MIN_Y) - (1.0f * paddle_collision_factor);
    
    // LEFT PADDLE
    float left_pad_top_y_distance = fabs(g_left_paddle_position.y - MAX_Y) - (1.0f * paddle_collision_factor);
    float left_pad_bottom_y_distance = fabs(g_left_paddle_position.y - MIN_Y) - (1.0f * paddle_collision_factor);
    
    // BALL
    float ball_top_y_distance = fabs(g_ball_position.y - MAX_Y) - (1.0f * ball_collision_factor);
    float ball_bottom_y_distance = fabs(g_ball_position.y - MIN_Y) - (1.0f * ball_collision_factor);
    float ball_left_pad_distance_X = fabs(g_ball_position.x - LEFT_PADDLE_INIT_POS.x) - ((1.0f * ball_paddle_collision_factor + 1.0f * ball_paddle_collision_factor) / 2);
    float ball_left_pad_distance_Y = fabs(g_ball_position.y - g_left_paddle_position.y) - ((1.0f * ball_paddle_collision_factor + 1.0f * ball_paddle_collision_factor) / 2);
    float ball_right_pad_distance_X = fabs(g_ball_position.x - RIGHT_PADDLE_INIT_POS.x) - ((1.0f * ball_paddle_collision_factor + 1.0f * ball_paddle_collision_factor) / 2);
    float ball_right_pad_distance_Y = fabs(g_ball_position.y - g_right_paddle_position.y) - ((1.0f * ball_paddle_collision_factor + 1.0f * ball_paddle_collision_factor) / 2);

    
    
    /** ———— RESETTING MODEL MATRIX ———— **/
    g_right_paddle_model_matrix = glm::mat4(1.0f);
    g_right_paddle_model_matrix = glm::translate(g_right_paddle_model_matrix, RIGHT_PADDLE_INIT_POS);
    
    g_left_paddle_model_matrix = glm::mat4(1.0f);
    g_left_paddle_model_matrix = glm::translate(g_left_paddle_model_matrix, LEFT_PADDLE_INIT_POS);
    
    g_ball_model_matrix = glm::mat4(1.0f);
    g_ball_model_matrix = glm::translate(g_ball_model_matrix, BALL_INIT_POS);
    
    // ------------ TRANSLATION --------------- //
    
    // RIGHT PADDLE
    if (right_pad_top_y_distance < 0.0f)
    {
        g_right_paddle_movement.y = (g_right_paddle_movement.y > 0.0f) ? 0.0f : g_right_paddle_movement.y;
    }
    if (right_pad_bottom_y_distance < 0.0f)
    {
        g_right_paddle_movement.y = (g_right_paddle_movement.y < 0.0f) ? 0.0f : g_right_paddle_movement.y;
    }
    g_right_paddle_position += g_right_paddle_movement * g_right_paddle_speed * delta_time;
    g_right_paddle_model_matrix = glm::translate(g_right_paddle_model_matrix, g_right_paddle_position);
    g_right_paddle_movement = glm::vec3(0.0f, 0.0f, 0.0f);
    
    // LEFT PADDLE
    if (left_pad_top_y_distance < 0.0f)
    {
        g_left_paddle_movement.y = (g_left_paddle_movement.y > 0.0f) ? 0.0f : g_left_paddle_movement.y;
    }
    if (left_pad_bottom_y_distance < 0.0f)
    {
        g_left_paddle_movement.y = (g_left_paddle_movement.y < 0.0f) ? 0.0f : g_left_paddle_movement.y;
    }
    g_left_paddle_position += g_left_paddle_movement * g_left_paddle_speed * delta_time;
    g_left_paddle_model_matrix = glm::translate(g_left_paddle_model_matrix, g_left_paddle_position);
    g_left_paddle_movement = glm::vec3(0.0f, 0.0f, 0.0f);
    
    // BALL
    LOG(ball_left_pad_distance_X);
    if (ball_top_y_distance < 0.0f)
    {
        g_ball_movement.y *= -1.0f;
    }
    if (ball_bottom_y_distance < 0.0f)
    {
        g_ball_movement.y *= -1.0f;
    }
    if (ball_left_pad_distance_X < 0.0f && ball_left_pad_distance_Y < 0.0f)
    {
        g_ball_movement.x *= -1.0f;
    }
    if (ball_right_pad_distance_X < 0.0f && ball_right_pad_distance_Y < 0.0f)
    {
        g_ball_movement.x *= -1.0f;
    }
    
    if (g_ball_position.x > MAX_X || g_ball_position.x < MIN_X){
        shutdown();
    }
    
    
    g_ball_position += g_ball_movement * g_ball_speed * delta_time;
    g_ball_model_matrix = glm::translate(g_ball_model_matrix, g_ball_position);
    
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f
    };

    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };
    
    glVertexAttribPointer(g_pong_program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_pong_program.positionAttribute);
    
    glVertexAttribPointer(g_pong_program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_pong_program.texCoordAttribute);
    
    draw_object(g_right_paddle_model_matrix, g_right_paddle_texture_id);
    draw_object(g_left_paddle_model_matrix, g_left_paddle_texture_id);
    draw_object(g_ball_model_matrix, g_ball_texture_id);
    
    
    glDisableVertexAttribArray(g_pong_program.positionAttribute);
    glDisableVertexAttribArray(g_pong_program.texCoordAttribute);
    
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }
