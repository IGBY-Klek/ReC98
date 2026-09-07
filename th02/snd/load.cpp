#pragma option -zCSHARED

#include "th02/snd/snd.h"
#include "th02/snd/impl.hpp"

extern char snd_load_fn[PF_FN_LEN];

void snd_load(const char fn[PF_FN_LEN], snd_load_func_t func)
{
	int i;
	_asm { push ds; }

	_CX = sizeof(snd_load_fn);
	i = 0;
	fn_copy: {
		snd_load_fn[i] = fn[i];
		i++;
		asm { loop	fn_copy; }
	}

	asm { mov	ax, func; }
	if((_AX == SND_LOAD_SONG) && snd_midi_active) {
		_BX = 0;
		do {
			_BX++;
		} while(snd_load_fn[_BX]);
		snd_load_fn[_BX+0] = 'm';
		snd_load_fn[_BX+1] = 'd';
		snd_load_fn[_BX+2] = 0;
	}

	// DOS file open
	(char near *)(_DX) = snd_load_fn;
	_AX = 0x3D00;
	geninterrupt(0x21);
	if(_FLAGS & 1) {
		// [MOD] Open failed; bail out instead of feeding the error code to
		// the sound driver as a file handle.
		_asm { pop ds; }
		return;
	}
	_BX = _AX;

	asm { mov	ax, func; }
	if((_AX == SND_LOAD_SONG) && snd_midi_active) {
		geninterrupt(MMD);
	} else {
		geninterrupt(PMD);
	}

	// DOS file read; song data address is in DS:DX
	_AX = 0x3F00;
	_CX = snd_load_size();
	geninterrupt(0x21);
	unsigned int buf_seg = _DS;
	unsigned int buf_off = _DX;

	_asm { pop ds; }

	// [DBG] dump the song data of the first MIDI song to a file
	if((func == SND_LOAD_SONG) && snd_midi_active) {
		static char dumpfn[] = "D:\\\\MMD_DUMP.MMD";
		static char dumped = 0;
		if(!dumped) {
			dumped = 1;
			_asm { push ds; }
			_AH = 0x3C; _CX = 0;
			(char near *)(_DX) = dumpfn;
			geninterrupt(0x21);
			if(!(_FLAGS & 1)) {
				_BX = _AX;
				_AH = 0x40;
				_CX = snd_load_size();
				_DS = buf_seg; _DX = buf_off;
				geninterrupt(0x21);
				_AH = 0x3E;
				geninterrupt(0x21);
			}
			_asm { pop ds; }
		}
	}

	// DOS file close
	_AH = 0x3E;
	geninterrupt(0x21);
}
