Audio Guide
===========

Please note: This is still currently under construction and is within the early versions of Astera! If you find any errors, please open an `issue <https://github.com/tek256/astera/issues/>`_ or submit a `pull request <https://github.com/tek256/astera/compare>`_!


Initialization
^^^^^^^^^^^^^^

In order to use the audio system, you'll want to create an audio context. To do so, you should call ``a_ctx_create``:

.. code-block:: c

  /* Create an audio context for playback
   * device - the device's name to use (NULL for default)
   * layers - the number of layers to create for managing sounds
   * max_sfx - the max amount of sfx for the context to handle
   * max_buffers - the max amount of audio buffers for the context to handle
   * max_fx - the max amount of audio fx for the context to handle
   * max_songs - the max amount of songs for the context to handle
   * pcm_size - the size of the PCM buffer for OGG vorbis decoding */
   a_ctx* a_ctx_create(const char* device, uint8_t layers, uint16_t max_sfx,
                       uint16_t max_buffers, uint16_t max_songs, uint16_t max_fx,
                       uint16_t max_filters, uint32_t pcm_size);

Buffers
^^^^^^^

Buffers or ``a_buf`` types are meant to be small individual sound effects that you use for ``a_sfx`` playback. You can use either ``WAV`` or ``OGG`` files for these. You can create these buffers by calling ``a_buf_create``: 

.. code-block:: c

  /* Create an audio buffer (generally small sounds)
   * ctx - the context to manage the buffer with
   * data - the raw data of the buffer
   * data_length - the length of the raw data
   * name - a string name for the buffer (optional)
   * is_ogg - if you want to decode using OGG or WAV format (1 = ogg, 0 = wav)
   * returns: ID of the buffer in the context (non-zero, 0 = fail) */
  uint16_t a_buf_create(a_ctx* ctx, unsigned char* data, uint32_t data_length,
                        const char* name, uint8_t is_ogg);


Songs
^^^^^

Songs or ``a_song`` types are meant to be longer music / song files which shouldn't be buffered & decoded all in one go. These types utilize a specified number of smaller buffers in order to buffer ahead of playback to allow time for decoding. Currently, these are only creatable using ``OGG`` files, tho in future versions it is a goal to use ``WAV`` file streaming as well. You can create these types using ``a_song_create``: 

.. code-block:: c

  /* Create a song
   * ctx - the context to create the song within
   * data - the raw (OGG Vorbis) file data
   * length - the length of the raw data
   * packets_per_buffer - the amount of packets to put into a buffer
   * buffers - the number of OpenAL buffers to use 
     returns: the song ID */
  uint16_t a_song_create(a_ctx* ctx, unsigned char* data, uint32_t data_length,
                         const char* name, uint16_t packets_per_buffer,
                         uint8_t buffers, uint32_t max_buffer_size);

Layers
^^^^^^

Layers in the audio system are meant to be a means to manage a number of like ``sfx`` and ``song``'s. You can automatically adjust the gain/volume of a layer with ``a_layer_set_gain``. As well as add / remove sfx and song's with ``a_layer_add_xxx`` and ``a_layer_remove_xxx``. 

Playback
^^^^^^^^

Playback in Astera is relatively straightforward. You manage realtime playback via an ``a_req`` type. Which you can create using ``a_req`` create: 

.. code-block:: c

  /* Create a request structure
   * position - the position in 3D space to play the song/sound
   * gain - the gain to apply to this song/sound
   * range - the range of the song/sound, pass 0 to set to default
   * loop - whether or not the song/sound should loop
   * fx - a list of effect IDs to use (OPTIONAL)
   * fx_count - the number of effects to use (OPTIONAL)
   * filters - a list of filter IDs to use (OPTIONAL)
   * filter_count - the number of filters to use (OPTIONAL)
   * returns: formatted request structure */
  a_req a_req_create(vec3 position, float gain, float range, uint8_t loop,
                     uint16_t* fx, uint16_t fx_count, uint16_t* filters,
                     uint16_t filter_count);


An example of creating a simple request looks something like this: 

.. code-block:: c

  vec3 position = {0.0f, 0.0f, 0.0f};
  // Position, gain, range, looping, no fx, no fx, no filters, no filters
  a_req request = a_req_create(position, 1.f, 100.f, 0, 0, 0, 0, 0);

After you have the request type, you can call ``a_sfx_play`` or ``a_song_play`` in order to play the song or buffer! 
Here's a small example: 

.. code-block:: c

  asset_t* song_data = asset_get("resources/audio/thingy.ogg");
  uint16_t song_id = a_song_create(audio_ctx, song_data->data, song_data->data_length,
                          "test", 32, 4, 4096 * 4);

  vec3 song_pos = {0.f, 0.f, 0.f};
  a_req req = a_req_create(song_pos, 1.f, 100.f, 1, 0, 0, 0, 0);
  a_song_play(audio_ctx, 0, song_id, &req);                         

  ... 

  // RUNTIME 
  while(1) {
    a_ctx_update(audio_ctx);
  }
