Asset Guide
===========

Please note: This is still currently under construction and is within the early versions of Astera! If you find any errors, please open an `issue <https://github.com/tek256/astera/issues/>`_ or submit a `pull request <https://github.com/tek256/astera/compare>`_!

Basic Files
^^^^^^^^^^^

The asset system tries to be out of your way when it comes to managing assets. As a result many of these functions can operate without having to initialize anything. Where this shines especially is the ``asset_get`` function. It's simply designed to get a file from the file system, and returned a formatted ``asset_t*`` struct pointer. In order to free any ``asset_t`` types you can call ``asset_free(asset_t*)`` which will free all the buffers of the file and the pointer allocated as well.

Here's the documentation from the header:

.. code-block:: c

  // Get a file from the local system 
  // file - the file path of the file
  // returns: formatted asset_t struct pointer with file data
  asset_t* asset_get(const char* file);

Now an example of it's usage:

.. code-block:: c

  // Get the text file's data
  asset_t* text_file = asset_get("test.txt");

  // Output it's length & contents
  printf("[%i]: %s\n", text_file->data_length, text_file->data);

  // Free up the allocated data 
  asset_free(text_file);

Asset Maps
^^^^^^^^^^

Asset maps in astera are meant to represent a collection of related assets needed together. You can modify which individual assets you have in the map at any given time, and manage/find the assets within the map with relative ease. In astera these maps can be created around pak files or the file system structure. Thus there are two different functions for creating the specific types:

.. code-block:: c

    /* Create an asset map
     * filename - point to a file to read from [OPTIONAL]
     * name - the name of the asset map capacity - the max number of
     * assets to store in the map
     * returns: formatted asset_map_t type with assets loaded in */
     asset_map_t asset_map_create(const char* filename, const char* name,
                                  uint32_t capacity);

As well as the memory based function:

.. code-block:: c
  
    /* Create an asset map
     * data - the data of a pak file [OPTIONAL]
     * data_length - the length of the pak file's data [OPTIONAL]
     * name - the name of the asset map capacity - the max number of
     * assets to store in the map
     * returns: formatted asset_map_t type with assets loaded in */
     asset_map_t asset_map_create_mem(unsigned char* data, uint32_t data_length,
                                      const char* name, uint32_t capacity);


From the asset map you can request specific files, and it'll fetch them for you. Once the map is created you can just call:

.. code-block:: c

   /* Get a file from the asset map's directed source */
   asset_t* asset_map_get(asset_map_t* map, const char* file);

If you want the asset map to automatically respect the ``asset_t`` flags for removal / requesting, then you have to call 

.. code-block:: c

   /* Update for any free requests made*/
   void asset_map_update(asset_map_t* map);

Once you're done with an asset map, you can free it and all it's contents with:

.. code-block:: c

    /* Destroy an asset map and all its resources
     * map - the map to destroy */
    void asset_map_destroy(asset_map_t* map);

If you want to free just an individual asset, you're able to do so using:

.. code-block:: c

    /* Free any memory used by the asset */
    void asset_free(asset_t* asset);



PAK File Type
^^^^^^^^^^^^^

In the engine, Astera uses it's own version of a PAK file. There are a few different tools provided, such as the ``pakutil`` tool in the ``tools/`` directory for creating / managing PAK files. To make sure that the tools directory is being built, at generation time enable ``-DASTERA_BUILD_TOOLS=ON``. As well, there are functions in the ``asset.h`` header for writing pak files. You can disable these from being included with ``ASTERA_NO_PAK_WRITE``. 
