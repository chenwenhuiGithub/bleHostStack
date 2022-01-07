#include "qhci.h"

/* Opcode Group Field (OGF) values */
#define HCI_OGF_LINK_CONTROL					                0x01   /* Link Control Commands */
#define HCI_OGF_LINK_POLICY						                0x02   /* Link Policy Commands */
#define HCI_OGF_CONTROLLER_BASEBAND					            0x03   /* Controller & Baseband Commands */
#define HCI_OGF_INFORMATIONAL_PARAM						        0x04   /* Informational Parameters */
#define HCI_OGF_STATUS_PARAM					                0x05   /* Status Parameters */
#define HCI_OGF_TESTING							                0x06   /* Testing Commands */
#define HCI_OGF_LE_CONTROLLER					                0x08   /* LE Controller Commands */
#define HCI_OGF_VENDOR							                0x3F   /* vendor Commands */

/* Opcode Command Field (OCF) values */
/* Link Control Commands */
#define HCI_OCF_INQUIRY							                0x01
#define HCI_OCF_INQUIRY_CANCEL					                0x02
#define HCI_OCF_PERIODIC_INQUIRY				                0x03
#define HCI_OCF_EXIT_PERIODIC_INQUIRY			                0x04
#define HCI_OCF_CREATE_CONNECT					                0x05
#define HCI_OCF_DISCONNECT						                0x06
#define HCI_OCF_CREATE_CONNECT_CANCEL			                0x08
#define HCI_OCF_ACCEPT_CONNECT_REQUEST			                0x09
#define HCI_OCF_REJECT_CONNECT_REQUEST			                0x0A
#define HCI_OCF_LINK_KEY_REQUEST_REPLY			                0x0B
#define HCI_OCF_LINK_KEY_REQUEST_NEG_REPLY		                0x0C
#define HCI_OCF_PIN_CODE_REQUEST_REPLY			                0x0D
#define HCI_OCF_PIN_CODE_REQUEST_NEG_REPLY		                0x0E
#define HCI_OCF_CHANGE_CONNECT_PACKET_TYPE		                0x0F
#define HCI_OCF_AUTH_REQUEST					                0x11
#define HCI_OCF_SET_CONNECT_ENCRYPTION			                0x13
#define HCI_OCF_CHANGE_CONNECT_LINK_KEY			                0x15
#define HCI_OCF_LINK_KEY_SELECTION				                0x17
#define HCI_OCF_REMOTE_NAME_REQUEST				                0x19
#define HCI_OCF_REMOTE_NAME_REQUEST_CANCEL		                0x1A
#define HCI_OCF_READ_REMOTE_SUPPORT_FEATURES	                0x1B
#define HCI_OCF_READ_REMOTE_EXTEND_FEATURES		                0x1C
#define HCI_OCF_READ_REMOTE_VERSION_INFO		                0x1D
#define HCI_OCF_READ_CLOCK_OFFSET				                0x1F
#define HCI_OCF_READ_LMP_HANDLE					                0x20
#define HCI_OCF_SETUP_SCO_CONNECT_REQUEST		                0x28
#define HCI_OCF_ACCEP_SCO_CONNECT_REQUEST		                0x29
#define HCI_OCF_REJECT_SCO_CONNECT_REQUEST		                0x2A
#define HCI_OCF_IO_CAP_REQUEST_REPLY			                0x2B
#define HCI_OCF_USER_CONFIRM_REQUEST_REPLY		                0x2C
#define HCI_OCF_USER_CONFIRM_REQUEST_NEG_REPLY	                0x2D
#define HCI_OCF_USER_PASSKEY_REQUEST_REPLY		                0x2E
#define HCI_OCF_USER_PASSKEY_REQUEST_NEG_REPLY	                0x2F
#define HCI_OCF_REMOTE_OOB_REQUEST_REPLY		                0x30
#define HCI_OCF_REMOTE_OOB_REQUEST_NEG_REPLY	                0x33
#define HCI_OCF_IO_CAP_REQUEST_NEG_REPLY		                0x34

/* Link Policy Commands */
#define HCI_OCF_HOLD_MODE						                0x01
#define HCI_OCF_SNIFF_MODE						                0x03
#define HCI_OCF_EXIT_SNIFF_MODE					                0x04
#define HCI_OCF_QOS_SETUP						                0x07
#define HCI_OCF_ROLE_DISCOVERY					                0x09
#define HCI_OCF_SWITCH_ROLE						                0x0B
#define HCI_OCF_READ_LINK_POLICY				                0x0C
#define HCI_OCF_WRITE_LINK_POLICY				                0x0D
#define HCI_OCF_READ_DEFAULT_LINK_POLICY		                0x0E
#define HCI_OCF_WRITE_DEFAULT_LINK_POLICY		                0x0F
#define HCI_OCF_FLOW_SPEC						                0x10
#define HCI_OCF_SNIFF_SUBRATING					                0x11

