#pragma once

#include "arduino_shim.h"

#include <cstddef>

const int CHESS_MAXSTEPS = 150;
const int CHESS_MAXDEPTH = 30;

typedef struct {
    signed char f1, f2;
    signed char c1, c2;
    signed char check;
    signed char type;
    short weight;
} step_t;

struct position_t {
    uint8_t w;
    uint8_t wrk, wrq, brk, brq;
    uint8_t pp;
    step_t steps[CHESS_MAXSTEPS + 1];
    int n_steps;
    int cur_step;
    step_t best;
    int check_on_table;
    short weight_w;
    short weight_b;
    short weight_s;
};

extern short pole[64];
extern position_t pos[CHESS_MAXDEPTH];
extern step_t game_steps[1000];
extern position_t game_pos;
extern int game_ply;
extern boolean game_w;
extern short game_pole[64];
extern unsigned long timelimith;
extern boolean halt;
extern int search_max_level;
extern unsigned long count;
extern int level;
extern int lastbestdepth;

boolean fen(const char *ss);
int fenout(int l, char *buf, size_t buflen);
void generate_steps(int l);
void movestep(int l, step_t &s);
void movepos(int l, step_t &s);
void backstep(int l, step_t &s);
boolean solve_step();
boolean check_w();
boolean check_b();
void sort_steps(int l);
