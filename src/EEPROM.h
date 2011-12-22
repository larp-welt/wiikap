#include <avr/eeprom.h>

static uint8_t checkNewConf = 147;
static uint8_t P8, I8, D8; //8 bits is much faster and the code is much shorter

typedef struct eep_entry_t{
  void *  var;
  uint8_t size;
};

// ************************************************************************************************************
// EEPROM Layout definition
// ************************************************************************************************************
static eep_entry_t eep_entry[] = {
  &checkNewConf, sizeof(checkNewConf)
, &P8, sizeof(P8)
, &I8, sizeof(I8) 
, &D8, sizeof(D8) 
};  
#define EEBLOCK_SIZE sizeof(eep_entry)/sizeof(eep_entry_t)
// ************************************************************************************************************

void readEEPROM() {
  uint8_t i, _address = eep_entry[0].size;
  for(i=1; i<EEBLOCK_SIZE; i++) {
    eeprom_read_block(eep_entry[i].var, (void*)(_address), eep_entry[i].size); _address += eep_entry[i].size;
  }  
}

void writeParams() {
  uint8_t i, _address = 0;
  for(i=0; i<EEBLOCK_SIZE; i++) {
    eeprom_write_block(eep_entry[i].var, (void*)(_address), eep_entry[i].size); _address += eep_entry[i].size;
  }  
  readEEPROM();
}

void checkFirstTime() {
  uint8_t test_val; eeprom_read_block((void*)&test_val, (void*)(0), sizeof(test_val));
  //if (test_val == checkNewConf) return;
  P8 = 40;
  I8 = 30;
  D8 = 23;
  writeParams();
}