/* Controller and Baseband Commands */
#define HCI_OCF_SET_EVENT_MASK					                0x01
#define HCI_OCF_RESET							                0x03
#define HCI_OCF_SET_EVENT_FILTER				                0x05
#define HCI_OCF_FLUSH							                0x08
#define HCI_OCF_READ_PIN_TYPE					                0x09
#define HCI_OCF_WRITE_PIN_TYPE					                0x0A
#define HCI_OCF_READ_STORED_LINK_KEY			                0x0D
#define HCI_OCF_WRITE_STORED_LINK_KEY			                0x11
#define HCI_OCF_DELETE_STORED_LINK_KEY			                0x12
#define HCI_OCF_WRITE_LOCAL_NAME				                0x13
#define HCI_OCF_READ_LOCAL_NAME					                0x14
#define HCI_OCF_READ_CONNECT_ACCEPT_TIMEOUT		                0x15
#define HCI_OCF_WRITE_CONNECT_ACCEPT_TIMEOUT	                0x16
#define HCI_OCF_READ_PAGE_TIMEOUT				                0x17
#define HCI_OCF_WRITE_PAGE_TIMEOUT				                0x18
#define HCI_OCF_READ_SCAN_ENABLE				                0x19
#define HCI_OCF_WRITE_SCAN_ENABLE				                0x1A
#define HCI_OCF_READ_PAGE_SCAN_ACTIVITY			                0x1B
#define HCI_OCF_WRITE_PAGE_SCAN_ACTIVITY		                0x1C
#define HCI_OCF_READ_INQUIRY_SCAN_ACTIVITY		                0x1D
#define HCI_OCF_WRITE_INQUIRY_SCAN_ACTIVITY		                0x1E
#define HCI_OCF_READ_AUTH_ENABLE				                0x1F
#define HCI_OCF_WRITE_AUTH_ENABLE				                0x20
#define HCI_OCF_READ_CLASS_OF_DEVICE			                0x23
#define HCI_OCF_WRITE_CLASS_OF_DEVICE			                0x24
#define HCI_OCF_READ_VOICE_SETTING				                0x25
#define HCI_OCF_WRITE_VOICE_SETTING				                0x26
#define HCI_OCF_READ_AUTO_FLUSH_TIMEOUT			                0x27
#define HCI_OCF_WRITE_AUTO_FLUSH_TIMEOUT		                0x28
#define HCI_OCF_READ_NUM_BROADCAST_RETRANS		                0x29
#define HCI_OCF_WRITE_NUM_BROADCAST_RETRANS		                0x2A
#define HCI_OCF_READ_HOLD_MODE_ACTIVITY			                0x2B
#define HCI_OCF_WRITE_HOLD_MODE_ACTIVITY		                0x2C
#define HCI_OCF_READ_TRANSMIT_POWER_LEVEL		                0x2D
#define HCI_OCF_READ_SCO_FLOW_CONTROL_ENABLE	                0x2E
#define HCI_OCF_WRITE_SCO_FLOW_CONTROL_ENABLE	                0x2F
#define HCI_OCF_SET_C2H_FLOW_CONTROL			                0x31
#define HCI_OCF_HOST_BUFF_SIZE					                0x33
#define HCI_OCF_HOST_NUM_OF_COMPLETED_PACKET	                0x35
#define HCI_OCF_READ_LINK_SUPERVISION_TIMEOUT	                0x36
#define HCI_OCF_WRITE_LINK_SUPERVISION_TIMEOUT	                0x37
#define HCI_OCF_READ_NUM_OF_SUPPORTED_IAC		                0x38
#define HCI_OCF_READ_CURRENT_IAC_LAP			                0x39
#define HCI_OCF_WRITE_CURRENT_IAC_LAP			                0x3A
#define HCI_OCF_SET_AFH_HOST_CHANNEL_CLASSIFICATION				0x3F
#define HCI_OCF_READ_INQUIRY_SCAN_TYPE			                0x42
#define HCI_OCF_WRITE_INQUIRY_SCAN_TYPE			                0x43
#define HCI_OCF_READ_INQUIRY_MODE				                0x44
#define HCI_OCF_WRITE_INQUIRY_MODE				                0x45
#define HCI_OCF_READ_PAGE_SCAN_TYPE				                0x46
#define HCI_OCF_WRITE_PAGE_SCAN_TYPE			                0x47
#define HCI_OCF_READ_EXTENDED_INQUIRY_RESPONSE                  0x51
#define HCI_OCF_WRITE_EXTENDED_INQUIRY_RESPONSE                 0x52
#define HCI_OCF_WRITE_SIMPLE_PAIRING_MODE		                0x56
#define HCI_OCF_READ_LE_HOST_SUPPORT			                0x6C
#define HCI_OCF_WRITE_LE_HOST_SUPPORT			                0x6D

/* Informational Parameters */
#define HCI_OCF_READ_LOCAL_VERSION_INFO			                0x01
#define HCI_OCF_READ_LOCAL_SUPPORTED_COMMANDS		            0x02
#define HCI_OCF_RAED_LOCAL_SUPPORTED_FEATURE	                0x03
#define HCI_OCF_READ_LOCAL_EXTENDED_FEATURE		                0x04
#define HCI_OCF_READ_BUFFER_SIZE				                0x05
#define HCI_OCF_READ_BD_ADDR					                0x09
#define HCI_OCF_READ_DATA_BLOCK_SIZE			                0x0A

/* Status Parameters */
#define HCI_OCF_READ_FAILED_CONTACT_COUNTER		                0x01
#define HCI_OCF_RESET_FAILED_CONTACT_COUNTER	                0x02
#define HCI_OCF_READ_LINK_QUALITY				                0x03
#define HCI_OCF_READ_RSSI						                0x05
#define HCI_OCF_READ_AFH_CHANNEL_MAP			                0x06
#define HCI_OCF_READ_CLOCK						                0x07
#define HCI_OCF_READ_ENCRYPTION_KEY_SIZE		                0x08

