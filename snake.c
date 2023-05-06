//
// Created by root on 4/28/23.
//

#include "snake.h"
#include "snake_list.h"
struct linked_list_t *snake;
statistics_t *statistics;
apple_t *apple;
pthread_t game, input, gameTimer, boosterThread, endGameThread;

sem_t boosterSemaphore;
sem_t endGameSemaphore;

int currentSnakeDirection = KEY_RIGHT;
pthread_mutex_t snakeDirectionMutex;
pthread_mutex_t terminalMutex;

static volatile int roundDuration = 50000u;
static volatile int isDoubled = 0;

booster_t *booster;

timer_t timerid;
struct itimerspec its;
struct sigevent sev;

_Noreturn void *snakeController(__attribute__((unused)) void *arg) {
    inf_loop:

    pthread_mutex_lock(&snakeDirectionMutex);

    moveSnake();

    pthread_mutex_unlock(&snakeDirectionMutex);
    if (detectCollision())
        gameOver();
    usleep(roundDuration);

    goto inf_loop;
}

_Noreturn void *inputReader(__attribute__((unused)) void *arg) {
    int move = KEY_RIGHT;
    inf_loop:
    ;
    int moveBeforeInput = move;
    move = getch();

    if (move != KEY_RIGHT && move != KEY_LEFT && move != KEY_UP && move != KEY_DOWN) {
        move = moveBeforeInput;
        goto inf_loop;
    }

    if (move == KEY_UP && moveBeforeInput == KEY_DOWN) {
        move = moveBeforeInput;
        goto inf_loop;
    }

    if (move == KEY_DOWN && moveBeforeInput == KEY_UP) {
        move = moveBeforeInput;
        goto inf_loop;
    }

    if (move == KEY_RIGHT && moveBeforeInput == KEY_LEFT) {
        move = moveBeforeInput;
        goto inf_loop;
    }

    if (move == KEY_LEFT && moveBeforeInput == KEY_RIGHT) {
        move = moveBeforeInput;
        goto inf_loop;
    }

    pthread_mutex_lock(&snakeDirectionMutex);
    currentSnakeDirection = move;
    pthread_mutex_unlock(&snakeDirectionMutex);
    drawArrow(currentSnakeDirection);

    goto inf_loop;
}

_Noreturn void *timerController(__attribute__((unused)) void *arg) {
    inf_loop:

    sleep(1u);

    if (statistics->timeSeconds == 59) {
        statistics->timeSeconds = 0;
        statistics->timeMinutes++;
    } else
        statistics->timeSeconds++;

    pthread_mutex_lock(&terminalMutex);
    mvprintw(STATISTICS_TOP_LEFT_CORNER_Y + 3, STATISTICS_TOP_LEFT_CORNER_X + 16, "%02d:%02d", statistics->timeMinutes, statistics->timeSeconds);
    pthread_mutex_unlock(&terminalMutex);
    refresh();

    goto inf_loop;
}

_Noreturn void *boosterGenerator(__attribute__((unused)) void *arg) {
    inf_loop:
    ;
    int sleepDuration = rand() % 12 + 6;
    booster_type_t boosterType = sleepDuration % 2;
    sleep(sleepDuration);
    int x, y;
    do {
        x = rand() % MAP_WIDTH + INITIAL_MAP_TOP_LEFT_CORNER_X + 1;
        y = rand() % MAP_HEIGHT + INITIAL_MAP_TOP_LEFT_CORNER_Y + 1;
    } while (checkIfFieldFree(x, y));

    booster->x = x;
    booster->y = y;
    booster->boosterType = boosterType;
    pthread_mutex_lock(&terminalMutex);
    mvaddch(y, x, BOOSTER);
    pthread_mutex_unlock(&terminalMutex);

    sem_wait(&boosterSemaphore);
    goto inf_loop;
}

void *waitForGameOver(__attribute__((unused)) void *arg) {
    sem_wait(&endGameSemaphore);
    endGame();
    endwin();
    return NULL;
}

