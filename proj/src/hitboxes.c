#include "hitboxes.h"
#include "database.h"

bool checkHitboxCollision(Sprite *sprite1, Sprite *sprite2) {
  if ((sprite1->x + sprite1->width) < sprite2->x || (sprite2->x + sprite2->width) < sprite1->x || (sprite1->y + sprite1->height) < sprite2->y || (sprite2->y + sprite2->height) < sprite1->y) return false;
  return true;
}

bool checkDuckGotShot(Sprite *sprite1, Sprite *sprite2) {
  if (checkHitboxCollision(sprite1, sprite2) && isMouseLBpressed) {
    sprite2->map = getDB()->images.duck_Shot.bytes;
    sprite2->state = Dead;
    sprite2->direction = Down;
    return true;
  }
  return false;
}
