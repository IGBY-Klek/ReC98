/// Game Over scoreboard + name entry (TH02/TH10+ style)
/// ---------------------------------------------------------------
/// Compiled into MAIN.EXE (BINARY == 'M') for both TH04 and TH05.
///
/// Layout and flow follow TH02's regist_menu():
///   * "HI SCORE" title + NAME/POINT/ST headers
///   * place-numbered rows (1..10) of name / score / stage
///   * a 3x17 alphabet grid for the name entry, entered AFTER the player
///     rejects the continue prompt
///   * uncolored (plain white text), no hi01.pi background
///
/// The alphabet grid uses TH04's own gaiji alphabet (th04/hiscore/
/// alphabet[data].asm) -- NOT TH02's, whose glyph values differ completely.
///
/// Flow (wired up in th04_main.asm / th05_main.asm):
///   GAME OVER animation -> overlay_wipe
///   -> gameover_scoreboard_put()   (draws the current playchar x rank table)
///   -> continue YES/NO prompt       (existing sub_E67A / sub_FAA3)
///   -> on rejection: gameover_regist()   (insert + name entry + save)
///   -> ES_SCORE -> palette fade -> GameExecl("op")
/// ---------------------------------------------------------------

#include "libs/master.lib/pc98_gfx.hpp"
#include "th02/hardware/frmdelay.h"
#include "th01/math/clamp.hpp"
#if (GAME == 5)
	#include "th05/hardware/input.h"
#else
	#include "th04/hardware/input.h"
#endif
#include "th04/main/playfld.hpp"

// scoredat.hpp, gaiji.h, playchar.h, score.hpp, stage.hpp, common.h, rank.h
// are already in scope, since this file is #included from score_rm.cpp after
// the hiscore unit.

// Loads [hi] with the score data for the current play character and the
// global [rank].
static void gameover_scoredat_load(void)
{
#if (GAME == 5)
	scoredat_load_for_cur();
#else
	hiscore_scoredat_load_for_cur();
#endif
}

// Saves [hi] for the current play character and the global [rank].
static void gameover_scoredat_save(void)
{
#if (GAME == 5)
	scoredat_save_cur();
#else
	hiscore_scoredat_save();
#endif
}

// Fills the playfield area of the text layer with transparent spaces.
void near overlay_wipe(void);

/// Coordinates (TRAM cells; each cell is one gaiji glyph)
/// ------------------------------------------------------
// Same layout as TH02's regist_menu(), which TH04's MAINE.EXE also reuses.
static const tram_cell_amount_t TABLE_TITLE_W = (8 * GAIJI_TRAM_W);
static const tram_cell_amount_t TABLE_HEADER_NAME_W = (4 * GAIJI_TRAM_W);
static const tram_cell_amount_t TABLE_HEADER_STAGE_W = (2 * GAIJI_TRAM_W);

static const tram_cell_amount_t TABLE_NAME_W = (
	SCOREDAT_NAME_LEN * GAIJI_TRAM_W
);
static const tram_cell_amount_t TABLE_SCORE_W = (SCORE_DIGITS * GAIJI_TRAM_W);

static const tram_x_t TABLE_PLACE_SINGLE_X = (
	PLAYFIELD_TRAM_LEFT + GAIJI_TRAM_W
);
static const tram_x_t TABLE_PLACE_DOUBLE_LEFT = (
	PLAYFIELD_TRAM_LEFT + (BINARY == 'M')
);

static const tram_x_t TABLE_NAME_LEFT = (
	TABLE_PLACE_SINGLE_X + (2 * GAIJI_TRAM_W)
);
static const tram_x_t TABLE_SCORE_LEFT = (
	TABLE_NAME_LEFT + TABLE_NAME_W + (2 * GAIJI_TRAM_W)
);
static const tram_x_t TABLE_STAGE_X = (
	TABLE_SCORE_LEFT + TABLE_SCORE_W + GAIJI_TRAM_W
);

static const tram_x_t TABLE_TITLE_LEFT = (
	PLAYFIELD_TRAM_CENTER_X - (TABLE_TITLE_W / 2)
);
static const tram_x_t TABLE_HEADER_NAME_LEFT = (
	TABLE_NAME_LEFT + ((TABLE_NAME_W / 2) - (TABLE_HEADER_NAME_W / 2))
);
static const tram_x_t TABLE_HEADER_SCORE_LEFT = (
	TABLE_SCORE_LEFT + 1 + (BINARY != 'M')
);
static const tram_x_t TABLE_HEADER_STAGE_LEFT = (
	TABLE_STAGE_X - (TABLE_HEADER_STAGE_W - GAIJI_TRAM_W) - (BINARY == 'M')
);

