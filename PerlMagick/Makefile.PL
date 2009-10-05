# Copyright (C) 2003 GraphicsMagick Group
# Copyright (C) 2002 ImageMagick Studio
# Copyright (C) 1998,1999 E. I. du Pont de Nemours and Company
#
# This program is covered by multiple licenses, which are described in
# Copyright.txt. You should have received a copy of Copyright.txt with this
# package; otherwise see http://www.graphicsmagick.org/www/Copyright.html.
#

use ExtUtils::MakeMaker;
use Config;

# Compute test specification
my $delegate_tests='t/*.t';
my $delegate;
foreach $delegate (qw/ bzlib jng jp2 jpeg lcms png ps tiff ttf wmf x xfig zlib/) {
  if( -d "t/$delegate" ) {
    $delegate_tests .= " t/$delegate/*.t";
  }
}

# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile
  (
   # Module description
   'ABSTRACT'	=> 'GraphicsMagick PERL Extension',

   # Perl module name is Graphics::Magick
   'NAME'	=> 'Graphics::Magick',

   # Module author
   'AUTHOR'    => 'GraphicsMagick Group',

   # Module version
   'VERSION'   => '1.1.11',

   # Preprocessor defines
   'DEFINE'	=> ' -D_FILE_OFFSET_BITS=64 -D_LARGE_FILES=1  -DHAVE_CONFIG_H',     # e.g., '-DHAVE_SOMETHING' 

   # Header search specfication and preprocessor flags
   'INC'	=> '-I../ -I.. -I/usr/include/freetype2 -I/usr/include/X11 -I/usr/include/libxml2',

   # C pre-processor flags (e.g. -I & -D options)
   # 'CPPFLAGS'    => "$Config{'cppflags'} -I/usr/include/freetype2 -I/usr/include/X11 -I/usr/include/libxml2",

   # C compiler flags (e.g. -O -g)
   'CCFLAGS'     => "$Config{'ccflags'} -Wall -g -fno-strict-aliasing -O2 -Wall -pthread",

   # Linker flags for building an executable
   'LDFLAGS'    =>  "-L/usr/lib $Config{'ldflags'}",

   # Linker flags for building a dynamically loadable module
   'LDDLFLAGS'  => "-L/usr/lib $Config{'lddlflags'}",

   # Install PerlMagick binary into GraphicsMagick bin directory
   'INSTALLBIN'	=> '/usr/bin',

   # Library specification
   'LIBS'	=> ['-L/usr/lib -lGraphicsMagick -L/usr/lib/X11 -L/usr/lib -L/usr/lib -llcms -ltiff -lfreetype -ljasper -ljpeg -lpng -lwmflite -lXext -lSM -lICE -lX11 -lbz2 -lxml2 -lz -lm -lpthread'],

   # Perl binary name (if a Perl binary is built)
   'MAP_TARGET'	=> 'PerlMagick',

   # Let CFLAGS drive optimization flags by setting OPTIMIZE to empty
   # 'OPTIMIZE'	=> '',

   # Use same compiler as GraphicsMagick
   'PERLMAINCC'	=> '',

   # Set Perl installation prefix to GraphicsMagick installation prefix
#   'PREFIX'	=> '/usr',

   # Include delegate directories in tests
   test		=> { TESTS	=>	$delegate_tests},

   ($Config{'archname'} =~ /-object$/i ? ('CAPI' => 'TRUE') : ()),
);


#
# Substitutions for "makeaperl" section.
#
sub MY::makeaperl {
     package MY; # so that "SUPER" works right
     my $inherited = shift->SUPER::makeaperl(@_);

     # Stinky ExtUtils::MM_Unix likes to append its own library path to $(CC),
     # prior to any user-specified library path so that an installed library is
     # used rather than the library just built.  This substitution function
     # tries to insert our library path first. Also, use the same compiler used
     # to build perlmain.c to link so that a C++ compiler may be used if
     # necessary.
     $inherited =~ s:MAP_LINKCMD\s.*\s*\$\(CC\):MAP_LINKCMD   = \$(PERLMAINCC) -L/usr/lib: ;
     $inherited;
 }

#
# Substitutions for "test" section.
#
sub MY::test {
     package MY; # so that "SUPER" works right
     my $inherited = shift->SUPER::test(@_);

     # Run tests in our environment
     $inherited =~ s:PERL_DL_NONLAZY=1:/bin/sh ../rungm.sh PERL_DL_NONLAZY=1:g ;
     $inherited;
 }
