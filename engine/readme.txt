RedWolf Design - Clonk Planet Engine - Source Code

Clonk Planet features an extensive 2D game engine with intricate landscape 
physics, open object and scenario technology including it's own scripting 
language, and network capabilities.

As long as you obey the rules in license.txt, you may do with this code whatever 
you like. As I have learned most of my programming skills from other people's 
working source code, some people might learn something from this. Somebody might 
even want to try porting Clonk to Linux.

The engine has been developed over a time span of four years. Some of the code 
it contains is pretty old and messy stuff from non-object-oriented times. Other 
game classes are well organized and very reuseable. 

Since networking support was not planned in the original engine architecture
and nonetheless requires an extremely high level of landscape and object
synchronization, the network code is rather complicated and extremely sensitive
to changes.

If you don't really know Clonk yet, play the game first and start out using 
the built-in developer mode. You should really know about the workings of 
scenarios and objects on player and developer level before looking into the 
source code. 

The developer mode documentation will be your main source of information.
Documentation of the engine code itself is limited. I might expand documenation
on some engine code subjects as the need arises.

To compile this project you will need Microsoft Visual C++ 6.0 and the
DirectX SDK 6 or 7 (the project was not tested with higher versions of the SDK). 
I strongly recommend setting the tab size to 2 when viewing the code.

To run the game you will need a complete installation of the Clonk Planet 
shareware version.

If you intend to release your own version of the engine to run with
original game content, use a version number between 4.90.0 and 4.99.9
(see C4Include.h) Those numbers are not going to be used by original
versions of the game. Also specify an additional name in the version
resource such as "4.95.2 Joe's Engine". The ProductVersion string will be
shown in the frontend's info screen.

Recommended directory structure:

engine        Engine source code
standard      Utility source code
intermediate  Temp directory (automatically created)
planet        Output directory (install shareware version here)

That's it. Have fun.

Matthes Bender
December 2000
http://www.clonk.de