/* Testing Commands */
#define HCI_OCF_READ_LOOPBACK_MODE				                0x01
#define HCI_OCF_WRITE_LOOPBACK_MODE				                0x02
#define HCI_OCF_ENABLE_DEVICE_UNDER_TEST_MODE	                0x03
#define HCI_OCF_WRITE_SIMPLE_PAIRING_DEBUG_MODE                 0x04
#define HCI_OCF_WRITE_SECURE_CONNECT_TEST_MODE	                0x0A

/* LE Commands */
#define HCI_OCF_LE_SET_EVENT_MASK				                0x01
#define HCI_OCF_LE_READ_BUFFER_SIZE				                0x02
#define HCI_OCF_LE_READ_LOCAL_SUPPORTED_FEATURE                 0x03
#define HCI_OCF_LE_SET_RANDOM_ADDRESS			                0x05
#define HCI_OCF_LE_SET_ADV_PARAM				                0x06
#define HCI_OCF_LE_SET_ADV_DATA					                0x08
#define HCI_OCF_LE_SET_ADV_ENABLE				                0x0A
#define HCI_OCF_LE_SET_SCAN_PARAM				                0x0B
#define HCI_OCF_LE_SET_SCAN_ENABLE				                0x0C
#define HCI_OCF_LE_CREATE_CONNECT				                0x0d

/* event codes */
#define HCI_EVENT_INQUIRY_COMPLETE								0x01
#define HCI_EVENT_INQUIRY_RESULT								0x02
#define HCI_EVENT_CONNECTION_COMPLETE							0x03
#define HCI_EVENT_CONNECTION_REQUEST							0x04
#define HCI_EVENT_DISCONNECTION_COMPLETE						0x05
#define HCI_EVENT_AUTH_COMPLETE									0x06
#define HCI_EVENT_REMOTE_NAME_REQUEST_COMPLETE					0x07
#define HCI_EVENT_ENCRYPTION_CHANGE								0x08
#define HCI_EVENT_CHANGE_CONNECTTION_LINK_KEY_COMPLETE			0x09
#define HCI_EVENT_LINK_KEY_TYPE_CHANGED							0x0A
#define HCI_EVENT_READ_REMOTE_SUPPORTED_FEATURES_COMPLETE		0x0B
#define HCI_EVENT_READ_REMOTE_VERSION_INFORMATION_COMPLETE		0x0C
#define HCI_EVENT_QOS_SETUP_COMPLETE							0x0D
#define HCI_EVENT_COMMAND_COMPLETE								0x0E
#define HCI_EVENT_COMMAND_STATUS								0x0F
#define HCI_EVENT_HARDWARE_ERROR								0x10
#define HCI_EVENT_FLUSH_OCCURRED								0x11
#define HCI_EVENT_ROLE_CHANGE									0x12
#define HCI_EVENT_NNMBER_OF_COMPLETED_PACKETS					0x13
#define HCI_EVENT_MODE_CHANGE									0x14
#define HCI_EVENT_RETURN_LINK_KEYS								0x15
#define HCI_EVENT_PIN_CODE_REQUEST								0x16
#define HCI_EVENT_LINK_KEY_REQUEST								0x17
#define HCI_EVENT_LINK_KEY_NOTIFICATION							0x18
#define HCI_EVENT_LOOPBACK_COMMAND								0x19
#define HCI_EVENT_DATA_BUFFER_OVERFLOW							0x1A
#define HCI_EVENT_MAX_SLOTS_CHANGE								0x1B
#define HCI_EVENT_READ_CLOCK_OFFSET_COMPLETE					0x1C
#define HCI_EVENT_CONNECTION_PACKET_TYPE_CHANGED				0x1D
#define HCI_EVENT_QOS_VIOLATION									0x1E
#define HCI_EVENT_PAGE_SCAN_REPETITION_MODE_CHANGE				0x20
#define HCI_EVENT_FLOW_SPECIFICATION_COMPLETE					0x21
#define HCI_EVENT_INQUIRY_RESULT_WITH_RSSI						0x22
#define HCI_EVENT_READ_REMOTE_EXTENDED_FEATURES_COMPLETE		0x23
#define HCI_EVENT_SCO_CONNECTION_COMPLETE						0x2C
#define HCI_EVENT_SCO_CONNECTION_CHANGED						0x2D
#define HCI_EVENT_SNIFF_SUBRATING								0x2E
#define HCI_EVENT_EXTENDED_INQUIRY_RESULT						0x2F
#define HCI_EVENT_ENCRYPTION_KEY_REFRESH_COMPLETE				0x30
#define HCI_EVENT_IO_CAPABILITY_REQUEST							0x31
#define HCI_EVENT_IO_CAPABILITY_RESPONSE						0x32
#define HCI_EVENT_USER_CONFIRMATION_REQUEST						0x33
#define HCI_EVENT_USER_PASSKEY_REQUEST							0x34
#define HCI_EVENT_REMOTE_OOB_DATA_REQUEST						0x35
#define HCI_EVENT_SIMPLE_PAIRING_COMPLETE						0x36
#define HCI_EVENT_LINK_SUPERVISION_TIMEOUT_CHANGED				0x38
#define HCI_EVENT_ENHANCED_FLUSH_COMPLETE						0x39
#define HCI_EVENT_USER_PASSKEY_NOTIFICATION						0x3B
#define HCI_EVENT_KEYPRESS_NOTIFICATION							0x3C
#define HCI_EVENT_REMOTE_HOST_SUPPORTED_FEATURES_NOTIFICATION	0x3D
#define HCI_EVENT_NUMBER_OF_COMPLETED_DATA_BLOCKS				0x48

