Render Reference
==================================

Please note: This is still currently under construction and is within the early versions of Astera! If you find any errors, please open an `issue <https://github.com/tek256/astera/issues/>`_ or submit a `pull request <https://github.com/tek256/astera/compare>`_!

Function Ordering
^^^^^^^^^^^^^^^^^

This is just a general outline of the ordering of functions in order to properly render with astera's rendering pipeline. A lot of these functions are interchangable and conditional, I'll try my best to mark them as such in the list. Keep in mind that if you're not using ``r_framebuffer`` or actively using an automatically created framebuffer, you do not need to use the ``r_framebuffer_x`` functions.

**Rendering:** 

1. ``r_framebuffer_bind`` 
2. ``r_window_clear``
3. ``r_ctx_update``
4. ``r_xxx_update`` (particles, baked sheet, sprite, etc)
5. ``r_xxx_draw`` (particles, baked sheet, sprite, etc)
6. ``r_ctx_draw``
7. ``r_framebuffer_draw``
8. ``r_window_swap_buffers``

**Initialization:**

1. ``r_window_params_create`` (or create window params struct yourself)
2. ``r_ctx_create``
3. ``r_xxx_create`` (sheet, anim, baked sheet, particles, etc)
4. ``r_ctx_make_current`` (for input, optional)
5. ``r_ctx_set_i_ctx`` (for input, optional) 

Camera
^^^^^^
The camera for astera is automatically created on context creation with ``r_ctx_create`` to match the window size in pixels. You can modify the camera with the ``r_camera_x`` functions. To force update a camera's view matrix, you can call ``r_camera_update``. Otherwise the camera's view matrix is updated within the ``r_ctx_update`` function. 

NOTE: You can bypass the ``r_camera_x`` functions by getting the pointer to the camera directly with ``r_ctx_get_camera`` if you wish. 

Attributes / Uniforms
^^^^^^^^^^^^^^^^^^^^^

The vertex attributes for shaders are as follows:

.. code-block:: c

  layout(location = 0) in vec3 VERTEX_POSITION;
  layout(location = 1) in vec2 VERTEX_TEXCOORD;


Other uniforms expected with the default batching pipeline are:

.. code-block:: c

  uniform mat4 mats[]; // the model matrix of each quad
  uniform vec4 colors[]; // the colors of each quad
  uniform vec4 coords[]; // the texture coords of each quad (min, max)
  uniform int  flip_x[]; // if the quad's texcoords should flip along the x axis
  uniform int  flip_y[]; // if the quad's texcoords should flip along the y axis

  uniform mat4 projection; // the projection matrix of the camera
  uniform mat4 view; // the view matrix of the camera


For framebuffer shaders the only uniform set is ``uniform float gamma;``. NOTE: Gamma can be checked/changed with ``r_window_get_gamma`` and ``r_window_set_gamma``.

Error Checking
^^^^^^^^^^^^^^

The render system has two functions for explicit error checking. Both ``r_check_error`` and ``r_check_error_loc`` do essentially the same, thing. ``r_check_error_loc`` however calls ``ASTERA_DBG`` (Astera's debug output macro) like so: 

.. code-block:: c

    ASTERA_DBG("GL Error: %i at location: %s\n", error, loc);

Particles 
^^^^^^^^^

Particles in Astera can function in one of two ways, default or custom methods. 

- default: spawn within ``size`` of the ``r_particles`` system, and move with ``particle_velocity`` set in there as well.
- custom: spawn with a custom spawn function, and/or, animate/update with a custom function. 

The definitions of these custom functions are set with ``r_particles_set_spawner`` and ``r_particles_set_animator``. Here are the expected function definitions are:

.. code-block:: c

  typedef void (*r_particle_animator)(r_particles*, r_particle*);
  typedef void (*r_particle_spawner)(r_particles*, r_particle*);

  // EXAMPLES

  // Called once per anticipated spawn of particle (time / spawn_rate)
  void spawn_functions(r_particles* system, r_particle* particle);
  // Called once per system update (frame)
  void animate_function(r_particles* system, r_particle* particle);

With this we can create functions to animate / update and spawn particles as we wish. For an example of this working in action, check out the `rendering example <https://github.com/tek256/astera/blob/master/examples/rendering/main.c>`_ in the GitHub repository!

For reference, from the ``render.h`` header, here are the parameters of the ``r_particles`` struct:

.. code-block:: c

  struct r_particles {
    // list - the array of particles
    r_particle* list;

    // capacity - the max amount of particles to buffer for
    // count - the amount of particles within the system currently
    // max_emission - the max amount of particles to emit (0 = infinite)
    // emission_count - the amount of particles emitted
    uint32_t capacity, count;
    uint32_t max_emission;
    uint32_t emission_count;

    // particle_layer - the base layer to set a particle to
    uint8_t particle_layer;

    // particle_life - the lifetime of the particle
    // system_life - the lifetype of the system (0 = infinite)
    // spawn_rate - the amount of particles to spawn per second
    float particle_life, system_life;
    float spawn_rate;

    // time - the internal timer of the particle system
    // spawn_time - the time remaining to next particle spawn
    float time, spawn_time;

    // position - The center position of the particle system
    // size - the size (width, height) of the particle system
    vec2 position, size;
    // particle_size - the size of particles (width, height)
    // particle_velocity - the default velocity of particles
    vec2 particle_size, particle_velocity;

    // sheet - the texture sheet to use (note: only needed for PARTICLE_TEXTURED)
    r_sheet* sheet;
    union {
      r_anim   anim;
      uint32_t subtex;
    } render;

    // Overall color of the system
    vec4 color;

    // GL Uniforms
    mat4x4*  mats;
    vec4*    colors;
    vec4*    coords;
    uint32_t uniform_count, uniform_cap;

    // For custom movement
    r_particle_animator animator_func;
    r_particle_spawner  spawner_func;

    // Flags
    // Calculate -- Whether to calculate data for render
    // Type -- Render type colored (yes | no) && (textured | animated)
    // Note: (Can color both other types)
    // Use Animator -- Whether to use custom animator
    //    function of type `void xxx(r_particles*, r_particle*)`
    // Use Spawner -- Whether to use custom spawning function
    //    of type `void xxx(r_particles*, r_particle* particle)
    int8_t calculate, type, use_animator, use_spawner;
  };

