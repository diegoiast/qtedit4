TEMPLATE	=	app
QT		=	gui core xml widgets network concurrent
CONFIG		+=	qt warn_on silent
DESTDIR		=	../../
INCLUDEPATH	=	../../src .
LIBS		=	-L../../ -lqate
TARGET          =       demo6-qate
SOURCES		=	main6.cpp

# TODO - this should be removed
DEFINES         +=      CORE_EXPORT=Q_DECL_EXPORT
