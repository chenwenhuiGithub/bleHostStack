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


### sm

support secure connection pairing method number_compare


### btsnoop

save hci data to files base on btsnoop format


### log

support debug, info, warning and error levels. currently, output the logs to terminal directly


### ringbuffer

using ringbuffer to save hci data received from controller


## TODO

1. support other cmd and event for hci
2. support permission check for gatt
3. support sm other pairing method
4. optimize send procedure from host to controller, alloc send buffer by hci level to avoid memory copy
5. support multi connections
6. support flow control
7. support 16 Bytes UUID at att/gatt protocol
8. code format based on clang-format

