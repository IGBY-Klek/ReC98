#include "th02/common.h"
#include "shiftjis.hpp"

const shiftjis_t *DESC[SHOTTYPE_COUNT][3] = {
	" 不使用陰陽玉的力量 ",
	" 廣範圍且注重機動力 ",
	"　強化高機動力形態　",

	" 　靈撃性能優秀 　",
	"　 性能保持平衡　 ",
	"　　防禦重視型形態　　",

	" 　使用陰陽玉之力戰鬥   ",
	"　攻撃力優秀　",
	"　　攻撃重視形態　　"
};
const shiftjis_t *CHOOSE = "請從下面三個選項中，選擇靈夢的戰鬥風格齷";
const shiftjis_t *EXTRA_NOTE[] = {
	"注意）ＥＸＴＲＡ模式時，無法對難易度、初始自機數及初始ＢＯＭＢ數進行更改",
	"這些設定將固定為：難易度ＥＸＴＲＡ、初期自機數３人以及１只ＢＯＭＢ"
};
const shiftjis_t *CLEARED = "  ☆☆ＣＬＥＡＲＥＤ☆☆  ";
