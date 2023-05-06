//
// Created by root on 4/28/23.
//

#ifndef SNAKE_SNAKE_H
#define SNAKE_SNAKE_H

#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <semaphore.h>

#define INITIAL_SIZE 3

#define INITIAL_MAP_TOP_LEFT_CORNER_X 45
#define INITIAL_MAP_TOP_LEFT_CORNER_Y 0
#define MAP_WIDTH 100
#define MAP_HEIGHT 25

#define BOOSTER_DURATION (__time_t) 8

#define INITIAL_HEAD_POS_X (INITIAL_MAP_TOP_LEFT_CORNER_X + INITIAL_SIZE + 1)
#define INITIAL_HEAD_POS_Y (INITIAL_MAP_TOP_LEFT_CORNER_Y + 2)

#define RECTANGLE (char) 0x2588
#define APPLE (char) 'T'
#define BOOSTER (char) 'B'

#define STATISTICS_TOP_LEFT_CORNER_X 2
#define STATISTICS_TOP_LEFT_CORNER_Y 1

#define INSTRUCTION_TOP_LEFT_CORNER_X STATISTICS_TOP_LEFT_CORNER_X
#define INSTRUCTION_TOP_LEFT_CORNER_Y (STATISTICS_TOP_LEFT_CORNER_Y + 10)

#define ARROW_HEIGHT 9

#define ARROW_TOP_LEFT_CORNER_X (INITIAL_MAP_TOP_LEFT_CORNER_X + MAP_WIDTH + 18)
#define ARROW_TOP_LEFT_CORNER_Y 2

typedef struct {
    int x;
    int y;
} component_t, apple_t;

typedef struct {
    int size;
    component_t *component;
} snake_t;

typedef struct {
    int snakeSize;

    char timeMinutes;
    char timeSeconds;

    int bestTime;
    char *username;

    int applesEaten;
} statistics_t;

typedef enum {
    DOUBLE_POINTS,
    SLOWER_SNAKE
} booster_type_t;

typedef struct {
    int x;
    int y;
    booster_type_t boosterType;
    time_t duration;
} booster_t;


int startGame(void);
struct linked_list_t *createSnake(void);
int endGame(void);
void drawMap(void);
void drawSnake(void);
void prepareMap(void);
void moveSnake(void);
void updateSnakePointPosition(void);
void drawUpArrow(void);
void drawDownArrow(void);
void drawLeftArrow(void);
void drawRightArrow(void);
void clearArrow(void);
void drawArrow(int index);
void prepareStatistics(void);
void generateApple(void);
int didHitHimself(void);
int didHitWall(void);
int checkIfFieldFree(int x, int y);
int didEatApple(void);
void gameOver(void);
int detectCollision(void);
int didEatBooster(void);
void stopBooster(__attribute__((unused)) union sigval sv);
int showWelcomeScreen(void);
void markOption(int choise);

#endif //SNAKE_SNAKE_H
