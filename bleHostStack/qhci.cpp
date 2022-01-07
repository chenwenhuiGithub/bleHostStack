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

void QHci::_assign_command(uint8_t* buf, uint8_t ogf, uint16_t ocf) {
    buf[0] = HCI_PACKET_TYPE_CMD;
    buf[1] = ocf & 0xFF;
    buf[2] = (ocf >> 8) | (ogf << 2);
}

void QHci::reset() {
    uint8_t sendData[4] = { 0x00 };
    _assign_command(sendData, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_RESET);
    serialPort.write((char*)sendData, sizeof(sendData));
    btsnoop.wirte(sendData, sizeof(sendData), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}
