#include <stdio.h>
#include <math.h>
#include <allegro.h>
#include <allegro_image.h>
#include <allegro_primitives.h>

#define LEVEL_NUMBERS 3
#define CURRENT_LEVEL 2

const float FPS = 60;

const int SCREEN_W = 1024;
const int SCREEN_H = 768;
const char RANKS[13] = {'A','2','3','4','5','6','7','8','9','D','J','Q','K'};
const char SUITS[4] = {'H','D','C','S'};
const unsigned char DESK_HEIGHTS[LEVEL_NUMBERS] = {3,5,6};
const unsigned char DESK_WIDTHS[LEVEL_NUMBERS] = {6,10,14};
const unsigned char DESK_SIZES[LEVEL_NUMBERS] = {18,50,84};
const unsigned char DESK_FPS[LEVEL_NUMBERS] = {4,2,2};
unsigned int source_card_width;
unsigned int source_card_height;

typedef
 struct {
 unsigned int x_pos;
 unsigned int y_pos;
 unsigned int r_height;
 unsigned int r_width;
 char rank; //2-9 , D -10, J,Q,K,A
 char suit; // H-hearts, D-diamonds, C-clubs, S - spades
 char rank_index;
 char suit_index;
 char flipped;
 char removed;
 ALLEGRO_BITMAP *face_bitmap;
 ALLEGRO_BITMAP *back_bitmap;
 } CARD;

 typedef
 struct {
 ALLEGRO_BITMAP *face_bitmap;
 ALLEGRO_BITMAP *back_bitmap;
  } CARD_BITMAPS;

 typedef
 struct {
 unsigned int bar_height;
 unsigned int bar_width;
 unsigned int bar_x1;
 unsigned int bar_x2;
 unsigned int bar_y1;
 unsigned int bar_y2;
 //ALLEGRO_BITMAP *bar_bitmap;
 } DRAWING_BAR;

 typedef
 struct {
 // input parameters for calculation
 float card_axises_ratio; // W/H of card
 float gap_ratio_horizontal; //which % of width is horizontal gap between cards
 float gap_ratio_vertical; // which % of height is vertical gap between cards
 float header_bar_ratio; // which % of screen height is header bar on top
 float footer_bar_ratio; // which % of screen height is footer bar in the bottom
 // numbers to calculate needed for drawing
 // header
 DRAWING_BAR header;
 // footer
 DRAWING_BAR footer;
 // card desk
 DRAWING_BAR card_desk;
 // cards on the desk
 unsigned int gap_size_horizontal;
 unsigned int gap_size_vertical;
 unsigned int card_size_horizontal;
 unsigned int card_size_vertical;
  } GRAPH_DESK_LAYOUT;

GRAPH_DESK_LAYOUT cards_desk_layout;





// Random number
unsigned char my_random(unsigned char ip_bound)
{
    unsigned char n;
    FILE *fl=fopen("/dev/urandom","r");
    do {
    fread(&n, sizeof(n),1,fl);
    } while (n>=ip_bound);
    fclose(fl);
    return n;
}

// Inits deck array
CARD *deck_init (unsigned char ip_desk_size)
{
 return calloc(ip_desk_size,sizeof(CARD));

}

// Inits deck array
//CARD_BITMAPS *deck_bitmaps_init (unsigned char ip_desk_size)
//{
 //return calloc(ip_desk_size,sizeof(CARD_BITMAPS));
//
//}


