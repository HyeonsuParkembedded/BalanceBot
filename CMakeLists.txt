cmake_minimum_required(VERSION 3.16.0)

# Exclude LCD components to avoid compilation issues
set(EXCLUDE_COMPONENTS "esp_lcd")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(BalanceBot)
