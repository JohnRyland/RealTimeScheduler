#
#  Bootstrap Makefile
#  (C) Copyright 2022
#  John Ryland
#

all: release

.modules/GenericMake/Generic.mak:
	@echo "Fetching GenericMake"
	@git clone https://github.com/JohnRyland/GenericMake.git ./.modules/GenericMake

-include .modules/GenericMake/Generic.mak

