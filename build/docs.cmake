FIND_PACKAGE(Doxygen)
FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/docs)
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/docs/Doxyfile.in ${CMAKE_BINARY_DIR}/docs/Doxyfile.norev)
ADD_CUSTOM_TARGET(docs
                  sed -e s/%PROJREV%/`git rev-parse HEAD | cut -c1-10`/ < ${CMAKE_BINARY_DIR}/docs/Doxyfile.norev >${CMAKE_BINARY_DIR}/docs/Doxyfile
                  COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/docs/Doxyfile
                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/docs)
