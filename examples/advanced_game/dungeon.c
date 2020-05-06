#include "dungeon.h"

/// Runtime functions
void dungeon_update(dungeon_t* dungeon, time_s delta);
void room_update(room_t* room, time_s delta);

/// Render functions
void dungeon_render(dungeon_t* dungeon, time_s delta);
void room_render(room_t* room, time_s delta);

/// Exit functions
void room_destroy(room_t* room);
void dungeon_destroy(dungeon_t* dungeon);

/// Creation functions

// Just so we can track how we're passing things
gen_config gen_config_create(int room_min_width, int room_max_width,
                             int room_min_height, int room_max_height,
                             int min_rooms, int max_rooms, int max_room_tries) {
  return (gen_config){.room_min_width  = room_min_width,
                      .room_max_width  = room_max_width,
                      .room_min_height = room_min_height,
                      .room_max_height = room_max_height,
                      .min_rooms       = min_rooms,
                      .max_rooms       = max_rooms,
                      .max_room_tries  = max_room_tries};
}

/* Returns the direction of the wall it's on from center */
dungeon_t dungeon_new(gen_config conf, long seed) {
  // Validate the gen_config
  if (conf.room_min_width > conf.room_max_width ||
      conf.room_min_width == conf.room_max_width ||
      conf.room_min_height > conf.room_max_height ||
      conf.room_min_height == conf.room_max_height ||
      conf.min_rooms > conf.max_rooms || conf.max_rooms - conf.min_rooms < 3 ||
      conf.max_room_tries == 0) {
    printf("Invalid gen_config passed.\n");
    return (dungeon_t){0};
  }

  int room_count = rnd_range(conf.min_rooms, conf.max_rooms);

  dungeon_t new_dungeon = (dungeon_t){0};

  int rooms_placed = 0;
  for (int i = 0; i < room_count; ++i) {
    int room_x, room_y, room_width, room_height;

    room_width  = rnd_range(conf.room_min_width, conf.room_max_width);
    room_height = rnd_range(conf.room_min_height, conf.room_max_height);

    if (rooms_placed == 0) {
      room_x = 0;
      room_y = 0;
    } else {
      int neighbor = rnd_range(0, rooms_palced);
    }
  }

  return new_dungeon;
}

void room_decorate(room_t* room);
void room_init_physics(room_t* room);

float room_closest_door(room_t* room, int x, int y);

/// Utility functions
int room_contains(room_t* room, vec2 point);
int room_tile_is_wall(room_t* room, int x, int y);

// A is the room being placed, B is the room already existing
int room_intersects_room(room_t* a, room_t* b, vec2 solve) {
  if (!a || !b) {
    printf("room_intersects_room: invalid parameters passed.\n");
    return -1;
  }

  c_aabb a_col, b_col;

  a_col.center[0] = a->x + (a->width / 2);
  a_col.center[1] = a->y + (a->height / 2);

  a_col.halfsize[0] = a->width / 2;
  a_col.halfsize[1] = a->height / 2;

  b_col.center[0] = b->x + (b->width / 2);
  b_col.center[1] = b->y + (b->height / 2);

  b_col.halfsize[0] = b->width / 2;
  b_col.halfsize[1] = b->height / 2;

  c_man manifold;

  int result = c_aabb_vs_aabb(&manifold, a, b);
  if (result) {
    solve[0] = manifold.penetration * manifold.normal[0];
    solve[1] = manifold.penetration * manifold.normal[1];

    return 1;
  }

  return 0;
}

/*// A is the room being placed, B is the room already existing
int room_intersects_room(room_t* a, room_t* b, vec2 solve);
 *
  vec2 n, closest;
  vec2_sub(n, a.center, b.center);
  vec2_dup(closest, n);

  closest[0] = fclamp(closest[0], -a.halfsize[0], a.halfsize[0]);
  closest[1] = fclamp(closest[1], -a.halfsize[1], a.halfsize[1]);

  int inside = 0;

  if (vec2_cmp(closest, n)) {
    inside = 1;
    if (_fabsf(n[0]) > _fabsf(n[1])) {
      closest[0] = _fabsf(a.halfsize[0]);
    } else {
      if (closest[1] > 0) {
        closest[1] = _fabsf(a.halfsize[1]);
      }
    }
  }

  vec2 normal;
  vec2_sub(normal, n, closest);

  float lensq = vec2_lensq(normal);
  float rad   = b.radius;

  if (lensq > rad * rad && !inside) {
    return 0;
  }

  if (man) {
    if (inside) {
      man->normal[0] = normal[0] * -1.f;
      man->normal[1] = normal[1] * -1.f;
      man->pen       = rad - lensq;
    } else {
      vec2_dup(man->normal, normal);
      man->pen = rad - lensq;
    }
  }

  return 1;
 */
