#include <VirtualPinToNote.h>

constexpr int8_t NUM_LOWER_KEYS = 44;
constexpr int8_t NUM_UPPER_KEYS = 44;
constexpr int8_t LOWER_KEY_SHIFT = 6;
constexpr int8_t UPPER_KEY_SHIFT = 10;

constexpr int8_t VPIN_TO_KEY_NUM[88] =
{
    7,
    15,
    23,
    31,
    39,
    47,
    55,
    63,
    71,
    79,
    87,
    3,
    11,
    19,
    27,
    35,
    43,
    51,
    59,
    67,
    75,
    83,
    5,
    13,
    21,
    29,
    37,
    45,
    53,
    61,
    69,
    77,
    85,
    1,
    9,
    17,
    25,
    33,
    41,
    49,
    57,
    65,
    73,
    81,
    6,
    14,
    22,
    30,
    38,
    46,
    54,
    62,
    70,
    78,
    86,
    2,
    10,
    18,
    26,
    34,
    42,
    50,
    58,
    66,
    74,
    82,
    4,
    12,
    20,
    28,
    36,
    44,
    52,
    60,
    68,
    76,
    84,
    0,
    8,
    16,
    24,
    32,
    40,
    48,
    56,
    64,
    72,
    80
};

uint8_t VirtualPinToNote(uint8_t vPin, int8_t lowOctave, int8_t upperOctave)
{
    int8_t keyNum = VPIN_TO_KEY_NUM[vPin];

    bool isUpper = keyNum >= NUM_LOWER_KEYS;

    if (isUpper)
    {
        keyNum = keyNum + upperOctave * 12 + UPPER_KEY_SHIFT;
    }
    else
    {
        keyNum = keyNum + lowOctave * 12 + LOWER_KEY_SHIFT;
    }

    return (uint8_t)keyNum;
}