static const tram_y_t TABLE_TITLE_Y = (PLAYFIELD_TRAM_TOP + 1);
static const tram_y_t TABLE_HEADER_Y = (TABLE_TITLE_Y + 2);
static const tram_y_t TABLE_PLACES_TOP = (TABLE_HEADER_Y + 2);

inline tram_y_t table_place_y(int i) {
	return (TABLE_PLACES_TOP + i);
}

static const tram_x_t ALPHABET_LEFT = TABLE_NAME_LEFT;
static const tram_y_t ALPHABET_TOP = (TABLE_PLACES_TOP + SCOREDAT_PLACES + 2);
/// ------------------------------------------------------

/// Alphabet grid (3 rows x 17 columns), using TH04's own gaiji alphabet.
/// --------------------------------------------------------------------
// TH04's alphabet (th04/hiscore/alphabet[data].asm) is a flat 51-character
// table: A..Z, gs_DOT, gs_END, ♫♥!?‼⁉, 0..9, 点夢弾←→SPACE END.
// Laid out as 3x17, with the four "control" glyphs on the last four columns
// of row 2: col 13 = ← (backspace), col 14 = → (advance), col 15 = SPACE,
// col 16 = END (accept).
static const int GAMEOVER_ALPHABET_ROWS = 3;
static const int GAMEOVER_ALPHABET_COLS = 17;

static const unsigned char GAMEOVER_ALPHABET[3][17] = {
	{ gb_A, gb_B, gb_C, gb_D, gb_E, gb_F, gb_G, gb_H, gb_I, gb_J, gb_K,
	  gb_L, gb_M, gb_N, gb_O, gb_P, gb_Q },
	{ gb_R, gb_S, gb_T, gb_U, gb_V, gb_W, gb_X, gb_Y, gb_Z,
	  gs_DOT, gs_END, gs_NOTES, gs_HEART_2, gs_EXCLAMATION, gs_QUESTION,
	  gs_DOUBLE_EXCLAMATION, gs_EXCLAMATION_QUESTION },
	{ gb_0, gb_1, gb_2, gb_3, gb_4, gb_5, gb_6, gb_7, gb_8, gb_9,
	  gs_TEN, gs_YUME, gs_TAMA, gs_ARROW_LEFT, gs_ARROW_RIGHT, gs_SPACE,
	  gs_END },
};
// Control columns on row 2 (mirroring TH02's regist_menu behavior).
#define ALPHABET_CTRL_SPACE_COL 15
#define ALPHABET_CTRL_BACKSPACE_COL 13
#define ALPHABET_CTRL_ADVANCE_COL 14
#define ALPHABET_CTRL_END_COL 16
/// --------------------------------------------------------------------

// Renders a single row of the score table: name, score, and stage.
static void gameover_row_put(int place)
{
	int c;
	tram_y_t y = table_place_y(place);
	char score_buf[SCORE_DIGITS + 1];

	// Place number (1..10, with 10 rendered as two glyphs)
	if(place != (SCOREDAT_PLACES - 1)) {
		gaiji_putca(TABLE_PLACE_SINGLE_X, y, (gb_1 + place), TX_WHITE);
	} else {
		gaiji_putca(TABLE_PLACE_DOUBLE_LEFT, y, gb_1, TX_WHITE);
		gaiji_putca((TABLE_PLACE_DOUBLE_LEFT + GAIJI_TRAM_W), y, gb_0, TX_WHITE);
	}

	// Name
	gaiji_putsa(
		TABLE_NAME_LEFT, y,
		reinterpret_cast<const char *>(hi.score.g_name[place]), TX_WHITE
	);

	// Score (most significant digit first). The digits are already stored as
	// gaiji codes (gb_0 + value), so they can be written out verbatim.
	for(c = 0; c < SCORE_DIGITS; c++) {
		score_buf[c] = hi.score.g_score[place].digits[SCORE_DIGITS - 1 - c];
	}
	score_buf[SCORE_DIGITS] = g_NULL;
	gaiji_putsa(TABLE_SCORE_LEFT, y, score_buf, TX_WHITE);

	// Stage
	{
		unsigned char stage = hi.score.g_stage[place];
		if(stage == g_NONE) {
			stage = g_HISCORE_STAGE_EMPTY;
		}
		gaiji_putca(TABLE_STAGE_X, y, stage, TX_WHITE);
	}
}

