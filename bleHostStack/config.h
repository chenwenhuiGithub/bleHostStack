#ifndef CONFIG_H
#define CONFIG_H

// config for ringbuffer
#define RINGBUFFER_SIZE                                                 1024

// config for hci
#define HCI_ADV_CHANNEL_MAP_37                                          0x01
#define HCI_ADV_CHANNEL_MAP_38                                          0x02
#define HCI_ADV_CHANNEL_MAP_39                                          0x04
#define HCI_ADV_CHANNEL_MAP                                             HCI_ADV_CHANNEL_MAP_37 | HCI_ADV_CHANNEL_MAP_38 | HCI_ADV_CHANNEL_MAP_39

#define HCI_ADV_TYPE_ADV_IND                                            0x00
#define HCI_ADV_TYPE_ADV_DIRECT_IND_HIGH_DUTY                           0x01
#define HCI_ADV_TYPE_ADV_SCAN_IND                                       0x02
#define HCI_ADV_TYPE_ADV_NONCONN_IND                                    0x03
#define HCI_ADV_TYPE_ADV_DIRECT_IND_LOW_DUTY                            0x04
#define HCI_ADV_TYPE                                                    HCI_ADV_TYPE_ADV_IND

#define HCI_ADV_FILTER_POLICY_SCAN_ALL_CONN_ALL                         0x00
#define HCI_ADV_FILTER_POLICY_SCAN_FILTER_CONN_ALL                      0x01
#define HCI_ADV_FILTER_POLICY_SCAN_ALL_CONN_FILETER                     0x02
#define HCI_ADV_FILTER_POLICY_SCAN_FILTER_CONN_FILETER                  0x03
#define HCI_ADV_FILTER_POLICY                                           HCI_ADV_FILTER_POLICY_SCAN_ALL_CONN_ALL

#define HCI_ADV_DATA_LENGTH                                             13 // flags:BR/EDR not supported, le general discoverable mode // device name:ble_demo
#define HCI_ADV_DATA                                                    {0x02, 0x01, 0x06, \
                                                                         0x09, 0x09, 0x62, 0x6c, 0x65, 0x5f, 0x64, 0x65, 0x6d, 0x6f, \
                                                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

#define HCI_ADV_INTERVAL_MIN                                            0x0800 // 1.28s(N*0.625ms)
#define HCI_ADV_INTERVAL_MAX                                            0x0800 // 1.28s(N*0.625ms)


#define HCI_SCAN_TYPE_PASSIVE                                           0x00
#define HCI_SCAN_TYPE_ACTIVE                                            0x01
#define HCI_SCAN_TYPE                                                   HCI_SCAN_TYPE_ACTIVE

#define HCI_SCAN_INTERVAL                                               0x0010 // 10ms(N*0.625ms)
#define HCI_SCAN_WINDOW                                                 0x0010 // 10ms(N*0.625ms)

#define HCI_SCAN_FILTER_POLICY_BASIC_UNFILTERED                         0x00
#define HCI_SCAN_FILTER_POLICY_BASIC_FILTERED                           0x01
#define HCI_SCAN_FILTER_POLICY_EXTENDED_UNFILTERED                      0x02
#define HCI_SCAN_FILTER_POLICY_EXTENDED_FILTERED                        0x03
#define HCI_SCAN_FILTER_POLICY                                          HCI_SCAN_FILTER_POLICY_BASIC_UNFILTERED


#define HCI_CONN_INTERVAL_MIN                                           0x0018 // 30ms(N*1.25ms)
#define HCI_CONN_INTERVAL_MAX                                           0x0018 // 30ms(N*1.25ms)
#define HCI_CONN_MAX_LATENCY                                            10
#define HCI_CONN_TIMEOUT                                                0x0080 // 1.28s(N*10ms)
#define HCI_CONN_MIN_CE_LENGTH                                          0x0028 // 25ms(N*0.625ms)
#define HCI_CONN_MAX_CE_LENGTH                                          0x0028 // 25ms(N*0.625ms)


#define HCI_CLASS_OF_DEVICE                                             {0x0c, 0x02, 0x20} // Smartphone: Audio
#define HCI_EVENT_MASK                                                  {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f}
#define HCI_LE_EVENT_MASK                                               {0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0x00}
#define HCI_ADDR_RANDOM_IRK                                             {0xcd, 0x79, 0x4e, 0x24, 0x8b, 0x47, 0x87, 0x75, 0xac, 0x64, 0x60, 0x2b, 0x86, 0x5c, 0x3f, 0x34}

// config for att
#define ATT_MTU_DEFAULT                                                 23

// config for sm
#define SM_IOCAP_DISPLAY_ONLY                                           0x00 // PASSKEY_I_INPUT_R_DISPLAY
#define SM_IOCAP_DISPLAY_YESNO                                          0x01 // legacy:PASSKEY_I_INPUT_R_DISPLAY, secure:NUMERIC_COMPARISON
#define SM_IOCAP_KEYBORAD_ONLY                                          0x02 // PASSKEY_I_DISPLAY_R_INPUT
#define SM_IOCAP_NO_INPUT_NO_OUTPUT                                     0x03 // JUST_WORKS
#define SM_IOCAP_KEYBOARD_DISPLAY                                       0x04 // legacy:PASSKEY_I_DISPLAY_R_INPUT, secure:NUMERIC_COMPARISON
#define SM_IOCAP                                                        SM_IOCAP_KEYBOARD_DISPLAY   

#define SM_OOB_DATA_FLAG_NOT_PRESENT                                    0x00
#define SM_OOB_DATA_FLAG_PRESENT                                        0x01
#define SM_OOB_DATA_FLAG                                                SM_OOB_DATA_FLAG_NOT_PRESENT

#define SM_AUTH_BONDING                                                 0x01
#define SM_AUTH_MITM                                                    0x04
#define SM_AUTH_SECURE_CONNECTION                                       0x08
#define SM_AUTH_KEYPRESS                                                0x10
#define SM_AUTH                                                         SM_AUTH_BONDING | SM_AUTH_MITM | SM_AUTH_SECURE_CONNECTION // secure connection pairing

#define SM_MIN_ENCRYPT_KEY_SIZE                                         7
#define SM_MAX_ENCRYPT_KEY_SIZE                                         16
#define SM_ENCRYPT_KEY_SIZE                                             16

// Initiator Key Distribution in Pairing Request:  defines the keys shall be distributed by the initiator to the responder
// Responder Key Distribution in Pairing Request:  defines the keys shall be distributed by the responder to the initiator
// Initiator Key Distribution in Pairing Response: defines the keys shall be distributed by the initiator to the responder
// Responder Key Distribution in Pairing Response: defines the keys shall be distributed by the responder to the initiator
#define SM_KEY_DISTRIBUTION_ENC                                         0x01 // LTK, EDIV, RAND
#define SM_KEY_DISTRIBUTION_ID                                          0x02 // IRK, ADDR_TPE, ADDR
#define SM_KEY_DISTRIBUTION_SIGN                                        0x04 // CSRK
#define SM_KEY_DISTRIBUTION_LINK                                        0x08
#define SM_INITIATOR_KEY_DISTRIBUTION                                   SM_KEY_DISTRIBUTION_ID | SM_KEY_DISTRIBUTION_SIGN
#define SM_RESPONDER_KEY_DISTRIBUTION                                   SM_KEY_DISTRIBUTION_ID | SM_KEY_DISTRIBUTION_SIGN

#define SM_DEVICE_DB_FILE_NAME                                          "device_db.dat"

#endif // CONFIG_H
