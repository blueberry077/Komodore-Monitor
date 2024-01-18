/* Include Standard Libraries */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <raylib.h>
/* Macros ................... */
#define CH_WIDTH		(45)
#define CH_HEIGHT		(40)
#define CH_SZ			(16)
#define WD_WIDTH		(CH_WIDTH * CH_SZ)
#define WD_HEIGHT		(CH_HEIGHT * CH_SZ)
#define CH_OFF			(3)
#define CH_SUBOFF		(CH_OFF*2)
#define CH_OFFX			(CH_SZ*CH_OFF)
#define CH_OFFY			(CH_SZ*CH_OFF)
#define SCR_MAX_X		(CH_WIDTH-CH_OFF*2)
#define SCR_MAX_Y		(CH_HEIGHT-CH_OFF*2)
/* Structures ............... */
struct v2i { int x, y; };
struct cursor {
	struct v2i p;
	int ds, t;
};
struct System {
	uint16_t PC;			// Program Counter
	uint16_t SP;			// Stack Pointer	
	char F;					// Flags
	char A;
	char B;
	char C;
	char D;
	char E;
	char H;
	char L;
	char I;
	char R;
	char IX;				// Index X
	char IY;				// Index Y
	char ROM[0xFFFF];		// Internal ROM
	char RAM[0xFFFF];		// Internal RAM
};
/* Global Variables ......... */
int frame_counter = 0;
char screen_data[SCR_MAX_X*SCR_MAX_Y] = {0};
Color border_color = BLACK;
Color background_color = BLACK;
struct System sys = {0};
/* Functions Prototypes ..... */
char sys_program_rom(void);
char sys_read_byte(uint16_t addr);
void sys_write_byte(uint16_t addr, char v);
//
void k_set_char(char c, int x, int y);
void k_set_char_at_cursor(char c, struct cursor * cs);
void k_move_cursor(struct cursor * cs, int dx, int dy);
void k_set_cursor(struct cursor * cs, int nx, int ny);
void k_set_text(char * c, int x, int y);
void k_render_text(Font font);
void k_draw_cursor(struct cursor * cs, int t, Color c);
/* Entry Point .............. */
int main(void)
{
	// Init
	InitWindow(WD_WIDTH, WD_HEIGHT, "Komodore Monitor v1.0");
	SetTargetFPS(60);
	// Variables
	struct cursor c = {
		.p	= {0, 5},
		.ds	= 1,
		.t	= 0,
	};
	Font font = LoadFontEx("fonts/atascii.ttf", CH_SZ, NULL, 0);
	// Init Microcode
	k_set_text("     *** Komodore Monitor v1.0 ***     ", 0, 1);
	k_set_text("READY.", 0, 4);
	// Loop
	while (!WindowShouldClose()) {
		// Updating
		// Move Cursor
		if (IsKeyPressed(KEY_UP))		{ k_move_cursor(&c, 0, -1); }
		if (IsKeyPressed(KEY_DOWN))		{ k_move_cursor(&c, 0, 1);  }
		if (IsKeyPressed(KEY_LEFT))		{ k_move_cursor(&c, -1, 0); }
		if (IsKeyPressed(KEY_RIGHT))	{ k_move_cursor(&c, 1, 0);  }
		// Write Characters
		int ch;
		if ((ch = GetCharPressed()) > 0) {
			if ((ch >= 32) && (ch <= 125)) {
				k_set_char_at_cursor(ch, &c);
			}
		}
		if (IsKeyPressed(KEY_ENTER)) {
			k_set_cursor(&c, 0, c.p.y + 1);
		}
		if (IsKeyPressed(KEY_BACKSPACE)) {
			k_move_cursor(&c, -1, 0);
		}
		frame_counter++;
		// Drawing
		BeginDrawing();
			ClearBackground(border_color);		// Borders
			DrawRectangle(						// Background
				CH_OFFX,
				CH_OFFY,
				CH_SZ*(CH_WIDTH-CH_SUBOFF),
				CH_SZ*(CH_HEIGHT-CH_SUBOFF),
				background_color
			);
			// Draw
			k_render_text(font);
			k_draw_cursor(&c, 30, WHITE);
		EndDrawing();
	}
	// Close
	UnloadFont(font);
	CloseWindow();
	return 0;
}
/* Functions ................ */
char sys_read_byte(uint16_t addr)
{
	return (addr & 0x80 == 0) ? sys.RAM[addr] : sys.RAM[addr];
}
void sys_write_byte(uint16_t addr, char v)
{
	sys.RAM[addr] = v;
}
//
void k_set_char(char c, int x, int y)
{
	screen_data[y * SCR_MAX_X + x] = c;
}
void k_set_char_at_cursor(char c, struct cursor * cs)
{
	screen_data[cs->p.y * SCR_MAX_X + cs->p.x] = c;
	k_move_cursor(cs, 1, 0);
}
void k_move_cursor(struct cursor * cs, int dx, int dy)
{
	cs->p.x += dx;
	if (cs->p.x < 0) {
		// check x 0
		cs->p.x = SCR_MAX_X - 1;
		cs->p.y -= 1;
	}
	if (cs->p.x > SCR_MAX_X - 1) {
		// check x SCR_MAX_X
		cs->p.x = 0;
		cs->p.y += 1;
	}
	cs->p.y += dy;
	if (cs->p.y < 0) {
		// check y 0
		cs->p.y = SCR_MAX_Y - 1;
	}
	if (cs->p.y > SCR_MAX_Y - 1) {
		// check y SCR_MAX_Y
		cs->p.y = 0;
	}
	cs->ds = 1;
	cs->t = 0;
}
void k_set_cursor(struct cursor * cs, int nx, int ny)
{
	// check x
	if (nx < 0) {
		while (nx < 0) {
			nx += SCR_MAX_X;
		}
	}
	if (nx > SCR_MAX_X - 1) {
		while (nx > SCR_MAX_X - 1) {
			nx -= SCR_MAX_X;
		}
	}
	cs->p.x = nx;
	// check y
	if (ny < 0) {
		while (ny < 0) {
			ny += SCR_MAX_Y;
		}
	}
	if (ny > SCR_MAX_Y - 1) {
		while (ny > SCR_MAX_Y - 1) {
			ny -= SCR_MAX_Y;
		}
	}
	cs->p.y = ny;
	cs->ds = 1;
	cs->t = 0;
}
void k_set_text(char * c, int x, int y)
{
	char * pt = c;
	int _x = 0;
	while (*pt) {
		k_set_char(*pt, x + _x, y);
		pt++;
		_x++;
	}
}
void k_render_text(Font font)
{
	char c[2];
	c[1] = '\0';
	for (int _y = 0; _y < SCR_MAX_Y; ++_y) {
		for (int _x = 0; _x < SCR_MAX_X; ++_x) {
			c[0] = screen_data[_y * SCR_MAX_X + _x];
			DrawTextEx(
				font, c,
				(Vector2){
					CH_OFFX + _x * CH_SZ,
					CH_OFFY + _y * CH_SZ
				},
				CH_SZ, 0, WHITE
			);
		}
	}
}
void k_draw_cursor(struct cursor * cs, int t, Color c)
{
	cs->t++;
	if (cs->ds) {
		DrawRectangle(
			CH_OFFX + cs->p.x * CH_SZ,
			CH_OFFY + cs->p.y * CH_SZ,
			CH_SZ,
			CH_SZ,
			c
		);
	}
	if (cs->t == t) {
		cs->ds ^= 1;		// toggle between 0 and 1
		cs->t = 0;
	}
}
/* .......................... */