// Renders the full score table, highlighting [highlight_place] (or none if
// negative). Uncolored: everything is rendered in plain white, matching the
// "remove the background and colors" design from AI.md.
static void gameover_table_draw(int highlight_place)
{
	int place;

	{
		static const char HI_SCORE[] = {
			gb_H, gb_I, gs_SPACE, gb_S, gb_C, gb_O, gb_R, gb_E, g_NULL
		};
		gaiji_putsa(TABLE_TITLE_LEFT, TABLE_TITLE_Y, HI_SCORE, TX_WHITE);
	}
	{
		static const char HDR_NAME[]  = { gb_N, gb_A, gb_M, gb_E, g_NULL };
		static const char HDR_SCORE[] = { gb_S, gb_C, gb_O, gb_R, gb_E, g_NULL };
		static const char HDR_STAGE[] = { gb_S, gb_T, gb_G, g_NULL };
		gaiji_putsa(
			TABLE_HEADER_NAME_LEFT, TABLE_HEADER_Y, HDR_NAME, TX_WHITE
		);
		gaiji_putsa(
			TABLE_HEADER_SCORE_LEFT, TABLE_HEADER_Y, HDR_SCORE, TX_WHITE
		);
		gaiji_putsa(
			TABLE_HEADER_STAGE_LEFT, TABLE_HEADER_Y, HDR_STAGE, TX_WHITE
		);
	}

	for(place = 0; place < SCOREDAT_PLACES; place++) {
		gameover_row_put(place);
	}
}

// Draws the score table for the current play character x rank.
// (The BGM switch to the name-registration track is done in the ASM call site,
// right before the continue prompt, so that it can be reverted on continue.)
void near gameover_scoreboard_put(void)
{
	gameover_scoredat_load();
	gameover_table_draw(-1);
}

// Returns the index where [score] should be inserted (0 ... SCOREDAT_PLACES-1),
// or -1 if [score] doesn't qualify for the table.
// Uses unsigned byte comparisons (the TH05 fix) to avoid the overflow-sorting
// bug from TH04.
static int gameover_score_find_place(void)
{
	int i;
	for(i = (SCOREDAT_PLACES - 1); i >= 0; i--) {
		int cmp = 0;
		int c;
		for(c = (SCORE_DIGITS - 1); c >= 0; c--) {
			unsigned char h = static_cast<unsigned char>(
				hi.score.g_score[i].digits[c] - gb_0
			);
			unsigned char s = score.digits[c];
			if(h < s) { cmp = -1; break; }
			if(h > s) { cmp =  1; break; }
		}
		if(cmp > 0) {
			break;
		}
	}
	if((i + 1) >= SCOREDAT_PLACES) {
		return -1;
	}
	return (i + 1);
}

// Shifts the table down and inserts [score] / the current stage / a default
// name at [place]. Leaves [hi] in decoded state for the name entry.
static void gameover_score_insert(int place)
{
	int j;
	int c;

	for(j = (SCOREDAT_PLACES - 2); j >= place; j--) {
		for(c = 0; c < SCOREDAT_NAME_LEN; c++) {
			hi.score.g_name[j + 1][c] = hi.score.g_name[j][c];
		}
		for(c = 0; c < SCORE_DIGITS; c++) {
			hi.score.g_score[j + 1].digits[c] = hi.score.g_score[j].digits[c];
		}
		hi.score.g_stage[j + 1] = hi.score.g_stage[j];
	}

	for(c = 0; c < SCORE_DIGITS; c++) {
		hi.score.g_score[place].digits[c] = (score.digits[c] + gb_0);
	}

	if(stage_id != STAGE_EXTRA) {
		hi.score.g_stage[place] = (gb_1 + stage_id);
	} else {
		hi.score.g_stage[place] = gb_1;
	}

	for(c = 0; c < SCOREDAT_NAME_LEN; c++) {
		hi.score.g_name[place][c] = gs_DOT;
	}
	hi.score.g_name[place][SCOREDAT_NAME_LEN] = g_NULL;
}

// Re-renders the name at [place], highlighting the character at [name_pos].
static void gameover_name_put(int place, int name_pos)
{
	gaiji_putsa(
		TABLE_NAME_LEFT, table_place_y(place),
		reinterpret_cast<const char *>(hi.score.g_name[place]), TX_WHITE
	);
	gaiji_putca(
		(TABLE_NAME_LEFT + (name_pos * GAIJI_TRAM_W)), table_place_y(place),
		hi.score.g_name[place][name_pos], (TX_GREEN | TX_REVERSE)
	);
}

