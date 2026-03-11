#include <stdio.h>
#include <unistd.h>
#include <msettings.h>
#include <string.h>

#include "sdl.h"
#include "defines.h"
#include "api.h"
#include "utils.h"
#include "i18n.h"

typedef enum {
	STATE_MENU = 0,
	STATE_README,
	STATE_INFO,
	STATE_SITE,
	STATE_POWER,
	STATE_CONNECTIONS,
	STATE_NEW_WALLET,
	STATE_IMPORT_WALLET,
	STATE_KEYBOARD,
	STATE_MESSAGE
} AirlockState;

static const char *menu_items[] = {
	"Read Me",
	"Connections",
	"New Wallet",
	"Import Wallet",
	"System Info",
	"Official Site",
	"Shutdown",
	"Reboot"
};
static const int menu_item_count = sizeof(menu_items) / sizeof(menu_items[0]);

static const char *readme_lines[] = {
	"For asset safety, please read carefully:",
	"1. Seed phrase is the ONLY key to your funds;",
	"2. Ensure no cameras/people are watching;",
	"3. AirLock is offline, NEVER connect to internet;",
	"4. Verify device authenticity before use;",
	"5. Double-check address/amount before signing;",
	"6. Keep device fire/waterproof;",
	"7. Lost device? Recover via seed on new wallet;",
	"8. Obey local laws, no illegal activities.",
	"Visit website for help. Stay Safe!"
};
static const int readme_line_count = sizeof(readme_lines) / sizeof(readme_lines[0]);

static const char *info_lines[] = {
	"Name: Airlock",
	"HW: TrimUI Brick (tg5040)",
	"Screen: 1024x768",
	"OS: TinaLinux",
	"Version: 1.0"
};
static const int info_line_count = sizeof(info_lines) / sizeof(info_lines[0]);

static const char *site_lines[] = {
	"Official Website",
	"Beware of phishing sites"
};
static const int site_line_count = sizeof(site_lines) / sizeof(site_lines[0]);

static const char *power_lines[] = {
	"Power Options",
	"Shutdown System?",
	"Reboot System?",
	"(Not wired yet)"
};
static const int power_line_count = sizeof(power_lines) / sizeof(power_lines[0]);

static const char *placeholder_lines[] = {
	"Not implemented yet.",
	"Porting in progress."
};
static const int placeholder_line_count = sizeof(placeholder_lines) / sizeof(placeholder_lines[0]);

#define KB_ROWS 5
#define KB_COLS 14

