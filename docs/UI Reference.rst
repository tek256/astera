UI Reference
============

Please note: This is still currently under construction and is within the early versions of Astera! If you find any errors, please open an `issue <https://github.com/tek256/astera/issues/>`_ or submit a `pull request <https://github.com/tek256/astera/compare>`_!

Definitions
^^^^^^^^^^^

- ``UI Element`` - Any drawable type of UI component (dropdown, text, box, etc)

Tree Usage
^^^^^^^^^^

In Astera the ``ui_tree`` type is meant to be a catch all for grouping ``ui_element``'s together. You can use multiple UI Trees at any given time, and they all hold their own input states, so creating menus / submenus is relatively easy to do with them. 

You can check for any input events using ``ui_tree_check``, here's the definition from the header: 

.. code-block:: c

  /* Check a UI Tree for any events
   * tree - the tree to check
   * uid - the uid/element to check for
   * returns: the event type / value */
  int32_t ui_tree_check_event(ui_tree* tree, uint32_t uid);


Element Drawing 
^^^^^^^^^^^^^^^

With the UI System you can draw elements in a few different ways. You can draw them individually, with their respective ``ui_xxxx_draw`` functions. You can draw them from a ``ui_tree`` which will automatically sort & order the elements to be drawn. Or you can draw them with the ``ui_im_xxx_draw`` functions. 

Input Usage
^^^^^^^^^^^

The UI system uses an abstract input system with ``next`` and ``prev`` in places for navigating lists/trees. On top of that you can pass event types to individual elements for checking / custom interaction. For example:

.. code-block:: c 

  // Create a button type and add it to a tree / context
  ui_button button;
  
  // ...
 
  // Assume the button is the selected `cursor` within a tree
  // Select the current cursor with event type/id of 1, as a non-mouse event
  ui_tree_select(ui_ctx, &tree, 1, 0);

  int event_type;
  if((event_type = ui_tree_check_event(&tree, button.id) == 1){
    printf("Pressed!\n");
  }


Attribute Definitions
^^^^^^^^^^^^^^^^^^^^^

Throughout the UI System there are attributes available for each element type to make creation more smooth. You can disable this extension at runtime by setting the attribute capacity to 0 and fixed with:

.. code-block:: c 

  /* Set the attributes to fixed
   * ctx - the context to affect */
  void ui_ctx_set_attribs_fixed(ui_ctx* ctx);

  /* Set the max capacity of the attributes map
   * ctx - the context to use
   * capacity - the max number of attributes
   *            (will be clamped to max possible attributes, UI_ATTRIB_LAST) */
  void ui_ctx_set_attribs_capacity(ui_ctx* ctx, uint32_t capacity);


Attribute Types
^^^^^^^^^^^^^^^

Here is a quick reference to what attributes there are:

.. code-block:: 

  UI_DEFAULT_FONT,

  UI_TEXT_FONT,
  UI_TEXT_FONT_SIZE,
  UI_TEXT_ALIGN,
  UI_TEXT_COLOR,
  UI_TEXT_SHADOW,
  UI_TEXT_SHADOW_SIZE,
  UI_TEXT_LINE_HEIGHT,
  UI_TEXT_SPACING,

  UI_BOX_SIZE,
  UI_BOX_BG,
  UI_BOX_BORDER_COLOR,
  UI_BOX_BG_HOVER,
  UI_BOX_BORDER_COLOR_HOVER,
  UI_BOX_BORDER_SIZE,
  UI_BOX_BORDER_RADIUS,

  UI_BUTTON_SIZE,
  UI_BUTTON_PADDING,
  UI_BUTTON_FONT,
  UI_BUTTON_FONT_SIZE,
  UI_BUTTON_TEXT_ALIGNMENT,
  UI_BUTTON_COLOR,
  UI_BUTTON_BG,
  UI_BUTTON_COLOR_HOVER,
  UI_BUTTON_BG_HOVER,
  UI_BUTTON_BORDER_RADIUS,
  UI_BUTTON_BORDER_COLOR,
  UI_BUTTON_BORDER_COLOR_HOVER,
  UI_BUTTON_BORDER_SIZE,

  UI_DROPDOWN_BORDER_RADIUS,
  UI_DROPDOWN_BORDER_SIZE,
  UI_DROPDOWN_BORDER_COLOR,
  UI_DROPDOWN_BORDER_COLOR_HOVER,
  UI_DROPDOWN_SIZE,
  UI_DROPDOWN_FONT_SIZE,
  UI_DROPDOWN_FONT,
  UI_DROPDOWN_COLOR,
  UI_DROPDOWN_COLOR_HOVER,
  UI_DROPDOWN_BG,
  UI_DROPDOWN_BG_HOVER,
  UI_DROPDOWN_SELECT_COLOR,
  UI_DROPDOWN_SELECT_BG,
  UI_DROPDOWN_SELECT_COLOR_HOVER,
  UI_DROPDOWN_SELECT_BG_HOVER,

  UI_OPTION_SIZE,
  UI_OPTION_IMAGE,
  UI_OPTION_IMAGE_SIZE,
  UI_OPTION_IMAGE_OFFSET,
  UI_OPTION_FONT,
  UI_OPTION_FONT_SIZE,
  UI_OPTION_TEXT_ALIGN,
  UI_OPTION_BG,
  UI_OPTION_BG_HOVER,
  UI_OPTION_COLOR,
  UI_OPTION_COLOR_HOVER,

  UI_LINE_THICKNESS,
  UI_LINE_COLOR,

  UI_PROGRESS_SIZE,
  UI_PROGRESS_FILL_PADDING,
  UI_PROGRESS_BG,
  UI_PROGRESS_FG,
  UI_PROGRESS_BORDER_COLOR,
  UI_PROGRESS_ACTIVE_BG,
  UI_PROGRESS_ACTIVE_FG,
  UI_PROGRESS_ACTIVE_BORDER_COLOR,
  UI_PROGRESS_BORDER_RADIUS,
  UI_PROGRESS_BORDER_SIZE,
  UI_PROGRESS_VERTICAL_FILL,

  UI_SLIDER_SIZE,
  UI_SLIDER_FILL_PADDING,
  UI_SLIDER_BG,
  UI_SLIDER_FG,
  UI_SLIDER_BORDER_COLOR,
  UI_SLIDER_ACTIVE_BG,
  UI_SLIDER_ACTIVE_FG,
  UI_SLIDER_ACTIVE_BORDER_COLOR,
  UI_SLIDER_BORDER_RADIUS,
  UI_SLIDER_BORDER_SIZE,
  UI_SLIDER_VERTICAL_FILL,
  UI_SLIDER_BUTTON_SIZE,
  UI_SLIDER_BUTTON_CIRCLE,
  UI_SLIDER_BUTTON_COLOR,
  UI_SLIDER_BUTTON_ACTIVE_COLOR,
  UI_SLIDER_BUTTON_BORDER_COLOR,
  UI_SLIDER_BUTTON_ACTIVE_BORDER_COLOR,
  UI_SLIDER_BUTTON_BORDER_SIZE,
  UI_SLIDER_BUTTON_BORDER_RADIUS,
  UI_SLIDER_FLIP,
  UI_SLIDER_AUTO_HIDE_BUTTON,
  UI_SLIDER_ALWAYS_HIDE_BUTTON,

