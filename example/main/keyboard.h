/**
 * @file keyboard.h
 * @author Forairaaaaa
 * @brief 
 * @version 0.1
 * @date 2023-09-22
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once
#include <iostream>
#include <vector>
#include "keymap.h"


namespace KEYBOARD 
{
    struct Chart_t 
    {
        uint8_t value;
        uint8_t x_1;
        uint8_t x_2;
    };

    struct Point2D_t 
    {
        int x;
        int y;
    };

    struct KeyValue_t 
    {
        const char* value_first;
        const int value_num_first;
        const char* value_second;
        const int value_num_second;
    };

    class Keyboard
    {
        public:
            
            struct KeysState
            {
                bool tab = false;
                bool fn = false;
                bool shift = false;
                bool ctrl = false;
                bool opt = false;
                bool alt = false;
                bool del = false;
                bool enter = false;
                bool space  = false;

                std::vector<char> values;
                std::vector<int> hidKey;

                void reset()
                {
                    tab = false;
                    fn = false;
                    shift = false;
                    ctrl = false;
                    opt = false;
                    alt = false;
                    del = false;
                    enter = false;
                    space = false;

                    values.clear();
                    hidKey.clear();
                }
            };


        private:
            std::vector<Point2D_t> _key_list_buffer;

            std::vector<Point2D_t> _key_values_without_special_keys;
            KeysState _keys_state_buffer;

            bool _is_caps_locked;
            uint8_t _last_key_size;

            void _set_output(const std::vector<int>& pinList, uint8_t output);
            uint8_t _get_input(const std::vector<int>& pinList);


        public:
            Keyboard() : 
                _is_caps_locked(false),
                _last_key_size(0)
                {}

            void init();

            Point2D_t getKey();

            uint8_t getKeyNum(Point2D_t keyCoor);

            void updateKeyList();
            inline std::vector<KEYBOARD::Point2D_t>& keyList() { return _key_list_buffer; }

            KeyValue_t getKeyValue(const Point2D_t& keyCoor);

            bool isKeyPressing(int keyNum);

            void updateKeysState();
            inline KeysState& keysState() { return _keys_state_buffer; }

            inline bool capslocked(void) { return _is_caps_locked; }
            inline void setCapsLocked(bool isLocked) { _is_caps_locked = isLocked; }

            bool isChanged();
            inline uint8_t isPressed() { return _key_list_buffer.size(); }
    };
}
