#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "shader.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// TO COMPILE: gcc main.c glad.c -o main -lglfw -lGL -ldl && ./main

int scr_width = 600;
int scr_height = 600;

void framebuffer_size_callback(GLFWwindow*, int, int);
void processInput(GLFWwindow*);

Shader shader;

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 10
int board[BOARD_HEIGHT][BOARD_WIDTH] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, -1, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

struct {
    float x, y;
    float width, height;
} Block = {-1.0f, -1.0f, 2.0f / (float)BOARD_WIDTH, 2.0f / (float)BOARD_HEIGHT};

typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT
} PlayerDir;
PlayerDir player_dir_new = RIGHT;

struct {
    int x, y;
    int length;
    PlayerDir direction;
} Player = {BOARD_WIDTH / 2, BOARD_HEIGHT / 2, 5, RIGHT};

float delta_time = 0.0f;

void update_board();
void update_positions();
void draw_block(float, float, int);
void set_player_pos(int*, GLFWwindow*);
void make_new_apple();

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(scr_width, scr_height, "Snake", NULL, NULL);
    if (window == NULL) {
        printf("Unable to create window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        return -1;
    }

    shader = init_shader("vertexShader.glsl", "fragmentShader.glsl");

    float vertices[] = {
        Block.x, Block.y,
        Block.x, Block.y + Block.height,
        Block.x + Block.width, Block.y,
        Block.x + Block.width, Block.y + Block.height,
    };

    unsigned int VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    float last_time = 0.0f;
    float current_time = 0.0f;

    float last_update_time = 0.0f;
    float current_update_time = 0.0f;

    int is_new_move = 1;

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // Calculate delta time
        current_time = (float)glfwGetTime();
        delta_time = current_time - last_time;
        last_time = current_time;

        current_update_time = current_time;
        if (current_update_time - last_update_time >= 0.4f) {
            is_new_move = 1;

            last_update_time = current_update_time;
            Player.direction = player_dir_new;
            update_positions();
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();
        glBindVertexArray(VAO);


        // Draw
        set_player_pos(&is_new_move, window);
        update_board();

        glfwSwapBuffers(window);
        glfwPollEvents();    
    }

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);

    shader.delete();

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    scr_width = width;
    scr_height = height;
}

void processInput(GLFWwindow* window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, GL_TRUE);

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE) return;

    PlayerDir player_dir_original = Player.direction;
    PlayerDir player_dir_temp = Player.direction;

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) player_dir_temp = UP;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) player_dir_temp = DOWN;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) player_dir_temp = LEFT;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) player_dir_temp = RIGHT;

    int is_valid_pos = 1;
    if (player_dir_temp == UP && player_dir_original == DOWN || player_dir_temp == DOWN && player_dir_original == UP || player_dir_temp == LEFT && player_dir_original == RIGHT || player_dir_temp == RIGHT && player_dir_original == LEFT) is_valid_pos = 0;
    if (player_dir_temp != player_dir_original && is_valid_pos) {
        player_dir_new = player_dir_temp;
    }
}

void make_new_apple() {
    int spots_open = 0;
    for (int posY = 0; posY < BOARD_HEIGHT; ++posY) {
        for (int posX = 0; posX < BOARD_WIDTH; ++posX) {
            if (board[posY][posX] == 0) {
                ++spots_open;
                break;
            }
        }
    }

    if (spots_open == 0) {
        printf("\nYou Win!\n");
        return;
    }

    srand(time(NULL));
    int spotY = rand() % BOARD_HEIGHT;
    int spotX = (rand() + 1) % BOARD_WIDTH;

    while (board[spotY][spotX] != 0) {
        spotX += 1;
        if (spotX % BOARD_WIDTH == 0) spotY == 1;
        if (spotY % BOARD_HEIGHT == 0) spotY = 0;
    }

    board[spotY][spotX] = -1;
}

void draw_block(float posX, float posY, int type) {
    float colorR, colorG, colorB;
    switch (type) {
        case 1: // Snake Green
            // colorR = 0.12549f;
            // colorG = 0.76078f;
            // colorB = 0.05490f;

            colorR = 0.14902f;
            colorG = 0.91372f;
            colorB = 0.06666f;
            break;
        case 2:
            colorR = 0.10196f;
            colorG = 0.61176f;
            colorB = 0.04314f;
            break;
        case 3: // Apple Red
            colorR = 1.0f;
            colorG = 0.0f;
            colorB = 0.0f;
            break;
    }

    shader.set_float("offsetX", posX);
    shader.set_float("offsetY", posY);
    shader.set_float("colorR", colorR);
    shader.set_float("colorG", colorG);
    shader.set_float("colorB", colorB);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void set_player_pos(int* is_new_move, GLFWwindow* window) {
    if (!*is_new_move) {
        board[Player.y][Player.x] = Player.length;
        return;
    }
    if (*is_new_move) {
        *is_new_move = 0;
        if (Player.y < 0 || Player.y >= BOARD_HEIGHT) {
            glfwSetWindowShouldClose(window, 1);
            printf("\nYou went out of bounds.\n");
            return;
        }
        if (Player.x < 0 || Player.x >= BOARD_WIDTH) {
            glfwSetWindowShouldClose(window, 1);
            printf("\nYou went out of bounds.\n");
            return;
        }
        
        if (board[Player.y][Player.x] > 0) {
            glfwSetWindowShouldClose(window, 1);
            printf("\nYou ran into yuourself.\n");
            return;
        }
        if (board[Player.y][Player.x] == -1) {
            Player.length += 1;
            for (int posY = 0; posY < BOARD_HEIGHT; ++posY) {
                for (int posX = 0; posX < BOARD_WIDTH; ++posX) {
                    if (board[posY][posX] <= 0) continue;
                    board[posY][posX] += 1;
                }
            }
            make_new_apple();
        }
    }
    board[Player.y][Player.x] = Player.length;
}

void update_board() {
    for (int posY = 0; posY < BOARD_HEIGHT; ++posY) {
        for (int posX = 0; posX < BOARD_WIDTH; ++posX) {
            if (board[posY][posX] == 0) continue;
            int block_type = 1;

            if (board[posY][posX] == Player.length) block_type = 2;
            if (board[posY][posX] == -1) block_type = 3;

            float draw_pos_x = (float)posX / BOARD_WIDTH * 2.0f;
            float draw_pos_y = (float)posY / BOARD_HEIGHT * 2.0f;

            draw_pos_y = fabs(draw_pos_y - 2.0f) - Block.height;
            draw_block(draw_pos_x, draw_pos_y, block_type);
        }
    }
}

void update_positions() {
    switch (Player.direction) {
        case UP:
            Player.y -= 1;
            break;
        case DOWN:
            Player.y += 1;
            break;
        case LEFT:
            Player.x -= 1;
            break;
        case RIGHT:
            Player.x += 1;
            break;
    }
    
    for (int posY = 0; posY < BOARD_HEIGHT; ++posY) {
        for (int posX = 0; posX < BOARD_WIDTH; ++posX) {
            if (board[posY][posX] <= 0) continue;
            board[posY][posX] -= 1;
        }
    }
}