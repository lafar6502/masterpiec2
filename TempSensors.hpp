#ifndef _TempSensors_hpp_Included_
#define _TempSensors_hpp_Included_

#include <stdint.h>
#include <string.h>
extern "C" {
    #include "onewire_library/onewire_library.h" 
    #include "onewire_library/ow_rom.h" 
    #include "onewire_library/ds18b20.h" 
}



#define TEMPSENSOR_MAXDEVS 8

/// @brief handle reading temperatures from DS18b20 temp sensors connected to a single data pin
class TempSensors 
{
    private:
        uint64_t romcode[TEMPSENSOR_MAXDEVS];
        OW _ow;
        uint8_t _numDevices = 0;
        uint _dataPin = 14;
    public:
        TempSensors() {

        }

        /// @brief initialize the communication
        /// return zero on success
        int Initialize(uint dataPin, PIO pio = pio0) 
        {
            _dataPin = dataPin;
            if (!pio_can_add_program (pio, &onewire_program)) 
            {
                return -1;
            }
            int offset = pio_add_program (pio, &onewire_program);
            if (offset < 0) {
                printf("Failed to add pio program %d\n", offset);
                return offset;
            }
            if (!ow_init(&_ow, pio, offset, _dataPin)) {
                printf("Failed to initialize onewire library\r\n");
                return -3;
            }
            
            int n = FindDevices();
            if (n < 0) {
                printf("Device scan failed %d\r\n", n);
                return n;
            }

            return 0;
        }

        int GetSensorCount() {
            return _numDevices;
        }

        /// @brief perform scan
        /// @return 
        int FindDevices() {

            uint64_t romcode[TEMPSENSOR_MAXDEVS];
            int num_devs = ow_romsearch (&_ow, romcode, TEMPSENSOR_MAXDEVS, OW_SEARCH_ROM);
            printf("Found %d devices\n", num_devs);  
            if (num_devs < 0) {
                return num_devs;
            }
            
            memcpy(this->romcode, romcode, sizeof(this->romcode));
            _numDevices = num_devs;
            return num_devs;
        }

        



};


#endif