#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <librdkafka/rdkafka.h>

#define KAFKA_BROKER "192.168.0.102:9092"

// Function to consume messages from the Kafka topic in a given partition
void process_messages(rd_kafka_t *kafka_instance, rd_kafka_topic_t *topic_handle, int part_id) {
    while (1) {
        rd_kafka_message_t *msg;
        msg = rd_kafka_consume(topic_handle, part_id, 0); // No timeout for consuming

        if (msg) {
            // Check if there was an error in the message
            if (msg->err) {
                fprintf(stderr, "Message consumption error: %s\n", rd_kafka_err2str(msg->err));
            } else {
                // Print topic name, partition, and message content
                printf("Topic: %s | Partition: %d | Message: %.*s\n",
                       rd_kafka_topic_name(topic_handle),
                       msg->partition,
                       (int)msg->len,
                       (char *)msg->payload);
            }
            rd_kafka_message_destroy(msg); // Free the message after processing
        }
        usleep(100); // Optional: add a small delay to avoid busy-waiting
    }
}

int main(int argc, char *argv[]) {
    // Ensure proper usage
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <topic_name> <partition_id>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *topic_name = argv[1];
    int part_id = atoi(argv[2]); // Convert partition argument to integer

    // Validate the partition ID
    if (part_id < 0) {
        fprintf(stderr, "Invalid partition ID: %d\n", part_id);
        return EXIT_FAILURE;
    }

    // Define Kafka configuration, consumer instance, and topic handle
    rd_kafka_conf_t *kafka_conf;
    rd_kafka_t *kafka_instance;
    rd_kafka_topic_t *topic_handle;
    char error_message[512]; // Buffer to hold error messages

    // Set up Kafka configuration
    kafka_conf = rd_kafka_conf_new();
    if (rd_kafka_conf_set(kafka_conf, "metadata.broker.list", KAFKA_BROKER, error_message, sizeof(error_message)) != RD_KAFKA_CONF_OK) {
        fprintf(stderr, "Failed to set broker configuration: %s\n", error_message);
        return EXIT_FAILURE;
    }

    // Create a new Kafka consumer instance
    kafka_instance = rd_kafka_new(RD_KAFKA_CONSUMER, kafka_conf, error_message, sizeof(error_message));
    if (!kafka_instance) {
        fprintf(stderr, "Could not create Kafka consumer instance: %s\n", error_message);
        return EXIT_FAILURE;
    }

    // Create a handle to the specified Kafka topic
    topic_handle = rd_kafka_topic_new(kafka_instance, topic_name, NULL);
    if (!topic_handle) {
        fprintf(stderr, "Could not create topic handle: %s\n", rd_kafka_err2str(rd_kafka_last_error()));
        rd_kafka_destroy(kafka_instance);
        return EXIT_FAILURE;
    }

    // Start consuming messages from the specified partition, beginning at the latest offset
    if (rd_kafka_consume_start(topic_handle, part_id, RD_KAFKA_OFFSET_END) != 0) {
        fprintf(stderr, "Could not start consuming on partition %d: %s\n", part_id, rd_kafka_err2str(rd_kafka_last_error()));
        rd_kafka_topic_destroy(topic_handle);
        rd_kafka_destroy(kafka_instance);
        return EXIT_FAILURE;
    }

    // Process messages from the topic in the specified partition
    process_messages(kafka_instance, topic_handle, part_id);

    // Stop consuming, clean up topic handle and consumer instance
    rd_kafka_consume_stop(topic_handle, part_id);
    rd_kafka_topic_destroy(topic_handle);
    rd_kafka_destroy(kafka_instance);

    return EXIT_SUCCESS;
}

