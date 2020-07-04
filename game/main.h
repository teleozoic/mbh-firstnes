/** (C) Matt Hughson 2020 */

#define DEBUG_ENABLED 1

#if DEBUG_ENABLED
#define PROFILE_POKE(val) //POKE((0x2001),(val));
#else
#define PROFILE_POKE(val)
#endif

// Nametable A: 	2400-2000 = 400
// Attributes: 		2400-23c0 = 0x40
// Patterns: 		0x400-0x40 = 0x3c0
#define NAMETABLE_SIZE 0x400
#define NAMETABLE_PATTERN_SIZE 0x3c0

#define BOARD_START_X_PX 96
#define BOARD_START_Y_PX 16
#define BOARD_END_X_PX 168
#define BOARD_END_Y_PX (184 + 16)

#define BOARD_OOB_END 3
#define BOARD_END_X_PX_BOARD 9 // left edge of last block (width = 10)
#define BOARD_END_Y_PX_BOARD 23 // top edge of last block (height = 24)

#define BOARD_SIZE 240 // 
#define BOARD_HEIGHT (BOARD_END_Y_PX_BOARD - BOARD_OOB_END)
#define BOARD_WIDTH (BOARD_END_X_PX_BOARD + 1)

#define TILE_TO_BOARD_INDEX(x,y) (((y) * 10) + (x))

#define BLINK_LEN (60 * 5)

#pragma bss-name(push, "ZEROPAGE")

// GLOBAL VARIABLES

struct block
{
    unsigned char x;
    unsigned char y;
};

struct cluster
{
    /*
        1 1 0 0 ─┬─ [ byte 0 ]
        0 1 1 0 ─┘
        0 0 0 0
        0 0 0 0
    */
    unsigned int layout;
    const unsigned int def[4];
    unsigned char sprite;
    unsigned char id;
};

unsigned char tick_count;
unsigned int tick_count_large;
unsigned char hit_reaction_remaining;
unsigned int attack_queue_ticks_remaining;
const unsigned int attack_delay = 600;
unsigned char pad1;
unsigned char pad1_new;
unsigned int scroll_y;

#define NUM_OPTIONS 3
unsigned char cur_option;

//const unsigned char text[] = "- PRESS START -";
enum { ATTACK_ON_LAND, ATTACK_ON_TIME, ATTACK_NEVER, ATTACK_NUM };
unsigned char attack_style;
#define ATTACK_STRING_LEN 7

unsigned char music_on;
#define OFF_ON_STRING_LEN 4

enum {BLOCK_STYLE_MODERN, BLOCK_STYLE_CLASSIC};
unsigned char block_style;
#define BLOCK_STYLE_STRING_LEN 7

enum { STATE_MENU, STATE_OPTIONS, STATE_GAME, STATE_PAUSE, STATE_OVER, STATE_SOUND_TEST };
unsigned char state = STATE_MENU;

// The block operates in "logical space" from 0 -> w/h. The logical
// space is converted to screen space at time of render (or ppu get).
struct block cur_block = { 0, 0 };


// How many frames need to pass before it falls 8 pixels.
unsigned char fall_rate = 48;
unsigned char cur_level = 0;

// Each entry in the array is a rotation.
// Stored as 4x4 16 bit matrix to support line (otherwise 3x3 would do it).
// TODO: Perhaps a special character could be user to terminate the array
//       prior to the end.
/*
    1 1 0 0
    0 1 1 0
    0 0 0 0
    0 0 0 0

    0 0 1 0
    0 1 1 0
    0 1 0 0
    0 0 0 0
*/


// CLASSIC

const unsigned int def_z_clust[4] = 
{ 
    0xc60,
    0x264,
    0xc60, // dupe.
    0x264, // dupe.
};


const unsigned int def_z_rev_clust[4] = 
{ 
    0x6C0,
    0x8C40,
    0x6C0, // dupe.
    0x8C40, // dupe.
};

