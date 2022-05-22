#include "database.h"

static Database *db;

void loadAllXPMs() {
  db = (Database *) malloc(sizeof(Database));
  db->sprites = (Sprites*)malloc(sizeof(Sprites));
  xpm_load(ducks_xpm, XPM_8_8_8_8, &db->images.ducks);
  xpm_load(background_xpm, XPM_8_8_8_8, &db->images.background);
  xpm_load(crosshair_xpm, XPM_8_8_8_8, &db->images.crosshair);
}

Database *getDB() {
  return db;
}

void drawBackground() {
  vg_draw_image(db->images.background, 0, 0);
}

void drawCrosshair() {
  draw_sprite(db->sprites->crosshair, get_video_mem());
}