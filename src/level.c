#include "level.h"

static void level_directional(int8_t *x_offset, int8_t *y_offset,
                              int8_t index) {
  if (!y_offset || !x_offset) {
    _l("WHY WOULD YOU DO THIS?!\n");
    return;
  }

  switch (index) {
  case 0:
    *y_offset = 1;
    break;
  case 1:
    *x_offset = 1;
    *y_offset = 1;
    break;
  case 2:
    *x_offset = 1;
    *y_offset = 0;
    break;
  case 3:
    *x_offset = 1;
    *y_offset = -1;
    break;
  case 4:
    *x_offset = 0;
    *y_offset = -1;
    break;
  case 5:
    *x_offset = -1;
    *y_offset = -1;
    break;
  case 6:
    *x_offset = -1;
    *y_offset = 0;
    break;
  case 7:
    *x_offset = -1;
    *y_offset = 1;
    break;
  default:
    _l("Well, how the fuck did we end up here?\n");
    break;
  }
}

static float l_min_max_sub(float a, float b) { return (a > b) ? a - b : b - a; }

// Overall update function for level context
void level_update(l_level *level, time_s delta) {
  if (!level) {
    return;
  }

  int8_t current_step = level->step + 1;

  // Update the viewer, then everything else
  vec2 viewer_move;
  vec2_scale(viewer_move, level->viewer.velocity, delta * MS_TO_SEC);
  vec2_add(level->viewer.position, level->viewer.position, viewer_move);

  static l_section **sections;
  static uint16_t section_cap = 0, section_count;

  if (!sections) {
    section_cap = LEVEL_SECTION_TEST_CAPACITY;
    sections = (l_section **)malloc(sizeof(l_section *) * section_cap);

    if (!sections) {
      _e("Unable to allocate space for the level section test\n");
      return;
    }

    memset(sections, 0, sizeof(l_section *) * section_cap);
  }

  vec4 viewer_bounds;
  vec4_bounds(viewer_bounds, level->viewer.position, level->viewer.size);
  section_count =
      level_sections_within_bounds(sections, section_cap, level, viewer_bounds);

  for (uint16_t i = 0; i < section_count; ++i) {
    l_section *section = sections[i];

    if (section->step == current_step) {
      continue;
    }

    if (!section) {
      continue;
    }

    vec4 section_bounds;
    vec4_bounds(section_bounds, section->position, section->size);
    float section_hw = section->size[0] / 2.f;
    float section_hh = section->size[1] / 2.f;

    vec2 sect_offset;
    vec2_clear(sect_offset);

    // Update pass
    for (uint32_t j = 0; j < section->ent_count; ++j) {
      l_entity *ent = section->ents[j];

      if (ent->is_static) {
        continue;
      }

      if (vec2_nonzero(ent->velocity)) {
        vec2 to_move;
        vec2_scale(to_move, ent->velocity, delta * MS_TO_SEC);

        vec2_add(ent->position, ent->position, to_move);
        ent->pos_change = 1;

        int8_t within =
            vec2_inside_bounds(sect_offset, section_bounds, ent->position);

        if (!within) {
          // Resolve the pointer into the correct section
          l_section *correct_section =
              level_section_by_point(level, ent->position);

          // NOTE: Edge case - Entity outside of bounds on min/max edges
          if (!correct_section) {
            continue;
          }

          section->ent_count--;

          correct_section->ents[correct_section->ent_count] = ent;
          ++correct_section->ent_count;

          for (uint32_t k = j; k < section->ent_count - 1; ++k) {
            section->ents[k] = section->ents[k + 1];
            j--;
          }
        }
      }
    }

    for (uint32_t j = 0; j < section->obj_count; ++j) {
      l_object *obj = section->objs[j];

      if (obj->is_static) {
        continue;
      }

      if (vec2_nonzero(obj->velocity)) {
        vec2 to_move;
        vec2_scale(to_move, obj->velocity, delta * MS_TO_SEC);

        vec2_add(obj->position, obj->position, to_move);
        obj->pos_change = 1;

        vec4 obj_bounds;
        vec4_bounds(obj_bounds, obj->position, obj->size);

        int8_t within;

        vec4 axial_offset;

        // Note: This breaks down if we have an object larger than the size of
        // the section
        axial_offset[0] = l_min_max_sub(obj_bounds[0], section_bounds[0]);
        axial_offset[1] = l_min_max_sub(obj_bounds[1], section_bounds[1]);
        axial_offset[2] = l_min_max_sub(obj_bounds[2], section_bounds[2]);
        axial_offset[3] = l_min_max_sub(obj_bounds[3], section_bounds[3]);

        int8_t is_left, is_right, is_above, is_below;
        is_left = is_right = is_above = is_below = 0;

        if (obj_bounds[0] < section_bounds[0]) {
          is_left = axial_offset[0] != 0.f;
        }

        if (obj_bounds[1] < section_bounds[1]) {
          is_above = axial_offset[1] != 0.f;
        }

        if (obj_bounds[2] > section_bounds[2]) {
          is_right = axial_offset[2] != 0.f;
        }

        if (obj_bounds[3] > section_bounds[3]) {
          is_below = axial_offset[3] != 0.f;
        }

        int8_t add_check = 0;
        l_section *next_sect = 0;

        if (is_left) {
          within = !is_above && !is_below;
          next_sect =
              level_get_section_relative(level, section->x - 1, section->y);
        } else if (is_right) {
          within = !is_left && !is_right;
          next_sect =
              level_get_section_relative(level, section->x + 1, section->y);
        }

        if (next_sect) {
          add_check += level_section_check_add_obj(next_sect, obj);
        }

        if (is_above) {
          within = !is_left && !is_right;
          next_sect =
              level_get_section_relative(level, section->x, section->y + 1);
        } else if (is_below) {
          within = !is_left && !is_right;
          next_sect =
              level_get_section_relative(level, section->x, section->y - 1);
        }

        if (next_sect) {
          add_check += level_section_check_add_obj(next_sect, obj);
        }

        if (is_above && (is_left || is_right)) {
          if (is_left) {
            next_sect = level_get_section_relative(level, section->x - 1,
                                                   section->y + 1);
          } else if (is_right) {
            next_sect = level_get_section_relative(level, section->x + 1,
                                                   section->y + 1);
          }
        } else if (is_below && (is_left || is_right)) {
          if (is_left) {
            next_sect = level_get_section_relative(level, section->x - 1,
                                                   section->y - 1);
          } else if (is_right) {
            next_sect = level_get_section_relative(level, section->x + 1,
                                                   section->y - 1);
          }
        }

        if (next_sect) {
          add_check += level_section_check_add_obj(next_sect, obj);
        }

        // vec4_bound_overlap(sect_offset, section_bounds, obj_bounds);

        if (!within) {
          section->obj_count--;

          for (uint32_t k = j; k < section->obj_count - 1; ++k) {
            section->objs[k] = section->objs[k + 1];
            j--;
          }
        } else {
        }
      }
    }
  }

  // Resolution pass
  for (uint16_t i = 0; i < section_count; ++i) {
  }
}

