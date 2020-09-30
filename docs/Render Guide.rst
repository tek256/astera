Render Guide
==================================

Please note: This is still currently under construction and is within the early versions of Astera! If you find any errors, please open an `issue <https://github.com/tek256/astera/issues/>`_ or submit a `pull request <https://github.com/tek256/astera/compare>`_!

Basic Rendering
^^^^^^^^^^^^^^^

In order to start rendering with Astera, you need to create the render context. You can do so after including ``astera/render.h`` and calling:

.. code-block:: c

  /* Create the initial render context
   * NOTE: this is needed to render with astera
   *
   * params - the window parameters for the game window
   * use_fbo - to use a framebuffer to render to or not (post-processing)
   * batch_count - the number of batches to create for different draw types
   * batch_size - the max amount of sprites to store in each given batch
   * anim_map_size - the amount of animations to allow to be cached / mapped
   * shader_map_size - the amount of shaders to allow to be cached / mapped */
   r_ctx* r_ctx_create(r_window_params params, uint8_t use_fbo,
                       uint32_t batch_count, uint32_t batch_size,
                       uint32_t anim_map_size, uint32_t shader_map_size);


To create the ``r_window_params`` type you can create the structure and set the values, or call the function:

.. code-block:: c

   /* Create a basic version of the window params structure for context creation
    * width - the width of the window
      * height - the height of the window
      * resizable - if the window should be resizable (0 = no, 1 = yes)
      * fullscreen - if the window should be fullscreen (0 = no, 1 = yes)
      * vsync = if the window should use vsync (0 = no, 1 = yes)
      * borderless = if the window should be borderless (0 = no, 1 = yes)
      * refresh_rate = the refresh rate the should be used (if fullscreen)
      * title - the title of the window
      * returns: formatted r_window_params struct */
     r_window_params r_window_params_create(uint32_t width, uint32_t height,
                                          uint8_t resizable, uint8_t fullscreen,
                                          uint8_t vsync, uint8_t borderless,
                                          uint16_t    refresh_rate,
                                          const char* title);

After that you're free to create sprites and textures. You can create your first texture sheet with: ``r_sheet_create`` or ``r_sheet_create_tiled``: 

.. code-block:: c

  /* Automatically create sprites based on a grid size
   * data - the image data
   * length - the length of the image data
   * sub_width - the width of the subsprite
   * sub_height - the height of the subsprite
   * width_pad - the internal padding between sprites on each X axis side
   * height_pad - the internal padding between sprites on each Y axis side */
   r_sheet r_sheet_create_tiled(unsigned char* data, uint32_t length,
                                uint32_t sub_width, uint32_t sub_height,
                                uint32_t width_pad, uint32_t height_pad);

Once you have that you have the basis for creating a sprite with: ``r_sprite_create`` and ``r_sprite_set_tex``

.. code-block:: c

   /* Create a sprite to draw
    * shader - the shader program to draw it with
      * pos - the position of the sprite
      * size - the size of the sprite in units */
     r_sprite r_sprite_create(r_shader shader, vec2 pos, vec2 size);

   /* Set a sprite's texture
    * sprite - the sprite to affect
    * sheet - the texture sheet to use
    * tex - the subtexture ID to use */
    void r_sprite_set_tex(r_sprite* sprite, r_sheet* sheet, uint32_t tex);


**Each frame:**
At the start of each frame you should call ``r_window_clear()`` to clear the framebuffer. When your render step is done, you should call ``r_ctx_draw(render_ctx_here)``. Once you're done with the frame, you can call ``r_window_swap_buffers(render_ctx_here`` to display the new frame!

Here is a small codeblock example of the whole process:

