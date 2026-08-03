feature_find_package(OpenGL REQUIRED)
feature_find_package(GLEW REQUIRED)
feature_find_package(Ktx 4.4 REQUIRED CONFIG)

defer_on_missing_required_packages()

add_library(engine_graphics_opengl)
target_sources(engine_graphics_opengl
        PUBLIC
        FILE_SET public_headers
        TYPE HEADERS
        BASE_DIRS include
        FILES
            include/engine/backends/graphics/opengl_backend.h
        PRIVATE
        FILE_SET private_headers
        TYPE HEADERS
        BASE_DIRS private
        FILES
            private/OpenGLBuffer.h
            private/OpenGLVertexArray.h
            private/OpenGLTexture.h
            private/OpenGLShaderProgram.h
            private/opengl_logging.h
        PRIVATE
            src/opengl_backend.cpp
            src/OpenGLBuffer.cpp
            src/OpenGLVertexArray.cpp
            src/OpenGLShaderProgram.cpp
            src/OpenGLTexture.cpp
)
target_link_libraries(engine_graphics_opengl PUBLIC engine::engine GLEW::GLEW OpenGL::OpenGL KTX::ktx)