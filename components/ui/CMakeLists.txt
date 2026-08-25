idf_component_register(
    SRCS "src/chess_geom.c" "src/chess_dirty.c" "src/chess_fsm.c" "src/chess_ui.c"
         "src/ui_font.c"
    INCLUDE_DIRS "include"
    REQUIRES board chess
)