void printStatistics() {
    mvprintw(STATISTICS_TOP_LEFT_CORNER_Y, STATISTICS_TOP_LEFT_CORNER_X, "+-----------------------+");
    mvprintw(STATISTICS_TOP_LEFT_CORNER_Y + 1, STATISTICS_TOP_LEFT_CORNER_X,  "|Statistics             |");
    mvprintw(STATISTICS_TOP_LEFT_CORNER_Y + 2, STATISTICS_TOP_LEFT_CORNER_X,  "|Apples eaten: 0        |");
    mvprintw(STATISTICS_TOP_LEFT_CORNER_Y + 3, STATISTICS_TOP_LEFT_CORNER_X, "|Gameplay time: %02d:%02d   |", 0, 0);
    mvprintw(STATISTICS_TOP_LEFT_CORNER_Y + 4, STATISTICS_TOP_LEFT_CORNER_X, "|Current high score: %d  |", 0);
    mvprintw(STATISTICS_TOP_LEFT_CORNER_Y + 5, STATISTICS_TOP_LEFT_CORNER_X, "+                       +");
    mvprintw(STATISTICS_TOP_LEFT_CORNER_Y + 6, STATISTICS_TOP_LEFT_CORNER_X, "+-----------------------+");

    refresh();
}

void drawArrow(int index) {
    pthread_mutex_lock(&terminalMutex);

    clearArrow();
    if (index == KEY_UP)
        drawUpArrow();
    else if (index == KEY_DOWN)
        drawDownArrow();
    else if (index == KEY_LEFT)
        drawLeftArrow();
    else if (index == KEY_RIGHT)
        drawRightArrow();

    pthread_mutex_unlock(&terminalMutex);
}

void drawUpArrow(void) {

    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 0, ARROW_TOP_LEFT_CORNER_X, "      /\\");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 1, ARROW_TOP_LEFT_CORNER_X, "     /||\\");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 2, ARROW_TOP_LEFT_CORNER_X, "    / || \\");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 3, ARROW_TOP_LEFT_CORNER_X, "   /  ||  \\");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 4, ARROW_TOP_LEFT_CORNER_X, "  /   ||   \\");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 5, ARROW_TOP_LEFT_CORNER_X, "      ||     ");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 6, ARROW_TOP_LEFT_CORNER_X, "      ||\n");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 7, ARROW_TOP_LEFT_CORNER_X, "      ||\n");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 8, ARROW_TOP_LEFT_CORNER_X, "      ||\n");
    refresh();
}

void clearArrow(void) {
    for (int i = 0; i < ARROW_HEIGHT + 10; i++)
        mvprintw(ARROW_TOP_LEFT_CORNER_Y + i, ARROW_TOP_LEFT_CORNER_X, "                   ");

    refresh();
}

void drawDownArrow(void) {

    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 0, ARROW_TOP_LEFT_CORNER_X, "      ||");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 1, ARROW_TOP_LEFT_CORNER_X, "      ||");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 2, ARROW_TOP_LEFT_CORNER_X, "      ||");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 3, ARROW_TOP_LEFT_CORNER_X, "      ||");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 4, ARROW_TOP_LEFT_CORNER_X, "      ||");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 5, ARROW_TOP_LEFT_CORNER_X, "  \\   ||   /");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 6, ARROW_TOP_LEFT_CORNER_X, "   \\  ||  /");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 7, ARROW_TOP_LEFT_CORNER_X, "    \\ || /");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 8, ARROW_TOP_LEFT_CORNER_X, "     \\||/");

    refresh();
}

void drawRightArrow(void) {

    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 0, ARROW_TOP_LEFT_CORNER_X, "          \\\\   ");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 1, ARROW_TOP_LEFT_CORNER_X, "           \\\\  ");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 2, ARROW_TOP_LEFT_CORNER_X, "            \\\\ ");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 3, ARROW_TOP_LEFT_CORNER_X, "             \\\\");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 4, ARROW_TOP_LEFT_CORNER_X, "===============  ");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 5, ARROW_TOP_LEFT_CORNER_X, "             //  ");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 6, ARROW_TOP_LEFT_CORNER_X, "            //   ");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 7, ARROW_TOP_LEFT_CORNER_X, "           //    ");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 8, ARROW_TOP_LEFT_CORNER_X, "          //     ");
    refresh();
}

