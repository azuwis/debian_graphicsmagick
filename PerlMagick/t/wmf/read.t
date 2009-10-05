#!/usr/local/bin/perl
# Copyright (C) 2003 GraphicsMagick Group
# Copyright (C) 2002 ImageMagick Studio
# Copyright (C) 1991-1999 E. I. du Pont de Nemours and Company
#
# This program is covered by multiple licenses, which are described in
# Copyright.txt. You should have received a copy of Copyright.txt with this
# package; otherwise see http://www.graphicsmagick.org/www/Copyright.html.
#
#
# Test reading WMF files
#
# Whenever a new test is added/removed, be sure to update the
# 1..n ouput.
#
BEGIN { $| = 1; $testnr=1; print "1..4\n"; }
END {print "not ok $testnr\n" unless $loaded;}
use Graphics::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't/wmf' || die 'Cd failed';

$test="$testnr - wizard test image # TODO Not robust against changes in libwmf";
testReadCompare('wizard.wmf', '../reference/wmf/wizard.miff',
                q//, 0.0002, 0.004);
++$testnr;
$test="$testnr - clock test image # TODO Not robust against changes in libwmf";
testReadCompare('clock.wmf', '../reference/wmf/clock.miff',
                q//, 0.0002, 0.004);
++$testnr;
$test="$testnr - ski test image # TODO Not robust against changes in libwmf";
testReadCompare('ski.wmf', '../reference/wmf/ski.miff',
                q//, 0.0002, 0.008);
++$testnr;
$test="$testnr - fjftest image # TODO Not robust against changes in libwmf";
testReadCompare('fjftest.wmf', '../reference/wmf/fjftest.miff',
                q//, 0.0003, 0.13);

