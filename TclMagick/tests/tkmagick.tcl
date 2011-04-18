# tkmagick.tcl -- Test the TkMagick functions.

# $Id: tkmagick.tcl,v 1.1.2.3 2005/01/22 19:35:06 bfriesen Exp $

package require Tk

set dirname [file dirname [info script]]

load [file join $dirname .. generic .libs libTclMagick[info sharedlibextension]]
load [file join $dirname .. generic .libs libTkMagick[info sharedlibextension]]

# magickimage --
#
#	Takes a file name, and returns the name of an image that can
#	be used with Tk.

proc magickimage {filename} {
    set magimg [magick create wand]
    set tkimg [image create photo]
    $magimg ReadImage $filename
    magicktophoto $magimg $tkimg
    magick delete $magimg
    return $tkimg
}

proc saveimage {} {
    global poolphoto
    set magimg [magick create wand]
    phototomagick $poolphoto $magimg
    $magimg WriteImage tmp.jpg
    magick delete $magimg
}

set poolphoto [magickimage [file join $dirname .. images pool.jpg]]
label .im
.im configure -image $poolphoto
pack .im
bind . <q> exit
bind . <s> saveimage