void drawLeftArrow(void) {

    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 0, ARROW_TOP_LEFT_CORNER_X, "   //           ");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 1, ARROW_TOP_LEFT_CORNER_X, "  //            ");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 2, ARROW_TOP_LEFT_CORNER_X, " //             ");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 3, ARROW_TOP_LEFT_CORNER_X, "//              ");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 4, ARROW_TOP_LEFT_CORNER_X, "================");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 5, ARROW_TOP_LEFT_CORNER_X, "\\\\            ");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 6, ARROW_TOP_LEFT_CORNER_X, " \\\\           ");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 7, ARROW_TOP_LEFT_CORNER_X, "  \\\\          ");
    mvprintw(ARROW_TOP_LEFT_CORNER_Y + 8, ARROW_TOP_LEFT_CORNER_X, "   \\\\         ");
    refresh();
}

void drawInstruction(void) {
    mvprintw(INSTRUCTION_TOP_LEFT_CORNER_Y + 0, INSTRUCTION_TOP_LEFT_CORNER_X, "Use arrows to control the snake");
    mvprintw(INSTRUCTION_TOP_LEFT_CORNER_Y + 1, INSTRUCTION_TOP_LEFT_CORNER_X, "Avoid hitting walls or yourself");
    mvprintw(INSTRUCTION_TOP_LEFT_CORNER_Y + 2, INSTRUCTION_TOP_LEFT_CORNER_X, "Press q to end the game");
    mvprintw(INSTRUCTION_TOP_LEFT_CORNER_Y + 3, INSTRUCTION_TOP_LEFT_CORNER_X, "Press p to pause");

    refresh();
}

void generateApple(void) {
    int x, y;
    do {
        x = rand() % (MAP_WIDTH - 1) + INITIAL_MAP_TOP_LEFT_CORNER_X;
        y = rand() % (MAP_HEIGHT - 1) + INITIAL_MAP_TOP_LEFT_CORNER_Y;
    } while (checkIfFieldFree(x, y));

    apple->x = x;
    apple->y = y;

    init_pair(1, COLOR_RED, COLOR_YELLOW);
    attron(COLOR_PAIR(1));
    pthread_mutex_lock(&terminalMutex);
    mvaddch(y, x, APPLE);
    pthread_mutex_unlock(&terminalMutex);
    attroff(COLOR_PAIR(1));

    refresh();
}

int checkIfFieldFree(int x, int y) {
    struct node_t *tempList = snake->head;
    for (int i = 0; i < snake->size; i++) {
        if (snake->head->component.x == x && snake->head->component.y == y)
            return 1;

        tempList = tempList->next;
    }

    return 0;
}

int detectCollision(void) {
    return didHitHimself() || didHitWall();
}

int didHitHimself(void) {
    int x = snake->head->component.x;
    int y = snake->head->component.y;
    struct node_t *tempNode = snake->head->next->next;
    for (int i = 2; i < snake->size; i++) {
        if (tempNode->component.x == x && tempNode->component.y == y)
            return 1;
        tempNode = tempNode->next;
    }

    return 0;
}

int didHitWall(void) {
    if (snake->head->component.x == INITIAL_MAP_TOP_LEFT_CORNER_X || snake->head->component.x == INITIAL_MAP_TOP_LEFT_CORNER_X + MAP_WIDTH)
        return 1;
    if (snake->head->component.y == INITIAL_MAP_TOP_LEFT_CORNER_Y || snake->head->component.y == INITIAL_MAP_TOP_LEFT_CORNER_Y + MAP_HEIGHT)
        return 1;
    return 0;
}

void gameOver(void) {
    sem_post(&endGameSemaphore);
}

int didEatApple(void) {
    return snake->head->component.x == apple->x && snake->head->component.y == apple->y;
}

int didEatBooster(void) {
    return snake->head->component.x == booster->x && snake->head->component.y == booster->y;
}

