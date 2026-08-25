idf_component_register(
    SRCS
        "src/chess_api.cpp"
        "src/chess_engine.cpp"
        "src/arduino_shim_idf.cpp"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES esp_timer freertos
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE CHESS_ENGINE_LIBRARY)

target_compile_options(${COMPONENT_LIB} PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:-Wno-parentheses;-Wno-sign-compare;-Wno-unused-parameter;-Wno-unused-variable;-Wno-type-limits;-Wno-dangling-else;-Wno-return-type;-Wno-format>"
)
