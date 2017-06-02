#include <pebble.h>     /* Pebble */

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_text_layer;
static TextLayer *s_ampm_layer;
static Layer *shape_layer;
static GFont s_time_font;
static GFont s_font;
static GBitmap *s_bitmap;
static BitmapLayer *s_bitmap_layer;
void drawCircle(int posL, int posH, int Offset, GContext *ctx);
void isColor(GColor watchcolor, GColor watchbw);

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H%M" : "%l%M", tick_time);
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
};

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
};

//Bluetooth notification actions
void bt_handler(bool connected) {
  if (connected) {
    //If connected remove notification
    layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layer), true);
  } else {
    //If disconnected add notification
    layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layer), false);
    //Turn on backlight
    light_enable_interaction();
    //Vibrate
    vibes_long_pulse();
  };
};


static void shape_update_proc(Layer *this_layer, GContext *ctx) {
  //Get the current time as a struct
  time_t rawtime; 
  time (&rawtime); 
  struct tm *tm_struct = localtime(&rawtime);
  //Break down the time into each digit so we can use
  //those digits to assign colors below
  int hour = tm_struct->tm_hour; //Get the hours
  
  int posH = 90;
  int posL = 18;
  #if defined(PBL_ROUND)
  posH = posH-13;
  posL = posL+17;
  #endif
  #if PBL_DISPLAY_WIDTH == 200
  posH = posH+21;
  posL = posL+26;
  #endif
  
  void isColor(GColor watchcolor, GColor watchbw){
    #if defined(PBL_COLOR) 
    graphics_context_set_fill_color(ctx, watchcolor);
    #else
    graphics_context_set_fill_color(ctx, watchbw);
    #endif
  }
  
  //Draw the shuttle (top horizontal line)
  isColor(GColorLightGray, GColorLightGray);
  graphics_fill_rect(ctx, GRect(0, posH-33, 200, 20), 0, GCornerNone);
  //Draw the 7 line (bottom horizontal line)
  isColor(GColorPurple, GColorLightGray);
  graphics_fill_rect(ctx, GRect(0, posH-11, 200, 20), 0, GCornerNone);
  //Draw the backround behind the 6th Ave lines (four horizontals)
  isColor(GColorWhite, GColorBlack);
  graphics_fill_rect(ctx, GRect(posL+11, posH-35, 72, 45), 0, GCornerNone);
  //Draw the 6th Ave lines (four horizontals)
  isColor(GColorOrange, GColorWhite);
  graphics_fill_rect(ctx, GRect(posL+14, 0, 20, 250), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(posL+37, 0, 20, 250), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(posL+60, 0, 20, 250), 0, GCornerNone);
  //Draw TS station (AM/PM dot)
  drawCircle(posL, posH-23, 102, ctx);
  //Draw 6th Ave stations (four dots)
  drawCircle(posL, posH, 23, ctx);
  drawCircle(posL, posH, 45, ctx);
  drawCircle(posL, posH, 69, ctx);
  
  if(hour > 9 || clock_is_24h_style()){
    isColor(GColorWhite, GColorBlack);
    graphics_fill_rect(ctx, GRect(posL-12, posH-35, 5, 45), 0, GCornerNone);
    isColor(GColorOrange, GColorWhite);
    graphics_fill_rect(ctx, GRect(posL-9, 0, 20, 250), 0, GCornerNone);
    drawCircle(posL, posH, 0, ctx);
  }
  
  //Add AM/PM indicator (A or P)
  if (hour > 12){
    text_layer_set_text(s_ampm_layer, "P"); //is PM
  }else{
    text_layer_set_text(s_ampm_layer, "A"); //Else is AM
  };
};

//Function to draw colored circles for 0111 - does not draw for first digit yet
void drawCircle(int posL, int posH, int Offset, GContext *ctx){
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Reached drawTimeCircle for %i", timeDiv);
    GPoint outerCircle = GPoint(posL + Offset, posH);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx, outerCircle, 5);
}


static void main_window_load(Window *window) {
  // Get information about the Window and set background
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  #if defined(PBL_COLOR) 
  window_set_background_color(s_main_window, GColorWhite);
  #else
  window_set_background_color(s_main_window, GColorBlack);
  #endif
  //Define circle layer
  shape_layer = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  //Add all layers to the app window
  layer_add_child(window_layer, shape_layer);
  #if defined(PBL_ROUND)
  s_time_layer = text_layer_create(GRect(-14, 81, bounds.size.w, 50));//Create hour text
  s_text_layer = text_layer_create(GRect(120, 87, 53, 45));//Create station stop text
  s_ampm_layer = text_layer_create(GRect(bounds.size.w-35, 42, 19, 20)); //Create AM/PM text
  #else
    #if defined(PBL_PLATFORM_EMERY)
    s_time_layer = text_layer_create(GRect(-15, 116, bounds.size.w, 50));//Create hour text
    s_text_layer = text_layer_create(GRect(120, 0, 53, 45));//Create station stop text
    s_ampm_layer = text_layer_create(GRect(bounds.size.w-19, 75, 19, 20)); //Create AM/PM text
    #else
    s_time_layer = text_layer_create(GRect(10, bounds.size.h/2+10, 100, 21));//Create time text
    s_text_layer = text_layer_create(GRect(101, 99, 53, 45));//Create station stop text
    s_ampm_layer = text_layer_create(GRect(bounds.size.w-19, 55, 19, 20)); //Create AM/PM text
    #endif
  #endif
  // Create the BitmapLayer
  s_bitmap_layer = bitmap_layer_create(GRect(126, 5, 15, 15));
  text_layer_set_text(s_text_layer, "42 st \n6 Ave");
  //Set time text attributes
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_ampm_layer, GColorBlack);
  //Color of 42nd st 6 ave text
  #if defined(PBL_COLOR) 
  text_layer_set_text_color(s_text_layer, GColorBlack);
  #else
  text_layer_set_text_color(s_text_layer, GColorWhite);
  #endif
  
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentRight);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_ampm_layer, GTextAlignmentCenter);
  //Load custom resources (same font in two sizes)
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_HELVETICATRACKED_20));
  s_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_HELVETICA_20));
  s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_ICON);
  //Apply custom fonts
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_font(s_text_layer, s_font);
  text_layer_set_font(s_ampm_layer, s_font);
  //Set the bitmap
//   bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_ampm_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));
};

static void main_window_unload(Window *window) {
  //Destroy layers on window unload
  layer_destroy(shape_layer);
  text_layer_destroy(s_time_layer);
  bitmap_layer_destroy(s_bitmap_layer);
};

static void init(void) {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  }); 
  // Subscribe to get connection events
  bluetooth_connection_service_subscribe(bt_handler);
  //Show the Window on the watch, with animated=false
  window_stack_push(s_main_window, false); 
  
  //Set default properties
  layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layer), true);  //Set BT icon to hidden
  text_layer_set_background_color(s_time_layer, GColorClear);  //Used to set default background
  text_layer_set_background_color(s_text_layer, GColorClear);  //Used to set default background
  text_layer_set_background_color(s_ampm_layer, GColorClear);  //Used to set default background
  layer_set_update_proc(shape_layer, shape_update_proc);  //Draw all of the shapes on the shape layer
  
  // Make sure the time is displayed from the start
  update_time();  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
};

static void deinit() {
  // Destroy Window
   window_destroy(s_main_window);
};

int main(void) {
  init();
  app_event_loop();
  deinit();
};