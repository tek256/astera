Asset Reference
===============

Please note: This is still currently under construction and is within the early versions of Astera! If you find any errors, please open an `issue <https://github.com/tek256/astera/issues/>`_ or submit a `pull request <https://github.com/tek256/astera/compare>`_!

Function Ordering
^^^^^^^^^^^^^^^^^

The only order specific functions for the asset header is the initialization of asset maps.

1. ``asset_map_create_zip`` or ``asset_map_create_pak``
2. ``asset_map_get``
3. ``asset_map_free``

Macros
^^^^^^

None currently.


PAK File Type
^^^^^^^^^^^^^

Astera writes its own version of PAK files, these files follow this outline:

.. code-block:: c

  /* HEADER LAYOUT (bytes)
   * 0 - ID
   * 4 - count
   * 8 - file_size (bytes)
   * 12 - checksum (64-bit)
   *
   * FILE LAYOUT (bytes)
   * 0  - pack_header_t
   * 20 - entries (* count)
   * n  - start of files */


