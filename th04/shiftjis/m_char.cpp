static const int PLAYCHAR_TITLE_LEN = 24;
static const int PLAYCHAR_TITLE_LINES = 2;
static const int SHOTTYPE_TITLE_LEN = 24;
static const int SHOTTYPE_CHOOSE_LEN = 21;

const shiftjis_t* PLAYCHAR_TITLE[PLAYCHAR_COUNT][PLAYCHAR_TITLE_LINES] = {
	{ "　博麗靈夢（巫女小姐） ", "     広範囲攻撃型　    " },
	{ " 霧雨魔理沙（魔法使）", "     攻撃力重視型      " },
};

const shiftjis_t* SHOTTYPE_TITLE[PLAYCHAR_COUNT][SHOTTYPE_COUNT] = {
	{ "  自動追蹤　  ", "    廣泛射撃    " },
	{ "幻影激光", "   快速射撃   " },
};

#define SHOTTYPE_CLEARED "☆"
#define SHOTTYPE_CHOOSE "選択攻撃方式"
