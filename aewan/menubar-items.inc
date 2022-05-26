static Menu menu[] = {
   { "[F1] File", 1, 0, 0,
       7,
       {
                 { 'N', "&New", COMMAND_NEW_FILE,
                                "Clears everything and starts from scratch" },
                 { 'L', "&Load...", COMMAND_LOAD_FILE,
                                    "Loads a file into the editor" },
                 { 'S', "&Save", COMMAND_SAVE_FILE,
                                    "Saves the current drawing" },
                 { 'A', "Save &As...", COMMAND_SAVE_FILE_AS,
                            "Saves the current drawing in a specified file" },
                 { 'M', "Edit &Meta...", COMMAND_EDIT_META,
                 "Opens an editor for you to edit this document's metadata" },
                 { ' ', "", COMMAND_UNDEFINED, NULL }, 
                 { 'Q', "&Quit", COMMAND_QUIT,
                                   "Quits program" },
       }
   },

   { "[F2] Edit", 16, 0, 0,
       8,
       {
                { 'C', "Drawing &color...", COMMAND_SHOW_COLOR_DLG,
                                     "Brings up the color selection dialog" },
                { 'I', "Toggle &Insert", COMMAND_TOGGLE_INSERT, 
                                     "Toggles insert/overwrite mode" },
                { 'K', "Pic&k color", COMMAND_PICK_COLOR,
                          "Picks color under cursor to use as current color" },
                { 'T', "&Tint cell", COMMAND_TINT_CELL,
                          "Applies current color to cell under cursor" },
                { ' ', "", COMMAND_UNDEFINED, NULL }, 
                { 'S', "Toggle &select mode", COMMAND_TOGGLE_SELECTION,
                          "Toggles select mode on/off" },
                { 'L', "Toggle &line-drawing", COMMAND_TOGGLE_LINE_MODE,
                  "Toggles line-drawing mode on/off. Use 0-9 to draw lines" },
                { 'P', "Toggle com&posite", COMMAND_TOGGLE_COMPOSITE,
                  "Toggles composite (all-layer) view on/off" },
       }
   },

   { "[F3] Layer", 32, 0, 0,
       9,
       {
                { 'A', "&Add layer (defaults)", COMMAND_ADD_LAYER_DEFAULTS,
                                      "Adds a layer with default parameters" },
                { 'L', "Add &layer (specify)...", COMMAND_ADD_LAYER_SPECIFY,
                                      "Adds a layer with custom parameters" },
                { 'D', "&Duplicate layer", COMMAND_DUP_LAYER,
      "Duplicates current layer. Copy is added to the bottom of the stack." },
                { 'R', "&Rename layer...", COMMAND_RENAME_LAYER,
                       "Renames the current layer" },
                { 'Z', "Resi&ze layer...", COMMAND_RESIZE_LAYER,
                        "Resizes the current layer" },
                { 'M', "&Manage layers...", COMMAND_SHOW_LAYER_DLG,
                       "Shows a dialog in which you can manipulate layers" },
                { ' ', "", COMMAND_UNDEFINED, NULL }, 
                { 'N', "Go to &next layer", COMMAND_NEXT_LAYER,
                       "Goes to the next layer" },
                { 'P', "Go to &previous layer", COMMAND_PREV_LAYER,
                       "Goes to the previous layer" },
       }
   },
   
   { "[F4] Help", 64, 0, 0,
       3,
       {
                { 'K', "&Key reference", COMMAND_SHOW_HELP_DLG,
                            "Shows a table of keys and their functions" },
                { ' ', "", COMMAND_UNDEFINED, NULL }, 
                { 'A', "&About Aewan", COMMAND_SHOW_ABOUT_DLG,
                                  "Shows information about the program" },
       }
   },
};
static int menu_count = 4;

