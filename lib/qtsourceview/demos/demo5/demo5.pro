TEMPLATE	=	app
QT		=	gui core xml widgets
CONFIG		+=	qt warn_on silent
DESTDIR		=	../../
INCLUDEPATH	=	../../src .
LIBS		=	-L../../ -lqsvte -lqvshl
TARGET          =       demo5
SOURCES		=	main5.cpp
