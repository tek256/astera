#include "game.h"

// for date()
#include <time.h>
#include <stdlib.h>
#include <stdio.h> // free - malloc
#include <string.h>

// spin until all possibilities have been exhausted,
// if they have then just return -1
// can add parameter to do max_tries before fail as well
// instead of all possible integers in domain of [min,max]
int rnd_except(int min, int max, int* except, int except_count) {
  int tries = max - min;
  for (int i = 0; i < tries; ++i) {
    int rnd   = min + (rand() % (tries));
    int allow = 0;

    for (int j = 0; j < except_count; ++j) {
      if (except[j] == rnd) {
        allow = 0;
        break;
      }
    }

    if (allow) {
      return rnd;
    }
  }

  return -1;
}

// simple random within range of [min, max]
int rnd_range(int min, int max) { return min + (rand() % (max - min)); }

void game_start() {
  game_state = GAME_GAME;
  level_new(time(NULL));
}

void game_exit() {
  game_state = GAME_EXIT;
  level_exit();
}

void level_exit() {
  free(g_world.level.tiles);
  // free(g_world.level.enemies);
}

void level_new(long seed) {
  level_t new_level = (level_t){0};
  srand(seed);

  int room_cap =
      DUNGEON_MIN_ROOMS + (rand() % (DUNGEON_MAX_ROOMS - DUNGEON_MIN_ROOMS));
  int room_min_width  = (DUNGEON_WIDTH / room_cap) / 2;
  int room_min_height = (DUNGEON_HEIGHT / room_cap) / 2;
  int room_max_width  = (DUNGEON_WIDTH / (room_cap - 1));
  int room_max_height = (DUNGEON_WIDTH / (room_cap - 1));

  room_t* rooms = (room_t*)malloc(sizeof(room_t) * room_cap);
  memset(rooms, 0, sizeof(room_t) * room_cap);
  int rooms_placed = 0;

  if (room_min_width < 2) {
    room_min_width = 2;
  }
  if (room_min_height < 2) {
    room_min_height = 2;
  }

  if (room_max_width < 8) {
    room_max_width = 8;
  }

  if (room_max_height < 8) {
    room_max_height = 8;
  }

  room_t* last_room = 0;
  for (int i = 0; i < room_cap; ++i) {
    int room_width =
        room_min_width + (rand() % (room_max_width - room_min_width));
    int room_height =
        room_min_height + (rand() % (room_max_height - room_min_height));

    int room_x = 0, room_y = 0;

    int fail = 0;

    for (int t = 0; t < ROOM_MAX_TRIES; ++t) {
      // pick a wall to expand upon
      if (last_room) {
        int dir = -1, mirror_dir = -1;
        int door_x = -1, door_y = -1;
        int new_door_x = -1, new_door_y = -1;

        if (last_room->door_count > 0) {
          rnd_except(0, 4, last_room->door_walls, last_room->door_count);

          if (dir == -1) {
            fail = 1;
            break;
          }

          last_room->doors = realloc(
              last_room->doors, (last_room->door_count * sizeof(door_t)) + 1);
          last_room->door_walls = realloc(
              last_room->door_walls, (last_room->door_count * sizeof(int)) + 1);
          ++last_room->door_count;
        } else {
          dir = rnd_range(DIR_NORTH, DIR_WEST);

          last_room->doors      = (door_t*)malloc(sizeof(door_t));
          last_room->door_walls = (int*)malloc(sizeof(int));
          last_room->door_count = 1;
        }

        if (dir == DIR_NORTH || dir == DIR_SOUTH) {
          door_x = rnd_range(last_room->x, last_room->width);

          if (dir == DIR_NORTH) {
            door_y     = last_room->y + last_room->height;
            mirror_dir = DIR_SOUTH;
          } else { // DIR_SOUTH
            door_y     = last_room->y;
            mirror_dir = DIR_NORTH;
          }
        } else if (dir == DIR_EAST || dir == DIR_WEST) {
          door_y = rnd_range(last_room->y, last_room->height);

          if (dir == DIR_EAST) {
            door_x     = last_room->x + last_room->width;
            mirror_dir = DIR_WEST;
          } else { // DIR_WEST
            door_y     = last_room->x;
            mirror_dir = DIR_EAST;
          }
        }

        new_door_x = door_x;
        new_door_y = door_y;

        if (mirror_dir == DIR_NORTH) {
          room_y = door_y - room_height;
          new_door_y += 1;
        } else if (mirror_dir == DIR_SOUTH) {
          room_y = door_y + room_height;
          new_door_y -= 1;
        } else {
          room_y = last_room->y;
        }

        if (mirror_dir == DIR_EAST) {
          room_x = door_x - room_width;
          new_door_x += 1;
        } else if (mirror_dir == DIR_WEST) {
          room_x = door_x + room_width;
          new_door_x -= 1;
        } else {
          room_x = last_room->x;
        }

        door_t* room_doors = (door_t*)malloc(sizeof(door_t));
        int*    room_walls = (int*)malloc(sizeof(int));
        room_walls[0]      = mirror_dir;
        room_doors[0]      = (door_t){new_door_x, new_door_y};

        rooms[rooms_placed] = (room_t){
            room_x,     room_y,     room_width, room_height, ROOM_EMPTY,
            room_doors, room_walls, 1,          0,           0};
        last_room = &rooms[rooms_placed];

        ++rooms_placed;
      } else {
        room_x = DUNGEON_WIDTH / 2;
        room_y = DUNGEON_HEIGHT / 2;

        rooms[0] = (room_t){room_x, room_y, room_width, room_height, ROOM_EMPTY,
                            0,      0,      0,          0,           0};

        last_room = &rooms[0];
      }
    }
  }

  int min_x = DUNGEON_WIDTH + 1, min_y = DUNGEON_HEIGHT + 1, max_x = -1,
      max_y = -1;

  // calculate tile range
  for (int i = 0; i < rooms_placed; ++i) {
    if (rooms[i].x < min_x) {
      min_x = rooms[i].x;
    }

    if (rooms[i].y < min_y) {
      min_y = rooms[i].y;
    }

    if (rooms[i].x + rooms[i].width > max_x) {
      max_x = rooms[i].x + rooms[i].width;
    }

    if (rooms[i].y + rooms[i].height > max_y) {
      max_y = rooms[i].y + rooms[i].height;
    }
  }

  for (int i = 0; i < rooms_placed; ++i) {
    // shift all rooms by min offset (memory safety)
    if (min_x != 0 || min_y != 0) {
      rooms[i].x -= min_x;
      rooms[i].y -= min_y;
    }
  }

  new_level.width      = max_x - min_x;
  new_level.height     = max_y - min_y;
  new_level.tile_count = new_level.width * new_level.height;

  int* tiles = (int*)malloc(sizeof(int) * new_level.tile_count);

  for (int i = 0; i < rooms_placed; ++i) {
    room_t* room = &rooms[i];
    // place walls
    for (int x = 0; x < room->width; ++x) {
      for (int y = 0; y < room->height; ++y) {
        int allow = 0;

        if (room->door_count != 0) {
          for (int d = 0; d < room->door_count; ++d) {
            if (x == room->doors[d].x && y == room->doors[d].y) {
              allow = 0;
            }
          }
        }

        if (x != 0 || x != room->width || y != 0 || y != room->height) {
          allow = 1;
        } else { // floor gang
        }

        if (allow) {
          tiles[(x + room->x) + (y + room->y) * DUNGEON_WIDTH] = TILE_WALL;
        }
      }
    }
  }
}

void world_update(time_s delta) {
  for (int i = 0; i < g_world.level.tile_count; ++i) {}
}

void world_render(time_s delta) {
  for (int x = 0; x < g_world.level.width; ++x) {
    for (int y = 0; y < g_world.level.height; ++y) {
      int type = g_world.level.tiles[x + y * g_world.level.width];
    }
  }
}

void world_exit() {}