void applyBooster(void) {
    if (booster->boosterType == DOUBLE_POINTS)
        isDoubled = 1;

    else if (booster->boosterType == SLOWER_SNAKE)
        roundDuration *= 2;

    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_function = stopBooster;
    sev.sigev_value.sival_ptr = NULL;

    timer_create(CLOCK_REALTIME, &sev, &timerid);

    its.it_value.tv_sec = BOOSTER_DURATION;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;

    timer_settime(timerid, 0, &its, NULL);

}

void stopBooster(__attribute__((unused)) union sigval sv) {
    if (booster->boosterType == DOUBLE_POINTS)
        isDoubled = 0;

    else if (booster->boosterType == SLOWER_SNAKE)
        roundDuration /= 2;

    sem_post(&boosterSemaphore);
}

int eatApple() {
    list_push(snake, snake->tail->component.x, snake->tail->component.y);

    generateApple();

    if (isDoubled)
        statistics->applesEaten++;
    mvprintw(STATISTICS_TOP_LEFT_CORNER_Y + 2, STATISTICS_TOP_LEFT_CORNER_X + 15,  "%d", ++statistics->applesEaten);
    return 0;
}

void moveSnake(void) {
    mvaddch(snake->tail->component.y, snake->tail->component.x, ' ');
    mvprintw(20, 2, "Y: %d X:%d", snake->tail->component.y, snake->tail->component.x);

    switch (currentSnakeDirection) {
        case KEY_RIGHT:
            mvaddch(snake->head->component.y, snake->head->component.x + 1, RECTANGLE);
            break;
        case KEY_LEFT:
            mvaddch(snake->head->component.y, snake->head->component.x - 1, RECTANGLE);
            break;
        case KEY_DOWN:
            mvaddch(snake->head->component.y + 1, snake->head->component.x, RECTANGLE);
            break;
        case KEY_UP:
            mvaddch(snake->head->component.y - 1, snake->head->component.x, RECTANGLE);
            break;
        default:
            ;  // TODO err
    }
    refresh();
    updateSnakePointPosition();
    if (didEatApple())
        eatApple();
    if (didEatBooster())
        applyBooster();

    refresh();
}


static void transferPosition(const struct node_t *sourceComponent, struct node_t *destComponent) {
    if (sourceComponent == NULL || destComponent == NULL)
        return;

    destComponent->component.x = sourceComponent->component.x;
    destComponent->component.y = sourceComponent->component.y;
}

void updateSnakePointPosition(void) {
    struct node_t *temp = snake->head;

    for (int i = snake->size - 1; i >= 0; i--) {
        struct node_t *src = list_get_node_by_index(snake, i);
        transferPosition(src, src->next);
        if (temp)
            temp = temp->next;
    }

    switch (currentSnakeDirection) {
        case KEY_RIGHT:
            snake->head->component.x++;
            break;
        case KEY_LEFT:
            snake->head->component.x--;
            break;
        case KEY_DOWN:
            snake->head->component.y++;
            break;
        case KEY_UP:
            snake->head->component.y--;
            break;
        default:
            ;  // TODO err
    }

}

void markOption(int choise) {
    attroff(A_REVERSE);
    mvprintw(7, 5, "New Game");
    mvprintw(8, 5, "Records (TODO)");
    mvprintw(9, 5, "Quit");
    attron(A_REVERSE);
    if (choise == 0)
        mvprintw(7, 5, "New Game");
    else if (choise == 1)
        mvprintw(8, 5, "Records (TODO)");
    else
        mvprintw(9, 5, "Quit");
    attroff(A_REVERSE);
    refresh();
}

int showWelcomeScreen(void) {

    initscr();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(FALSE);

    int ch, menuAt = 0;
    box(stdscr, '|', '-');
    attron(A_BOLD);
    mvprintw(5, 5, "Welcome to the snake game!");
    attron(A_REVERSE);
    mvprintw(7, 5, "New Game");
    attroff(A_REVERSE);
    mvprintw(8, 5, "Records (TODO)");
    mvprintw(9, 5, "Quit");

    while (1) {
        ch = getch();
        if (ch == KEY_DOWN) {
            menuAt++;
            menuAt %= 3;
        }
        if (ch == KEY_UP) {
            if (!menuAt)
                menuAt = 2;
            else --menuAt;
        }
        if (ch == 10) {
            if (menuAt == 0)
                break;
            else if (menuAt == 2) {
                clear();
                endwin();
                return 1;
            }
            else {
                ;  // TODO
            }
        }
        markOption(menuAt);
        refresh();
    }

    attroff(A_BOLD);
    refresh();
    clear();
    endwin();

    return 0;
}

