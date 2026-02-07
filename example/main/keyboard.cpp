/**
 * @file keyboard.cpp
 * @author Forairaaaaa
 * @brief 
 * @version 0.1
 * @date 2023-09-22
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "keyboard.h"
#include <driver/gpio.h>


#define digitalWrite(pin, level) gpio_set_level((gpio_num_t)pin, level)
#define digitalRead(pin) gpio_get_level((gpio_num_t)pin)


using namespace KEYBOARD;


static const std::vector<int> output_list = {8, 9, 11};
static const std::vector<int> input_list = {13, 15, 3, 4, 5, 6, 7};

static const Chart_t X_map_chart[7] = 
{
    {1, 0, 1},
    {2, 2, 3},
    {4, 4, 5},
    {8, 6, 7},
    {16, 8, 9},
    {32, 10, 11},
    {64, 12, 13}
};

static const KeyValue_t _key_value_map[4][14] = {
{{"`", KEY_GRAVE, "~", KEY_GRAVE},
    {"1", KEY_1, "!", KEY_1},
    {"2", KEY_2, "@", KEY_2},
    {"3", KEY_3, "#", KEY_3},
    {"4", KEY_4, "$", KEY_4},
    {"5", KEY_5, "%", KEY_5},
    {"6", KEY_6, "^", KEY_6},
    {"7", KEY_7, "&", KEY_7},
    {"8", KEY_8, "*", KEY_KPASTERISK},
    {"9", KEY_9, "(", KEY_KPLEFTPAREN},
    {"0", KEY_0, ")", KEY_KPRIGHTPAREN},
    {"-", KEY_MINUS, "_", KEY_KPMINUS},
    {"=", KEY_EQUAL, "+", KEY_KPPLUS},
    {"del", KEY_BACKSPACE, "del", KEY_BACKSPACE}},
{{"tab", KEY_TAB, "tab", KEY_TAB},
    {"q", KEY_Q, "Q", KEY_Q},
    {"w", KEY_W, "W", KEY_W},
    {"e", KEY_E, "E", KEY_E},
    {"r", KEY_R, "R", KEY_R},
    {"t", KEY_T, "T", KEY_T},
    {"y", KEY_Y, "Y", KEY_Y},
    {"u", KEY_U, "U", KEY_U},
    {"i", KEY_I, "I", KEY_I},
    {"o", KEY_O, "O", KEY_O},
    {"p", KEY_P, "P", KEY_P},
    {"[", KEY_LEFTBRACE, "{", KEY_LEFTBRACE},
    {"]", KEY_RIGHTBRACE, "}", KEY_RIGHTBRACE},
    {"\\", KEY_BACKSLASH, "|", KEY_BACKSLASH}},
{{"fn", 0, "fn", 0},
    {"shift", 0, "shift", 0},
    {"a", KEY_A, "A", KEY_A},
    {"s", KEY_S, "S", KEY_S},
    {"d", KEY_D, "D", KEY_D},
    {"f", KEY_F, "F", KEY_F},
    {"g", KEY_G, "G", KEY_G},
    {"h", KEY_H, "H", KEY_H},
    {"j", KEY_J, "J", KEY_J},
    {"k", KEY_K, "K", KEY_K},
    {"l", KEY_L, "L", KEY_L},
    {";", KEY_SEMICOLON, ":", KEY_SEMICOLON},
    {"'", KEY_APOSTROPHE, "\"", KEY_APOSTROPHE},
    {"enter", KEY_ENTER, "enter", KEY_ENTER}},
{{"ctrl", KEY_LEFTCTRL, "ctrl", KEY_LEFTCTRL},
    {"opt", 0, "opt", 0},
    {"alt", KEY_LEFTALT, "alt", KEY_LEFTALT},
    {"z", KEY_Z, "Z", KEY_Z},
    {"x", KEY_X, "X", KEY_X},
    {"c", KEY_C, "C", KEY_C},
    {"v", KEY_V, "V", KEY_V},
    {"b", KEY_B, "B", KEY_B},
    {"n", KEY_N, "N", KEY_N},
    {"m", KEY_M, "M", KEY_M},
    {",", KEY_COMMA, "<", KEY_COMMA},
    {".", KEY_DOT, ">", KEY_DOT},
    {"/", KEY_KPSLASH, "?", KEY_KPSLASH},
    {"space", KEY_SPACE, "space", KEY_SPACE}}};

KeyValue_t Keyboard::getKeyValue(const Point2D_t& keyCoor) 
{ 
    return _key_value_map[keyCoor.y][keyCoor.x]; 
}

void Keyboard::_set_output(const std::vector<int>& pinList, uint8_t output)
{
    output = output & 0B00000111;

    digitalWrite(pinList[0], (output & 0B00000001));
    digitalWrite(pinList[1], (output & 0B00000010));
    digitalWrite(pinList[2], (output & 0B00000100));
}


uint8_t Keyboard::_get_input(const std::vector<int>& pinList)
{
    uint8_t buffer = 0x00;
    uint8_t pin_value = 0x00;

    for (int i = 0; i < 7; i++) 
    {
        pin_value = (digitalRead(pinList[i]) == 1) ? 0x00 : 0x01;
        pin_value = pin_value << i;
        buffer = buffer | pin_value;
    }

    return buffer;
}


void Keyboard::init()
{
    // for (auto i : output_list) {
    //     gpio_reset_pin((gpio_num_t)i);
    //     pinMode(i, OUTPUT);
    //     digitalWrite(i, 0);
    // }

    // for (auto i : input_list) {
    //     gpio_reset_pin((gpio_num_t)i);
    //     pinMode(i, INPUT_PULLUP);
    // }

    // _set_output(output_list, 0);


    for (auto i : output_list) 
    {
        gpio_reset_pin((gpio_num_t)i);
        gpio_set_direction((gpio_num_t)i, GPIO_MODE_OUTPUT);
        gpio_set_pull_mode((gpio_num_t)i, GPIO_PULLUP_PULLDOWN);
        digitalWrite(i, 0);
    }

    for (auto i : input_list) 
    {
        gpio_reset_pin((gpio_num_t)i);
        gpio_set_direction((gpio_num_t)i, GPIO_MODE_INPUT);
        gpio_set_pull_mode((gpio_num_t)i, GPIO_PULLUP_ONLY);
    }

    _set_output(output_list, 0);
}


Point2D_t Keyboard::getKey()
{
    Point2D_t coor;
    coor.x = -1;
    coor.y = -1;

    uint8_t input_value = 0;

    for (int i = 0; i < 8; i++) {
        _set_output(output_list, i);
        // printf("% 3d,\t", get_input(inputList));

        input_value = _get_input(input_list);

        /* If key pressed */
        if (input_value) {

            /* Get X */
            for (int j = 0; j < 7; j++) {
                if (input_value == X_map_chart[j].value) {
                    coor.x = (i > 3) ? X_map_chart[j].x_1 : X_map_chart[j].x_2;
                    break;
                }
            }

            /* Get Y */
            coor.y = (i > 3) ? (i - 4) : i;

            /* Keep the same as picture */
            coor.y = -coor.y;
            coor.y = coor.y + 3;


            break;
        }
    }

    // printf("%d,%d\n", x, y);
    return coor;
}


