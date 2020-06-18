UI Guide
========

This is guide is meant to help getting started with the UI system in Astera.
NOTE: This is still under construction, feedback is welcome!

Function Ordering
^^^^^^^^^^^^^^^^^

This is just a general outline of the ordering of functions in order to properly draw / use UI  with astera's UI pipeline. 

**Runtime** 

1. ``ui_ctx_update(ui_ctx* ctx, vec2 mouse_pos);`` (NOTE: mouse_pos is optional)
2. ``ui_tree_xxx(ui_tree* tree);``
3. ``ui_frame_start(ui_ctx* ctx);`` 
4. ``ui_tree_draw(ui_ctx* ctx, ui_tree* tree);``
5. ``ui_xxxx_draw(ui_ctx* ctx, ui_xxx* xxx);``
6. ``ui_frame_end(ui_ctx* ctx);``
7. ``ui_tree_xxx();``
8. ``ui_tree_check(ui_ctx* ctx);``

**Initialization:**

1. ``ui_ctx_create(vec2 screen_size, float pixel_scale, int8_t use_mouse, int8_t antialias);``
2. ``ui_xxx_create();`` (dropdown, text, button, etc)
3. ``ui_tree_create(uint16_t capacity);``
4. ``ui_tree_add(...);``

Example
^^^^^^^

This is just an outline of how you can use astera's UI System. For working examples check the ``examples/`` directory in the astera repository!

.. code-block:: c 

 // ---- INITIALIZATION ----

 // Window size of 720p
 vec2 window_size = {1280.f, 720.f};
 // arguments: window_size, pixel scale, use_mouse, antialias
 ui_ctx* ctx = ui_ctx_create(window_size, 1.f, 1, 1);

 // Load the font data into memory
 asset_t* font_data = asset_get("resources/fonts/monogram.ttf");

 // Create a font we can use from the data
 ui_font font = ui_font_create(ctx, font_data->data, font_data->data_length, "monogram");

 // position & size is in relative screen units
 vec2 button_pos = {0.5f, 0.5f};
 vec2 button_size = {0.15f, 0.075f};

 // center the text of the button
 ui_button button = ui_button_create(ctx, button_pos, button_size, "Hello world!", UI_ALIGN_CENTER | UI_ALIGN_MIDDLE, 32.f); 

 button.font = font;

 // Get some colors to use
 vec4 red, black;
 ui_get_color(red, "#fb222d");
 ui_get_color(black, "#0a0a0a");

 // Order: background, hover background, foreground, hover foreground, border, hover border
 ui_button_set_colors(&button, black, black, red, red, 0, 0);

 // Create a tree to hold the element(s)
 ui_tree tree = ui_tree_create(1);

 // Add the button to the tree
 ui_tree_add(ctx, &tree, &button, UI_BUTTON, 0, 1, 0);

 
 // ---- RUNTIME ----  (each frame)

 // this can be taken from astera's input system or your own
 vec2 mouse_position; 
 ui_ctx_update(ctx, mouse_position);

 if(...) { // Arbitrary input 
   // Order: context, tree, type, is_mouse
   ui_tree_select(ctx, &tree, 1, 0);
 }

 if(...) { // Arbitrary _mouse_ input
  ui_tree_select(ctx, &tree, 1, 1);
 }

 if(ui_tree_check(&tree, button.id) == 1){
    printf("Button pressed!\n");
 }

 // Clear the window
 r_window_clear(); // same as glClear DEPTH & COLOR buffers

 // Start the frame for drawing
 ui_frame_start(ctx);

 // Draw all the elements in the tree
 ui_tree_draw(ctx, &tree);

 // End and draw the frame
 ui_frame_end(ctx);

 // Show the current frame
 r_window_swap_buffers(render_ctx); // same as glfwSwapBuffers


