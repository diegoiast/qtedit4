# QtEdit4 - a text editor build in Qt6  

![qtedit4](qtedit4.png)

qtedit4 is a text editor, which eventually will grow to be a full featured IDE.
It is still under heavy developement.

## History

The code was (once) hosted under [GoogleCode svn](https://code.google.com/archive/p/qtedit4/). Since,
Google [killed this project](https://killedbygoogle.com/) the code has been migrated to git, and is 
hosted in github. Each of the sub tools got its own git repo, (which was not trivial! but doable,
using `svndumpfilter` if you ask). Now, sub tools are loaded using CPM,
as child CMake projects.

Why `qtedit4`? When Qt3 was a thing QtDesginer had the ability to edit text, and was looking
like the idea was to make it a VisualBasic kind of IDE. Back then, the designer
had an internal text editor, I think the idea was to make QtCreator more like
VisualBasic. That was abandoned.

When Qt4 was released, my original code broke and I needed a soft reboot. I 
started a new project, called qtedit4. 

This means that some of the code originated in Qt3, and got migrated to Qt6 over the last
(too much) years.

## License
Code is GPLv2 or higher. 

## Releases
There is no stable release yet. Use the git's `master` HEAD as the "best" version.

