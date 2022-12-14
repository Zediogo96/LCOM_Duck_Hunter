// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include "i8042.h"
#include "i8254.h"

#include "crosshair.h"
#include "database.h"
#include "duck.h"
#include "hitboxes.h"
#include "keyboard.h"
#include "mouse.h"
#include "timer.h"
#include "utils.h"
#include "vbe.h"
#include "video_gr.h"

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/proj/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/proj/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

extern uint8_t out_byte;
uint64_t no_interrupts = 0; // prob change inside the loop

int(proj_main_loop)(int argc, char *argv[]) {

  uint8_t bit_no_kbd = KBD_IRQ;
  uint8_t bit_no_timer = TIMER0_IRQ;
  uint8_t bit_no_mouse = MOUSE_IRQ;

  uint32_t kbd_irq = BIT(bit_no_kbd);
  uint32_t timer_irq = BIT(bit_no_timer);
  uint32_t mouse_irq = BIT(bit_no_mouse);

  int r, ipc_status, size = 0;
  message msg;

  uint8_t bytes[2];

  // For Mouse Handling
  uint8_t mouse_byte_count = 0;
  struct packet pp;
  uint8_t bytes_mouse[3];
  mouse_enable_data_reporting(); // Still using LCF version

  vg_init(MODE_1152x864_DIRECT);

  loadAllXPMs();

  Database *db = getDB();

  createCrosshair(db);

  create_Duck(db);

  if (timer_subscribe_int(&bit_no_timer)) {
    printf("%s failed!", __func__);
    return 1;
  }

  if (kbd_subscribe_int(&bit_no_kbd)) {
    printf("%s failed!", __func__);
    return 1;
  }

  if (mouse_subscribe_int(&bit_no_mouse)) {
    printf("%s failed!", __func__);
    return 1;
  }

  while (bytes[0] != KBD_BREAKCODE_ESC) { /* You may want to use a different condition */

    /* Get a request message. */
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { /* received notification */
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE: /* hardware interrupt notification */
          if (msg.m_notify.interrupts & timer_irq) {

            timer_int_handler();
            drawBackground();
            drawCrosshair();
            if (!(db->sprites->duck->state == Dead)) drawDuck();

            checkDuckGotShot(db->sprites->crosshair, db->sprites->duck);
            update_Duck(db->sprites->duck);

            // vg_draw_image(db->images.duck_Left, 400, 500);
            // vg_draw_image(db->images.duck_Right, 550, 500);

            // vg_draw_image(db->images.duck_Up_Right, 500, 200);
            // vg_draw_image(db->images.duck_Up_Left, 400, 200);
            // vg_draw_image(db->images.duck_Up, 150, 300);
            // vg_draw_image(db->images.duck_Shot, 800, 300);
            // vg_draw_image(db->images.duck_Falling, 800, 450);
          }
          if (msg.m_notify.interrupts & mouse_irq) {
            mouse_ih();
            if (out_byte & BIT(3) || mouse_byte_count != 0) {
              bytes_mouse[mouse_byte_count] = out_byte;
              mouse_byte_count++;
            }
            if (mouse_byte_count == 3) {
              mouse_byte_count = 0;

              pp = process_packets(bytes_mouse);
              update_mouse(pp, db);
            }
          }
          if (msg.m_notify.interrupts & kbd_irq) { /* subscribed interrupt */
            kbc_ih();
            bytes[size] = out_byte;

            if (out_byte == 0)
              return 1;

            if (out_byte != 0xE0) {
              size = 0;
            }
            else
              size++;
          }
          break;
        default:
          break; /* no other notifications expected: do nothing */
      }
    }
    else { /* received a standard message, not a notification */
      /* no standard messages expected: do nothing */
    }
  }

  /**
   * STUFF TO DISABLE MOUSE AND MOUSE DATA REPORTING
   **/

  if (timer_unsubscribe_int()) {
    printf("%s failed!", __func__);
    return 1;
  }

  if (kbd_unsubscribe_int()) {
    printf("%s failed!", __func__);
    return 1;
  }

  if (mouse_disable_reporting() != 0) {
    printf("%s: mouse_disable_reporting() failed\n", __func__);
  }

  if (mouse_unsubscribe_int()) {
    printf("%s failed!", __func__);
    return 1;
  }

  vg_exit();

  return 0;
}
