#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <librdkafka/rdkafka.h>
#include "readcsv/readcsv.h"

#define KAFKA_BROKER "192.168.0.102:9092"  // Kafka broker address
#define MAX_AIR_ENTRIES 50000  // Maximum number of air data entries
#define MAX_WATER_ENTRIES 50000  // Maximum number of water data entries
#define MAX_EARTH_ENTRIES 50000  // Maximum number of earth data entries
#define AIR_PARTITION_COUNT 2  // Number of partitions for Air topic
#define WATER_PARTITION_COUNT 1  // Number of partitions for Water topic
#define EARTH_PARTITION_COUNT 1  // Number of partitions for Earth topic

// Struct to hold data and count for Air thread
typedef struct {
    AirData *airEntries;
    int airEntryCount;
} AirThreadInfo;

// Struct to hold data and count for Water thread
typedef struct {
    WaterData *waterEntries;
    int waterEntryCount;
} WaterThreadInfo;

// Struct to hold data and count for Earth thread
typedef struct {
    EarthData *earthEntries;
    int earthEntryCount;
} EarthThreadInfo;

// Hashing function to determine partition based on station name
int determine_partition(const char *station, int partition_count) {
    unsigned long hash = 0;
    // Compute hash value for station name
    while (*station) {
        hash = (hash * 31) + (unsigned char)(*station++);
    }
    return hash % partition_count;  // Return partition number based on hash
}

// Function to produce messages to Kafka topic
void produce_message(const char *broker_list, const char *topic_name, int partition, const char *message) {
    rd_kafka_t *producer;  // Kafka producer instance
    rd_kafka_topic_t *topic;  // Kafka topic instance
    rd_kafka_conf_t *config;  // Kafka configuration
    char error_message[512];  // Buffer to store error message
    int error_code;

    // Set Kafka configuration
    config = rd_kafka_conf_new();
    if (rd_kafka_conf_set(config, "metadata.broker.list", broker_list, error_message, sizeof(error_message)) != RD_KAFKA_CONF_OK) {
        fprintf(stderr, "Error setting brokers: %s\n", error_message);
        exit(1);
    }

    // Create Kafka producer
    producer = rd_kafka_new(RD_KAFKA_PRODUCER, config, error_message, sizeof(error_message));
    if (!producer) {
        fprintf(stderr, "Failed to create producer: %s\n", error_message);
        exit(1);
    }

    // Create Kafka topic
    topic = rd_kafka_topic_new(producer, topic_name, NULL);
    if (!topic) {
        fprintf(stderr, "Failed to create topic: %s\n", rd_kafka_err2str(rd_kafka_last_error()));
        rd_kafka_destroy(producer);
        exit(1);
    }

    // Produce message to Kafka
    error_code = rd_kafka_produce(
        topic, partition, RD_KAFKA_MSG_F_COPY,
        (void *)message, strlen(message), NULL, 0, NULL
    );

    if (error_code) {
        fprintf(stderr, "Failed to produce to topic %s: %s\n", topic_name, rd_kafka_err2str(error_code));
    } else {
        printf("Message produced to topic %s on partition %d: %s\n", topic_name, partition, message);
    }

    // Clean up Kafka resources
    rd_kafka_topic_destroy(topic);
    rd_kafka_flush(producer, 10 * 1000);  // Wait for message to be sent
    rd_kafka_destroy(producer);  // Destroy Kafka producer
}

