CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS

LIBS += -lgdi32


HEADERS += \
    math/rectI.h \
    math/vector2I.h \
    qrCodeGen/c/qrcodegen.h \
    render/frame.h \
    render/pixel.h \
    render/renderer.h \
    render/scene.h \
    renderable/renderable.h \
    renderable/sprite.h

SOURCES += \
        main.cpp \
        math/rectI.c \
        math/vector2I.c \
        qrCodeGen/c/qrcodegen.c \
        render/frame.c \
        render/pixel.c \
        render/renderer.c \
        render/scene.c \
        renderable/renderable.c \
        renderable/sprite.c


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
