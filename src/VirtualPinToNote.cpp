#include <VirtualPinToNote.h>

constexpr uint8_t VPIN_TO_NOTE_BASE[88] =
{
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
    92,
    8,
    16,
    24,
    32,
    40,
    48,
    56,
    64,
    72,
    80,
    88,
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
    90,
    06,
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
    91,
    07,
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
    89,
    05,
    13,
    21,
    29,
    37,
    45,
    53,
    61,
    69,
    77,
    85
};

uint8_t VirtualPinToNote(uint8_t vPin)
{
    return VPIN_TO_NOTE_BASE[vPin] + 24; // TEMP: Bump by octave
}