CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS

LIBS += -lgdi32

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    example/windowbackend.h \
    math/mat3.h \
    math/mat4.h \
    math/types.h \
    math/vec2.h \
    math/vec3.h \
    math/vec4.h \
    render/backend.h \
    render/pixel.h \
    render/rasterizer.h \
    render/renderable.h \
    render/renderer.h \
    render/scene.h \
    render/sprite.h \
    render/texture.h

SOURCES += \
    example/main.c \
    example/windowbackend.c \
    math/mat3.c \
    math/mat4.c \
    math/vec2.c \
    math/vec3.c \
    math/vec4.c \
    render/pixel.c \
    render/rasterizer.c \
    render/renderable.c \
    render/renderer.c \
    render/scene.c \
    render/sprite.c \
    render/texture.c

