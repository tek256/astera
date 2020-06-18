Input Reference
===============

Bindings 
^^^^^^^^

All of the types (key, mouse button, joy button / axis) can be used within an ``i_binding`` type. Note, an ``i_binding``'s ``value`` (event to trigger on) and ``alt`` (alternate event to trigger on) can both be different types, which are set with their respective ``type`` values. 

In order to create a binding, you can do so by calling: 

.. code-block:: c

  void i_binding_add(i_ctx* ctx, const char* name, int value, int type);
  void i_binding_add_alt(i_ctx* ctx, const char* name, int value, int type);

If you want to modify a binding in real time, you can set ``i_enable_binding_track`` which will then set the next input to the respective value:

.. code-block:: c

  // Set a key binding's value (alt = 1, value = 0) to the next input type
  void i_enable_binding_track(i_ctx* ctx, const char* key_binding, uint8_t alt);

Otherwise, the ``i_binding`` type has all of the usual ``up``, ``down``,  ``clicked``, ``released`` functions.

Scroll Input
^^^^^^^^^^^^

Scroll input acts similar to mouse movement in the sense that it has an X & Y value, as well as a delta-X and delta-Y (dx / dy) value. This value can be reset with the ``i_scroll_reset`` in order to allow for window / scroll position tracking with the values.

Macros
^^^^^^

Many of the button macro values are modified based on the target system being compiled for. This is due to different operating system drivers remapping different buttons / axes to different numbers. 

- ``ASTERA_KB_NAMELEN`` - The max length of a key binding's name
- ``KEY_A`` - ``KEY_Z``
- ``KEY_0`` - ``KEY_9``
- ``XBOX_A``
- ``XBOX_B``
- ``XBOX_X``
- ``XBOX_Y``
- ``XBOX_L1``
- ``XBOX_R1``
- ``XBOX_SELECT``
- ``XBOX_START``
- ``XBOX_LEFT_STICK``
- ``XBOX_RIGHT_STICK``
- ``MOUSE_LEFT``
- ``MOUSE_RIGHT``
- ``MOUSE_MIDDLE``
- ``KEY_SPACE``
- ``KEY_BACKSPACE``
- ``KEY_DELETE``
- ``KEY_UP``
- ``KEY_DOWN``  
- ``KEY_RIGHT``
- ``KEY_LEFT``
- ``KEY_HOME``
- ``KEY_TAB``
- ``KEY_ESC``        
- ``KEY_ESCAPE``
- ``KEY_LEFT_SHIFT``
- ``KEY_RIGHT_SHIFT``
- ``KEY_ENTER``
- ``KEY_LEFT_CTRL``
- ``KEY_RIGHT_CTRL``
- ``KEY_LEFT_ALT``
- ``KEY_RIGHT_ALT``
- ``KEY_LEFT_SUPER``
- ``KEY_RIGHT_SUPER``


