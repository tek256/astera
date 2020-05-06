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

void game_render_tile(int tile_type, int x, int y) {
  //
}

void game_start() {
  r_init_anim_map(8);
  r_init_shader_map(1);
  r_init_batches(1);

  // camera size in worldspace (units)
  r_cam_set_size(320, 240);

  asset_t* vert_data = asset_get("resources/shaders/main.vert");
  asset_t* frag_data = asset_get("resources/shaders/main.frag");

  game_shader = r_shader_create(vert_data.data, vert_data.data_length,
                                frag_data.data, frag_data.data_length);

  asset_free(vert_data);
  asset_free(frag_data);

  asset_t* sheet_data = asset_get("resources/textures/dungeon_tileset.png");
  tile_sheet = r_sheet_create(sheet_data, sheet_data.data_length, 16, 16);
  asset_free(sheet_data);

  asset_t* sprite_data = asset_get("resources/textures/dungeon_sprites.png");
  sprite_sheet = r_sheet_create(sprite_data, sprite_data.data_length, 16, 16);
  asset_free(sprite_data);

  // TODO --- load animations from file (or just manually load them)

  game_state = GAME_GAME;
  level_new(time(NULL));
}

void game_exit() {
  game_state = GAME_EXIT;
  world_exit();
}

void level_exit(level_t* level) {
  r_baked_sheet_destroy(level->bg);
  free(level->tiles);
  free(level->enemies);

  for (int i = 0; i < level->room_count; ++i) {
    free(level->rooms[i].doors);
    free(level->rooms[i].door_walls);
    free(level->rooms[i].walls);
  }

  free(level->rooms);
}

