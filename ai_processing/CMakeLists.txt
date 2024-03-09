add_library(ai_processing STATIC )
add_library(ai_processing::core ALIAS ai_processing )
target_include_directories(ai_processing PUBLIC include)
add_subdirectory(source)

target_link_libraries(ai_processing PUBLIC
  simple_enum::simple_enum
  small_vectors::small_vectors
  stralgo::stralgo
  Boost::system
  Boost::thread
  OpenSSL::SSL
  OpenSSL::Crypto
  fmt::fmt
  glaze::glaze
)

target_compile_options( ai_processing PUBLIC ${BUILD_CXX_FLAGS} ${BASIC_CXX_FLAGS})