// Function to send air data in a separate thread
void *send_air_data(void *arg) {
    AirThreadInfo *info = (AirThreadInfo *)arg;
    for (int i = 0; i < info->airEntryCount; i++) {
        char message[512];
        snprintf(message, sizeof(message),
            "Air: Time: %s, Station: %s, Temperature: %.1f, Moisture: %.1f, Light: %.1f, Total_Rainfall: %.1f, Rainfall: %.1f, Wind_direction: %.1f, PM25: %.1f, PM10: %.1f, CO: %.1f, NOX: %.1f, SO2: %.1f",
            info->airEntries[i].time,
            info->airEntries[i].station,
            (double)info->airEntries[i].temperature,
            (double)info->airEntries[i].moisture,
            (double)info->airEntries[i].light,
            (double)info->airEntries[i].total_rainfall,
            (double)info->airEntries[i].rainfall,
            (double)info->airEntries[i].wind_direction,
            (double)info->airEntries[i].pm25,
            (double)info->airEntries[i].pm10,
            (double)info->airEntries[i].co,
            (double)info->airEntries[i].nox,
            (double)info->airEntries[i].so2
        );

        int partition;
        // Determine partition based on station name
        if (strcmp(info->airEntries[i].station, "SVDT1") == 0) {
            partition = 0;
        } else if (strcmp(info->airEntries[i].station, "SVDT3") == 0) {
            partition = 1;
        } else {
            partition = determine_partition(info->airEntries[i].station, AIR_PARTITION_COUNT);
        }

        printf("Sending message to partition: %d\n", partition);  // Debugging statement
        produce_message(KAFKA_BROKER, "Air", partition, message);
    }
    return NULL;
}

// Function to send water data in a separate thread
void *send_water_data(void *arg) {
    WaterThreadInfo *info = (WaterThreadInfo *)arg;
    for (int i = 0; i < info->waterEntryCount; i++) {
        char message[256];
        snprintf(message, sizeof(message), "Time: %s, Station: %s, pH: %.1f, DO: %.1f",
                 info->waterEntries[i].time, info->waterEntries[i].station, info->waterEntries[i].ph, info->waterEntries[i].do_content);
        int partition = determine_partition(info->waterEntries[i].station, WATER_PARTITION_COUNT);
        produce_message(KAFKA_BROKER, "Water", partition, message);
    }
    return NULL;
}

// Function to send earth data in a separate thread
void *send_earth_data(void *arg) {
    EarthThreadInfo *info = (EarthThreadInfo *)arg;
    for (int i = 0; i < info->earthEntryCount; i++) {
        char message[256];
        snprintf(message, sizeof(message), "Time: %s, Station: %s, Moisture: %.1f, Temperature: %.1f",
                 info->earthEntries[i].time, info->earthEntries[i].station, info->earthEntries[i].moisture, info->earthEntries[i].temperature);
        int partition = determine_partition(info->earthEntries[i].station, EARTH_PARTITION_COUNT);
        produce_message(KAFKA_BROKER, "Earth", partition, message);
    }
    return NULL;
}

int main() {
    // Allocate memory for data
    AirData *airData = malloc(sizeof(AirData) * MAX_AIR_ENTRIES);
    WaterData *waterData = malloc(sizeof(WaterData) * MAX_WATER_ENTRIES);
    EarthData *earthData = malloc(sizeof(EarthData) * MAX_EARTH_ENTRIES);

    // Read data from CSV files
    int airEntryCount = readAirData("data/AIR2308.csv", airData, MAX_AIR_ENTRIES);
    int waterEntryCount = readWaterData("data/WATER2308.csv", waterData, MAX_WATER_ENTRIES);
    int earthEntryCount = readEarthData("data/EARTH2308.csv", earthData, MAX_EARTH_ENTRIES);

    // Create threads for sending data
    pthread_t air_thread, water_thread, earth_thread;

    AirThreadInfo airThreadData = {airData, airEntryCount};
    WaterThreadInfo waterThreadData = {waterData, waterEntryCount};
    EarthThreadInfo earthThreadData = {earthData, earthEntryCount};

    pthread_create(&air_thread, NULL, send_air_data, &airThreadData);
    pthread_create(&water_thread, NULL, send_water_data, &waterThreadData);
    pthread_create(&earth_thread, NULL, send_earth_data, &earthThreadData);

    // Wait for threads to finish execution
    pthread_join(air_thread, NULL);
    pthread_join(water_thread, NULL);
    pthread_join(earth_thread, NULL);

    // Free allocated memory
    free(airData);
    free(waterData);
    free(earthData);

    return 0;
}