.. code-block:: c

   r_window_params params = r_window_params_create(1280, 720, 0, 0, 1, 0, 0, "Untitled");
   r_ctx* render_ctx = r_ctx_create(params, 0, 2, 256, 16, 2);
   
   // asset_t is from <astera/asset.h>, this function just loads the raw file data
   asset_t* vert_data = asset_get("resources/shaders/main.vert"); // Vertex Shader
   asset_t* frag_data = asset_get("resources/shaders/main.frag"); // Fragment Shader 
   asset_t* sheet_data = asset_get("resources/textures/DungeonTileset.png"); // Example tilesheet

   r_shader shader = r_shader_create(vert_data->data, vert_data->data_length, frag->data, frag->data_length);
   r_sheet sheet = r_sheet_create_tiled(sheet_data->data, sheet_data->data_length, 16, 16, 0, 0);
   vec2 sprite_size = {64.f, 64.f};
   r_sprite sprite = r_sprite_create(shader, 0, sprite_size);
   r_sprite_set_tex(&sprite, &sheet, 0);

   while(!r_window_should_close(render_ctx)){
      r_window_clear();

      // Only important if you're animating / moving the sprite
      r_sprite_update(render_ctx, 16.f);

      // Call for the sprite to be drawn
      r_sprite_draw(render_ctx, &sprite);
      r_ctx_draw(render_ctx, &sprite);
      r_window_swap_buffers(render_ctx);
   }
    

Shaders
^^^^^^^

Using custom shaders with Astera is considered the primary approach. You can find premade shaders that will work, bundled within the examples of the repository (under examples/resources/shaders/). 

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


NOTE: the array size should be set to the batch size you set in the render pipeline for astera

**Caveats:**
The Z coordinate in the vertex position attribute is only non-zero when passed / used by an ``r_baked_sheet`` type.  
Baked Sheets should be rendered as just a whole mesh, since all of the vertex & texture coordinate data is baked into the vertex buffer. The vertex attribute layout is still the same. 
Particles can be rendered with a shader written for the batching system, but do not use ``flip_x`` or ``flip_y``

Drawing / Batching
^^^^^^^^^^^^^^^^^^

Sprites aren't individually managed by astera, rather treated as an intermediate type. This means that you can request any copy of the sprite to be drawn and it will be added to the batch determined to fit. Each of these batches can hold up to a certain number of sprites before issuing an instanced draw call, to draw what is within it's buffers at once. Keep in mind that this is often limited by the shader and graphics driver. Play with buffer size to see what works best for you. 
You set the batch's buffer size as a constant for all buffers on creation of the render context ``r_ctx_create``. From the header: 

.. code-block:: c

 // Create the initial render context
 // NOTE: this is needed to render with astera
 //
 // params - the window parameters for the game window
 // use_fbo - to use a framebuffer to render to or not (post-processing)
 // batch_count - the number of batches to create for different draw types
 // batch_size - the max amount of sprites to store in each given batch
 // anim_map_size - the amount of animations to allow to be cached / mapped
 // shader_map_size - the amount of shaders to allow to be cached / mapped
 r_ctx* r_ctx_create(r_window_params params, uint8_t use_fbo,
                     uint32_t batch_count, uint32_t batch_size,
                     uint32_t anim_map_size, uint32_t shader_map_size);


**Best Practices:**
You should sort your sprite draw calls by shader and texture sheet. 
If a sprite has a different shader or texture sheet and is requested to draw with no free batches, the smallest batch will be drawn and swapped to fill the new combination. 


Particles
^^^^^^^^^

Astera exposes both a default and custom option for handling particles. You can create and use the particle system without having to pass custom handling functions simply by calling the functions to setup preferences: 

.. code-block:: c

  /* Create a particle system
   * emit_rate - the amount of particles to emit per second
   * particle_capacity - the maximum amount of particles alive at any given
   * moment emit_count - the max amount of particles to emit (0 = infinite)
   * particle_type - reference r_particle_type
   *                 (i.e PARTICLE_COLORED |  PARTICLE_ANIMATED)
   * calculate - whether or not to automatically calculate uniform arrays
   *             NOTE: You should only disable this if you want to use your own
   *                   methods of rendering  */
  r_particles r_particles_create(uint32_t emit_rate, float particle_life,
                                 uint32_t particle_capacity, uint32_t emit_count,
                                 int8_t particle_type, int8_t calculate,
                                 uint16_t uniform_cap);

  /* Set particle system variables related to individual particles
   * NOTE: vectors can be passed as 0/NULL, and they won't be set
   * system - the particle system to affect
   * color - the color to set particles by default
   * particle_life - the duration of each particle's lifespan in milliseconds
   *                 NOTE: if passed 0 the value won't be set
   * particle_size - the size of particles in size unit
   * particle_velocity - the velocity of a particle (units per second) */
  void r_particles_set_particle(r_particles* system, vec4 color,
                                float particle_life, vec2 particle_size,
                                vec2 particle_velocity);

  /* This function uses the assumed uniforms for rendering
   * ctx - the context to use for rendering
   * particles - the particle system to draw
   * shader - the shader to draw the particles with */
  void r_particles_draw(r_ctx* ctx, r_particles* particles, r_shader shader);

  /* Update the simulation of the particles */
  void r_particles_update(r_particles* system, time_s delta);
  
  /* Set a particle system's default particle animation
   * particles - the particle system to affect
   * anim - the animation to set as default */
  void r_particles_set_anim(r_particles* particles, r_anim* anim);
  
  /* Set a particle system's default sub texture
   * particles - the particle system to affect
   * sheet - the texture sheet to use
   * subtex - the subtex to set as default */
  void r_particles_set_subtex(r_particles* particles, r_sheet* sheet,
                              uint32_t subtex);
  
  /* Set a particle system's default particle color
   * particles - the particle system to affect
   * color - the color of the particle
   * color_only - if to set the system to only render colored particles */
  void r_particles_set_color(r_particles* particles, vec4 color,
                           uint8_t color_only);