static float closest_door(room_t* room, int x, int y) {
  float closest_dist  = FLOAT_MAX;
  int   closest_index = -1;
  for (int i = 0; i < room->door_count; ++i) {
    int   dist_x = room->doors[i].x - x;
    int   dist_y = room->doors[i].y - y;
    float dist   = sqrt(dist_x * dist_x + dist_y * dist_y);

    if (dist < closest_dist) {
      closest_dist = dist;
    }
  }

  return closest_dist;
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

        // for variation in positioning, and not just direct stacking
        int room_half_width  = room_width / 2,
            room_half_height = room_height / 2;

        if (mirror_dir == DIR_NORTH) {
          room_x = last_room->x +
                   rnd_range(-(room_half_width - 1), room_half_width - 1);
          room_y = door_y - room_height;
          new_door_y += 1;
        } else if (mirror_dir == DIR_SOUTH) {
          room_x =
              last_room->x + rnd_range(-room_half_width, room_half_width - 1);
          room_y = door_y + room_height;
          new_door_y -= 1;
        } else if (mirror_dir == DIR_EAST) {
          room_x = door_x - room_width;
          room_y = last_room->y +
                   rnd_range(-(room_half_height - 1), room_half_height - 1);
          new_door_x += 1;
        } else if (mirror_dir == DIR_WEST) {
          room_y = last_room->y +
                   rnd_range(-(room_half_height - 1), room_half_height - 1);
          room_x = door_x + room_width;
          new_door_x -= 1;
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

  if (!rooms_placed) {
    printf("No rooms placed in dungeon generation, failing.\n");
    return;
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

  int start_room = rnd_range(0, rooms_placed);
  int end_room   = rnd_range(0, rooms_placed);

  int tries     = 0;
  int max_tries = 3;
  while (end_room == start_room) {
    if (tries == max_tries) {
      printf("Unable to assign end room, what's happening to me!?!?!\n");
      break;
    }

    end_room = rnd_range(0, rooms_placed);
    ++tries;
  }

  // shift all rooms to lowest possible bounds
  // as well as assign types
  room_t* last_room = 0;
  for (int i = 0; i < rooms_placed; ++i) {
    // shift all rooms by min offset (memory safety)
    if (min_x != 0 || min_y != 0) {
      rooms[i].x -= min_x;
      rooms[i].y -= min_y;
    }

    int room_type = 0;
    if (i == start_room) {
      room_type = ROOM_START;
    } else if (i == end_room) {
      room_type = ROOM_END;
    } else {
      // all possible types except for room_start & room_end
      room_type = rnd_range(ROOM_EMPTY, ROOM_AMBUSH);
    }

    room_t* room = &rooms[i];
    room->type   = room_type;

    switch (room_type) {
      case ROOM_START: {
        int start_x   = rnd_range(room->x, room->x + room->width),
            start_y   = rnd_range(room->y, room->y + room->height);
        float closest = closest_door(room, start_x, start_y);

        int start_works = 0;
        if (closest < 1.5f) {
          // try at least 3 times before breaking
          for (int t = 0; t < 3; ++t) {
            start_x = rnd_range(room->x, room->x + room->width);
            start_y = rnd_range(room->y, room->y + room->height);
            closest = closest_door(room, start_x, start_y);

            if (closest > 1.5f) {
              start_works = 1;
              break;
            }
          }

          if (!start_works) {
            printf("Unable to place start point.\n");
            // TODO clean exit
            return;
          }
        } else {
          start_works = 1;
        }

        for (int x = 0; x < room->width; ++x) {
          for (int y = 0; y < room->height; ++y) {
            int new_tile_type = TILE_NONE;

            if (x != start_x && y != start_y) {
              if (x == 0) {
                new_tile_type = TILE_WALL;

              } else if (x == room->width - 1) {
                new_tile_type = TILE_WALL;
              } else if (y == 0) {
                new_tile_type = TILE_WALL;
              } else if (y == room->height - 1) {
                new_tile_type = TILE_WALL;
              } else {
                new_tile_type = TILE_FLOOR;
              }
            } else {
              new_tile_type = TILE_START;
            }

            tiles[(x + room->x) + (y + room->y) * new_level.width] =
                new_tile_type;
          }
        }
      } break;
      case ROOM_END: {
        int end_x         = rnd_range(room->x, room->x + room->width),
            end_y         = rnd_range(room->y, room->y + room->height);
        float end_closest = closest_door(room, end_x, end_y);

        int end_works = 0;
        if (closest < 1.5f) {
          // try at least 3 times before breaking
          for (int t = 0; t < 3; ++t) {
            end_x       = rnd_range(room->x, room->x + room->width);
            end_y       = rnd_range(room->y, room->y + room->height);
            end_closest = closest_door(room, end_x, end_y);

            if (end_closest > 1.5f) {
              end_works = 1;
              break;
            }
          }

          if (!end_works) {
            printf("Unable to place end point.\n");
            // TODO clean exit
            return;
          }
        } else {
          end_works = 1;
        }

        for (int x = 0; x < room->width; ++x) {
          for (int y = 0; y < room->height; ++y) {
            int new_tile_type = TILE_NONE;

            if (x != end_x && y != end_y) {
              if (x == 0) {
                new_tile_type = TILE_WALL;

              } else if (x == room->width - 1) {
                new_tile_type = TILE_WALL;
              } else if (y == 0) {
                new_tile_type = TILE_WALL;
              } else if (y == room->height - 1) {
                new_tile_type = TILE_WALL;
              } else {
                new_tile_type = TILE_FLOOR;
              }
            } else {
              new_tile_type = TILE_END;
            }

            tiles[(x + room->x) + (y + room->y) * new_level.width] =
                new_tile_type;
          }
        }
      } break;
      case ROOM_PIT: {
        int quarter_width  = room->width / 4;
        int quarter_height = room->height / 4;
        int walkway_size =
            rnd_range(2, (quarter_width > quarter_height) ? quarter_height
                                                          : quarter_width);

        for (int x = 0; x < room->width; ++x) {
          for (int y = 0; y < room->height; ++y) {
            int new_tile_type = TILE_NONE;

            if (x != end_x && y != end_y) {
              if (x == 0) {
                new_tile_type = TILE_WALL;

              } else if (x == room->width - 1) {
                new_tile_type = TILE_WALL;
              } else if (y == 0) {
                new_tile_type = TILE_WALL;
              } else if (y == room->height - 1) {
                new_tile_type = TILE_WALL;
              } else {
                if (x > walkway_size && x < room->width - walkway_size &&
                    y > walkway_size && y < room->height - walkway_size) {
                  new_tile_type = TILE_PIT;
                } else {
                  new_tile_type = TILE_FLOOR;
                }
              }
            } else {
              new_tile_type = TILE_END;
            }

            tiles[(x + room->x) + (y + room->y) * new_level.width] =
                new_tile_type;
          }
        }

      } break;
      case ROOM_EMPTY: {
        for (int x = 0; x < room->width; ++x) {
          for (int y = 0; y < room->height; ++y) {
            int new_tile_type = TILE_FLOOR;

            if (x != end_x && y != end_y) {
              if (x == 0) {
                new_tile_type = TILE_WALL;

              } else if (x == room->width - 1) {
                new_tile_type = TILE_WALL;
              } else if (y == 0) {
                new_tile_type = TILE_WALL;
              } else if (y == room->height - 1) {
                new_tile_type = TILE_WALL;
              } else {
                new_tile_type = TILE_FLOOR;
              }
            }

            tiles[(x + room->x) + (y + room->y) * new_level.width] =
                new_tile_type;
          }
        }
      } break;
      case ROOM_AMBUSH: {
        for (int x = 0; x < room->width; ++x) {
          for (int y = 0; y < room->height; ++y) {
            int new_tile_type = TILE_FLOOR;

            if (x != end_x && y != end_y) {
              if (x == 0) {
                new_tile_type = TILE_WALL;

              } else if (x == room->width - 1) {
                new_tile_type = TILE_WALL;
              } else if (y == 0) {
                new_tile_type = TILE_WALL;
              } else if (y == room->height - 1) {
                new_tile_type = TILE_WALL;
              } else {
                new_tile_type = TILE_FLOOR;
              }
            }

            tiles[(x + room->x) + (y + room->y) * new_level.width] =
                new_tile_type;
          }
        }

        int  spawns        = rnd_range(2, 6);
        int* spawns_x      = (int*)malloc(sizeof(int) * spawns);
        int* spawns_y      = (int*)malloc(sizeof(int) * spawns);
        int  spawns_placed = 0;
        for (int i = 0; i < spawns; ++i) {
          int spawn_x = rnd_except(room->x + 1, room->x + room->width - 1,
                                   spawns_x, spawns_placed);

          int spawn_y = rnd_except(room->y + 1, room->y + room->height - 1,
                                   spawns_y, spawns_placed);

          spawns_x[spawns_placed] = spawn_x;
          spawns_y[spawns_placed] = spawn_y;
          ++spawns_placed;
        }
      } break;
    }
  }

  int baked_quad_count = 0;
  for (int i = 0; i < tiles[i]; ++i) {
    if (tiles[i] != TILE_EMPTY) {
      ++baked_quad_count;
    }
  }

  r_baked_quad* baked_quads =
      (r_baked_quad*)malloc(sizeof(r_baked_quad) * baked_quad_count);

  int baked = 0;
  for (int i = 0; i < tile_count; ++i) {
    if (tiles[i] != TILE_EMPTY) {
      baked_quads[baked].sub_id = tiles[i];
      baked_quads[baked].x      = i % level.width;
      baked_quads[baked].y      = i / level.width;
      baked_quads[baked].sub_id = tiles[i];
      ++baked;
    }
  }

  vec2 baked_pos = {0.f, 0.f};

  level.bg = r_baked_sheet_create(tile_sheet, baked_quads, baked_quad_count,
                                  baked_pos, 0);

  free(baked_quads);

  level.rooms      = realloc(rooms, sizeof(room_t) * rooms_placed);
  level.room_count = rooms_placed;

  level.tiles      = tiles;
  level.tile_count = tile_count;

  // disable world rendering while we load in the new one
  can_render_world = 0;

  // destroy old level
  level_destroy(&g_world.level);

  g_world.level = level;

  // allow rendering now that everything is all loaded in
  can_render_world = 1;
}

void world_update(time_s delta) {
  for (int i = 0; i < g_world.level.tile_count; ++i) {
    //
  }
}

void world_render(time_s delta) {
  if (!can_render_world)
    return;

  r_baked_sheet_draw(game_shader, &g_world.level.bg);

  for (int i = 0; i < g_world.level.enemy_count; ++i) {
    enemy_t* enemy  = g_world.level.enemies[i];
    int      tex_id = r_sprite_get_tex_id(enemy->sprite);

    r_sprite_draw(enemy->sprite);

    r_shader_sprite_uniform(sprite, tex_ids, &tex_id);
    r_shader_sprite_uniform(sprite, models, &sprite.model);
    r_shader_sprite_uniform(sprite, flip_x, &sprite.flip_x);
    r_shader_sprite_uniform(sprite, flip_y, &sprite.flip_y);
  }
}

void world_exit() {
  r_baked_sheet_destroy(&bg_sheet);
  free(g_world.level.tiles);
  free(g_world.level.enemies);
  free(g_world.level.rooms);
}
