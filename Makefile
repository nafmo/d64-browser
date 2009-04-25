# Makefile for D64 browser

d64.cgi: d64.cc
	g++ -o d64.cgi d64.cc

clean:
	-rm d64.cgi

all: d64.cgi

.PHONY: all
