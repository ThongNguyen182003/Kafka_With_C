# Giới thiệu

## Thư viện **librdkafka**
**librdkafka** là một thư viện mã nguồn mở, hiệu suất cao, được phát triển bằng ngôn ngữ C để làm client kết nối với Apache Kafka. Nó được thiết kế để cung cấp kết nối mạnh mẽ và hiệu quả tới các cluster Kafka, cho phép người dùng tương tác với các broker Kafka trong nhiều môi trường và ngôn ngữ khác nhau.

Các tính năng chính của **librdkafka** bao gồm:

1. **Hỗ trợ Producer và Consumer**: Thư viện này hỗ trợ cả việc sản xuất (produce) và tiêu thụ (consume) tin nhắn, giúp người dùng có thể tương tác đầy đủ với các topic của Kafka.
2. **Hiệu suất cao và độ trễ thấp**: Thư viện được tối ưu hóa cho hiệu suất cao và độ trễ thấp, phù hợp với các ứng dụng truyền tải dữ liệu thời gian thực.
3. **An toàn với nhiều luồng**: Hỗ trợ các thao tác an toàn với nhiều luồng, cho phép thực hiện các hoạt động đồng thời với nhiều producer và consumer.
4. **Tuân thủ giao thức Kafka**: librdkafka triển khai giao thức Kafka một cách đầy đủ, đảm bảo tính tương thích với các thành phần khác của hệ sinh thái Kafka.
5. **Hỗ trợ nén tin nhắn**: Thư viện hỗ trợ nén tin nhắn (gzip, snappy, vv), giúp giảm băng thông và tối ưu hóa không gian lưu trữ.
6. **Đảm bảo độ tin cậy**: Thư viện đảm bảo tin cậy trong việc gửi tin nhắn với các tính năng như tự động thử lại, xác nhận (acknowledgements), và các cam kết về độ tin cậy trong việc truyền tải tin nhắn.
7. **Tích hợp với các ngôn ngữ khác**: Mặc dù được viết bằng C, librdkafka cũng cung cấp các bindings cho các ngôn ngữ khác như Python, Java, và Go, cho phép các nhà phát triển sử dụng Kafka trong ngôn ngữ mình ưa thích.

Thư viện **librdkafka** là công cụ quan trọng cho các ứng dụng cần tương tác với hệ thống Kafka, từ việc xử lý dữ liệu thời gian thực cho đến các ứng dụng yêu cầu hiệu suất và độ tin cậy cao.

## Sử dụng thread để gửi đa luồng

```
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
```
## Cách sử dụng

1. Cài đặt thự viện **librdkafka**.

2. **Trên máy 1**. Chỉnh sửa broker trong file **producer.c** 

```
#define KAFKA_BROKER "192.168.0.102:9092"  // Kafka broker address
```

Sau đó:
```
gcc producer.c readcsv/readcsv.c -o producer -lrdkafka
```
3. **Trên máy 2**. Chỉnh sửa broker trong file **consumer.c**

```
#define KAFKA_BROKER "192.168.0.102:9092"  // Kafka broker address
```
Sau đó: 
```
gcc consumer.c -o consumer -lrdkafka
```

4. Nhận data từ topic và partition cụ thể
```
./consumer <topic> <partition> // Example: ./consumer Air 1
```

5. Gửi data

```
./producer
```