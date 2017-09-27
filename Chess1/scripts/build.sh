#!/bin/bash

. bash_lib.sh

AddFlag -std=c++11
AddFlag -g

AddLib SDL2 /usr/local/Cellar/sdl2/2.0.4
AddLib SDL2_ttf /usr/local/Cellar/sdl2_ttf/2.0.14
AddLib SDL2_image /usr/local/Cellar/sdl2_image/2.0.1_1
AddInclude /usr/local/Cellar/nlohmann_json/2.0.9
AddIncludeRaw /usr/local/Cellar/sdl2/2.0.4/include/SDL2

Compile
