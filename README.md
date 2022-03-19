# bleHostStack 

## introduction

this is a core ble host stack based on c++ using QT

It completes the simplest protocols of ble, such as hci based on H4, l2cap, att, gatt etc...


### hci

support acl, part cmd and event, such as reset_cmd, complete_event ...


### l2cap

support the channels of att(0x0004)


### att

support part operations, such as mtu_exchange, read_by_type, read_by_group_type, find_information ...


### gatt 

give one att_db demo, client can discover services, characteristics, descriptors and values without permission check


### btsnoop

save hci data to files base on btsnoop format


### log

support debug, info, warning and error levels. currently, output the logs to terminal directly


### ringbuffer

using ringbuffer to save hci data received from controller


## TODO

1. support other cmd and event for hci
2. support ble-signal(0x0005) channel for l2cap
3. support other operations for att
4. support permission check for gatt
5. support sm protocol
6. save log to files instead of terminal
7. code format based on clang-format
8. optimize send procedure from host to controller, alloc send buffer by hci level to avoid memory copy
9. move private functions and declarations from .h to .cpp files, add static and "__" as prefix
10. support multi connections
11. support flow control 
12. fill send data as much as possible at att/gatt level with att_mtu check
13. move gatt demo code to new file gatt_service_demo.cpp
14. support 16 Bytes UUID at att/gatt protocol