//Randomly puts card pairs to deck array
int deck_randomize (unsigned char ip_desk_size, CARD *ip_deck)
{
// if (ip_deck_size%2!=0 ) {return -1;}
 unsigned char v_counter, v_rank, v_suit, v_random_base, v_rank_index, v_suit_index;

 unsigned int v_tmp_int, v_tmp_int1;
 v_tmp_int=ip_desk_size;
 v_tmp_int1 = sizeof(CARD);
 for (v_counter=0;v_counter<ip_desk_size/2;v_counter++)
 {
   v_rank_index =my_random(13);
   v_rank=RANKS[v_rank_index];
   v_suit_index= my_random(4);
   v_suit=SUITS[v_suit_index];
   v_random_base=0; // element deck[0] goes first
   while ((*(ip_deck+v_random_base)).rank != 0)
   {
    v_tmp_int1=my_random(v_tmp_int-1);
    v_random_base=v_tmp_int1+1;
   }
   (*(ip_deck+v_random_base)).rank=v_rank;
   (*(ip_deck+v_random_base)).suit=v_suit;
   (*(ip_deck+v_random_base)).rank_index=v_rank_index;
   (*(ip_deck+v_random_base)).suit_index=v_suit_index;
   (*(ip_deck+v_random_base)).flipped = 0;
   (*(ip_deck+v_random_base)).removed = 0;
   while ((*(ip_deck+(int)v_random_base)).rank != 0)
   {
    v_tmp_int1=my_random(v_tmp_int-1);
    v_random_base=v_tmp_int1+1;
   }
   (*(ip_deck+v_random_base)).rank=v_rank;
   (*(ip_deck+v_random_base)).suit=v_suit;
   (*(ip_deck+v_random_base)).rank_index=v_rank_index;
   (*(ip_deck+v_random_base)).suit_index=v_suit_index;
   (*(ip_deck+v_random_base)).flipped = 0;
   (*(ip_deck+v_random_base)).removed = 0;
 }

 return 0;
}

CARD *deck_find_clicked_card (CARD *ip_deck, unsigned int ip_click_x, unsigned int ip_click_y, unsigned char ip_desk_size)
{
 unsigned char i;
 for (i=0; i<ip_desk_size; i++)
     if (   ((*(ip_deck+i)).x_pos <= ip_click_x) &&
            ((*(ip_deck+i)).x_pos+(*(ip_deck+i)).r_width >= ip_click_x) &&
            ((*(ip_deck+i)).y_pos <= ip_click_y) &&
            ((*(ip_deck+i)).y_pos+(*(ip_deck+i)).r_height >= ip_click_y) &&
            ((*(ip_deck+i)).removed ==0)
        )
        {
            return ip_deck+i;
        }
  return NULL;
}


