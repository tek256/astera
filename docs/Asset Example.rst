Asset Example
=============

Please note: This is still currently under construction and is within the early versions of Astera! If you find any errors, please open an `issue <https://github.com/tek256/astera/issues/>`_ or submit a `pull request <https://github.com/tek256/astera/compare>`_!


Overview 
^^^^^^^^

The asset example is meant to exemplify how to use the various functions in the Asset system in Astera. 

If ASTERA_PAK_WRITE is enabled in the build it will include a pak writing example as well. _NOTE: You can enable this by setting it in your CMakeLists.txt or passing the argument `-DASTERA_PAK_WRITE=ON` initially._ This is meant to show how to use the Astera pak writing utility to store multiple files into astera's .PAK file type. For more on the specifics of this type, check out the Asset Guide's PAK Files section.


Note
^^^^

If you want to recreate the pak functionality outside of runtime, astera includes a ``pakutil`` tool in the ``tools/`` directory. You can enable the compilation of this with the engine by passing ``-DASTERA_BUILD_TOOLS=ON`` or setting ``ASTERA_BUILD_TOOLS`` to ON in your ``CMakeLists.txt``. 
