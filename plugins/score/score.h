#ifndef _SCORE_H_
#define _SCORE_H_

void add_score(int s);
int get_score();
int get_level();
void register_on_level_up(void (*callback)(int new_level));

#endif