int calculate_graph_desk_layout(GRAPH_DESK_LAYOUT *ip_layout, unsigned int ip_screen_h, unsigned int ip_screen_w, unsigned char ip_layout_id)
{

unsigned int v_target_card_h, v_target_card_w;

// calculating basic card sizes

// necessary parameters check
if (((*ip_layout).card_size_horizontal==0)&&((*ip_layout).card_size_vertical==0)) {fprintf(stderr, "ERROR: zero basic card size!\n"); return -1;};
if (((*ip_layout).card_size_horizontal!=0)&&((*ip_layout).card_size_vertical==0)&&((*ip_layout).card_axises_ratio==0.0)) {fprintf(stderr, "ERROR: horizontal card size is defined but ratio is 0!\n"); return -1;};
if (((*ip_layout).card_size_horizontal==0)&&((*ip_layout).card_size_vertical!=0)&&((*ip_layout).card_axises_ratio==0.0)) {fprintf(stderr, "ERROR: vertical card size is defined but ratio is 0!\n"); return -1;};


// both of sizes are defined
if (((*ip_layout).card_size_horizontal!=0)&&((*ip_layout).card_size_vertical!=0))
{
(*ip_layout).card_axises_ratio=1.0*(*ip_layout).card_size_horizontal/(*ip_layout).card_size_vertical;
}

// vertical size and ratio are defined
if (((*ip_layout).card_size_horizontal=0)&&((*ip_layout).card_axises_ratio!=0.0))
{
(*ip_layout).card_size_horizontal=round((*ip_layout).card_axises_ratio*(*ip_layout).card_size_vertical);
}

// horizontal size and ratio are defined
if (((*ip_layout).card_size_vertical=0)&&((*ip_layout).card_axises_ratio!=0.0))
{
(*ip_layout).card_size_vertical=round((*ip_layout).card_axises_ratio*(*ip_layout).card_size_horizontal);
}

//calculating header
(*ip_layout).header.bar_height = round(ip_screen_h*(*ip_layout).header_bar_ratio );
(*ip_layout).header.bar_width = ip_screen_w;
(*ip_layout).header.bar_x1=0;
(*ip_layout).header.bar_y1=0;
(*ip_layout).header.bar_x2=ip_screen_w;
(*ip_layout).header.bar_y2=(*ip_layout).header.bar_height;

// calculating footer
(*ip_layout).footer.bar_height =  round(ip_screen_h* (*ip_layout).footer_bar_ratio);
(*ip_layout).footer.bar_width = ip_screen_w;
(*ip_layout).footer.bar_x1 = 0;
(*ip_layout).footer.bar_x2 = ip_screen_w;
(*ip_layout).footer.bar_y1 = ip_screen_h -(*ip_layout).footer.bar_height;
(*ip_layout).footer.bar_y2 = ip_screen_h;

// calculating main cards desk to begin ajusting

v_target_card_w = ip_screen_w/(DESK_WIDTHS[ip_layout_id]+(*ip_layout).gap_ratio_horizontal*(DESK_WIDTHS[ip_layout_id]-1));
v_target_card_h = (ip_screen_h -((*ip_layout).header.bar_height+(*ip_layout).footer.bar_height))/(DESK_HEIGHTS[ip_layout_id]+(*ip_layout).gap_ratio_vertical*(DESK_HEIGHTS[ip_layout_id]-1));


if (v_target_card_h <v_target_card_w/(*ip_layout).card_axises_ratio)
{
    v_target_card_w=v_target_card_h*(*ip_layout).card_axises_ratio;
}
else if (v_target_card_w< v_target_card_h*(*ip_layout).card_axises_ratio)
{
    v_target_card_h = v_target_card_w/(*ip_layout).card_axises_ratio;

}
// set card size
(*ip_layout).card_size_horizontal = v_target_card_w;
(*ip_layout).card_size_vertical = v_target_card_h;

// set gaps sizes
(*ip_layout).gap_size_horizontal = v_target_card_w*(*ip_layout).gap_ratio_horizontal;
(*ip_layout).gap_size_vertical = v_target_card_h*(*ip_layout).gap_ratio_vertical;

// set height and width for desk area and center it on the screen
(*ip_layout).card_desk.bar_height = DESK_HEIGHTS[ip_layout_id]*v_target_card_h+(DESK_HEIGHTS[ip_layout_id]-1)*v_target_card_h*(*ip_layout).gap_ratio_vertical;
(*ip_layout).card_desk.bar_y1 = (ip_screen_h -((*ip_layout).header.bar_height+(*ip_layout).footer.bar_height))/2 - (*ip_layout).card_desk.bar_height/2 + (*ip_layout).header.bar_height;
(*ip_layout).card_desk.bar_y2 = (*ip_layout).card_desk.bar_y1 + (*ip_layout).card_desk.bar_height;

(*ip_layout).card_desk.bar_width = DESK_WIDTHS[ip_layout_id]*v_target_card_w+(DESK_WIDTHS[ip_layout_id]-1)*v_target_card_w*(*ip_layout).gap_ratio_horizontal;
(*ip_layout).card_desk.bar_x1 = ip_screen_w/2 - (*ip_layout).card_desk.bar_width/2;
(*ip_layout).card_desk.bar_x2 = (*ip_layout).card_desk.bar_x1 + (*ip_layout).card_desk.bar_width;


return 0;
}

//Destroys deck array
void deck_destroy (CARD *ip_deck)
{
free(ip_deck);
}

void flip_card (ALLEGRO_DISPLAY *ip_display, ALLEGRO_BITMAP *ip_bitmap_from, ALLEGRO_BITMAP *ip_bitmap_to, DRAWING_BAR ip_area, ALLEGRO_COLOR ip_background_color, unsigned char ip_fps)
{
 ALLEGRO_BITMAP *v_tmp_drawing_area;
 int i=0;

 v_tmp_drawing_area = al_create_bitmap(ip_area.bar_width, ip_area.bar_height);
 al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP);


 al_set_target_bitmap(v_tmp_drawing_area);
 al_clear_to_color(ip_background_color);
 al_set_target_backbuffer(ip_display);

 for (i=0;i< ip_area.bar_width/2+1;i+=ip_fps)
    {
    al_draw_bitmap(v_tmp_drawing_area,ip_area.bar_x1,ip_area.bar_y1, 0);
    al_draw_scaled_bitmap(ip_bitmap_from,0, 0, al_get_bitmap_width(ip_bitmap_from) , al_get_bitmap_height(ip_bitmap_from),
        ip_area.bar_x1+i,
        ip_area.bar_y1,
        ip_area.bar_width-2*i,
        ip_area.bar_height,
        0);
    al_update_display_region (ip_area.bar_x1,ip_area.bar_y1,ip_area.bar_width,ip_area.bar_height);
    }
   for (i=ip_area.bar_width/2;i>0 ;i-=ip_fps)
    {
    al_draw_bitmap(v_tmp_drawing_area,ip_area.bar_x1,ip_area.bar_y1, 0);
    al_draw_scaled_bitmap(ip_bitmap_to,0, 0, al_get_bitmap_width(ip_bitmap_to) , al_get_bitmap_height(ip_bitmap_to),
        ip_area.bar_x1+i,
        ip_area.bar_y1,
        ip_area.bar_width-2*i,
        ip_area.bar_height,
        0);
    al_update_display_region (ip_area.bar_x1,ip_area.bar_y1,ip_area.bar_width,ip_area.bar_height);
    }

