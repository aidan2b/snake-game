#include "snake.h"

#define WINDOW_X 0
#define WINDOW_Y 0
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000

#define GRID_SIZE 20
#define GRID_DIM 1000

enum {
    SNAKE_UP,
    SNAKE_DOWN,
    SNAKE_LEFT,
    SNAKE_RIGHT,
};

typedef struct {
    int x;
    int y;
} apple;

apple Apple;

struct snake {
    int x;
    int y;
    int dir;

    struct snake *next;
};
typedef struct snake Snake;

Snake *head;
Snake *tail;

void init_snake() {
    head = tail = malloc(sizeof(Snake));
    int initialDir = rand() % 4; // Randomly choose a direction from 0 to 3

    *head = (Snake){ .x = rand() % (GRID_SIZE / 2) + (GRID_SIZE / 4),
                     .y = rand() % (GRID_SIZE / 2) + (GRID_SIZE / 4),
                     .dir = initialDir, .next = NULL };

    for(int i = 0; i < 3; i++) { // Initialize the trailing body cells in the opposite direction
        Snake *new = malloc(sizeof(Snake));

        switch(initialDir) {
            case SNAKE_UP:
                new->x = head->x;
                new->y = head->y + 1 + i;
                break;
            case SNAKE_DOWN:
                new->x = head->x;
                new->y = head->y - 1 - i;
                break;
            case SNAKE_LEFT:
                new->x = head->x + 1 + i;
                new->y = head->y;
                break;
            case SNAKE_RIGHT:
                new->x = head->x - 1 - i;
                new->y = head->y;
                break;
        }

        new->dir = initialDir;
        new->next = NULL;

        tail->next = new;
        tail = new;
    }
}


void increase_snake() {
    Snake *new = malloc(sizeof(Snake));
    new->x = tail->x - (tail->dir == SNAKE_LEFT) + (tail->dir == SNAKE_RIGHT);
    new->y = tail->y - (tail->dir == SNAKE_DOWN) + (tail->dir == SNAKE_UP);
    new->dir = tail->dir; // Assigning the direction of the tail to the new segment
    new->next = NULL;
    tail->next = new;
    tail = new;
}

void move_snake() {
    int prev_x = head->x, prev_y = head->y, prev_dir = head->dir;
    head->x += (head->dir == SNAKE_RIGHT) - (head->dir == SNAKE_LEFT);
    head->y += (head->dir == SNAKE_DOWN) - (head->dir == SNAKE_UP);

    for(Snake *track = head->next; track; track = track->next) {
        int save_x = track->x, save_y = track->y, save_dir = track->dir;
        *track = (Snake){ .x = prev_x, .y = prev_y, .dir = prev_dir, .next = track->next };
        prev_x = save_x, prev_y = save_y, prev_dir = save_dir;
    }
}

void gen_apple() {
    for(;;) {
        bool in_snake = false;
        Apple.x = rand() % GRID_SIZE;
        Apple.y = rand() % GRID_SIZE;

        for(Snake *track = head; track != NULL; track = track->next)
            if(track->x == Apple.x && track->y == Apple.y) {
                in_snake = true;
                break;
            }

        if(!in_snake) break;
    }
}

void reset_snake() {
    for(Snake *track = head; track; track = head) {
        head = head->next;
        free(track);
    }
    init_snake();
    gen_apple(); // Respawning the apple in a new location when the snake resets
}


void render_snake(SDL_Renderer *renderer, int x, int y) {
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    int seg_size = GRID_DIM / GRID_SIZE;

    for(Snake *track = head; track; track = track->next)
        SDL_RenderFillRect(renderer, &(SDL_Rect){ .x = x + track->x * seg_size,
                                                  .y = y + track->y * seg_size,
                                                  .w = seg_size, .h = seg_size });
}


void render_grid(SDL_Renderer *renderer, int x, int y, bool drawOutline) {
    SDL_SetRenderDrawColor(renderer, 0x55, 0x55, 0xff, 255);

    if (drawOutline) {
        SDL_Rect outline = { x, y, GRID_DIM, GRID_DIM };
        SDL_RenderDrawRect(renderer, &outline);
    } else {
        int cell_size = GRID_DIM / GRID_SIZE;
        SDL_Rect cell = { x, y, cell_size, cell_size };

        for(int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                cell.x = x + i * cell_size;
                cell.y = y + j * cell_size;
                SDL_RenderDrawRect(renderer, &cell);
            }
        }
    }
}


void render_apple(SDL_Renderer *renderer, int x, int y) {
    SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 255);
    SDL_RenderFillRect(renderer, &(SDL_Rect){.w = GRID_DIM / GRID_SIZE, .h = GRID_DIM / GRID_SIZE, .x = x + Apple.x * (GRID_DIM / GRID_SIZE), .y = y + Apple.y * (GRID_DIM / GRID_SIZE)});
}

void detect_apple() {
    if(head->x == Apple.x && head->y == Apple.y){
        gen_apple();
        increase_snake();
    }
}

void detect_crash()
{
    if(head->x < 0 || head->x >= GRID_SIZE || head->y < 0 || head->y >= GRID_SIZE) {
        reset_snake();
    }

    Snake *track = head->next;

    while (track != NULL) {
        if (track->x == head->x && track->y == head->y) {
            reset_snake();
        }
        track = track->next;
    }

    return;
}

int initialize_SDL(SDL_Window** window, SDL_Renderer** renderer) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "ERROR: %s\n", SDL_GetError());
        return -1;
    }

    *window = SDL_CreateWindow("Snake", WINDOW_X, WINDOW_Y, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_BORDERLESS);
    if (!*window) {
        fprintf(stderr, "ERROR: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (!*renderer) {
        fprintf(stderr, "ERROR: %s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return -1;
    }
    return 0;
}


int main(int argc, char *argv[]) {
    srand(time(0));
    init_snake();
    gen_apple();

    SDL_Window *window;
    SDL_Renderer *renderer;

    if (initialize_SDL(&window, &renderer) < 0)
        return 1;

    int grid_x = (WINDOW_WIDTH / 2) - (GRID_DIM / 2);
    int grid_y = (WINDOW_HEIGHT / 2) - (GRID_DIM / 2);
    bool quit = false;
    SDL_Event event;

    while(!quit) {
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT) quit = true;
            if(event.type == SDL_KEYDOWN) {
                switch(event.key.keysym.sym) {
                    case SDLK_ESCAPE: quit=true; break;
                    case SDLK_UP: head->dir = SNAKE_UP; break;
                    case SDLK_DOWN: head->dir = SNAKE_DOWN; break;
                    case SDLK_LEFT: head->dir = SNAKE_LEFT; break;
                    case SDLK_RIGHT: head->dir = SNAKE_RIGHT; break;
                }
            }
        }

        SDL_RenderClear(renderer);
        move_snake(); detect_apple(); detect_crash();
        render_grid(renderer, grid_x, grid_y,true);
        render_apple(renderer, grid_x, grid_y);
        render_snake(renderer, grid_x, grid_y);
        SDL_SetRenderDrawColor(renderer, 0x11, 0x11, 0x11, 255);
        SDL_RenderPresent(renderer);
        SDL_Delay(100);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