Once you're done with the particle system, you can simply destroy it with: 

.. code-block:: c

  /* Destroy all resources for the particles
   * NOTE: This will not destroy the textures / anims & shaders used */
  void r_particles_destroy(r_particles* particles);

Baked Sheets
^^^^^^^^^^^^

Baked sheets attempt to wrap multiple sprites into one draw call. Tho the main limiter is often that the baked sheet can only access one given sheet. Thus it's generally used for a sprite sheet based system for drawing environments/backgrounds/etc. 

To put it another way, baked sheets take multiple static (non-animated) sprites and pushes them into one vertex buffer. You can create a baked sheet by calling:

.. code-block:: c

  /* Create a baked sheet (series of quads) to render
   * sheet - the texture sheet you want to use
   * quads - the quads you want to put within the baked_sheet
   * quad_count - the number of quads
   * position - the offset of the baked sheet (top-left)
   * layer - the layer (z index) of the baked_sheet overall
   * NOTE: After initialization, you're able to free the `quads` array */
  r_baked_sheet r_baked_sheet_create(r_sheet* sheet, r_baked_quad* quads,
                                   uint32_t quad_count, vec2 position);

Once you have it created you can draw it by calling:
_NOTE: If you want to see the layout of baked sheet shaders, see the :render reference:`baked-sheet-reference`.

.. code-block:: c

  /* Draw the baked sheet
   * ctx - the render context to use
   * shader - the shader to use
   * sheet - the baked sheet to draw */
  void r_baked_sheet_draw(r_ctx* ctx, r_shader shader, r_baked_sheet* sheet);


Once you're done with the baked sheet you can destroy it with: 

.. code-block:: c

  /* Destroy a baked sheet
   * NOTE: This will not destroy shaders & textures,
   *       just the baked sheet's vertex data */
  void r_baked_sheet_destroy(r_baked_sheet* sheet);


Framebuffers
^^^^^^^^^^^^

Astera exposes some basic functionality for framebuffer usage with OpenGL. The implementation accounts for the displaying & binding of a simple depth and stencil framebuffer currently.  

You can create, use, and destroy framebufferes with the following functions:

.. code-block:: c

  /* Create an OpenGL Framebuffer & quad to draw it on
   * width - the width in pixels
   * height - the height in pixels
   * shader - the shader program to render it with */
  r_framebuffer r_framebuffer_create(uint32_t width, uint32_t height,
                                     r_shader shader);

  /* Bind a framebuffer for OpenGL to draw to
   * fbo - the framebuffer to bind */
  void r_framebuffer_bind(r_framebuffer fbo);

  /* Draw a framebuffer to it's quad
   * ctx - the context to get the gamma parameter from
   * fbo - the framebuffer to draw */
  void r_framebuffer_draw(r_ctx* ctx, r_framebuffer fbo);

  /* Destroy the OpenGL Framebuffer and it's quad (the shader is unaffected)
   * fbo - the framebuffer to destroy */
  void r_framebuffer_destroy(r_framebuffer fbo);


If you want to create your own framebuffer shader, these are the uniforms & layouts you should use: 

.. code-block:: c

  VERTEX SHADER:
  layout(location = 0) in vec3 in_vert;
  layout(location = 1) in vec2 in_texc;

  uniform float depth_offset = 0.0f;

  FRAGMENT SHADER:
  uniform sampler2D screen_tex;
  uniform float gamma = 1.0;


