Render Reference
==================================

This is reference is meant to be a programmer's companion when using astera!
NOTE: This is still under construction, feedback is welcome!

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


