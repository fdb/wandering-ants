#include <math.h>
#include <stdlib.h>

#include "../vendor/nanovg/nanovg.h"
#include <OpenGL/GL.h>
#include <OpenGL/gl3.h>
#include <SDL2/SDL.h>
//#include <SDL2/SDL_opengl.h>

#define NANOVG_GL3_IMPLEMENTATION
#include "../vendor/nanovg/nanovg_gl.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../vendor/stb/stb_image_write.h"

#define BUFFER_WIDTH 1920
#define BUFFER_HEIGHT 1080
#define MAX_ANTS 1000000
#define ANT_SPEED 2.0
#define ANT_INITIAL_COUNT 30
#define ANT_SPAWN_DISTANCE_REQUIRED 36
#define ANT_COOL_DOWN_TIME 25
#define ANT_RADIUS 2.0
#define TRAIL_CAPACITY 5000
#define SKIP_FRAMES 0

typedef struct {
  float x;
  float y;
} point_t;

typedef struct {
  int life;
  float x;
  float y;
  float dx;
  float dy;
  int cool_down;
  int trail_len;
  point_t *trail;
} ant_t;

typedef struct {
  ant_t *ants;
  int len;
  int capacity;
} ant_colony_t;

float rand_float(float min, float max) {
  float r = (float)rand() / RAND_MAX;
  return min + r * (max - min);
}

void render_trails(struct NVGcontext *vg, ant_colony_t *colony) {
  nvgStrokeColor(vg, nvgRGBA(0, 0, 0, 40));
  nvgBeginPath(vg);
  for (int i = 0; i < colony->len; i++) {
    ant_t *ant = &colony->ants[i];
    for (int j = 0; j < ant->trail_len; j++) {
      point_t pt = ant->trail[j];
      if (j == 0) {
        nvgMoveTo(vg, pt.x, pt.y);
      } else {
        nvgLineTo(vg, pt.x, pt.y);
      }
    }
  }
  nvgStroke(vg);
}

void render_ants(struct NVGcontext *vg, ant_colony_t *colony) {
  nvgFillColor(vg, nvgRGBA(0, 0, 0, 200));
  for (int i = 0; i < colony->len; i++) {
    ant_t *ant = &colony->ants[i];
    float r = ANT_RADIUS;
    if (ant->life < 10) {
      r = ANT_RADIUS + (10 - ant->life);
    }
    nvgBeginPath(vg);
    nvgCircle(vg, ant->x, ant->y, r);
    nvgFill(vg);
  }
}

