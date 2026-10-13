workspace "UWr"
    configurations { "Debug", "Release" }
    architecture "x86_64"

project "Engine"
    kind "StaticLib"
    language "C++"
    targetdir "build/%{cfg.buildcfg}"
    cppdialect "C++20"

    files { "engine/**.hpp", "engine/**.cpp" }

    filter "configurations:*"
        includedirs { "vendor/include/", "." }
        libdirs { "vendor/lib/" }

    filter "system:windows"
        buildoptions { "/utf-8" }
        defines { "WINDOWS" }
        links { "opengl32", "dbghelp", "shell32", "ole32" }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        links { "sfml-system-d", "sfml-window-d", "sfml-graphics-d", "sfml-network-d", "sfml-audio-d", "glfw3"}
        postbuildcommands { "{COPY} %{wks.location}/vendor/bin/*.dll %{cfg.targetdir}"}

    filter "configurations:Release"
        defines { "RELEASE" }
        optimize "On"
        links { "sfml-system", "sfml-window", "sfml-graphics", "sfml-network", "sfml-audio", "glfw3"}
        postbuildcommands { "{COPY} %{wks.location}/vendor/bin/*.dll %{cfg.targetdir}"}

project "Tests"
  kind "ConsoleApp"
  language "C++"
  targetdir "build/%{cfg.buildcfg}"
  cppdialect "C++20"

  files { "tests/**.hpp", "tests/**.cpp" }

  filter "configurations:*"
    includedirs { "vendor/include/", "." }
    libdirs { "vendor/lib/" }

  filter "configurations:Debug"
    defines { "DEBUG" }
    symbols "On"
    links { "Engine" }

  filter "configurations:Release"
    defines { "RELEASE" }
    optimize "On"
    links { "Engine" }

project "Game"
  kind "ConsoleApp"
  language "C++"
  targetdir "build/%{cfg.buildcfg}"
  cppdialect "C++20"

  files { "game/**.hpp", "game/**.cpp" }

  filter "configurations:*"
    includedirs { "vendor/include/", "." }
    libdirs { "vendor/lib/" }

  filter "configurations:Debug"
    defines { "DEBUG" }
    symbols "On"
    links { "Engine" }

  filter "configurations:Release"
    defines { "RELEASE" }
    optimize "On"
    links { "Engine" }