static void gameover_alphabet_put(int row, int col, int atrb)
{
	gaiji_putca(
		(ALPHABET_LEFT + (col * GAIJI_TRAM_W)), (ALPHABET_TOP + row),
		GAMEOVER_ALPHABET[row][col], atrb
	);
}

// Inserts the current score into the table and asks for a name, saving the
// result. Called after the player rejects a continue.
void near gameover_regist(void)
{
	int place;
	int name_pos;
	int row;
	int col;
	int input_locked;
	unsigned char input_delay;

	overlay_wipe();
	gameover_scoredat_load();

	place = gameover_score_find_place();
	if(place < 0) {
		// Not good enough for the table: just show it and wait for a key.
		gameover_table_draw(-1);
		input_wait_for_change(0);
		return;
	}

	gameover_score_insert(place);
	gameover_table_draw(place);

	// Draw the alphabet grid.
	for(row = 0; row < GAMEOVER_ALPHABET_ROWS; row++) {
		for(col = 0; col < GAMEOVER_ALPHABET_COLS; col++) {
			gameover_alphabet_put(row, col, TX_WHITE);
		}
	}
	gameover_alphabet_put(0, 0, (TX_GREEN | TX_REVERSE));

	name_pos = 0;
	row = 0;
	col = 0;
	input_locked = 0;
	input_delay = 0;
	gameover_name_put(place, name_pos);

	while(1) {
		input_reset_sense_interface();
		if(!input_locked) {
			if(key_det & INPUT_UP) {
				gameover_alphabet_put(row, col, TX_WHITE);
				row = (row > 0) ? (row - 1) : (GAMEOVER_ALPHABET_ROWS - 1);
				gameover_alphabet_put(row, col, (TX_GREEN | TX_REVERSE));
			}
			if(key_det & INPUT_DOWN) {
				gameover_alphabet_put(row, col, TX_WHITE);
				row = (row < (GAMEOVER_ALPHABET_ROWS - 1)) ? (row + 1) : 0;
				gameover_alphabet_put(row, col, (TX_GREEN | TX_REVERSE));
			}
			if(key_det & INPUT_LEFT) {
				gameover_alphabet_put(row, col, TX_WHITE);
				col = (col > 0) ? (col - 1) : (GAMEOVER_ALPHABET_COLS - 1);
				gameover_alphabet_put(row, col, (TX_GREEN | TX_REVERSE));
			}
			if(key_det & INPUT_RIGHT) {
				gameover_alphabet_put(row, col, TX_WHITE);
				col = (col < (GAMEOVER_ALPHABET_COLS - 1)) ? (col + 1) : 0;
				gameover_alphabet_put(row, col, (TX_GREEN | TX_REVERSE));
			}
			if((key_det & INPUT_OK) || (key_det & INPUT_SHOT)) {
				if((row != 2) || (col < 13)) {
					// Regular character
					hi.score.g_name[place][name_pos] =
						GAMEOVER_ALPHABET[row][col];
					clamp_inc(name_pos, (SCOREDAT_NAME_LEN - 1));
				} else if(col == ALPHABET_CTRL_BACKSPACE_COL) {
					// <- : backspace
					clamp_dec(name_pos, 0);
					hi.score.g_name[place][name_pos] = gs_SPACE;
				} else if(col == ALPHABET_CTRL_ADVANCE_COL) {
					// -> : advance
					clamp_inc(name_pos, (SCOREDAT_NAME_LEN - 1));
				} else if(col == ALPHABET_CTRL_SPACE_COL) {
					// SPACE
					hi.score.g_name[place][name_pos] = gs_SPACE;
					clamp_inc(name_pos, (SCOREDAT_NAME_LEN - 1));
				} else {
					// END
					break;
				}
				gameover_name_put(place, name_pos);
			}
			if(key_det & INPUT_BOMB) {
				clamp_dec(name_pos, 0);
				hi.score.g_name[place][name_pos] = gs_SPACE;
				gameover_name_put(place, name_pos);
			}
			if(key_det & INPUT_CANCEL) {
				break;
			}
		}
		frame_delay(1);
		input_locked = key_det;
		if(input_locked) {
			input_delay++;
			if((input_delay > 30) && ((input_delay & 1) == 0)) {
				input_locked = 0;
			}
		} else {
			input_delay = 0;
		}
	}

	gameover_scoredat_save();
}