// al_flip_display();

// al_update_display_region (ip_area.bar_x1,ip_area.bar_y1,ip_area.bar_width,ip_area.bar_height);
}


int main(int argc, char **argv)
{
   ALLEGRO_DISPLAY *display = NULL;
   ALLEGRO_EVENT_QUEUE *event_queue = NULL;
   ALLEGRO_TIMER *timer = NULL;
   ALLEGRO_BITMAP *full_deck = NULL;
   ALLEGRO_BITMAP *basic_card = NULL;
   ALLEGRO_BITMAP *back_card = NULL;
   bool redraw = true;
unsigned int i,j;
unsigned int v_bitmap_w, v_bitmap_h;
unsigned char game_state = 0;
CARD *flipped_1;




   if(!al_init()) {
      fprintf(stderr, "failed to initialize allegro!\n");
      return -1;
   }

   if(!al_install_mouse()) {
      fprintf(stderr, "failed to initialize the mouse!\n");
      return -1;
   }

   timer = al_create_timer(1.0 / FPS);
   if(!timer) {
      fprintf(stderr, "failed to create timer!\n");
      return -1;
   }

//   flip_timer = al_create_timer(1.0 / FPS);
//   if(!timer) {
//      fprintf(stderr, "failed to create timer!\n");
//      return -1;
//   }

   display = al_create_display(SCREEN_W, SCREEN_H);
   if(!display) {
      fprintf(stderr, "failed to create display!\n");
      al_destroy_timer(timer);
      return -1;
   }


   if(!al_init_image_addon()) {
      fprintf(stderr, "failed to init image addon!\n");
      al_destroy_display(display);
      al_destroy_timer(timer);
      return -1;
   }

   if(!al_init_primitives_addon()) {
      fprintf(stderr, "failed to init primitive addon!\n");
      al_destroy_display(display);
      al_destroy_timer(timer);
      return -1;
   }

   full_deck = al_load_bitmap("full_deck.bmp");
   if(!full_deck) {
      fprintf(stderr, "failed to create full_deck bitmap!\n");
      al_destroy_display(display);
      al_destroy_timer(timer);
      return -1;
   }

   basic_card = al_load_bitmap("hearts_ace.bmp");
   if(!basic_card) {
      fprintf(stderr, "failed to create basic card bitmap!\n");
      al_destroy_display(display);
      al_destroy_timer(timer);
      return -1;
   }

   back_card = al_load_bitmap("card_back.bmp");
   if(!back_card) {
      fprintf(stderr, "failed to create card back bitmap!\n");
      al_destroy_display(display);
      al_destroy_timer(timer);
      return -1;
   }

 al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP&ALLEGRO_MIN_LINEAR&ALLEGRO_MAG_LINEAR);
 al_set_window_title (display,"-= MemCards =-");
// recalculate drawing layout
cards_desk_layout.card_size_horizontal = al_get_bitmap_width(basic_card);
cards_desk_layout.card_size_vertical = al_get_bitmap_height(basic_card);
source_card_width = al_get_bitmap_width(basic_card);
source_card_height = al_get_bitmap_height(basic_card);

cards_desk_layout.footer_bar_ratio = 0.05;
cards_desk_layout.header_bar_ratio = 0.05;
cards_desk_layout.gap_ratio_horizontal = 0.1;
cards_desk_layout.gap_ratio_vertical = 0.1;

// prepare and randomize cards desk

   CARD *deck = NULL;
   CARD *clicked_card = NULL;
//   CARD_BITMAPS *deck_bitmaps = NULL;

   deck=deck_init(DESK_SIZES[CURRENT_LEVEL]);
