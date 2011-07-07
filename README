Foobar2000 Squeezebox server plug-in

Description
-----------

This is a Foobar2000 plug-in that emulates a Squeezebox server.
Simply point your Squeezebox player to the IP address where Foobar 
is running and whatever that is being played will be streamed to
the player.  This essentially makes your player a remote output
for Foobar.

This software was developed with the Squeezebox Receiver, which
does not have a user-interface.  To change the IP address of
this device, use the Net::UDAP configuration tool:

http://projects.robinbowes.com/Net-UDAP

For other Squeezebox players, you can probably set this in the
configuration menu.  Note that the plug-in does not currently
support text output of the song title to the player, since I
do not have a player that has a display.


Use and Configuration
---------------------

Slim port is the slim protocol control port, which defaults to 3483.
HTTP port is the streaming port, which defaults to 9000.  
The bits per sample setting sets whether 16-bit or 24-bit audio 
is sent to the player.  

Note that the sampling rate is set by the playback capture.
Some squeezebox player such as the Receiver have a hardware
limit on the SPDIF output of 48 kHz.  I recommend using the
the foobar SoX resampler plugin 

(http://www.hydrogenaudio.org/forums/index.php?showtopic=67373)

to downsample rates above 48 kHz.  If you use the modified
0.6.0 version, you can specify that only rates above 48 kHz
are downsampled, while passing through anything below that.

You can point your web browser to port 9000 to get a list of
active clients.


Compilation
-----------

You will need:

Foobar2000 SDK (http://www.foobar2000.org/SDK)
Include files from the Windows Template Library (http://sourceforge.net/projects/wtl/)
Boost C++ libraries (www.boost.org) (You can find pre-built binaries at boostpro.com)
Boost HTXML (http://www.ibusy.com/articles/software-development/boost-htxml-library.html)

Project and solution files are currently for Visual Studio 2010.