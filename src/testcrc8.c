#include <stdio.h>
#include <stdint.h>
#include <time.h>

#define DATA_SIZE  65536
#define POLYNOMIAL 0x31

static void generate_lookup(uint8_t *_lookup, uint8_t _gen) {
    uint32_t i;
    uint8_t b;
    uint8_t a;
    for (i = 0; i < 256; i++) {
        b = i;
        for (a = 0; a < 8; a++) {
            if ((b & 0x80) != 0) {
                b <<= 1;
                b ^= _gen;
            } else b <<= 1;
        }
        _lookup[i] = b;
    }
}

static uint8_t crc8(uint8_t _dataIn, uint8_t _dataLast) {
    uint8_t a;
    for(a=0; a<8; a++) {
        if ( ((_dataIn ^ _dataLast) & 0x80) != 0 ) {
            _dataLast <<= 1;
            _dataLast ^= POLYNOMIAL;
        } else _dataLast <<= 1;
        _dataIn <<= 1;
    }
    return _dataLast;
}

int main(int argc, char *argv[]) {
    uint8_t lookup[256];
    uint8_t crc;
    uint8_t data[DATA_SIZE];
    uint32_t i;
    uint8_t s;
    uint8_t p;
    double elapsed;
    struct timespec start, stop;

    if (argc<2) {
        p=POLYNOMIAL;
    } else {
        p=(uint8_t)strtol(argv[1], NULL, 16);
    }

    srand(time(0));
    for (i=0; i<DATA_SIZE; i++) data[i]=rand()%256;

    clock_gettime( CLOCK_REALTIME, &start);
    generate_lookup(lookup,p);
    clock_gettime( CLOCK_REALTIME, &stop);
    elapsed = ( stop.tv_sec - start.tv_sec ) + ( stop.tv_nsec - start.tv_nsec ) / 1000000000.0;

    printf("/* polynomial 0x%02X */\n",p);
    printf("static const uint8_t lookup[] = {\n\t");
    s=0;
    for (i=0; i<256; i++) {
        printf("0x%02X",lookup[i]);
        if (i==255) {
            printf("\n");
            break;
        }
        s++;
        if (s==16) {
            printf(",\n\t");
            s=0;
        } else printf(", ");
    }
    printf("};\n");

    printf("lookup generated at %lfs\n",elapsed);

    crc=0;
    clock_gettime( CLOCK_REALTIME, &start);
    ////////////////////////////////////////////////
    for (i=0; i<DATA_SIZE; i++) crc=crc8(data[i],crc);
    ////////////////////////////////////////////////
    clock_gettime( CLOCK_REALTIME, &stop);
    elapsed = ( stop.tv_sec - start.tv_sec ) + ( stop.tv_nsec - start.tv_nsec ) / 1000000000.0;
    printf("calculated crc8 [0x%02X] by bits at %lfs\n",crc,elapsed);

    crc=0;
    clock_gettime( CLOCK_REALTIME, &start);
    ////////////////////////////////////////////////
    for (i=0; i<DATA_SIZE; i++) crc=lookup[data[i]^crc];
    ////////////////////////////////////////////////
    clock_gettime( CLOCK_REALTIME, &stop);
    elapsed = ( stop.tv_sec - start.tv_sec ) + ( stop.tv_nsec - start.tv_nsec ) / 1000000000.0;
    printf("calculated crc8 [0x%02X] by lookup at %lfs\n",crc,elapsed);

    return 0;
}
