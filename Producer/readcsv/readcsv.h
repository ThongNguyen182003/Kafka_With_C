#ifndef READCSV_H
#define READCSV_H

#include <stdio.h>

// Struct for AirData
typedef struct {
    char time[20];
    char station[10];
    float temperature;
    float moisture;
    float light;
    float total_rainfall;
    float rainfall;
    int wind_direction;
    float pm25;
    float pm10;
    float co;
    float nox;
    float so2;
} AirData;

/* Struct for WaterData*/
typedef struct {  
    char time[20];
    char station[10];
    float ph;
    float do_content; // DO = Dissolved Oxygen
    float temperature;
    float salinity;
}WaterData;

/* Struct for EarthData */
typedef struct {
    char time[20];
    char station[10];
    float moisture;
    float temperature;
    float salinity;
    float ph;
    int water_root;
    int water_leaf;
    float water_level;
    float voltage;
}EarthData;

void printAirData(const AirData *data);
void printWaterData(const WaterData *data);
void printEarthData(const EarthData *data);

int readAirData(const char *filename, AirData *data, int max_count);
int readWaterData(const char *filename, WaterData *data, int max_count);
int readEarthData(const char *filename, EarthData *data, int max_count);

#endif // READCSV_H