void render(struct NVGcontext *vg, ant_colony_t *colony) {
  glEnable(GL_BLEND);
  glClearColor(0.95, 0.99, 0.95, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  nvgBeginFrame(vg, BUFFER_WIDTH, BUFFER_HEIGHT, 1);
  render_trails(vg, colony);
  render_ants(vg, colony);

  nvgEndFrame(vg);
}

void ant_random_dir(ant_t *ant) {
  float dir = rand_float(0, M_PI * 2);
  ant->dx = cos(dir) * ANT_SPEED;
  ant->dy = sin(dir) * ANT_SPEED;
}

void ant_init(ant_t *ant) {
  ant->x = BUFFER_WIDTH / 4 + rand_float(0, BUFFER_WIDTH / 2);
  ant->y = BUFFER_HEIGHT / 4 + rand_float(0, BUFFER_HEIGHT / 2);
  ;
  ant_random_dir(ant);
  ant->trail = calloc(TRAIL_CAPACITY, sizeof(point_t));
}

// Spawn a new ant. If the colony is over capacity, return NULL.
ant_t *ant_colony_spawn(ant_colony_t *colony) {
  if (colony->len >= colony->capacity) {
    return NULL;
  }
  ant_t *new_ant = &colony->ants[colony->len];
  ant_init(new_ant);
  colony->len++;
  return new_ant;
}

void ant_colony_wander(ant_colony_t *colony) {
  for (int i = 0; i < colony->len; i++) {
    ant_t *ant = &colony->ants[i];
    ant->x += ant->dx;
    ant->y += ant->dy;

    if (ant->trail_len >= TRAIL_CAPACITY) {
      memmove(ant->trail, ant->trail + 1, (ant->trail_len) * sizeof(point_t));
      ant->trail_len--;
    }
    point_t *pt = &ant->trail[ant->trail_len];
    pt->x = ant->x;
    pt->y = ant->y;
    ant->trail_len++;
    if (rand_float(0, 1) > 0.9f) {
      ant_random_dir(ant);
    }
  }
}

void ant_colony_update_state(ant_colony_t *colony) {
  for (int i = 0; i < colony->len; i++) {
    ant_t *ant = &colony->ants[i];
    if (ant->cool_down > 0) {
      ant->cool_down--;
    }
    ant->life++;
  }
}

void ant_colony_vicinity_breed(ant_colony_t *colony) {
  // New ants will spawn at the end. We will keep the current len.
  int current_len = colony->len;
  for (int i = 0; i < current_len - 1; i++) {
    for (int j = i + 1; j < current_len; j++) {
      ant_t *a = &colony->ants[i];
      ant_t *b = &colony->ants[j];
      if (a->cool_down <= 0 && b->cool_down <= 0) {
        float dsq =
            (a->x - b->x) * (a->x - b->x) + (a->y - b->y) * (a->y - b->y);
        if (dsq < ANT_SPAWN_DISTANCE_REQUIRED) {
          ant_t *new_ant = ant_colony_spawn(colony);
          if (new_ant) {
            new_ant->x = (a->x + b->x) / 2.0f;
            new_ant->y = (a->y + b->y) / 2.0f;
            new_ant->cool_down = ANT_COOL_DOWN_TIME;
          }
          a->cool_down = ANT_COOL_DOWN_TIME;
          b->cool_down = ANT_COOL_DOWN_TIME;
        }
      }
    }
  }
}

int save_ppm(int frame, uint8_t *buffer, int width, int height) {
  char filename[512];
  snprintf(filename, 512, "/Users/fdb/Desktop/_out/frame%04d.ppm", frame);
  FILE *fp = fopen(filename, "wb");
  if (!fp) {
    fprintf(stderr, "Error: could not open file %s for writing.\n", filename);
    return -1;
  }

  // Write header
  fprintf(fp, "P6\n%d %d\n255\n", width, height);

  // Write data
  fwrite(buffer, 1, width * height * 3, fp);

  fclose(fp);
  return 0;
}

int save_png(int frame, uint8_t *buffer, int width, int height) {
  char filename[512];
  snprintf(filename, 512, "/Users/fdb/Desktop/_out/frame%04d.png", frame);
  return stbi_write_png(filename, width, height, 3, buffer, width * 3);
}

int save_tga(int frame, uint8_t *buffer, int width, int height) {
  char filename[512];
  snprintf(filename, 512, "/Users/fdb/Desktop/_out/frame%04d.tga", frame);
  return stbi_write_tga(filename, width, height, 3, buffer);
}

int main() {
  // 10
  // 12
  // 32
  srand(32);

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "Could not init video.\n");
    exit(-1);
  }

  ant_colony_t ant_colony = {0};
  ant_colony.capacity = MAX_ANTS;
  ant_colony.ants = calloc(ant_colony.capacity, sizeof(ant_t));
  for (int i = 0; i < ANT_INITIAL_COUNT; i++) {
    ant_t *new_ant = ant_colony_spawn(&ant_colony);
    new_ant->life = 100;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

  SDL_Window *window = SDL_CreateWindow(
      "Wandering Ants", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      BUFFER_WIDTH / 2, BUFFER_HEIGHT / 2, SDL_WINDOW_ALLOW_HIGHDPI);

  SDL_GLContext ctx = SDL_GL_CreateContext(window);
  if (!ctx) {
    fprintf(stderr, "Could not init OpenGL: %s.\n", SDL_GetError());
  }

  if (SDL_GL_SetSwapInterval(1) < 0) {
    fprintf(stderr, "Warning: Unable to set VSync! SDL Error: %s\n",
            SDL_GetError());
  }

  struct NVGcontext *vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
  uint8_t *pixel_buffer = calloc(BUFFER_WIDTH * BUFFER_HEIGHT * 3, 1);

  int running = 1;
  int frame = 0;
  while (running) {
    printf("%4d\n", frame);
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_QUIT) {
        running = 0;
      }
    }
    for (int i = 0; i <= SKIP_FRAMES; i++) {
      ant_colony_wander(&ant_colony);
      ant_colony_update_state(&ant_colony);
      ant_colony_vicinity_breed(&ant_colony);
    }
    render(vg, &ant_colony);

    glReadPixels(0, 0, BUFFER_WIDTH, BUFFER_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE,
                 pixel_buffer);
    save_tga(frame, pixel_buffer, BUFFER_WIDTH, BUFFER_HEIGHT);
    SDL_GL_SwapWindow(window);
    frame++;
  }

  return 0;
}
