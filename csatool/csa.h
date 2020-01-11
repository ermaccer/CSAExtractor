#pragma once

#define NOSFERATU_FBI_LABEL 0
#define STAR_GAMES_LABEL 1048576


struct csa_header {
	int    header; // GEEK
	int    version; 
	int    firstFileOffset;
	int    gameLabel;
	int    files;
	int    mainStructSize; // same as firstFileOffset
	int    unknown;
};

struct csa_entry {
	char  fileName[128] = {};
	int   offset;
	int   size;
};