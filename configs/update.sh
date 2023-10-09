#!/bin/bash
#
# Real-time OS
# Copyright (c) 2023, John Ryland
# All rights reserved.
#
# This script iterates over all the config files and compares each config file
# with a default template config file in null/config.h. It uses the diff and patch
# commands to do this, so the files need to be similar to start with, so there are
# assumptions that the files list the config options in alphabetical order and that
# only relatively small changes are made at a time. It does automate the process of
# updateing all the config files when a new config option is added to null/config.h.
# It can also remove options that that have been deleted, however the patch command
# will detect if an option was removed from the null/config.h file which was
# uncommented in one of the platform config.h file, and will ask if you want to apply
# the patch anyway. This is where the user needs to intervene and decide if this is
# what they want.
#

for CONFIG in */config.h
do
  # Save original version of the config file
  cp $CONFIG .tmp.config.orig

  # Copy back this original version but with all entires commented
  sed 's,^#def,//#def,' .tmp.config.orig > $CONFIG

  # Find the delta between this and a full/new null config file
  # (using stdin avoids being able to accidently patch null/config.h)
  cat null/config.h | diff -C 0 $CONFIG - > .tmp.patch

  # Move the original back with uncommented lines to the target location for patching
  mv .tmp.config.orig $CONFIG

  # Apply the diff which will add new commented lines from the new null config file
  patch < .tmp.patch

  # Clean up
  rm .tmp.patch
done

