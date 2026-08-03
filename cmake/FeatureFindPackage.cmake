#[=======================================================================[.rst:
wrapped_find_package
--------------------

Wrapper around :command:`find_package` that integrates package TYPE
metadata with :command:`feature_summary`.

The macro intentionally does **not** forward the ``REQUIRED`` keyword
to :command:`find_package`. Instead, package severity is recorded using
:command:`set_package_properties` so that
``feature_summary(FATAL_ON_MISSING_REQUIRED_PACKAGES)`` can centrally
handle dependency failures.

Imported targets created by :command:`find_package` remain globally
available exactly as with a direct call to :command:`find_package`.

Component package names follow the convention:

.. code-block:: cmake

  <PackageName><ComponentName>

For example:

.. code-block:: cmake

  wrapped_find_package(
    Qt6
    REQUIRED_COMPONENTS
      Core
      Widgets
    OPTIONAL_COMPONENTS
      WebEngineWidgets
  )

Produces package TYPE metadata for:

.. code-block:: text

  Qt6Core
  Qt6Widgets
  Qt6WebEngineWidgets

Synopsis
^^^^^^^^

.. code-block:: cmake

  wrapped_find_package(<PackageName>
    [REQUIRED]
    [RECOMMENDED]
    [RUNTIME]
    [OPTIONAL]

    [COMPONENTS <components>...]
    [REQUIRED_COMPONENTS <components>...]
    [RECOMMENDED_COMPONENTS <components>...]
    [RUNTIME_COMPONENTS <components>...]
    [OPTIONAL_COMPONENTS <components>...]

    [CONFIG]
    [MODULE]
    [QUIET]
    [GLOBAL]
    [BYPASS_PROVIDER]
    [UNWIND_INCLUDE]
  )

Package Types
^^^^^^^^^^^^^

The package TYPE is inferred from one of the following keywords:

``REQUIRED``
  Marks the package as required.

``RECOMMENDED``
  Marks the package as recommended.

``RUNTIME``
  Marks the package as a runtime dependency.

``OPTIONAL``
  Marks the package as optional.

If no package TYPE keyword is specified, ``OPTIONAL`` is used.

Only one package TYPE keyword may be specified.

Component Types
^^^^^^^^^^^^^^^

Components may be assigned independent TYPE metadata using:

``REQUIRED_COMPONENTS``
  Components required for the project.

``RECOMMENDED_COMPONENTS``
  Recommended components.

``RUNTIME_COMPONENTS``
  Runtime-only components.

``OPTIONAL_COMPONENTS``
  Optional components.

The standard ``COMPONENTS`` keyword is treated as an alias for
``REQUIRED_COMPONENTS`` for compatibility with standard
:command:`find_package` conventions.

Since :command:`find_package` only supports ``COMPONENTS`` and
``OPTIONAL_COMPONENTS``, recommended and runtime components are queried
internally as optional components.

Examples
^^^^^^^^

Mark an entire package as required:

.. code-block:: cmake

  wrapped_find_package(Qt6 REQUIRED)

Specify required and optional components:

.. code-block:: cmake

  wrapped_find_package(
    Qt6
    REQUIRED_COMPONENTS
      Core
      Widgets
    OPTIONAL_COMPONENTS
      WebEngineWidgets
  )

Use with :command:`feature_summary`:

.. code-block:: cmake

  feature_summary(
    WHAT ALL
    FATAL_ON_MISSING_REQUIRED_PACKAGES
  )

#]=======================================================================]

#[=======================================================================[.rst:
multiset_package_properties
---------------------------

Apply identical package properties to multiple packages.

This is a convenience wrapper around
:command:`set_package_properties`.

Synopsis
^^^^^^^^

.. code-block:: cmake

  multiset_package_properties(
    <package>...
    PROPERTIES <property> <value>...
  )

Example
^^^^^^^

