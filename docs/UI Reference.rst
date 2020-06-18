UI Reference
============

This is reference is meant to be a programmer's companion when using astera!
NOTE: This is still under construction, feedback is welcome!

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