int startGame(void) {
    if (showWelcomeScreen())
        return EXIT_SUCCESS;

    srand((unsigned int) time(NULL));

    createSnake();
    if (!snake)
        return EXIT_FAILURE;

    prepareMap();
    drawInstruction();
    prepareStatistics();
    printStatistics();
    apple = malloc(sizeof(apple_t));
    if (!apple)
        return EXIT_FAILURE;

    generateApple();

    booster = calloc(1, sizeof(booster_t));

    pthread_mutex_init(&snakeDirectionMutex, NULL);
    pthread_mutex_init(&terminalMutex, NULL);
    sem_init(&boosterSemaphore, 0, 0u);
    sem_init(&endGameSemaphore, 0, 0u);

    pthread_create(&game, NULL, &snakeController, NULL);
    pthread_create(&input, NULL, &inputReader, NULL);
    pthread_create(&gameTimer, NULL, &timerController, NULL);
    pthread_create(&boosterThread, NULL, &boosterGenerator, NULL);
    pthread_create(&endGameThread, NULL, &waitForGameOver, NULL);

    pthread_join(endGameThread, NULL);

    return EXIT_SUCCESS;
}

void prepareStatistics(void) {
    statistics = malloc(sizeof(statistics_t));
    statistics->bestTime = 0;  // TODO
    statistics->snakeSize = INITIAL_SIZE;
    statistics->timeMinutes = 0;
    statistics->timeSeconds = 0;
    statistics->username = "";  // TODO
}

void prepareMap(void) {
    initscr();
    keypad(stdscr, TRUE);
    curs_set(FALSE);
    noecho();
    drawMap();
    drawSnake();

    refresh();
}

void drawMap(void) {

    for (int i = 0; i < MAP_HEIGHT + 1; i++)
        mvaddch(INITIAL_MAP_TOP_LEFT_CORNER_Y + i, INITIAL_MAP_TOP_LEFT_CORNER_X, RECTANGLE);

    for (int i = 0; i < MAP_HEIGHT + 1; i++)
        mvaddch(INITIAL_MAP_TOP_LEFT_CORNER_Y + i, INITIAL_MAP_TOP_LEFT_CORNER_X + MAP_WIDTH, RECTANGLE);

    for (int i = 0; i < MAP_WIDTH; i++)
        mvaddch(INITIAL_MAP_TOP_LEFT_CORNER_Y, INITIAL_MAP_TOP_LEFT_CORNER_X + i, RECTANGLE);

    for (int i = 0; i < MAP_WIDTH; i++)
        mvaddch(INITIAL_MAP_TOP_LEFT_CORNER_Y + MAP_HEIGHT, INITIAL_MAP_TOP_LEFT_CORNER_X + i, RECTANGLE);

}

void drawSnake(void) {
    struct node_t *node = snake->head;
    for (int i = 0; i < snake->size; i++) {
        pthread_mutex_lock(&terminalMutex);
        mvaddch(node->component.y, node->component.x, RECTANGLE);
        pthread_mutex_unlock(&terminalMutex);

        node = node->next;
    }

    refresh();
}

int endGame(void) {
    // TODO free list rec

    pthread_cancel(game);
    pthread_cancel(gameTimer);
    pthread_cancel(input);
    pthread_cancel(boosterThread);
    free(snake);
    free(statistics);
    free(apple);
    pthread_mutex_destroy(&snakeDirectionMutex);
    pthread_mutex_destroy(&terminalMutex);
    sem_destroy(&boosterSemaphore);
    sem_destroy(&endGameSemaphore);

    return EXIT_SUCCESS;
}

struct linked_list_t *createSnake(void) {
    snake = malloc(sizeof(snake_t));
    if (!snake)
        return NULL;

    snake = listCreate();

    return snake;
}