.. code-block:: cmake

  multiset_package_properties(
    Qt6Core
    Qt6Widgets
    PROPERTIES
      TYPE REQUIRED
      URL "https://www.qt.io"
  )

#]=======================================================================]
#[=======================================================================[.rst:
.. command:: return_on_missing_required_packages

  Return from the current directory, function, or macro scope if any
  package marked as ``TYPE REQUIRED`` was not found.

  This command performs a lightweight check similar to
  ``feature_summary(FATAL_ON_MISSING_REQUIRED_PACKAGES)`` but does not
  emit a fatal error.

  The command is intended for projects that defer dependency validation
  until a later top-level :command:`feature_summary` call while allowing
  subsystems to stop configuring early.

  Only packages registered through
  :command:`set_package_properties` are considered.

  Example
  ^^^^^^^

  .. code-block:: cmake

    wrapped_find_package(
      Qt6
      REQUIRED_COMPONENTS
        Core
        Widgets
    )

    return_on_missing_required_packages()

    add_library(mygui ...)
    target_link_libraries(mygui PRIVATE Qt6::Widgets)

#]=======================================================================]
include_guard(GLOBAL)
include(FeatureSummary)

function(multiset_package_properties)
    list(FIND ARGN "PROPERTIES" _MSPP_properties_index)

    if(_MSPP_properties_index EQUAL -1)
        message(FATAL_ERROR
            "multiset_package_properties(): missing PROPERTIES keyword")
    endif()

    list(SUBLIST ARGN 0 ${_MSPP_properties_index} _MSPP_packages)

    math(EXPR _MSPP_properties_start
        "${_MSPP_properties_index} + 1"
    )

    list(SUBLIST ARGN
        ${_MSPP_properties_start}
        -1
        _MSPP_properties
    )

    foreach(_MSPP_package IN LISTS _MSPP_packages)
        set_package_properties(
            ${_MSPP_package}
            PROPERTIES
                ${_MSPP_properties}
        )
    endforeach()
endfunction(multiset_package_properties)

