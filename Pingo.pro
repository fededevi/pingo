CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS

LIBS += -lgdi32

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    math/mat3.h \
    math/vec2.h \
    render/backend.h \
    render/frame.h \
    render/pixel.h \
    render/renderable.h \
    render/renderer.h \
    render/scene.h \
    render/sprite.h

SOURCES += \
    math/mat3.c \
    math/vec2.c \
    render/frame.c \
    render/pixel.c \
    render/renderable.c \
    render/renderer.c \
    render/scene.c \
    render/sprite.c

