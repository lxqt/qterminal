
#ifndef QTERMWIDGET_EXPORT_H
#define QTERMWIDGET_EXPORT_H

#ifdef QTERMWIDGET5_STATIC_DEFINE
#  define QTERMWIDGET_EXPORT
#  define QTERMWIDGET5_NO_EXPORT
#else
#  ifndef QTERMWIDGET_EXPORT
#    ifdef qtermwidget5_EXPORTS
        /* We are building this library */
#      define QTERMWIDGET_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define QTERMWIDGET_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef QTERMWIDGET5_NO_EXPORT
#    define QTERMWIDGET5_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef QTERMWIDGET5_DEPRECATED
#  define QTERMWIDGET5_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef QTERMWIDGET5_DEPRECATED_EXPORT
#  define QTERMWIDGET5_DEPRECATED_EXPORT QTERMWIDGET_EXPORT QTERMWIDGET5_DEPRECATED
#endif

#ifndef QTERMWIDGET5_DEPRECATED_NO_EXPORT
#  define QTERMWIDGET5_DEPRECATED_NO_EXPORT QTERMWIDGET5_NO_EXPORT QTERMWIDGET5_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef QTERMWIDGET5_NO_DEPRECATED
#    define QTERMWIDGET5_NO_DEPRECATED
#  endif
#endif

#endif
