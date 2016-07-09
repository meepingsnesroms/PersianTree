#include "palmwrapper.h"

string m68kstr(CPTR strptr){
	string little;
	char chr;
	while(true){
		chr = (char)get_byte(strptr);
		little += chr;
		//the null terminator must be appended incase the string is written back to palm memory
		if(chr == '\0')break;
		strptr++;
	}
	return little;
}

void memswap(UBYTE *start, size_t_68k size){
	UBYTE ex;
	offset_68k bunny;
	for(bunny = 0;bunny < size;bunny += 2){//no inc_for increments by 2
		ex = start[bunny];
		start[bunny] = start[bunny + 1];
		start[bunny + 1] = ex;
	}
}

void readbytearray(CPTR loc, UBYTE *dest, size_t_68k size){
	offset_68k goo;
	for(goo = 0;goo < size;goo++){
		dest[goo] = get_byte(loc + goo);
	}
}

//if size == 0 treat as string
void writebytearray(CPTR loc,UBYTE* start,size_t_68k size){
	offset_68k goo = 0;
	if(size == 0){
		while(true){
			put_byte(loc + goo,start[goo]);
			if(start[goo] == '\0')break;
			else goo++;
		}
	}
	else{
		for(;goo < size;goo++){
			put_byte(loc + goo,start[goo]);
		}
	}
}

CPTR emulink;

void pushlonglink(ULONG val){
	emulink -= 4;
	put_long(emulink,val);
}

ULONG poplonglink(){
	ULONG retval;
	retval = get_long(emulink);
	emulink += 4;
	return retval;
}

void pushwordlink(UWORD val){
	emulink -= 2;
	put_word(emulink,val);
}

UWORD popwordlink(){
	UWORD retval;
	retval = get_word(emulink);
	emulink += 2;
	return retval;
}

void pushbytelink(UBYTE val){
	emulink -= 1;
	put_byte(emulink,val);
}

UBYTE popbytelink(){
	UBYTE retval;
	retval = get_byte(emulink);
	emulink += 1;
	return retval;
}
