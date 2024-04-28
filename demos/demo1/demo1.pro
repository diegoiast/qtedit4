TEMPLATE	=	app
QT		=	gui core xml widgets
CONFIG		+=	qt warn_on silent
DESTDIR		=	../../
INCLUDEPATH	=	../../src .
LIBS		=	-L../../ -lqvshl
TARGET          =       demo1
SOURCES		=	main1.cpp  