typedef enum {
    HCI_PACKET_TYPE_CMD = 1,
    HCI_PACKET_TYPE_ACL,
    HCI_PACKET_TYPE_SCO,
    HCI_PACKET_TYPE_EVT
} HCI_PACKET_TYPE;


QHci::QHci(QSerialPort& serialPort, QBtsnoop& btsnoop) : serialPort(serialPort), btsnoop(btsnoop)
{

}

void QHci::_assign_cmd(uint8_t* buf, uint8_t ogf, uint16_t ocf) {
    buf[0] = HCI_PACKET_TYPE_CMD;
    buf[1] = ocf & 0xFF;
    buf[2] = (ocf >> 8) | (ogf << 2);
}

void QHci::send_cmd_reset() {
    uint8_t sendData[4] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_RESET);
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_read_buffer_size() {
    uint8_t sendData[4] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_INFORMATIONAL_PARAM, HCI_OCF_READ_BUFFER_SIZE);
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_read_local_version_info() {
    uint8_t sendData[4] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_INFORMATIONAL_PARAM, HCI_OCF_READ_LOCAL_VERSION_INFO);
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_read_bd_addr() {
    uint8_t sendData[4] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_INFORMATIONAL_PARAM, HCI_OCF_READ_BD_ADDR);
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_read_local_supported_commands() {
    uint8_t sendData[4] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_INFORMATIONAL_PARAM, HCI_OCF_READ_LOCAL_SUPPORTED_COMMANDS);
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_write_simple_pairing_mode(uint8_t simple_pairing_mode) {
    uint8_t sendData[5] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_WRITE_SIMPLE_PAIRING_MODE);
    sendData[3] = 1;
    sendData[4] = simple_pairing_mode;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_set_event_mask(uint8_t* event_mask) {
    uint8_t sendData[12] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_SET_EVENT_MASK);
    sendData[3] = 8;
    memcpy_s(&sendData[4], 8, event_mask, 8);
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_write_class_of_device(uint32_t class_of_device) {
    uint8_t sendData[7] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_WRITE_CLASS_OF_DEVICE);
    sendData[3] = 3;
    sendData[4] = class_of_device & 0xFF;
    sendData[5] = (class_of_device >> 8) & 0xFF;
    sendData[6] = (class_of_device >> 16) & 0xFF;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_write_page_timeout(uint16_t page_timeout) {
    uint8_t sendData[6] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_WRITE_PAGE_TIMEOUT);
    sendData[3] = 2;
    sendData[4] = page_timeout & 0xFF;
    sendData[5] = (page_timeout >> 8) & 0xFF;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_write_local_name(const char* local_name) {
    uint8_t sendData[252] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_WRITE_LOCAL_NAME);
    sendData[3] = 248;
    memcpy_s(&sendData[4], strlen(local_name), local_name, strlen(local_name));
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_write_inquiry_mode(uint8_t inquiry_mode) {
    uint8_t sendData[5] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_WRITE_INQUIRY_MODE);
    sendData[3] = 1;
    sendData[4] = inquiry_mode;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_write_scan_enable(uint8_t scan_enable) {
    uint8_t sendData[5] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_WRITE_SCAN_ENABLE);
    sendData[3] = 1;
    sendData[4] = scan_enable;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_inquiry(uint32_t inquiry_lap, uint8_t inquiry_len, uint8_t num_response) {
    uint8_t sendData[9] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_CONTROL, HCI_OCF_INQUIRY);
    sendData[3] = 5;
    sendData[4] = inquiry_lap & 0xFF;
    sendData[5] = (inquiry_lap >> 8) & 0xFF;
    sendData[6] = (inquiry_lap >> 16) & 0xFF;
    sendData[7] = inquiry_len;
    sendData[8] = num_response;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_inquiry_cancel() {
    uint8_t sendData[4] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_CONTROL, HCI_OCF_INQUIRY_CANCEL);
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_create_connection(uint8_t* bd_addr, uint16_t packet_type, uint8_t page_scan_repetition_mode, uint16_t clock_offset, uint8_t allow_role_switch) {
    uint8_t sendData[17] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_CONTROL, HCI_OCF_CREATE_CONNECT);
    sendData[3] = 13;
    memcpy_s(&sendData[4], 6, bd_addr, 6);
    sendData[10] = packet_type & 0xFF;
    sendData[11] = (packet_type >> 8) & 0xFF;
    sendData[12] = page_scan_repetition_mode;
    sendData[13] = 0;
    sendData[14] = clock_offset & 0xFF;
    sendData[15] = (clock_offset >> 8) & 0xFF;
    sendData[16] = allow_role_switch;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_accept_connection_request(uint8_t* bd_addr, uint8_t role) {
    uint8_t sendData[11] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_CONTROL, HCI_OCF_ACCEPT_CONNECT_REQUEST);
    sendData[3] = 7;
    memcpy_s(&sendData[4], 6, bd_addr, 6);
    sendData[10] = role;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_reject_connection_request(uint8_t* bd_addr, uint8_t reason) {
    uint8_t sendData[11] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_CONTROL, HCI_OCF_REJECT_CONNECT_REQUEST);
    sendData[3] = 7;
    memcpy_s(&sendData[4], 6, bd_addr, 6);
    sendData[10] = reason;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_link_key_request_reply(uint8_t* bd_addr, uint8_t* link_key) {
    uint8_t sendData[26] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_CONTROL, HCI_OCF_LINK_KEY_REQUEST_REPLY);
    sendData[3] = 22;
    memcpy_s(&sendData[4], 6, bd_addr, 6);
    memcpy_s(&sendData[10], 16, link_key, 16);
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_link_key_request_negative_reply(uint8_t* bd_addr) {
    uint8_t sendData[10] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_CONTROL, HCI_OCF_LINK_KEY_REQUEST_NEG_REPLY);
    sendData[3] = 6;
    memcpy_s(&sendData[4], 6, bd_addr, 6);
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_pin_code_request_reply(uint8_t* bd_addr, uint8_t pin_code_len, uint8_t* pin_code) {
    uint8_t sendDataLen = 4 + 6 + 1 + pin_code_len;
    QByteArray sendData;
    sendData.resize(sendDataLen);
    _assign_cmd((uint8_t*)(sendData.data()), HCI_OGF_LINK_CONTROL, HCI_OCF_PIN_CODE_REQUEST_REPLY);
    sendData[3] = sendDataLen - 4;
    memcpy_s(&sendData[4], 6, bd_addr, 6);
    sendData[10] = pin_code_len;
    memcpy_s(&sendData[11], pin_code_len, pin_code, pin_code_len);
    serialPort.write(sendData);
    btsnoop.wirte((uint8_t*)(sendData.data()), sendDataLen, BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_pin_code_request_negative_reply(uint8_t* bd_addr) {
    uint8_t sendData[10] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_CONTROL, HCI_OCF_PIN_CODE_REQUEST_NEG_REPLY);
    sendData[3] = 6;
    memcpy_s(&sendData[4], 6, bd_addr, 6);
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_read_remote_supported_features(uint16_t connection_handle) {
    uint8_t sendData[6] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_CONTROL, HCI_OCF_READ_REMOTE_SUPPORT_FEATURES);
    sendData[3] = 2;
    sendData[4] = connection_handle & 0xFF;
    sendData[5] = (connection_handle >> 8) & 0x0F; // 12 bits meaningful
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_read_remote_extended_features(uint16_t connection_handle, uint8_t page_number) {
    uint8_t sendData[7] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_CONTROL, HCI_OCF_READ_REMOTE_EXTEND_FEATURES);
    sendData[3] = 3;
    sendData[4] = connection_handle & 0xFF;
    sendData[5] = (connection_handle >> 8) & 0x0F; // 12 bits meaningful
    sendData[6] = page_number;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_setup_sco_connection(uint16_t connection_handle, uint32_t transmit_bandwidth, uint32_t receive_bandwidth, uint16_t max_latency,
                                        uint16_t voice_setting, uint8_t retransmission_effort, uint16_t packet_type) {
    uint8_t sendData[21] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_CONTROL, HCI_OCF_SETUP_SCO_CONNECT_REQUEST);
    sendData[3] = 17;
    sendData[4] = connection_handle & 0xFF;
    sendData[5] = (connection_handle >> 8) & 0x0F;
    sendData[6] = transmit_bandwidth & 0xFF;
    sendData[7] = (transmit_bandwidth >> 8) & 0xFF;
    sendData[8] = (transmit_bandwidth >> 16) & 0xFF;
    sendData[9] = (transmit_bandwidth >> 24) & 0xFF;
    sendData[10] = receive_bandwidth & 0xFF;
    sendData[11] = (receive_bandwidth >> 8) & 0xFF;
    sendData[12] = (receive_bandwidth >> 16) & 0xFF;
    sendData[13] = (receive_bandwidth >> 24) & 0xFF;
    sendData[14] = max_latency & 0xFF;
    sendData[15] = (max_latency >> 8) & 0xFF;
    sendData[16] = voice_setting & 0xFF;
    sendData[17] = (voice_setting >> 8) & 0x03; // 10 bits meaningful
    sendData[18] = retransmission_effort;
    sendData[19] = packet_type & 0xFF;
    sendData[20] = (packet_type >> 8) & 0xFF;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_accept_sco_connection_request(uint16_t connection_handle, uint32_t transmit_bandwidth, uint32_t receive_bandwidth, uint16_t max_latency,
    uint16_t voice_setting, uint8_t retransmission_effort, uint16_t packet_type) {
    uint8_t sendData[21] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_CONTROL, HCI_OCF_ACCEP_SCO_CONNECT_REQUEST);
    sendData[3] = 17;
    sendData[4] = connection_handle & 0xFF;
    sendData[5] = (connection_handle >> 8) & 0x0F;
    sendData[6] = transmit_bandwidth & 0xFF;
    sendData[7] = (transmit_bandwidth >> 8) & 0xFF;
    sendData[8] = (transmit_bandwidth >> 16) & 0xFF;
    sendData[9] = (transmit_bandwidth >> 24) & 0xFF;
    sendData[10] = receive_bandwidth & 0xFF;
    sendData[11] = (receive_bandwidth >> 8) & 0xFF;
    sendData[12] = (receive_bandwidth >> 16) & 0xFF;
    sendData[13] = (receive_bandwidth >> 24) & 0xFF;
    sendData[14] = max_latency & 0xFF;
    sendData[15] = (max_latency >> 8) & 0xFF;
    sendData[16] = voice_setting & 0xFF;
    sendData[17] = (voice_setting >> 8) & 0x03;
    sendData[18] = retransmission_effort;
    sendData[19] = packet_type & 0xFF;
    sendData[20] = (packet_type >> 8) & 0xFF;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_reject_sco_connection_request(uint8_t* bd_addr, uint8_t reason) {
    uint8_t sendData[11] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_CONTROL, HCI_OCF_REJECT_SCO_CONNECT_REQUEST);
    sendData[3] = 7;
    memcpy_s(&sendData[4], 6, bd_addr, 6);
    sendData[10] = reason;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_io_capability_request_reply(uint8_t* bd_addr, uint8_t io_capability, uint8_t oob_data_present, uint8_t auth_requirements) {
    uint8_t sendData[13] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_CONTROL, HCI_OCF_IO_CAP_REQUEST_REPLY);
    sendData[3] = 9;
    memcpy_s(&sendData[4], 6, bd_addr, 6);
    sendData[10] = io_capability;
    sendData[11] = oob_data_present;
    sendData[12] = auth_requirements;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_io_capability_request_negative_reply(uint8_t* bd_addr, uint8_t reason) {
    uint8_t sendData[11] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_CONTROL, HCI_OCF_IO_CAP_REQUEST_NEG_REPLY);
    sendData[3] = 7;
    memcpy_s(&sendData[4], 6, bd_addr, 6);
    sendData[10] = reason;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_user_confirm_request_reply(uint8_t* bd_addr) {
    uint8_t sendData[10] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_CONTROL, HCI_OCF_USER_CONFIRM_REQUEST_REPLY);
    sendData[3] = 6;
    memcpy_s(&sendData[4], 6, bd_addr, 6);
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_user_confirm_request_negative_reply(uint8_t* bd_addr) {
    uint8_t sendData[10] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_CONTROL, HCI_OCF_USER_CONFIRM_REQUEST_NEG_REPLY);
    sendData[3] = 6;
    memcpy_s(&sendData[4], 6, bd_addr, 6);
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_user_passkey_request_reply(uint8_t* bd_addr, uint32_t num_value) {
    uint8_t sendData[14] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_CONTROL, HCI_OCF_USER_PASSKEY_REQUEST_REPLY);
    sendData[3] = 10;
    memcpy_s(&sendData[4], 6, bd_addr, 6);
    sendData[10] = num_value & 0xFF;
    sendData[11] = (num_value >> 8) & 0xFF;
    sendData[12] = (num_value >> 16) & 0xFF;
    sendData[13] = (num_value >> 24) & 0xFF;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_user_passkey_request_negative_reply(uint8_t* bd_addr) {
    uint8_t sendData[10] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_CONTROL, HCI_OCF_USER_PASSKEY_REQUEST_NEG_REPLY);
    sendData[3] = 6;
    memcpy_s(&sendData[4], 6, bd_addr, 6);
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_hold_mode(uint16_t connection_handle, uint16_t max_interval, uint16_t min_interval) {
    uint8_t sendData[10] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_POLICY, HCI_OCF_HOLD_MODE);
    sendData[3] = 6;
    sendData[4] = connection_handle & 0xFF;
    sendData[5] = (connection_handle >> 8) & 0x0F;
    sendData[6] = max_interval & 0xFF;
    sendData[7] = (max_interval >> 8) & 0xFF;
    sendData[8] = min_interval & 0xFF;
    sendData[9] = (min_interval >> 8) & 0xFF;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_sniff_mode(uint16_t connection_handle, uint16_t max_interval, uint16_t min_interval, uint16_t attempt, uint16_t timeout) {
    uint8_t sendData[14] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_POLICY, HCI_OCF_SNIFF_MODE);
    sendData[3] = 10;
    sendData[4] = connection_handle & 0xFF;
    sendData[5] = (connection_handle >> 8) & 0x0F;
    sendData[6] = max_interval & 0xFF;
    sendData[7] = (max_interval >> 8) & 0xFF;
    sendData[8] = min_interval & 0xFF;
    sendData[9] = (min_interval >> 8) & 0xFF;
    sendData[10] = attempt & 0xFF;
    sendData[11] = (attempt >> 8) & 0xFF;
    sendData[12] = timeout & 0xFF;
    sendData[13] = (timeout >> 8) & 0xFF;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_exit_sniff_mode(uint16_t connection_handle) {
    uint8_t sendData[6] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_POLICY, HCI_OCF_EXIT_SNIFF_MODE);
    sendData[3] = 2;
    sendData[4] = connection_handle & 0xFF;
    sendData[5] = (connection_handle >> 8) & 0x0F;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_role_discovery(uint16_t connection_handle) {
    uint8_t sendData[6] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_POLICY, HCI_OCF_ROLE_DISCOVERY);
    sendData[3] = 2;
    sendData[4] = connection_handle & 0xFF;
    sendData[5] = (connection_handle >> 8) & 0x0F;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_switch_role(uint16_t connection_handle, uint8_t role) {
    uint8_t sendData[7] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_POLICY, HCI_OCF_SWITCH_ROLE);
    sendData[3] = 3;
    sendData[4] = connection_handle & 0xFF;
    sendData[5] = (connection_handle >> 8) & 0x0F;
    sendData[6] = role;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_read_link_policy_settings(uint16_t connection_handle) {
    uint8_t sendData[6] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_POLICY, HCI_OCF_READ_LINK_POLICY);
    sendData[3] = 2;
    sendData[4] = connection_handle & 0xFF;
    sendData[5] = (connection_handle >> 8) & 0x0F;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_write_link_policy_settings(uint16_t connection_handle, uint16_t link_policy_settings) {
    uint8_t sendData[8] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_POLICY, HCI_OCF_WRITE_LINK_POLICY);
    sendData[3] = 4;
    sendData[4] = connection_handle & 0xFF;
    sendData[5] = (connection_handle >> 8) & 0x0F;
    sendData[6] = link_policy_settings & 0xFF;
    sendData[7] = (link_policy_settings >> 8) & 0xFF;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_read_default_link_policy_settings() {
    uint8_t sendData[4] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_POLICY, HCI_OCF_READ_DEFAULT_LINK_POLICY);
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_write_default_link_policy_settings(uint16_t link_policy_settings) {
    uint8_t sendData[6] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_POLICY, HCI_OCF_WRITE_DEFAULT_LINK_POLICY);
    sendData[3] = 2;
    sendData[4] = link_policy_settings & 0xFF;
    sendData[5] = (link_policy_settings >> 8) & 0xFF;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_set_event_filter(uint8_t filter_type, uint8_t filter_condition_type, uint8_t* condition) {
    uint8_t conditionLen = 0;

    if (filter_type == 1) {
        if ((filter_condition_type == 1) || (filter_condition_type == 2)) {
            conditionLen = 6;
        }
    }
    if (filter_type == 2) {
        if (filter_condition_type == 0) {
            conditionLen = 1;
        }
        if ((filter_condition_type == 1) || (filter_condition_type == 2)) {
            conditionLen = 7;
        }
    }

    uint8_t sendDataLen = 4 + 2 + conditionLen;
    QByteArray sendData;
    sendData.resize(sendDataLen);
    _assign_cmd((uint8_t*)(sendData.data()), HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_SET_EVENT_FILTER);
    sendData[3] = sendDataLen - 4;
    sendData[4] = filter_type;
    sendData[5] = filter_condition_type;
    if (conditionLen > 0) {
        memcpy_s(&sendData[6], conditionLen, condition, conditionLen);
    }
    serialPort.write(sendData);
    btsnoop.wirte((uint8_t*)(sendData.data()), sendDataLen, BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_read_pin_type() {
    uint8_t sendData[4] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_READ_PIN_TYPE);
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_write_pin_type(uint8_t pin_type) {
    uint8_t sendData[5] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_WRITE_PIN_TYPE);
    sendData[3] = 1;
    sendData[4] = pin_type;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_read_stored_link_type(uint8_t* bd_addr, uint8_t read_all) {
    uint8_t sendData[11] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_READ_STORED_LINK_KEY);
    sendData[3] = 7;
    memcpy_s(&sendData[4], 6, bd_addr, 6);
    sendData[10] = read_all;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_write_stored_link_key(uint8_t num_keys_to_write, uint8_t* bd_addr, uint8_t* link_key) {
    uint8_t sendDataLen = 4 + 1 + (22 * num_keys_to_write);
    QByteArray sendData;
    sendData.resize(sendDataLen);
    _assign_cmd((uint8_t*)(sendData.data()), HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_WRITE_STORED_LINK_KEY);
    sendData[3] = sendDataLen - 4;
    sendData[4] = num_keys_to_write;
    memcpy_s(&sendData[5], 6 * num_keys_to_write, bd_addr, 6 * num_keys_to_write);
    memcpy_s(&sendData[5 + 6 * num_keys_to_write], 16 * num_keys_to_write, link_key, 16 * num_keys_to_write);
    serialPort.write(sendData);
    btsnoop.wirte((uint8_t*)(sendData.data()), sendDataLen, BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_delete_stored_link_key(uint8_t* bd_addr, uint8_t delete_all) {
    uint8_t sendData[11] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_DELETE_STORED_LINK_KEY);
    sendData[3] = 7;
    memcpy_s(&sendData[4], 6, bd_addr, 6);
    sendData[10] = delete_all;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_read_voice_setting() {
    uint8_t sendData[4] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_READ_VOICE_SETTING);
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_write_voice_setting(uint16_t voice_setting) {
    uint8_t sendData[6] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_WRITE_VOICE_SETTING);
    sendData[3] = 2;
    sendData[4] = voice_setting & 0xFF;
    sendData[5] = (voice_setting >> 8) & 0x03;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_read_auto_flush_timeout(uint16_t connection_handle) {
    uint8_t sendData[6] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_READ_AUTO_FLUSH_TIMEOUT);
    sendData[3] = 2;
    sendData[4] = connection_handle & 0xFF;
    sendData[5] = (connection_handle >> 8) & 0x0F;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_write_auto_flush_timeout(uint16_t connection_handle, uint16_t flush_timeout) {
    uint8_t sendData[8] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_WRITE_AUTO_FLUSH_TIMEOUT);
    sendData[3] = 4;
    sendData[4] = connection_handle & 0xFF;
    sendData[5] = (connection_handle >> 8) & 0x0F;
    sendData[6] = flush_timeout & 0xFF;
    sendData[7] = (flush_timeout >> 8) & 0xFF;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_set_c2h_flow_control(uint8_t flow_control_enable) {
    uint8_t sendData[5] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_SET_C2H_FLOW_CONTROL);
    sendData[3] = 1;
    sendData[4] = flow_control_enable;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_host_buffer_size(uint16_t acl_packet_len, uint8_t sco_packet_len, uint16_t acl_packet_total_num, uint16_t sco_packet_total_num) {
    uint8_t sendData[11] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_HOST_BUFF_SIZE);
    sendData[3] = 7;
    sendData[4] = acl_packet_len & 0xFF;
    sendData[5] = (acl_packet_len >> 8) & 0xFF;
    sendData[6] = sco_packet_len;
    sendData[7] = acl_packet_total_num & 0xFF;
    sendData[8] = (acl_packet_total_num >> 8) & 0xFF;
    sendData[9] = sco_packet_total_num & 0xFF;
    sendData[10] = (sco_packet_total_num >> 8) & 0xFF;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_host_num_of_completed_packets(uint8_t num_handles, uint8_t* connection_handle, uint8_t* num_of_completed_packets) {
    uint8_t sendDataLen = 4 + 1 + (4 * num_handles);
    QByteArray sendData;
    sendData.resize(sendDataLen);
    _assign_cmd((uint8_t*)(sendData.data()), HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_HOST_NUM_OF_COMPLETED_PACKET);
    sendData[3] = sendDataLen - 4;
    sendData[4] = num_handles;
    memcpy_s(&sendData[5], 2 * num_handles, connection_handle, 2 * num_handles);
    memcpy_s(&sendData[5 + 2 * num_handles], 2 * num_handles, num_of_completed_packets, 2 * num_handles);
    serialPort.write(sendData);
    btsnoop.wirte((uint8_t*)(sendData.data()), sendDataLen, BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_write_extended_inquiry_response(uint8_t fec_required, uint8_t* extended_inquiry_response) {
    uint8_t sendData[245] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_WRITE_EXTENDED_INQUIRY_RESPONSE);
    sendData[3] = 241;
    sendData[4] = fec_required;
    memcpy_s(&sendData[5], 240, extended_inquiry_response, 240);
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_read_local_supported_features() {
    uint8_t sendData[4] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_INFORMATIONAL_PARAM, HCI_OCF_RAED_LOCAL_SUPPORTED_FEATURE);
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_read_local_extended_features(uint8_t page_number) {
    uint8_t sendData[5] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_INFORMATIONAL_PARAM, HCI_OCF_READ_LOCAL_EXTENDED_FEATURE);
    sendData[3] = 1;
    sendData[4] = page_number;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_read_data_block_size() {
    uint8_t sendData[4] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_INFORMATIONAL_PARAM, HCI_OCF_READ_DATA_BLOCK_SIZE);
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_flush(uint16_t connection_handle) {
    uint8_t sendData[6] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_FLUSH);
    sendData[3] = 2;
    sendData[4] = connection_handle & 0xFF;
    sendData[5] = (connection_handle >> 8) & 0x0F;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_write_connection_accept_timeout(uint16_t timeout) {
    uint8_t sendData[6] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_WRITE_CONNECT_ACCEPT_TIMEOUT);
    sendData[3] = 2;
    sendData[4] = timeout & 0xFF;
    sendData[5] = (timeout >> 8) & 0xFF;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_write_auth_enable(uint8_t auth_enable) {
    uint8_t sendData[5] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_WRITE_AUTH_ENABLE);
    sendData[3] = 1;
    sendData[4] = auth_enable;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_disconnect(uint16_t connection_handle, uint8_t reason) {
    uint8_t sendData[7] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_CONTROL, HCI_OCF_DISCONNECT);
    sendData[3] = 3;
    sendData[4] = connection_handle & 0xFF;
    sendData[5] = (connection_handle >> 8) & 0x0F;
    sendData[6] = reason;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_create_connection_cancel(uint8_t* bd_addr) {
    uint8_t sendData[10] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_CONTROL, HCI_OCF_CREATE_CONNECT_CANCEL);
    sendData[3] = 6;
    memcpy_s(&sendData[4], 6, bd_addr, 6);
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_change_connection_packet_type(uint16_t connection_handle, uint16_t packet_type) {
    uint8_t sendData[8] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_CONTROL, HCI_OCF_CHANGE_CONNECT_PACKET_TYPE);
    sendData[3] = 4;
    sendData[4] = connection_handle & 0xFF;
    sendData[5] = (connection_handle >> 8) & 0x0F;
    sendData[6] = packet_type & 0xFF;
    sendData[7] = (packet_type >> 8) & 0xFF;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_auth_requested(uint16_t connection_handle) {
    uint8_t sendData[6] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_CONTROL, HCI_OCF_AUTH_REQUEST);
    sendData[3] = 2;
    sendData[4] = connection_handle & 0xFF;
    sendData[5] = (connection_handle >> 8) & 0x0F;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_set_connection_encryption(uint16_t connection_handle, uint8_t encryption_enable) {
    uint8_t sendData[7] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_CONTROL, HCI_OCF_SET_CONNECT_ENCRYPTION);
    sendData[3] = 3;
    sendData[4] = connection_handle & 0xFF;
    sendData[5] = (connection_handle >> 8) & 0x0F;
    sendData[6] = encryption_enable;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_change_connection_link_key(uint16_t connection_handle) {
    uint8_t sendData[6] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_CONTROL, HCI_OCF_CHANGE_CONNECT_LINK_KEY);
    sendData[3] = 2;
    sendData[4] = connection_handle & 0xFF;
    sendData[5] = (connection_handle >> 8) & 0x0F;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void QHci::send_cmd_link_key_selection(uint8_t key_flag) {
    uint8_t sendData[5] = { 0x00 };
    _assign_cmd(sendData, HCI_OGF_LINK_CONTROL, HCI_OCF_LINK_KEY_SELECTION);
    sendData[3] = 1;
    sendData[4] = key_flag;
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}
