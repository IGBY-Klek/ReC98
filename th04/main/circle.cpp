#include "th04/main/circle.hpp"
#include "th04/hardware/grcg.hpp"
#include "th04/main/playfld.hpp"
#include "th02/main/entity.hpp"
#include "libs/master.lib/pc98_gfx.hpp"
#include "th04/main/item/item.hpp"

// More like 17 though, due to the quirks documented below.
static const int CIRCLE_FRAMES = 16;

// Note the slight semantic differences to the item_splash_t structure.
struct circle_t {
	entity_flag_t flag;
	unsigned char age;
	screen_point_t center;
	pixel_t radius_cur;
	pixel_t radius_delta;
};

static const int CIRCLE_COUNT = ((GAME == 5) ? 8 : 16);

extern circle_t circles[CIRCLE_COUNT];

#define circle_init(p, center_x, center_y, radius_delta_) { \
	p->flag = F_ALIVE; \
	p->age = 0; \
	p->center.x = (PLAYFIELD_LEFT + (center_x / SUBPIXEL_FACTOR)); \
	p->center.y = (PLAYFIELD_TOP  + (center_y / SUBPIXEL_FACTOR)); \
	p->radius_cur = ( \
		4 + ((radius_delta_ > 0) ? 0 : (CIRCLE_FRAMES * -(radius_delta_))) \
	); \
	p->radius_delta = radius_delta_; \
}

void pascal circles_add_growing(subpixel_t center_x, subpixel_t center_y)
{
	circle_t near *p;
	int i;
	for((p = circles, i = 0); i < CIRCLE_COUNT; (i++, p++)) {
		if(p->flag != F_FREE) {
			continue;
		}
		circle_init(p, center_x, center_y, 8);
		break;
	}
}

void pascal circles_add_shrinking(subpixel_t center_x, subpixel_t center_y)
{
	circle_t near *p;
	int i;
	for((p = circles, i = 0); i < CIRCLE_COUNT; (i++, p++)) {
		if(p->flag != F_FREE) {
			continue;
		}
		circle_init(p, center_x, center_y, -8);
		break;
	}
}

// Once an item has been triggered to be pulled to the player — by crossing
// the Point of Collection line, or by bombing — keep [items_pull_to_player]
// active until every one of those items has been collected or left the
// playfield. Without this, the pull would be lost as soon as the player drops
// back below the line or the bomb's item-collect phase ends, and items that
// were already halfway to the player would just stop and fall back down.
//
// This needs to run every frame, and before [items_update]. It also has to
// run during the tail end of a bomb, when [player_invalidate] doesn't, so
// [circles_update] is the ideal place for it.
void near items_pull_keep_alive(void)
{
	if(items_pull_to_player) {
		return;
	}
	for(int i = 0; i < ITEM_COUNT; i++) {
		if(items[i].flag == F_ALIVE && items[i].pulled_to_player) {
			items_pull_to_player = true;
			return;
		}
	}
}

void near circles_update(void)
{
	items_pull_keep_alive();

	circle_t near *p;
	int i;
	for((p = circles, i = 0); i < CIRCLE_COUNT; (i++, p++)) {
		if(p->flag == F_REMOVE) {
			p->flag = F_FREE;
		}
		if(p->flag != F_ALIVE) {
			continue;
		}

		// ZUN quirk: This runs before [boss_update] or the bomb update/render
		// function. Any circles spawned there will therefore bypass this
		// update on their first frame and render at their initial radius.
		p->radius_cur += p->radius_delta;
		p->age++;
		if(p->age > CIRCLE_FRAMES) {
			// ZUN quirk: Deferring the removal until the next update means
			// that this circle will still be rendered on this frame.
			p->flag = F_REMOVE;
		}
	}
}

void near circles_render(void)
{
	grcg_setcolor_direct(circles_color);
	circle_t near *p;
	int i;
	for((p = circles, i = 0); i < CIRCLE_COUNT; (i++, p++)) {
		if(p->flag != F_ALIVE) {
			continue;
		}
		grcg_circle(p->center.x, p->center.y, p->radius_cur);
	}
}