// Generic switch case thing
int8_t level_col_resolve(l_col *a, l_col *b, int8_t allow_resolution) {
  switch (a->col_type) {
  case BOX:
    switch (b->col_type) {
    case BOX:
      break;
    case CIRCLE:
      break;
    case LINE:
      break;
    case COMPLEX:
      break;
    }
    break;
  case CIRCLE:
    switch (b->col_type) {
    case BOX:
      break;
    case CIRCLE:
      break;
    case LINE:
      break;
    case COMPLEX:
      break;
    }
    break;
  case LINE:
    switch (b->col_type) {
    case BOX:
      break;
    case CIRCLE:
      break;
    case LINE:
      break;
    case COMPLEX:
      break;
    }
    break;
  case COMPLEX:
    switch (b->col_type) {
    case BOX:
      break;
    case CIRCLE:
      break;
    case LINE:
      break;
    case COMPLEX:
      break;
    }
    break;
  }
}

l_entity *level_get_entity(l_level *level, uint32_t uid) {
  if (!level) {
    return 0;
  }

  for (uint32_t i = 0; i < level->ent_count; ++i) {
    if (level->ents[i].uid == uid) {
      return &level->ents[i];
    }
  }

  return 0;
}

l_object *level_get_object(l_level *level, uint32_t uid) {
  if (!level) {
    return 0;
  }

  for (uint32_t i = 0; i < level->obj_count; ++i) {
    if (level->objs[i].uid == uid) {
      return &level->objs[i].uid;
    }
  }

  return 0;
}

