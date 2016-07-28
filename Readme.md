Setup
-----

The project has two dependencies : assimp and SOIL

To install assimp :
  * Download assimp v3.1.1 (the installator for 3.2 seems to be bugged) at http://www.assimp.org/main_downloads.html
  * Extract, open terminal in folder
  * cmake -G 'Unix Makefiles'
  * make
  * sudo make install

If the librairy was installed into /usr/local/lib, you may need to copy it into /usr/lib

 To install SOIL :
  * Download the latest SOIL version at http://www.lonesock.net/soil.html
  * Extract, open terminal in folder
  * Go to prokects/makefile
  * mv makefile Makefile
  * mkdir obj
  * make
  * sudo make install

