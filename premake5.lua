local action = _ACTION or ""

solution "astera"
    location "build"
    configurations { "Debug", "Release" }
    platforms { "x64", "x32" }

    project "astera"
        kind "ConsoleApp"
        language "C"
        includedirs { "src", "dep", "dep/glfw/include", "dep/openal-soft/include", "dep/glfw/src", "dep/openal-soft", "dep/glad/include" }
        files { "src/*.h", "src/*.c" }
        targetdir "build"
        -- NOTE(dbechrd): Not sure what `-lm` switch does in Makefile
        links { "opengl32", "glfw3", "openal32" }

        filter "Debug"
            defines { "DEBUG" }
            symbols "On"
            warnings "Extra"

        filter "Release"
            defines { "NDEBUG" }
            optimize "On"
            warnings "Extra"

        filter { "system:windows" }
            -- openal32.lib
            -- opengl32.lib
            -- gdi32.lib
            -- user32.lib
            -- shell32.lib
            -- glfw3_d.lib
            -- ole32.lib

            --links { "gdi32" }
            links { "opengl32", "glfw3", "openal32" }
            defines { "_CRT_SECURE_NO_WARNINGS" }

        -- NOTE(dbechrd): I don't have the 32-bit libs, not sure if Astera is trying to support 32-bit Windows
        filter { "system:windows", "platforms:x32" }
            syslibdirs { "lib/Win32" }

        filter { "system:windows", "platforms:x64" }
            syslibdirs { "lib/Win64" }

        -- configuration { "linux" }
        --      linkoptions { "`pkg-config --libs glfw3`" }
        --      links { "foo" }

        -- configuration { "macosx" }
        --     links { "glfw3" }
        --     linkoptions { "-framework OpenGL", "-framework Cocoa", "-framework IOKit", "-framework CoreVideo" }