const unsigned int def_line_clust[4] =
{
    0xf0,
    0x4444,
    0xf0, // dupe.
    0x4444 // dupe.
};

const unsigned int def_box_clust[4] =
{
    0x660,    
    0x660,    
    0x660,
    0x660,
};

const unsigned int def_tee_clust[4] =
{
    0x4e00,    
    0x4640,    
    0xe40,
    0x4c40,
};

const unsigned int def_L_clust[4] =
{
    0xe80,    
    0xc440,    
    0x2e00,
    0x4460,
};

const unsigned int def_L_rev_clust[4] =
{
    0xe20,    
    0x44c0,    
    0x8e00,
    0x6440,
};

// MODERN

const unsigned int def_elbow_clust[4] =
{
    0x4600,    
    0x0640,    
    0xc40,
    0x4c00,
};

const unsigned int def_short_line_clust[4] =
{
    0xe0,
    0x4440,
    0xe0, // dupe.
    0x4440 // dupe.
};


const unsigned int def_triangle_clust[4] =
{
    0x4a00,    
    0x4240,    
    0xa40,
    0x4840,
};

const unsigned int def_Y_clust[4] =
{
    0x4c44,    
    0x2f00,    
    0x2232,
    0xf4,
};

const unsigned int def_Y_rev_clust[4] =
{
    0x4644,    
    0xf20,    
    0x2262,
    0x4f0,
};

const unsigned int def_cross_clust[4] =
{
    0x4e40,
    0x4e40,
    0x4e40,
    0x4e40,
};

const unsigned int def_steps_clust[4] =
{
    0x4630,
    0x364,
    0xc62,
    0x26c0,
};


#define NUM_CLUSTERS 7
const unsigned int* cluster_defs_modern [NUM_CLUSTERS] =
{
    def_cross_clust, // def_z_clust,
    def_steps_clust, // def_z_rev_clust,
    def_short_line_clust, // def_line_clust,
    def_triangle_clust, // def_box_clust,
    def_elbow_clust, // def_tee_clust,
    def_Y_clust, // def_L_clust,
    def_Y_rev_clust, // def_L_rev_clust,
};

const unsigned int* cluster_defs_classic [NUM_CLUSTERS] =
{
    def_z_clust,
    def_z_rev_clust,
    def_line_clust,
    def_box_clust,
    def_tee_clust,
    def_L_clust,
    def_L_rev_clust,
};

unsigned char cur_rot;

struct cluster cur_cluster;// = { def_z_clust }; // 165 1010 0101
struct cluster next_cluster;
#define ATTACK_QUEUE_SIZE 3
#define ATTACK_MAX 10

unsigned char attack_row_status[BOARD_WIDTH];

unsigned char cluster_sprites[NUM_CLUSTERS] =
{
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6
};

unsigned char cluster_offsets[NUM_CLUSTERS] = 
{
    3, 3, 2, 3, 4, 3, 3,
    //10,10,10,10,10,10,10
};

unsigned char horz_button_delay;
const unsigned char button_delay = 5;
unsigned char require_new_down_button;
unsigned char fall_frame_counter;
unsigned char lines_cleared_one;
unsigned char lines_cleared_ten;
unsigned char lines_cleared_hundred;
unsigned char cur_nt;
unsigned char off_nt;

// movement()
char hit;
unsigned char temp_fall_rate;
unsigned char old_x;
// spawn_new_cluster()
unsigned char id;
// put_cur_cluster()
unsigned char min_y;
unsigned char max_y;
// set_block()
unsigned char in_x; 
unsigned char in_y; 
unsigned char in_id;
// draw_gameplay_sprites()
unsigned char local_start_x;
unsigned char local_start_y;
unsigned char local_ix;
unsigned char local_iy;
unsigned int local_t;
unsigned char local_bit;
unsigned char local_row_status;
const unsigned char OOB_TOP = (BOARD_START_Y_PX + (BOARD_OOB_END << 3));

