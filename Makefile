#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

export TOOLS 	:= $(CURDIR)/tools
export LIB 		:= $(CURDIR)/lib


.PHONY: all clean

all: spotify.rel

spotify.rel:
	$(MAKE) -s -C Spotify
	@cp Spotify/$@ $@

clean:
	@rm ./*.rel
	$(MAKE) -s -C Spotify clean