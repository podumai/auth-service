cmake_minimum_required(VERSION 3.30)
project(
  auth_service
  VERSION 1.0.0
  LANGUAGES CXX
)

option(BUILD_TESTS "Build unit tests for auth-service" ON)
option(BUILD_BENCHMARKS "Build benchmarks for auth-service" OFF)

add_subdirectory(auth-service)