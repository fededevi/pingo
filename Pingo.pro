CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS

LIBS += -lgdi32

# MAIN
SOURCES += main.cpp


# MATH
HEADERS += \
    math/vec2.h \
    math/mat3.h

SOURCES += \
    math/vec2.c \
    math/mat3.c


# RENDER
HEADERS += \
    render/frame.h \
    render/pixel.h \
    render/renderer.h \
    render/scene.h \
    renderable/qrcode.h \
    renderable/renderable.h \
    renderable/sprite.h

SOURCES += \
    render/frame.c \
    render/pixel.c \
    render/renderer.c \
    render/scene.c \
    renderable/qrcode.c \
    renderable/renderable.c \
    renderable/sprite.c


# QRCODE
HEADERS += \
    qrCodeGen/c/qrcodegen.h

SOURCES += \
    qrCodeGen/c/qrcodegen.c


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
