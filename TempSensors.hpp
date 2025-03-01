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
        int16_t _temps[TEMPSENSOR_MAXDEVS];
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

        /// @brief request temperature read from all sensors
        /// timing: {750,375,188,94}ms for a {12,11,10,9}-bit conversion
        /// @param wait 
        void RequestConversion(bool wait = true) 
        {
            unsigned int stm = time_us_32();
            ow_reset (&_ow);
            ow_send (&_ow, OW_SKIP_ROM);
            ow_send (&_ow, DS18B20_CONVERT_T);
            
            // wait for the conversions to finish
            if (wait) {
                WaitForConversion();
            }
            unsigned int stm2 = time_us_32();
            printf("conversion time %d us\n", stm2 - stm);
        }

        /// @brief 
        /// @return -1 if timed out, 0 otherwise 
        int WaitForConversion() {
            int cnt = 0;
            while (ow_read(&_ow) == 0)
            {
                sleep_ms(2);
                if (++cnt > 2000) {
                    return -1;
                }
            }
            return 0;
        }

        int ReadResults(bool waitForConversion = true) {
            if (waitForConversion) {
                if (WaitForConversion() < 0) {
                    return -1; //timeout
                }
            }
            for (int i = 0; i < _numDevices; i += 1) {
                ow_reset (&_ow);
                ow_send (&_ow, OW_MATCH_ROM);
                for (int b = 0; b < 64; b += 8) {
                    ow_send (&_ow, romcode[i] >> b);
                }
                ow_send (&_ow, DS18B20_READ_SCRATCHPAD);
                int16_t temp = 0;
                temp = ow_read (&_ow) | (ow_read (&_ow) << 8);
                printf ("\t%d: %f", i, temp / 16.0);
                _temps[i] = temp;
            }
            return _numDevices;
        }

};


#endif