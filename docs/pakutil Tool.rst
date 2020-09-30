pakutil Tool
============

Please note: This is still currently under construction and is within the early versions of Astera! If you find any errors, please open an `issue <https://github.com/tek256/astera/issues/>`_ or submit a `pull request <https://github.com/tek256/astera/compare>`_!


Overview 
^^^^^^^^

If you need to build this, make sure that in your ``CMakeLists.txt`` you include astera with has ``ASTERA_BUILD_TOOLS`` set to on, or you pass the CLI argument: ``-DASTERA_BUILD_TOOLS=ON`` with your CMake command. 

The ``pakutil`` tool is meant to be a non runtime tool for you to use outside of your game to update & check assets.


Creating a pak file
^^^^^^^^^^^^^^^^^^^

The pakutil has the option to create a pak file for you. In order to do so you must call the tool from your command-line with the argument ``make`` and then pass files for it to add. 

From the pak util's help page: 


.. code-block:: 

        Pak Util Make: Create a pak file
        Usage: ./pakutil make dst.pak filepath name ... filepath n name n
        Ex: ./pakutil make example.pak resources/shaders/main.vert main.vert 
                                       resources/shaders/main.frag main.frag


Checking a pak file
^^^^^^^^^^^^^^^^^^^

If you want to check a pak file for all of it's contents or a specific file you can pass the ``check`` (or ``c``) option.

.. code-block::

        Pak Util (c)heck: Check a pak file for file(s)
        Usage: ./pakutil check dst.pak (list out all files) OR
               ./pakutil check dst.pak filename ... filename n

        Ex 1: ./pakutil check example.pak
        Ex 2: ./pakutil check example.pak main.frag
 

Getting data from a pak file
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you want to get the raw data out of a file, you can pass the ``data`` (or ``d``) option.

NOTE: There is no output padding between files, so if you're wanting to do comparisons, it's recommended to run the command for singular files.

.. code-block::
        Pak Util Data: Get the raw data of file(s) in the pak file
        Usage: ./pakutil data dst.pak filename .. filename n
        Ex: ./pakutil data example.pak test2.txt main.vert



