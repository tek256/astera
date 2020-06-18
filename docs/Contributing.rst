Contributing 
============

Here are some things you can do to help contribute!

* `Bug Review & Search <https://github.com/tek256/astera/labels/bug>`_
* `Wiki / Code Documentation <https://github.com/tek256/astera/tree/master/docs>`_
* `Example Creation <https://github.com/tek256/astera/tree/master/examples>`_
* Platform Testing
* Optimizations
* `Feature Implementations or Requests <https://github.com/tek256/astera/labels/feature-request>`_
* `Task Completion <https://github.com/tek256/astera/projects>`_
* Become a `GitHub Sponsor <https://github.com/sponsors/tek256>`_

Community
^^^^^^^^^

- `Discord <https://discord.com/invite/QQVAEkf>`_
- `Live Stream <https://twitch.tv/tek256>`_

Code Style Guide
^^^^^^^^^^^^^^^^

**Boolean type functions are to return the smallest possible unsigned integer type**  

For example the render system function ``r_window_should_close()`` which is used to tell if the window is requested to close. It should return ``uint8_t`` instead of ``int` `int8_t`` or ``unsigned int``.   

Do to this, standard bool returns should be ``0 = false`` and ``1 = true`` or alternatively ``0 = fail` `1 = success``. 

**Errors should be passed to the context struct for whichever system it is part of.**  

Instead of returning whatever the error type is, set the context's error flag to whatever the designated error code should be.

**Systems should use a context**

In attempts to prevent global declarations, each system should have a ``_ctx`` type to hold any variables commonly be globally declared. For example, render has ``r_ctx`` and audio ``a_ctx``. 

**Context's should be privately declared**  

This means that the context struct should be ``typedef``'d in the header but ``defined`` in the relevent .c file. 
For example:


.. code-block ::

  render.h:
  typedef struct r_ctx r_ctx;


.. code-block ::

  render.c:
  struct r_ctx {
    ... relevent variables
  };


**Code should be C99 Compliant**  

Not GNU99 or using a lot of compiler extensions. Simplicity is key.

**C Code should remove as many pedantic warnings as possible** 

Not strictly held, but greatly appreciated. 

**All code should be ran thru clang-format**  

To keep code consistently formatted with the rest it's asked to run clang-format on whatever code being submitted / added:

.. code-block::

  "AllowShortIfStatementsOnASingleLine" : "false",
  "IndentCaseLabels" : "true",
  "IndentWidth" : 2,
  "PointerAlignment" : "Left",
  "TabWidth" : 2,
  "AlignConsecutiveMacros" : "true",
  "AlignConsecutiveAssignments" : "true",
  "AlignConsecutiveDeclarations" : "true",
  "AlignEscapedNewlines" : "true",
  "AlignTrailingComments" : "true",
  "AlignOperands" : "true",
  "AllowShortBlocksOnASingleLine" : "true",
  "AllowShortFunctionsOnASingleLine" : "true",
  "ColumnLimit" : 80,
  "KeepEmptyLinesAtTheStartOfBlocks" : "false",
  "SortIncludes" : "false",
  "SpaceAfterCStyleCast" : "false"


**All shell scripts should try to be posix compliant**

Where possible, exceptions can be made.

**All shell scripts should pass shell-check**

Simple enough, unix shell / bash scripts should be ran thru shell-check in order to catch any edge cases.

