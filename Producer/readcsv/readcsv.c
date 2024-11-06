#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "readcsv.h"

// Function to print air data
void printAirData(const AirData *airData) {
    printf("Time: %s, Station: %s, Temperature: %.1f, Moisture: %.1f, Light: %.1f, "
           "Total Rainfall: %.1f, Rainfall: %.1f, Wind Direction: %d, "
           "PM2.5: %.1f, PM10: %.1f, CO: %.1f, NOx: %.1f, SO2: %.1f\n",
           airData->time, airData->station, airData->temperature, airData->moisture, airData->light,
           airData->total_rainfall, airData->rainfall, airData->wind_direction,
           airData->pm25, airData->pm10, airData->co, airData->nox, airData->so2);
}

// Function to print water data
void printWaterData(const WaterData *waterData) {
    printf("Time: %s, Station: %s, pH: %.1f, DO: %.1f, Temperature: %.1f, Salinity: %.1f\n",
           waterData->time, waterData->station, waterData->ph, waterData->do_content, waterData->temperature, waterData->salinity);
}

// Function to print earth data
void printEarthData(const EarthData *earthData) {
    printf("Time: %s, Station: %s, Moisture: %.1f, Temperature: %.1f, Salinity: %.1f, "
           "pH: %.1f, Water Root: %d, Water Leaf: %d, Water Level: %.1f, Voltage: %.1f\n",
           earthData->time, earthData->station, earthData->moisture, earthData->temperature,
           earthData->salinity, earthData->ph, earthData->water_root, earthData->water_leaf,
           earthData->water_level, earthData->voltage);
}

// Function to read air data from CSV file
int readAirData(const char *filePath, AirData *airData, int maxRecords) {
    FILE *file = fopen(filePath, "r");
    if (!file) {
        perror("Error opening file");
        return -1;
    }

    char line[256];
    fgets(line, sizeof(line), file); // Skip header line

    int recordCount = 0;
    while (fgets(line, sizeof(line), file) && recordCount < maxRecords) {
        int itemsRead = sscanf(line, "%19[^,],%9[^,],%f,%f,%f,%f,%f,%d,%f,%f,%f,%f,%f",
                               airData[recordCount].time, airData[recordCount].station, &airData[recordCount].temperature,
                               &airData[recordCount].moisture, &airData[recordCount].light,
                               &airData[recordCount].total_rainfall, &airData[recordCount].rainfall,
                               &airData[recordCount].wind_direction, &airData[recordCount].pm25,
                               &airData[recordCount].pm10, &airData[recordCount].co, &airData[recordCount].nox, &airData[recordCount].so2);

        if (itemsRead != 13) { // Check if all fields are read correctly
            fprintf(stderr, "Warning: Invalid data format on line: %s", line);
            continue; // Skip this line
        }

        recordCount++;
    }

    fclose(file);
    return recordCount; // Return the number of records read
}

// Function to read water data from CSV file
int readWaterData(const char *filePath, WaterData *waterData, int maxRecords) {
    FILE *file = fopen(filePath, "r");
    if (!file) {
        perror("Error opening file");
        return -1;
    }

    char line[256];
    fgets(line, sizeof(line), file); // Skip header line

    int recordCount = 0;
    while (fgets(line, sizeof(line), file) && recordCount < maxRecords) {
        int itemsRead = sscanf(line, "%19[^,],%9[^,],%f,%f,%f,%f",
                               waterData[recordCount].time, waterData[recordCount].station,
                               &waterData[recordCount].ph, &waterData[recordCount].do_content,
                               &waterData[recordCount].temperature, &waterData[recordCount].salinity);

        if (itemsRead != 6) { // Check if all fields are read correctly
            fprintf(stderr, "Warning: Invalid data format on line: %s", line);
            continue; // Skip this line
        }

        recordCount++;
    }

    fclose(file);
    return recordCount; // Return the number of records read
}

// Function to read earth data from CSV file
int readEarthData(const char *filePath, EarthData *earthData, int maxRecords) {
    FILE *file = fopen(filePath, "r");
    if (!file) {
        perror("Error opening file");
        return -1;
    }

    char line[256];
    fgets(line, sizeof(line), file); // Skip header line

    int recordCount = 0;
    while (fgets(line, sizeof(line), file) && recordCount < maxRecords) {
        int itemsRead = sscanf(line, "%19[^,],%9[^,],%f,%f,%f,%f,%d,%d,%f,%f",
                               earthData[recordCount].time, earthData[recordCount].station,
                               &earthData[recordCount].moisture, &earthData[recordCount].temperature,
                               &earthData[recordCount].salinity, &earthData[recordCount].ph,
                               &earthData[recordCount].water_root, &earthData[recordCount].water_leaf,
                               &earthData[recordCount].water_level, &earthData[recordCount].voltage);

        if (itemsRead != 10) { // Check if all fields are read correctly
            fprintf(stderr, "Warning: Invalid data format on line: %s", line);
            continue; // Skip this line
        }

        recordCount++;
    }

    fclose(file);
    return recordCount; // Return the number of records read
}