unsigned char test_song;
unsigned char test_song_active;
unsigned char test_sound;

enum { MUSIC_TITLE, MUSIC_GAMEPLAY, MUSIC_STRESS, MUSIC_PAUSE };
enum 
{ 
    SOUND_ROTATE, SOUND_LAND, SOUND_ROW, SOUND_MULTIROW, SOUND_GAMEOVER, 
    SOUND_START, SOUND_BLOCKED, SOUND_LEVELUP, SOUND_LEVELUP_MULTI, 
    SOUND_PAUSE, SOUND_MENU_HIGH, SOUND_MENU_LOW, SOUND_GAMEOVER_SONG};

unsigned char attack_queued;

char tenatcle_offsets[4] = { -1, 0, 1, 0 };

#define NUM_GARBAGE_TYPES 3
unsigned char garbage_types[NUM_GARBAGE_TYPES] = { 0x60, 0x70, 0x2f };
unsigned char cur_garbage_type;

#define DELAY_LOCK_LEN 20
signed char delay_lock_remaining;
unsigned char delay_lock_skip;







#pragma bss-name(push, "BSS")

unsigned char game_board[BOARD_SIZE];
unsigned char game_board_temp[BOARD_SIZE];
char empty_row[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
char full_row[10] =  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
unsigned char full_col[20] =  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
const unsigned char option_empty[] = {0x0, 0x0};
const unsigned char option_icon[] = {0x25, 0x26};

// copy_board_to_nt()
char copy_board_data[BOARD_HEIGHT];
char lines_cleared_y[4];

unsigned char temp_pal[16];

const unsigned char palette_bg[16]={ 0x0f,0x22,0x31,0x30,0x0f,0x00,0x17,0x28,0x0f,0x2a,0x16,0x37,0x0f,0x22,0x26,0x37 };
const unsigned char palette_sp[16]={ 0x0f,0x22,0x31,0x30,0x0f,0x0f,0x26,0x37,0x0f,0x16,0x31,0x37,0x0f,0x22,0x26,0x37 };
const unsigned char palette_bg_options[16]={ 0x0f,0x22,0x31,0x30,0x0f,0x30,0x0f,0x26,0x0f,0x22,0x0f,0x26,0x0f,0x22,0x26,0x37 };

// const unsigned char pal_changes[20] = 
// {
//     0x01, 0x21, // blues
//     0x13, 0x23, // purples
//     0x1c, 0x26, // dark blue + oj
//     0x0b, 0x1b, // dark green, soft green
//     0x06, 0x15, // reds
//     0x2c, 0x39, // limey blue
//     0x03, 0x35, // purples
//     0x16, 0x26, // oranges
//     0x11, 0x2b, // light green, blue
//     0x0f, 0x15, // bright reds
// };

#define NUM_TIMES_OF_DAY 8
const unsigned char palette_bg_list[NUM_TIMES_OF_DAY][16]=
{
    { 0x0f,0x22,0x31,0x30,0x0f,0x00,0x17,0x28,0x0f,0x2a,0x16,0x37,0x0f,0x22,0x26,0x37 },
    { 0x0f,0x22,0x37,0x30,0x0f,0x00,0x17,0x27,0x0f,0x2a,0x16,0x38,0x0f,0x22,0x26,0x37 },
    { 0x0f,0x1c,0x37,0x30,0x0f,0x00,0x17,0x27,0x0f,0x1a,0x16,0x38,0x0f,0x1c,0x26,0x37 },
    { 0x0f,0x0c,0x23,0x34,0x0f,0x00,0x07,0x10,0x0f,0x1a,0x17,0x27,0x0f,0x0c,0x16,0x27 },
    { 0x0f,0x0c,0x1c,0x10,0x0f,0x00,0x07,0x22,0x0f,0x1b,0x07,0x17,0x0f,0x0c,0x11,0x22 },
    { 0x0f,0x0c,0x22,0x27,0x0f,0x00,0x18,0x27,0x0f,0x1a,0x17,0x27,0x0f,0x0c,0x22,0x37 },
    { 0x0f,0x1c,0x36,0x30,0x0f,0x00,0x18,0x26,0x0f,0x2a,0x16,0x38,0x0f,0x1c,0x26,0x37 },
    { 0x0f,0x23,0x32,0x30,0x0f,0x00,0x17,0x28,0x0f,0x2a,0x16,0x37,0x0f,0x23,0x26,0x37 },
};

unsigned char time_of_day;

/*
Level	Frames per Gridcell
00	        48
01	        43
02	        38
03	        33
04	        28
05	        23
06	        18
07	        13
08	        8
09	        6
10–12	    5
13–15	    4
16–18	    3
19–28	    2
29+	        1
*/

// https://tetris.fandom.com/wiki/Tetris_(NES,_Nintendo)
unsigned char fall_rates_per_level[] =
{
    48,
    43,
    38,
    33,
    28,
    23,
    18,
    13,
    8,
    6,
    5, // 10-12
    5, // 10-12
    5, // 10-12
    4, // 13-15
    4, // 13-15
    4, // 13-15
    3, // 16–18
    3, // 16–18
    3, // 16–18
    2, // 19–28
    2, // 19–28
    2, // 19–28
    2, // 19–28
    2, // 19–28
    2, // 19–28
    2, // 19–28
    2, // 19–28
    2, // 19–28
    2, // 19–28
    1, // 29+
};

unsigned char attack_style_strings[3][ATTACK_STRING_LEN] = 
{
    "FIXED",
    "TIMED",
    "CLASSIC"
};
unsigned char off_on_string[2][OFF_ON_STRING_LEN] = 
{
    "OFF",
    "ON"
};
unsigned char block_style_strings[2][BLOCK_STYLE_STRING_LEN] =
{
    "MODERN",
    "CLASSIC"
};

unsigned char text_push_start[] = { "PUSH START" };
unsigned char clear_push_start[] = { "          " };

// PROTOTYPES
void draw_menu_sprites(void);
void draw_gameplay_sprites(void);
void movement(void);

// Set a block in x, y (board space)
void set_block(/*unsigned char x, unsigned char y, unsigned char id*/);
void set_block_nt(unsigned char x, unsigned char y, unsigned char id, unsigned char nt);
// x, y in board space.
void clear_block(unsigned char x, unsigned char y);

// Drops the current cluster at its current location.
void put_cur_cluster();

// x, y in map space.
unsigned char get_block(unsigned char x, unsigned char y);

// x, y in map space.
unsigned char is_block_free(unsigned char x, unsigned char y);

// Checks if the entire cluster is currently hitting any other blocks.
unsigned char is_cluster_colliding();

// creates a new cluster at the top of the play area.
void spawn_new_cluster();

// Rotate the current cluster by 90degs.
void rotate_cur_cluster(char dir);

// Transition to a new state.
void go_to_state(unsigned char new_state);

void inc_lines_cleared();
void display_lines_cleared();
void display_level();

// CLEAR PHASES

// First clear all the rows in CPU memory.
void clear_rows_in_data(unsigned char start_y);

// Then show the empty rows, 2 columns at a time...
void reveal_empty_rows_to_nt();

// Then collapse the empty rows in CPU memory...
void try_collapse_empty_row_data(void);

// Finally reveal to resolved board with all blocks in final position.
void copy_board_to_nt();

void add_block_at_bottom();

void reset_gameplay_area();

void display_song();
void display_sound();
void display_options();

void fade_to_black();
void fade_from_black();


// DEBUG
#if DEBUG_ENABLED
void debug_fill_nametables(void);
void debug_draw_board_area(void);
void debug_copy_board_data_to_nt(void);
void debug_display_number(unsigned char num, unsigned char index);
#endif