uint8_t Keyboard::getKeyNum(Point2D_t keyCoor)
{
    uint8_t ret = 0;

    if ((keyCoor.x < 0) || (keyCoor.y < 0)) {
        return 0;
    }

    ret = (keyCoor.y * 14) + (keyCoor.x + 1);

    return ret;
}


void Keyboard::updateKeyList()
{
    _key_list_buffer.clear();


    Point2D_t coor;

    uint8_t input_value = 0;

    for (int i = 0; i < 8; i++) {
        _set_output(output_list, i);
        // printf("% 3d,\t", get_input(inputList));

        input_value = _get_input(input_list);
        

        /* If key pressed */
        if (input_value) {

            // printf("c: ");
            
            /* Get X */
            for (int j = 0; j < 7; j++) {

                // if (input_value == X_map_chart[j].value) {
                //     coor.x = (i > 3) ? X_map_chart[j].x_1 : X_map_chart[j].x_2;
                //     break;
                // }



                if (input_value & (0x01 << j)) {
                    coor.x = (i > 3) ? X_map_chart[j].x_1 : X_map_chart[j].x_2;
                
                    /* Get Y */
                    coor.y = (i > 3) ? (i - 4) : i;
                    // printf("%d,%d\t", coor.x, coor.y); 


                    /* Keep the same as picture */
                    coor.y = -coor.y;
                    coor.y = coor.y + 3;

                    _key_list_buffer.push_back(coor);
                }


            }

            // printf("\n");


            // /* Get Y */
            // coor.y = (i > 3) ? (i - 4) : i;
        }
    }
}


