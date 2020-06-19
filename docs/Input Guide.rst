Input Guide
===========

Please note: This is still currently under construction and is within the early versions of Astera! If you find any errors, please open an `issue <https://github.com/tek256/astera/issues/>`_ or submit a `pull request <https://github.com/tek256/astera/compare>`_!

Preface
^^^^^^^

The input system is reliant on the rendering system due to both using GLFW's features. Due to this reason you'll want to make sure you link the two together for usage. The specific order of initialization doesn't make too much of a difference, but for runtime usage you want to make sure to call ``r_ctx_set_i_ctx`` with the pointer to both your render and input contexts. This just enables the callbacks to make sure the input system get's the information it needs.

Function Ordering
^^^^^^^^^^^^^^^^^

1. ``i_ctx_create``
2. ``r_ctx_set_i_ctx`` (for callbacks)
3. **RUNTIME** (``i_ctx_update`` & ``i_poll_events``)
4. ``i_ctx_destroy``

Basic Input 
^^^^^^^^^^^

Every frame you'll want to call ``i_ctx_update`` and ``i_poll_events`` in order to update the states for your system. After this you can call the input check functions you want. There are 3 base types of input's you can check:

1. ``key``
2. ``mouse``
3. ``joy``

On top of those you can use the ``i_binding`` type to include any of the 3 base types into an easily checked type. All 4 of these types have the following functions:

1. ``i_xxx_clicked`` - if was clicked this frame (not last)
2. ``i_xxx_released`` - if was released this frame (not last)
3. ``i_xxx_down`` - if is pressed this frame
4. ``i_xxx_up`` - if is not pressed this frame

As well you can check a joystick's axes with ``i_joy_axis`` which will return the floating point value of that specific joy axis.If your joystick isn't automatically working, you should call ``i_joy_create`` with the ID of the joystick created.

Character Callback
^^^^^^^^^^^^^^^^^^

If you want to get input that a user is typing in the form of characters, you can do so by setting ``i_set_char_tracking`` and then getting the characters with: ``i_get_chars``

Mouse Movement
^^^^^^^^^^^^^^

Within the input system, both the mouse & the mouse scroll are tracked in similar ways. You can interact with the buttons using the standard checks like ``i_mouse_clicked`` and whatnot (similar to keys / joy buttons). However, in order to check for the mouse position, delta, scroll position, or scroll delta, you have to use one of the following functions:

.. code-block:: c

  void   i_mouse_get_pos(i_ctx* ctx, double* x, double* y);
  double i_mouse_get_x(i_ctx* ctx);
  double i_mouse_get_y(i_ctx* ctx);

  void   i_mouse_get_delta(i_ctx* ctx, double* x, double* y);
  double i_mouse_get_dx(i_ctx* ctx);
  double i_mouse_get_dy(i_ctx* ctx);


Basic Example
^^^^^^^^^^^^^

This is meant to be an outline of the functions you should call in order to get input working in your program.

.. code-block:: c

  i_ctx* input_ctx = i_ctx_create(4, 32, 4, 16, 16, 32);

  ... 

  // Link the input ctx to render ctx for callbacks
  r_ctx_set_i_ctx(render_ctx, input_ctx);

  while(1){
    i_ctx_update(input_ctx);
    i_poll_events();

    if(i_key_clicked(input_ctx, KEY_SPACE)){
      printf("Hello world!\n");
    }
  }

