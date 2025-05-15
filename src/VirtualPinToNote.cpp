#include <VirtualPinToNote.h>
#include <Constants.h>

// These used to be the way the keys were ordered but now they are ordered more logically.
// constexpr uint8_t VPIN_TO_KEY_NUM[88] =
// {
//     7, 15, 23, 31, 39, 47, 55, 63, 71, 79, 87,
//     3, 11, 19, 27, 35, 43, 51, 59, 67, 75, 83, 
//     5, 13, 21, 29, 37, 45, 53, 61, 69, 77, 85,
//     1, 9,  17, 25, 33, 41, 49, 57, 65, 73, 81,
//     6, 14, 22, 30, 38, 46, 54, 62, 70, 78, 86,
//     2, 10, 18, 26, 34, 42, 50, 58, 66, 74, 82,
//     4, 12, 20, 28, 36, 44, 52, 60, 68, 76, 84,
//     0, 8,  16, 24, 32, 40, 48, 56, 64, 72, 80
// };

// constexpr uint8_t KEY_NUM_TO_VPIN[88] = 
// {
//     77, 33, 55, 11, 66, 22, 44, 0,
//     78, 34, 56, 12, 67, 23, 45, 1,
//     79, 35, 57, 13, 68, 24, 46, 2,
//     80, 36, 58, 14, 69, 25, 47, 3,
//     81, 37, 59, 15, 70, 26, 48, 4,
//     82, 38, 60, 16, 71, 27, 49, 5,
//     83, 39, 61, 17, 72, 28, 50, 6,
//     84, 40, 62, 18, 73, 29, 51, 7,
//     85, 41, 63, 19, 74, 30, 52, 8,
//     86, 42, 64, 20, 75, 31, 53, 9,
//     87, 43, 65, 21, 76, 32, 54, 10
// };

uint8_t VirtualPinToKeyNum(uint8_t vPin)
{
    return vPin - NOTES_VPIN_START;
}

uint8_t KeyNumToVirtualPin(uint8_t keyNum)
{
    return keyNum + NOTES_VPIN_START;
}

bool IsUpperKey(uint8_t keyNum) // Is key on the upper keyboard?
{
    return keyNum >= NUM_LOWER_KEYS;
}