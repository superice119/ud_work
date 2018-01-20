#ifndef __RFID_TEST_MODE_H__
#define __RFID_TEST_MODE_H__

#include "rfid.h" 


int menu();

void Open_Comm();

void Close_Comm();

void ReaderReset();

void ReadTemperatureTagData();


int test_mode();

#endif //__RFID_TEST_MODE_H_