bool Keyboard::isKeyPressing(int keyNum)
{
    if (_key_list_buffer.size())
    {
        for (const auto& i : _key_list_buffer)
        {
            if (getKeyNum(i) == keyNum)
                return true;
        }
    }
    return false;
}


#include <cstring>

void Keyboard::updateKeysState()
{
    _keys_state_buffer.reset();
    _key_values_without_special_keys.clear();

    // Get special keys 
    for (auto& i : _key_list_buffer)
    {
        int key_code = getKeyValue(i).value_num_first;
        const char* key_str = getKeyValue(i).value_first; // Fallback for fn/opt which have 0 key_code

        if (key_code == KEY_TAB)
        {
            _keys_state_buffer.tab = true;
            continue;
        }
        if (key_code == 0 && strcmp(key_str, "fn") == 0)
        {
            _keys_state_buffer.fn = true;
            continue;
        }
        if (key_code == 0 && strcmp(key_str, "shift") == 0)
        {
            _keys_state_buffer.shift = true;
            continue;
        }
        if (key_code == KEY_LEFTCTRL)
        {
            _keys_state_buffer.ctrl = true;
            continue;
        }
        if (key_code == 0 && strcmp(key_str, "opt") == 0)
        {
            _keys_state_buffer.opt = true;
            continue;
        }
        if (key_code == KEY_LEFTALT)
        {
            _keys_state_buffer.alt = true;
            continue;
        }
        if (key_code == KEY_BACKSPACE)
        {
            _keys_state_buffer.del = true;
            _keys_state_buffer.hidKey.push_back(KEY_BACKSPACE);
            continue;
        }
        if (key_code == KEY_ENTER)
        {
            _keys_state_buffer.enter = true;
            _keys_state_buffer.hidKey.push_back(KEY_ENTER);
            continue;
        }
        if (key_code == KEY_SPACE)
        {
            _keys_state_buffer.space = true;
            _keys_state_buffer.hidKey.push_back(KEY_SPACE);
            continue;
        }

        _key_values_without_special_keys.push_back(i);
    }

    // Deal what left
    for (auto& i : _key_values_without_special_keys)
    {
        if (_keys_state_buffer.ctrl || _keys_state_buffer.shift || _is_caps_locked)
        {
            _keys_state_buffer.values.push_back(*getKeyValue(i).value_second);
            _keys_state_buffer.hidKey.push_back(getKeyValue(i).value_num_second);
        }
        else
        {
            _keys_state_buffer.values.push_back(*getKeyValue(i).value_first);
            _keys_state_buffer.hidKey.push_back(getKeyValue(i).value_num_first);
        }
    }
}


bool Keyboard::isChanged() 
{
    if (_last_key_size != _key_list_buffer.size()) 
    {
        _last_key_size = _key_list_buffer.size();
        return true;
    } 
    return false;
}