//   deck_bitmaps= deck_bitmaps_init(DESK_SIZES[CURRENT_LEVEL]);
   deck_randomize(DESK_SIZES[CURRENT_LEVEL],deck);
//   deck_generate_bitmaps(deck_bitmaps, DESK_HEIGHTS[CURRENT_LEVEL], DESK_WIDTHS[CURRENT_LEVEL]);

calculate_graph_desk_layout(&cards_desk_layout,SCREEN_H,SCREEN_W,CURRENT_LEVEL);

al_clear_to_color(al_map_rgb(85,170,85));

// Draw header
al_draw_filled_rounded_rectangle(cards_desk_layout.header.bar_x1 , cards_desk_layout.header.bar_y1, cards_desk_layout.header.bar_x2,cards_desk_layout.header.bar_y2,5.0,5.0, al_map_rgb(100,100,200));

// Draw footer
al_draw_filled_rounded_rectangle(cards_desk_layout.footer.bar_x1 , cards_desk_layout.footer.bar_y1, cards_desk_layout.footer.bar_x2,cards_desk_layout.footer.bar_y2,5.0,5.0, al_map_rgb(200,100,100));

// Draw desk area
al_draw_filled_rectangle(cards_desk_layout.card_desk.bar_x1 , cards_desk_layout.card_desk.bar_y1, cards_desk_layout.card_desk.bar_x2,cards_desk_layout.card_desk.bar_y2, al_map_rgb(85,170,85));



// 0 - no card clicked
// 1 - one card clicked and waiting for another
//

for (i=0; i<DESK_HEIGHTS[CURRENT_LEVEL]; i++)
    for (j=0; j<DESK_WIDTHS[CURRENT_LEVEL]; j++)
   {
   v_bitmap_w = al_get_bitmap_width(back_card);
   v_bitmap_h = al_get_bitmap_height(back_card);

   (*(deck+DESK_WIDTHS[CURRENT_LEVEL]*i+j)).back_bitmap = back_card;
   (*(deck+DESK_WIDTHS[CURRENT_LEVEL]*i+j)).face_bitmap = al_create_bitmap(v_bitmap_w, v_bitmap_h);
   al_set_target_bitmap((*(deck+DESK_WIDTHS[CURRENT_LEVEL]*i+j)).face_bitmap);

   al_draw_bitmap_region(full_deck,
        (*(deck+DESK_WIDTHS[CURRENT_LEVEL]*i+j)).rank_index*v_bitmap_w+1*((*(deck+DESK_WIDTHS[CURRENT_LEVEL]*i+j)).rank_index>0),
        (*(deck+DESK_WIDTHS[CURRENT_LEVEL]*i+j)).suit_index*v_bitmap_h+1*((*(deck+DESK_WIDTHS[CURRENT_LEVEL]*i+j)).suit_index>0),
        v_bitmap_w, v_bitmap_h,0,0,0 );

   (*(deck+DESK_WIDTHS[CURRENT_LEVEL]*i+j)).x_pos = cards_desk_layout.card_desk.bar_x1+j*(cards_desk_layout.card_size_horizontal+cards_desk_layout.gap_size_horizontal );
   (*(deck+DESK_WIDTHS[CURRENT_LEVEL]*i+j)).y_pos = cards_desk_layout.card_desk.bar_y1+i*(cards_desk_layout.card_size_vertical+cards_desk_layout.gap_size_vertical);
   (*(deck+DESK_WIDTHS[CURRENT_LEVEL]*i+j)).r_width = cards_desk_layout.card_size_horizontal;
   (*(deck+DESK_WIDTHS[CURRENT_LEVEL]*i+j)).r_height = cards_desk_layout.card_size_vertical;

   al_set_target_backbuffer(display);
   al_draw_scaled_bitmap((*(deck+DESK_WIDTHS[CURRENT_LEVEL]*i+j)).back_bitmap ,0, 0, v_bitmap_w, v_bitmap_h,
        (*(deck+DESK_WIDTHS[CURRENT_LEVEL]*i+j)).x_pos,
        (*(deck+DESK_WIDTHS[CURRENT_LEVEL]*i+j)).y_pos,
        (*(deck+DESK_WIDTHS[CURRENT_LEVEL]*i+j)).r_width,
        (*(deck+DESK_WIDTHS[CURRENT_LEVEL]*i+j)).r_height,
        0);

    }


