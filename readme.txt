This is QtEdit4 - a text editor build in Qt4

Compiling
---------
The code currently compiles using Qt4, and Qt5. 

To build using Qt4 + cmake:

  mkdir cbuild
  cmake ../
  make -j 4
  
  
To build using Qt5 + qmake5:
  mkdir qbuild
  cd qbuild
  qmake ../ CONFIG+=silent
  make -j 4
  
 
Note that building using qmake will also work when using Qt4.

Code compiles and runs on Windows and Linux. I have some rather funny crashes on Mac, which
might be related to qmdilib, I am stil debugging.

License
-------
Code is GPLv2 or higher. 

Releases
--------
There is no stable release yet. Use the svn's HEAD as the "best" version.

- diego