static const char *keyboard_lower[KB_ROWS][KB_COLS] = {
	{"`", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", ""},
	{"q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "\\", ""},
	{"a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "", "", ""},
	{"z", "x", "c", "v", "b", "n", "m", ",", ".", "/", "", "", "", ""},
	{"shift", "space", "enter", "", "", "", "", "", "", "", "", "", "", ""}
};

static const char *keyboard_upper[KB_ROWS][KB_COLS] = {
	{"~", "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "_", "+", ""},
	{"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "|", ""},
	{"A", "S", "D", "F", "G", "H", "J", "K", "L", ":", "\"", "", "", ""},
	{"Z", "X", "C", "V", "B", "N", "M", "<", ">", "?", "", "", "", ""},
	{"shift", "space", "enter", "", "", "", "", "", "", "", "", "", "", ""}
};

static const char *keyboard_special[KB_ROWS][KB_COLS] = {
	{"~", "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "_", "+", ""},
	{"{", "}", "|", "\\", "<", ">", "?", "\"", ";", ":", "[", "]", "\\", ""},
	{"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "", ""},
	{".", ",", "/", "?", "!", "@", "#", "$", "%", "^", "&", "*", "", ""},
	{"shift", "space", "enter", "", "", "", "", "", "", "", "", "", "", ""}
};

typedef struct {
	int active;
	int row;
	int col;
	int layout;
	int max_len;
	char title[64];
	char text[256];
} KeyboardState;

typedef struct {
	const char *title;
	const char **lines;
	int line_count;
} MessageState;

static void blit_centered(SDL_Surface *dst, TTF_Font *font_face, const char *text, SDL_Color color, int y)
{
	SDL_Surface *label = TTF_RenderUTF8_Blended(font_face, text, color);
	if (!label) return;
	SDL_Rect pos = {(FIXED_WIDTH - label->w) / 2, y, 0, 0};
	SDL_BlitSurface(label, NULL, dst, &pos);
	SDL_FreeSurface(label);
}

static void blit_left(SDL_Surface *dst, TTF_Font *font_face, const char *text, SDL_Color color, int x, int y)
{
	SDL_Surface *label = TTF_RenderUTF8_Blended(font_face, text, color);
	if (!label) return;
	SDL_Rect pos = {x, y, 0, 0};
	SDL_BlitSurface(label, NULL, dst, &pos);
	SDL_FreeSurface(label);
}

static int compute_visible_rows(int start_y, int line_height, int footer_height)
{
	int available = FIXED_HEIGHT - start_y - footer_height;
	if (line_height <= 0) return 0;
	if (available < line_height) return 1;
	return available / line_height;
}

static void draw_footer(SDL_Surface *screen, SDL_Color color)
{
	blit_left(screen, font.tiny, "B: Back   A: Select", color, SCALE1(16), FIXED_HEIGHT - SCALE1(32));
}

static void draw_list(SDL_Surface *screen, const char *title, const char **lines, int line_count, int scroll)
{
	SDL_Color text = {255, 255, 255, 255};
	int title_y = SCALE1(16);
	int content_y = title_y + TTF_FontHeight(font.large) + SCALE1(12);
	int line_height = TTF_FontHeight(font.medium) + SCALE1(6);
	int visible_rows = compute_visible_rows(content_y, line_height, SCALE1(40));

	GFX_clear(screen);
	blit_centered(screen, font.large, title, text, title_y);

	for (int i = 0; i < visible_rows; i++) {
		int idx = scroll + i;
		if (idx >= line_count) break;
		blit_left(screen, font.medium, lines[idx], text, SCALE1(24), content_y + i * line_height);
	}

	draw_footer(screen, text);
}

static void draw_menu(SDL_Surface *screen, int selected)
{
	SDL_Color text = {255, 255, 255, 255};
	SDL_Color invert = {0, 0, 0, 255};
	int title_y = SCALE1(16);
	int content_y = title_y + TTF_FontHeight(font.large) + SCALE1(12);
	int line_height = TTF_FontHeight(font.medium) + SCALE1(8);
	int visible_rows = compute_visible_rows(content_y, line_height, SCALE1(40));
	int scroll = 0;

	if (selected >= visible_rows) {
		scroll = selected - (visible_rows - 1);
	}

	GFX_clear(screen);
	blit_centered(screen, font.large, "Airlock", text, title_y);

	for (int i = 0; i < visible_rows; i++) {
		int idx = scroll + i;
		if (idx >= menu_item_count) break;
		int y = content_y + i * line_height;
		if (idx == selected) {
			SDL_Rect rect = {SCALE1(16), y - SCALE1(2), FIXED_WIDTH - SCALE1(32), line_height};
			SDL_FillRect(screen, &rect, RGB_WHITE);
			blit_left(screen, font.medium, menu_items[idx], invert, SCALE1(24), y);
		} else {
			blit_left(screen, font.medium, menu_items[idx], text, SCALE1(24), y);
		}
	}

	draw_footer(screen, text);
}

static const char *kb_key_label(const char *key)
{
	if (!key || key[0] == '\0') return "";
	if (strcmp(key, "shift") == 0) return "SHIFT";
	if (strcmp(key, "space") == 0) return "SPACE";
	if (strcmp(key, "enter") == 0) return "ENTER";
	return key;
}

static const char *const (*kb_layout(int layout))[KB_COLS]
{
	if (layout == 1) return keyboard_upper;
	if (layout == 2) return keyboard_special;
	return keyboard_lower;
}

static int kb_row_length(const char *const row[KB_COLS])
{
	int len = 0;
	for (int i = 0; i < KB_COLS; i++) {
		if (row[i] && row[i][0] != '\0') len++;
	}
	return len;
}

static int kb_count_row_length(const char *const layout[KB_ROWS][KB_COLS], int row)
{
	if (row < 0 || row >= KB_ROWS) return 0;
	return kb_row_length(layout[row]);
}

static int kb_column_offset(const char *const layout[KB_ROWS][KB_COLS], int from_row, int to_row)
{
	int from_len = kb_count_row_length(layout, from_row);
	int to_len = kb_count_row_length(layout, to_row);
	return (to_len - from_len) / 2;
}

static int kb_adjust_exit_last_row(int offset, int column)
{
	if (column == 0) return offset - 1;
	if (column == 2) return offset + 1;
	return offset;
}

static int kb_adjust_enter_last_row(int offset, int col, int center)
{
	if (col > center) return offset - 1;
	if (col < center) return offset + 1;
	return offset;
}

static void kb_cursor_rescue(KeyboardState *kb, const char *const layout[KB_ROWS][KB_COLS])
{
	if (kb->row < 0) kb->row = 0;
	if (kb->row >= KB_ROWS) kb->row = KB_ROWS - 1;
	if (kb->col < 0) kb->col = 0;
	if (kb->col >= KB_COLS) kb->col = KB_COLS - 1;

	while (kb->col >= 0 && layout[kb->row][kb->col][0] == '\0') {
		kb->col--;
	}
	if (kb->col < 0) {
		kb->col = 0;
		while (kb->col < KB_COLS && layout[kb->row][kb->col][0] == '\0') {
			kb->col++;
		}
		if (kb->col >= KB_COLS) kb->col = 0;
	}
}

static void kb_draw(SDL_Surface *screen, const KeyboardState *kb)
{
	SDL_Color text = {255, 255, 255, 255};
	SDL_Color invert = {0, 0, 0, 255};
	const char *const (*layout)[KB_COLS] = kb_layout(kb->layout);

	GFX_clear(screen);

	if (kb->title[0] != '\0') {
		blit_centered(screen, font.large, kb->title, text, SCALE1(16));
	}

	{
		SDL_Surface *input = TTF_RenderUTF8_Blended(font.medium, kb->text, text);
		SDL_Surface *placeholder = TTF_RenderUTF8_Blended(font.medium, "p", text);
		SDL_Rect input_bg = {SCALE1(40), placeholder->h * 2, FIXED_WIDTH - SCALE1(80), placeholder->h + SCALE1(6)};
		SDL_FillRect(screen, &input_bg, SDL_MapRGB(screen->format, TRIAD_DARK_GRAY));
		if (input) {
			SDL_Rect input_pos = {(FIXED_WIDTH - input->w) / 2, input_bg.y + SCALE1(3), input->w, input->h};
			SDL_BlitSurface(input, NULL, screen, &input_pos);
			SDL_FreeSurface(input);
		}
		SDL_FreeSurface(placeholder);
	}

	int start_y = TTF_FontHeight(font.medium) * 4;
	int default_key_w = TTF_FontHeight(font.medium);
	int default_key_h = default_key_w;
	int row_spacing = 5;
	int col_spacing = 5;

	int shift_w = 0, space_w = 0, enter_w = 0;
	TTF_SizeUTF8(font.medium, "SHIFT", &shift_w, NULL);
	TTF_SizeUTF8(font.medium, "SPACE", &space_w, NULL);
	TTF_SizeUTF8(font.medium, "ENTER", &enter_w, NULL);
	int special_key_w = shift_w;
	if (space_w > special_key_w) special_key_w = space_w;
	if (enter_w > special_key_w) special_key_w = enter_w;
	special_key_w += (col_spacing * 4);

	for (int row = 0; row < KB_ROWS; row++) {
		int row_len = kb_count_row_length(layout, row);
		if (row_len == 0) continue;
		int total_w = (row_len * default_key_w) + ((row_len - 1) * col_spacing);
		if (row == 4) {
			total_w = (special_key_w * 3) + (2 * col_spacing);
		}
		int start_x = (FIXED_WIDTH - total_w) / 2;

		for (int col = 0; col < row_len; col++) {
			const char *key = layout[row][col];
			if (!key || key[0] == '\0') continue;
			int key_w = default_key_w;
			if (strcmp(key, "shift") == 0 || strcmp(key, "space") == 0 || strcmp(key, "enter") == 0) {
				key_w = special_key_w;
			}
			SDL_Rect key_rect = {
				start_x + (col * (key_w + col_spacing)),
				start_y + (row * (default_key_h + row_spacing)),
				key_w,
				default_key_h
			};
			int is_sel = (row == kb->row && col == kb->col);
			Uint32 bg = SDL_MapRGB(screen->format, is_sel ? TRIAD_WHITE : TRIAD_DARK_GRAY);
			SDL_FillRect(screen, &key_rect, bg);

			const char *label = kb_key_label(key);
			SDL_Surface *key_text = TTF_RenderUTF8_Blended(font.medium, label, is_sel ? invert : text);
			if (key_text) {
				SDL_Rect text_pos = {
					key_rect.x + (key_rect.w - key_text->w) / 2,
					key_rect.y + (key_rect.h - key_text->h) / 2,
					key_text->w,
					key_text->h
				};
				SDL_BlitSurface(key_text, NULL, screen, &text_pos);
				SDL_FreeSurface(key_text);
			}
		}
	}

	blit_left(screen, font.tiny, "A: Select  B: Backspace  X: OK  Y: Cancel", text, SCALE1(16), FIXED_HEIGHT - SCALE1(32));
}

static int kb_handle_input(KeyboardState *kb)
{
	const char *const (*layout)[KB_COLS] = kb_layout(kb->layout);
	int max_row = KB_ROWS;
	int max_col = KB_COLS;

	if (PAD_justRepeated(BTN_UP)) {
		if (kb->row > 0) {
			int offset = kb_column_offset(layout, kb->row, kb->row - 1);
			if (kb->row == max_row - 1) offset = kb_adjust_exit_last_row(offset, kb->col);
			kb->col += offset;
			kb->row--;
		} else {
			int offset = kb_column_offset(layout, 0, max_row - 1);
			int row_length = kb_count_row_length(layout, 0);
			int center = (row_length - 1) / 2;
			if (!((row_length & 1) == 0 && kb->col == center - 1))
				offset = kb_adjust_enter_last_row(offset, kb->col, center);
			kb->col += offset;
			kb->row = max_row - 1;
		}
		kb_cursor_rescue(kb, layout);
		return 0;
	}
	if (PAD_justRepeated(BTN_DOWN)) {
		if (kb->row < max_row - 1) {
			int offset = kb_column_offset(layout, kb->row, kb->row + 1);
			int row_length = kb_count_row_length(layout, kb->row);
			int center = (row_length - 1) / 2;
			if (kb->row + 1 == max_row - 1 && (kb->col > center || (row_length & 1 && kb->col < center)))
				offset = kb_adjust_enter_last_row(offset, kb->col, center);
			kb->col += offset;
			kb->row++;
		} else {
			int offset = kb_column_offset(layout, max_row - 1, 0);
			offset = kb_adjust_exit_last_row(offset, kb->col);
			kb->col += offset;
			kb->row = 0;
		}
		kb_cursor_rescue(kb, layout);
		return 0;
	}
	if (PAD_justRepeated(BTN_LEFT)) {
		if (kb->col > 0) kb->col--;
		else kb->col = max_col - 1;
		kb_cursor_rescue(kb, layout);
		return 0;
	}
	if (PAD_justRepeated(BTN_RIGHT)) {
		kb->col++;
		if (kb->col >= max_col) kb->col = 0;
		kb_cursor_rescue(kb, layout);
		return 0;
	}
	if (PAD_justPressed(BTN_SELECT)) {
		kb->layout = (kb->layout + 1) % 3;
		kb_cursor_rescue(kb, kb_layout(kb->layout));
		return 0;
	}
	if (PAD_justPressed(BTN_B)) {
		size_t len = strlen(kb->text);
		if (len > 0) kb->text[len - 1] = '\0';
		return 0;
	}
	if (PAD_justPressed(BTN_Y)) {
		return 1;
	}
	if (PAD_justPressed(BTN_X)) {
		return 2;
	}
	if (PAD_justPressed(BTN_A)) {
		const char *key = layout[kb->row][kb->col];
		if (!key || key[0] == '\0') return 0;
		if (strcmp(key, "shift") == 0) {
			kb->layout = (kb->layout + 1) % 3;
			kb_cursor_rescue(kb, kb_layout(kb->layout));
		} else if (strcmp(key, "space") == 0) {
			size_t len = strlen(kb->text);
			if (len + 1 < (size_t)kb->max_len) {
				kb->text[len] = ' ';
				kb->text[len + 1] = '\0';
			}
		} else if (strcmp(key, "enter") == 0) {
			return 2;
		} else {
			size_t len = strlen(kb->text);
			if (len + strlen(key) < (size_t)kb->max_len) {
				strcat(kb->text, key);
			}
		}
		return 0;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	PWR_setCPUSpeed(CPU_SPEED_MENU);

	SDL_Surface *screen = GFX_init(MODE_MAIN);
	PAD_init();
	PWR_init();
	InitSettings();
	I18N_init();

	SDL_Event event;
	int quit = 0;
	AirlockState state = STATE_MENU;
	int menu_idx = 0;
	int scroll = 0;
	KeyboardState kb = {0};
	MessageState msg = {0};

	while (!quit) {
		GFX_startFrame();
		PAD_poll();

		if (PAD_justPressed(BTN_MENU)) {
			quit = 1;
		}

		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quit = 1;
			}
		}

		if (state == STATE_MENU) {
			if (PAD_justPressed(BTN_UP)) {
				menu_idx = (menu_idx - 1 + menu_item_count) % menu_item_count;
			} else if (PAD_justPressed(BTN_DOWN)) {
				menu_idx = (menu_idx + 1) % menu_item_count;
			} else if (PAD_justPressed(BTN_A) || PAD_justPressed(BTN_START)) {
				scroll = 0;
				switch (menu_idx) {
				case 0: state = STATE_README; break;
				case 1: state = STATE_CONNECTIONS; break;
				case 2:
					state = STATE_KEYBOARD;
					memset(&kb, 0, sizeof(kb));
					kb.active = 1;
					kb.layout = 0;
					kb.row = 0;
					kb.col = 0;
					kb.max_len = (int)sizeof(kb.text);
					snprintf(kb.title, sizeof(kb.title), "Set 25th Phrase");
					break;
				case 3:
					state = STATE_KEYBOARD;
					memset(&kb, 0, sizeof(kb));
					kb.active = 1;
					kb.layout = 0;
					kb.row = 0;
					kb.col = 0;
					kb.max_len = (int)sizeof(kb.text);
					snprintf(kb.title, sizeof(kb.title), "Set Payment Password");
					break;
				case 4: state = STATE_INFO; break;
				case 5: state = STATE_SITE; break;
				case 6: state = STATE_POWER; break;
				case 7: state = STATE_POWER; break;
				default: break;
				}
			} else if (PAD_justPressed(BTN_B)) {
				quit = 1;
			}
		} else if (state == STATE_KEYBOARD) {
			int action = kb_handle_input(&kb);
			if (action == 1) {
				state = STATE_MENU;
			} else if (action == 2) {
				msg.title = "Saved";
				msg.lines = placeholder_lines;
				msg.line_count = placeholder_line_count;
				state = STATE_MESSAGE;
				scroll = 0;
			}
		} else if (state == STATE_MESSAGE) {
			if (PAD_justPressed(BTN_B) || PAD_justPressed(BTN_A)) {
				state = STATE_MENU;
			}
		} else {
			int line_height = TTF_FontHeight(font.medium) + SCALE1(6);
			int title_y = SCALE1(16);
			int content_y = title_y + TTF_FontHeight(font.large) + SCALE1(12);
			int visible_rows = compute_visible_rows(content_y, line_height, SCALE1(40));
			int max_scroll = 0;

			if (state == STATE_README) max_scroll = (readme_line_count > visible_rows) ? readme_line_count - visible_rows : 0;
			if (state == STATE_INFO) max_scroll = (info_line_count > visible_rows) ? info_line_count - visible_rows : 0;
			if (state == STATE_SITE) max_scroll = (site_line_count > visible_rows) ? site_line_count - visible_rows : 0;
			if (state == STATE_POWER) max_scroll = (power_line_count > visible_rows) ? power_line_count - visible_rows : 0;
			if (state == STATE_CONNECTIONS || state == STATE_NEW_WALLET || state == STATE_IMPORT_WALLET) {
				max_scroll = (placeholder_line_count > visible_rows) ? placeholder_line_count - visible_rows : 0;
			}

			if (PAD_justPressed(BTN_UP) && scroll > 0) {
				scroll--;
			} else if (PAD_justPressed(BTN_DOWN) && scroll < max_scroll) {
				scroll++;
			} else if (PAD_justPressed(BTN_B)) {
				state = STATE_MENU;
			} else if (PAD_justPressed(BTN_A)) {
				/* no-op */
			}
		}

		switch (state) {
		case STATE_MENU:
			draw_menu(screen, menu_idx);
			break;
		case STATE_README:
			draw_list(screen, "Important Notice", readme_lines, readme_line_count, scroll);
			break;
		case STATE_INFO:
			draw_list(screen, "System Info", info_lines, info_line_count, scroll);
			break;
		case STATE_SITE:
			draw_list(screen, "Official Website", site_lines, site_line_count, scroll);
			break;
		case STATE_POWER:
			draw_list(screen, "Power Options", power_lines, power_line_count, scroll);
			break;
		case STATE_CONNECTIONS:
			draw_list(screen, "Connections", placeholder_lines, placeholder_line_count, scroll);
			break;
		case STATE_NEW_WALLET:
			draw_list(screen, "New Wallet", placeholder_lines, placeholder_line_count, scroll);
			break;
		case STATE_IMPORT_WALLET:
			draw_list(screen, "Import Wallet", placeholder_lines, placeholder_line_count, scroll);
			break;
		case STATE_KEYBOARD:
			kb_draw(screen, &kb);
			break;
		case STATE_MESSAGE:
			if (msg.title && msg.lines) {
				draw_list(screen, msg.title, msg.lines, msg.line_count, scroll);
			}
			break;
		default:
			draw_menu(screen, menu_idx);
			break;
		}

		GFX_flip(screen);
	}

	PAD_quit();
	return 0;
}
