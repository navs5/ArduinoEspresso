#ifndef THERMISTORTABLES_H
#define THERMISTORTABLES_H

#include "lookupTable.hpp"

constexpr DataPoint_1D thermTable_ntc3950_100K[] =
{
    // reistor [Ohms], temperature [deg C]
    {  2940.0F, 130.0F},
    {  3850.0F, 120.0F},
    {  5070.0F, 110.0F},
    {  6710.0F, 100.0},
    {  9100.0F, 90.0F},
    { 12540.0F, 80.0F},
    { 17550.0F, 70.0F},
    { 25000.0F, 60.0F},
    { 35899.9F, 50.0F},
    { 53500.0F, 40.0F},
    { 81000.0F, 30.0F},
    {125245.0F, 20.0F},
    {199990.0F, 10.0F},
    {327240.0F, 0.0F},
    {551410.0F, -10.0F},
};

#endif  // THERMISTORTABLES_H