macro(feature_find_package PACKAGE_NAME)
    set(_WFP_options
        REQUIRED
        RECOMMENDED
        RUNTIME
        OPTIONAL

        CONFIG
        MODULE
        QUIET
        GLOBAL
        BYPASS_PROVIDER
        UNWIND_INCLUDE
    )

    set(_WFP_one_value_args)

    set(_WFP_multi_value_args
        COMPONENTS

        REQUIRED_COMPONENTS
        RECOMMENDED_COMPONENTS
        RUNTIME_COMPONENTS
        OPTIONAL_COMPONENTS
    )

    cmake_parse_arguments(WFP
        "${_WFP_options}"
        "${_WFP_one_value_args}"
        "${_WFP_multi_value_args}"
        ${ARGN}
    )

    #
    # Determine package TYPE.
    #
    set(_WFP_package_type OPTIONAL)
    set(_WFP_package_type_explicit FALSE)

    foreach(_WFP_type
        REQUIRED
        RECOMMENDED
        RUNTIME
        OPTIONAL
    )
        if(WFP_${_WFP_type})
            if(_WFP_package_type_explicit)
                message(FATAL_ERROR
                    "wrapped_find_package(${PACKAGE_NAME}): "
                    "multiple package TYPE keywords specified"
                )
            endif()

            set(_WFP_package_type "${_WFP_type}")
            set(_WFP_package_type_explicit TRUE)
        endif()
    endforeach()

    #
    # Build find_package() arguments.
    #
    # Intentionally do NOT pass:
    #
    #   REQUIRED
    #   OPTIONAL
    #   RECOMMENDED
    #   RUNTIME
    #
    set(_WFP_find_package_args)

    foreach(_WFP_option
        CONFIG
        MODULE
        QUIET
        GLOBAL
        BYPASS_PROVIDER
        UNWIND_INCLUDE
    )
        if(WFP_${_WFP_option})
            list(APPEND _WFP_find_package_args
                ${_WFP_option}
            )
        endif()
    endforeach()

    #
    # COMPONENTS are treated as REQUIRED_COMPONENTS
    # for compatibility with standard find_package().
    #
    set(_WFP_required_components
        ${WFP_COMPONENTS}
        ${WFP_REQUIRED_COMPONENTS}
    )

    if(_WFP_required_components)
        list(APPEND _WFP_find_package_args
            COMPONENTS
                ${_WFP_required_components}
        )
    endif()

    #
    # CMake only understands:
    #
    #   COMPONENTS
    #   OPTIONAL_COMPONENTS
    #
    # Therefore RECOMMENDED/RUNTIME components are
    # requested as OPTIONAL_COMPONENTS.
    #
    set(_WFP_optional_components
        ${WFP_OPTIONAL_COMPONENTS}
        ${WFP_RECOMMENDED_COMPONENTS}
        ${WFP_RUNTIME_COMPONENTS}
    )

    if(_WFP_optional_components)
        list(APPEND _WFP_find_package_args
            OPTIONAL_COMPONENTS
                ${_WFP_optional_components}
        )
    endif()

    find_package(
        ${PACKAGE_NAME}
        ${_WFP_find_package_args}
    )

    #
    # Package TYPE.
    #
    if(NOT _WFP_required_components
       AND NOT WFP_OPTIONAL_COMPONENTS
       AND NOT WFP_RECOMMENDED_COMPONENTS
       AND NOT WFP_RUNTIME_COMPONENTS)
        set_package_properties(
            ${PACKAGE_NAME}
            PROPERTIES
                TYPE ${_WFP_package_type}
        )
    endif()

    #
    # REQUIRED components.
    #
    if(_WFP_required_components)
        set(_WFP_packages)

        foreach(_WFP_component
            IN LISTS _WFP_required_components
        )
            list(APPEND _WFP_packages
                "${PACKAGE_NAME}${_WFP_component}"
            )
        endforeach()

        multiset_package_properties(
            ${_WFP_packages}
            PROPERTIES
                TYPE REQUIRED
        )

        unset(_WFP_packages)
    endif()

    #
    # RECOMMENDED components.
    #
    if(WFP_RECOMMENDED_COMPONENTS)
        set(_WFP_packages)

        foreach(_WFP_component
            IN LISTS WFP_RECOMMENDED_COMPONENTS
        )
            list(APPEND _WFP_packages
                "${PACKAGE_NAME}${_WFP_component}"
            )
        endforeach()

        multiset_package_properties(
            ${_WFP_packages}
            PROPERTIES
                TYPE RECOMMENDED
        )

        unset(_WFP_packages)
    endif()

    #
    # RUNTIME components.
    #
    if(WFP_RUNTIME_COMPONENTS)
        set(_WFP_packages)

        foreach(_WFP_component
            IN LISTS WFP_RUNTIME_COMPONENTS
        )
            list(APPEND _WFP_packages
                "${PACKAGE_NAME}${_WFP_component}"
            )
        endforeach()

        multiset_package_properties(
            ${_WFP_packages}
            PROPERTIES
                TYPE RUNTIME
        )

        unset(_WFP_packages)
    endif()

    #
    # OPTIONAL components.
    #
    if(WFP_OPTIONAL_COMPONENTS)
        set(_WFP_packages)

        foreach(_WFP_component
            IN LISTS WFP_OPTIONAL_COMPONENTS
        )
            list(APPEND _WFP_packages
                "${PACKAGE_NAME}${_WFP_component}"
            )
        endforeach()

        multiset_package_properties(
            ${_WFP_packages}
            PROPERTIES
                TYPE OPTIONAL
        )

        unset(_WFP_packages)
    endif()

    #
    # Cleanup macro variables.
    #
    unset(_WFP_options)
    unset(_WFP_one_value_args)
    unset(_WFP_multi_value_args)

    unset(_WFP_package_type)
    unset(_WFP_package_type_explicit)

    unset(_WFP_find_package_args)

    unset(_WFP_required_components)
    unset(_WFP_optional_components)

    unset(_WFP_option)
    unset(_WFP_type)
    unset(_WFP_component)

    unset(WFP_REQUIRED)
    unset(WFP_RECOMMENDED)
    unset(WFP_RUNTIME)
    unset(WFP_OPTIONAL)

    unset(WFP_CONFIG)
    unset(WFP_MODULE)
    unset(WFP_QUIET)
    unset(WFP_GLOBAL)
    unset(WFP_BYPASS_PROVIDER)
    unset(WFP_UNWIND_INCLUDE)

    unset(WFP_COMPONENTS)

    unset(WFP_REQUIRED_COMPONENTS)
    unset(WFP_RECOMMENDED_COMPONENTS)
    unset(WFP_RUNTIME_COMPONENTS)
    unset(WFP_OPTIONAL_COMPONENTS)

    unset(WFP_KEYWORDS_MISSING_VALUES)
    unset(WFP_UNPARSED_ARGUMENTS)