static int8_t level_section_test_point(l_section *section, vec2 point) {
  if (section->position[0] < point[0] &&
      section->position[0] + section->size[0] > point[0]) {
    if (section->position[1] < point[1] &&
        section->position[1] + section->size[1] > point[1]) {
      return 1;
    }
  }
  return 0;
}

l_section *level_get_section_relative(l_level *level, uint32_t x, uint32_t y) {
  for (uint32_t i = 0; i < level->section_count; ++i) {
    l_section section = level->sections[i];
    if (section.x == x && section.y == y) {
      return &level->sections[i];
    }
  }
  return 0;
}

l_section *level_section_by_point(l_level *level, vec2 point) {
  if (!level) {
    return 0;
  }

  for (uint32_t i = 0; i < level->section_count; ++i) {
    l_section *section = &level->sections[i];
    int8_t result = level_section_test_point(section, point);
    if (result != 0) {
      return section;
    }
  }

  return 0;
}

uint32_t level_sections_within_grid(l_section ***dst, uint32_t dst_max,
                                    l_level *level, uint32_t x_min,
                                    uint32_t y_min, uint32_t x_max,
                                    uint32_t y_max) {
  if (!level || !dst || !dst_max) {
    return 0;
  }

  uint32_t found = 0;

  for (uint32_t i = 0; i < level->section_count; ++i) {
    l_section *section = &level->sections[i];

    if (!section)
      continue;

    uint32_t x = section->x;
    uint32_t y = section->y;

    if (x > x_min && x < x_max && y > y_min && y < y_max) {
      dst[found] = section;
      ++found;

      if (found == dst_max - 1) {
        break;
      }
    }
  }

  return found;
}

uint32_t level_sections_within_bounds(l_section ***dst, uint32_t dst_max,
                                      l_level *level, vec4 bounds) {
  uint32_t found = 0;

  for (uint32_t i = 0; i < level->section_count; ++i) {
    l_section *section = &level->sections[i];

    if (!section)
      continue;

    if (vec4_bound_overlap(0, section, bounds)) {
      dst[found] = section;
      ++found;
    }

    if (found == dst_max - 1) {
      break;
    }
  }

  return found;
}

// 0 = didn't contain, added 1 = contained, didn't add
int8_t level_section_check_add_obj(l_section *section, l_object *obj) {
  if (!section) {
    return -1;
  }

  if (section->obj_count == section->obj_capacity) {
    return 1;
  }

  for (uint32_t i = 0; i < section->obj_count; ++i) {
    if (section->objs[i]->uid == obj->uid) {
      return 1;
    }
  }

  section->objs[section->obj_count] = obj;
  ++section->obj_count;

  return 0;
}

int8_t level_section_check_add_ent(l_section *section, l_object *ent) {
  if (!section) {
    return -1;
  }

  if (section->ent_count == section->ent_capacity) {
    return 1;
  }

  for (uint32_t i = 0; i < section->ent_count; ++i) {
    if (section->ents[i]->uid == ent->uid) {
      return 1;
    }
  }

  section->ents[section->ent_count] = ent;
  ++section->ent_count;

  return 0;
}