al_flip_display();

   event_queue = al_create_event_queue();
   if(!event_queue) {
      fprintf(stderr, "failed to create event_queue!\n");
      al_destroy_display(display);
      al_destroy_timer(timer);
      return -1;
   }

   al_register_event_source(event_queue, al_get_display_event_source(display));

   al_register_event_source(event_queue, al_get_timer_event_source(timer));

   al_register_event_source(event_queue, al_get_mouse_event_source());

//   al_clear_to_color(al_map_rgb(0,0,0));

//   al_flip_display();

//   al_start_timer(timer);
// al_draw_bitmap(bouncer, 10, 10, 0);
// al_flip_display();
   while(1)
   {
      ALLEGRO_EVENT ev;
      DRAWING_BAR flip_area;

      al_wait_for_event(event_queue, &ev);

      if(ev.type == ALLEGRO_EVENT_TIMER) {
         redraw = true;
      }
      else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
         break;
      }
      /*
      else if(ev.type == ALLEGRO_EVENT_MOUSE_AXES ||
              ev.type == ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY) {

         bouncer_x = ev.mouse.x;
         bouncer_y = ev.mouse.y;
      }*/
      else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
          if (ev.mouse.button & 2 ) {break;}
          if (ev.mouse.button & 1 )
          {
                clicked_card = deck_find_clicked_card(deck,ev.mouse.x,ev.mouse.y,DESK_SIZES[CURRENT_LEVEL]);
                if (clicked_card != NULL)
                    {
                    if (game_state == 0)
                    {
                       fprintf(stderr, "Clicked %c%c\n", (*clicked_card).rank, (*clicked_card).suit );
                       flip_area.bar_width = (*clicked_card).r_width;
                       flip_area.bar_height = (*clicked_card).r_height;
                       flip_area.bar_x1 = (*clicked_card).x_pos;
                       flip_area.bar_y1 = (*clicked_card).y_pos;
                       flip_card(display,(*clicked_card).back_bitmap,(*clicked_card).face_bitmap,flip_area,al_map_rgb(85,170,85), DESK_FPS[CURRENT_LEVEL] );
                       (*clicked_card).flipped = 1;
                       game_state = 1;
                       flipped_1=clicked_card;

                     }
                    else if (game_state == 1)
                    {
                       fprintf(stderr, "Clicked %c%c\n", (*clicked_card).rank, (*clicked_card).suit );
                       flip_area.bar_width = (*clicked_card).r_width;
                       flip_area.bar_height = (*clicked_card).r_height;
                       flip_area.bar_x1 = (*clicked_card).x_pos;
                       flip_area.bar_y1 = (*clicked_card).y_pos;
                       flip_card(display,(*clicked_card).back_bitmap,(*clicked_card).face_bitmap,flip_area,al_map_rgb(85,170,85), DESK_FPS[CURRENT_LEVEL] );
                       (*clicked_card).flipped = 1;
                       if (((*clicked_card).rank != (*flipped_1).rank) || ((*clicked_card).suit != (*flipped_1).suit))
                        {
                       flip_card(display,(*clicked_card).face_bitmap,(*clicked_card).back_bitmap,flip_area,al_map_rgb(85,170,85), DESK_FPS[CURRENT_LEVEL] );
                       flip_area.bar_width = (*flipped_1).r_width;
                       flip_area.bar_height = (*flipped_1).r_height;
                       flip_area.bar_x1 = (*flipped_1).x_pos;
                       flip_area.bar_y1 = (*flipped_1).y_pos;
                       flip_card(display,(*flipped_1).face_bitmap,(*flipped_1).back_bitmap,flip_area,al_map_rgb(85,170,85), DESK_FPS[CURRENT_LEVEL] );
                       }
                       game_state = 0;
                     }
                     }

                    }

          }
//          if  ev.mouse.x <=
      }

      if(redraw && al_is_event_queue_empty(event_queue)) {
         redraw = false;

//         al_clear_to_color(al_map_rgb(0,0,0));

//         al_draw_bitmap(bouncer, bouncer_x, bouncer_y, 0);

//         al_flip_display();
      }

   deck_destroy(deck);
   al_destroy_bitmap(basic_card);
   al_destroy_bitmap(full_deck);
   al_destroy_timer(timer);
   al_destroy_display(display);
   al_destroy_event_queue(event_queue);

   return 0;
}