endmacro()


macro(return_on_missing_required_packages)
    #
    # FeatureSummary stores package names in the global property:
    #
    #   PACKAGES_FOUND
    #
    # and package metadata in:
    #
    #   _CMAKE_<package>_TYPE
    #   _CMAKE_<package>_FOUND
    #
    get_property(
        _ROMRP_packages
        GLOBAL
        PROPERTY PACKAGES_FOUND
    )

    foreach(_ROMRP_package IN LISTS _ROMRP_packages)
	    message(STATUS "package ${_ROMRP_package}")
        get_property(
            _ROMRP_type
            GLOBAL
            PROPERTY "_CMAKE_${_ROMRP_package}_TYPE"
        )
	    message(STATUS "type ${_ROMRP_type}")

        if(NOT _ROMRP_type STREQUAL "REQUIRED")
            continue()
        endif()

        get_property(
            _ROMRP_found
	    CACHE "${_ROMRP_package}_FOUND"
            PROPERTY "${_ROMRP_package}_FOUND"
        )
	    message(STATUS "found ${_ROMRP_found}")

        if("${_ROMRP_found}" MATCHES "NOT_FOUND")
            unset(_ROMRP_packages)
            unset(_ROMRP_package)
            unset(_ROMRP_type)
            unset(_ROMRP_found)

	    message(STATUS "NOT _ROMRP_found")

            return()
        endif()
    endforeach()

    unset(_ROMRP_packages)
    unset(_ROMRP_package)
    unset(_ROMRP_type)
    unset(_ROMRP_found)
endmacro()

macro(defer_on_missing_required_packages)
	block(PROPAGATE requiredPkgNotFound)
		set(requiredPkgNotFound TRUE)
		get_property(_ROMRP_reqpkgs GLOBAL PROPERTY FeatureSummary_REQUIRED_PKG_TYPES)
		get_property(_ROMRP_pkgnotfound GLOBAL PROPERTY PACKAGES_NOT_FOUND)
		foreach(_pkg ${_ROMRP_pkgnotfound})
			message(STATUS "pkg: ${_pkg}")
			foreach(_req_type ${_ROMRP_reqpkgs})
				get_property(_currentPkgType GLOBAL PROPERTY _CMAKE_${_pkg}_TYPE)
				message(STATUS "type: ${currentPkgType} req: ${_req_type}")
				if("${_currentPkgType}" STREQUAL "${_req_type}")
					set(requiredPkgNotFound TRUE)
					message(STATUS "REQUIRED package not found: ${_pkg}")
					return(PROPAGATE requiredPkgNotFound)
				endif()
			endforeach()
		endforeach()
		set(requiredPkgNotFound FALSE)	
	endblock()
	if(requiredPkgNotFound)
		unset(requiredPkgNotFound)
		message(STATUS "REQUIRED package not found")
		return()
	endif()
	unset(requiredPkgNotFound)
endmacro()
