Render Guide
==================================

This guide overviews how to render in a few different methods using Astera.
NOTE: This is still under construction, feedback is welcome!

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

Framebuffers
^^^^^^^^^^^^

To be written
