//
// Created by development on 8/11/15.
//

#if !defined(MYWATCH_SPRITE_H)
#define MYWATCH_SPRITE_H 1


#include <inttypes.h>
#include <ctype.h>


struct Item {
    int x_l;
    int y_h;
};

struct Sprite {
    int x;
    int y;
    Item *item;
};

struct Pos {
    int x;
    int y;
};


#define MAX_SPRITES 40




class SpritePainter {

    Sprite sprites[MAX_SPRITES]= {};

    uint8_t *line_pattern;

    bool IsLineInPattern(uint8_t line);

public:
    SpritePainter();
    bool UpdateEPDLineData(uint8_t *line_data, uint8_t line);
    void BuildNumber(int digit, int display_number);
    void AddSprite(Sprite *sprite);

    void StartList();
    void EndList();

    Sprite *GetSprite();

    void AddScreenLinePattern(uint8_t *data_line_pattern);
    uint8_t *GetScreenLinePattern();


    Sprite *current_sprite;
};

// Define Data for Sprites - could be in a separate include







#endif //MYWATCH_SPRITE_H
