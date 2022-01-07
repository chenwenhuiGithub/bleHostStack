#ifndef QHCI_H
#define QHCI_H

#include <stdint.h>
#include <QSerialPort>
#include "qbtsnoop.h"

class QHci
{
public:
    QHci(QSerialPort& serialPort, QBtsnoop& btsnoop);

    /* Link Control APIs */
    void inquiry(uint32_t inquiry_lap, uint8_t inquiry_len, uint8_t num_response);
    void inquiry_cancel();
    void create_connection(uint8_t* bd_addr, uint16_t packet_type, uint8_t page_scan_repetition_mode, uint16_t clock_offset, uint8_t allow_role_switch);
    void disconnect(uint16_t connection_handle, uint8_t reason);
    void create_connection_cancel(uint8_t* bd_addr);
    void accept_connection_request(uint8_t* bd_addr, uint8_t role);
    void reject_connection_request(uint8_t* bd_addr, uint8_t reason);
    void link_key_request_reply(uint8_t* bd_addr, uint8_t* link_key);
    void link_key_request_negative_reply(uint8_t* bd_addr);
    void pin_code_request_reply(uint8_t* bd_addr, uint8_t pin_code_len, uint8_t* pin_code);
    void pin_code_request_negative_reply(uint8_t* bd_addr);
    void change_connection_packet_type(uint16_t connection_handle, uint16_t packet_type);
    void auth_requested(uint16_t connection_handle);
    void set_connection_encryption(uint16_t connection_handle, uint8_t encryption_enable);
    void change_connection_link_key(uint16_t connection_handle);
    void link_key_selection(uint8_t key_flag);
    void read_remote_supported_features(uint16_t connection_handle);
    void read_remote_extended_features(uint16_t connection_handle, uint8_t page_number);
    void setup_sco_connection(uint16_t connection_handle, uint32_t transmit_bandwidth, uint32_t receive_bandwidth,
                              uint16_t max_latency, uint16_t voice_setting, uint8_t retransmission_effort, uint16_t packet_type);
    void accept_sco_connection_request(uint16_t connection_handle, uint32_t transmit_bandwidth, uint32_t receive_bandwidth,
                                       uint16_t max_latency, uint16_t voice_setting, uint8_t retransmission_effort, uint16_t packet_type);
    void reject_sco_connection_request(uint8_t* bd_addr, uint8_t reason);
    void io_capability_request_reply(uint8_t* bd_addr, uint8_t io_capability, uint8_t oob_data_present, uint8_t auth_requirements);
    void io_capability_request_negative_reply(uint8_t* bd_addr, uint8_t reason);
    void user_confirm_request_reply(uint8_t* bd_addr);
    void user_confirm_request_negative_reply(uint8_t* bd_addr);
    void user_passkey_request_reply(uint8_t* bd_addr, uint32_t num_value);
    void user_passkey_request_negative_reply(uint8_t* bd_addr);

    /* Link Policy APIs */
    void hold_mode(uint16_t connection_handle, uint16_t max_interval, uint16_t min_interval);
    void sniff_mode(uint16_t connection_handle, uint16_t max_interval, uint16_t min_interval, uint16_t attempt, uint16_t timeout);
    void exit_sniff_mode(uint16_t connection_handle);
    void role_discovery(uint16_t connection_handle);
    void switch_role(uint16_t connection_handle, uint8_t role);
    void read_link_policy_settings(uint16_t connection_handle);
    void write_link_policy_settings(uint16_t connection_handle, uint16_t link_policy_settings);
    void read_default_link_policy_settings();
    void write_default_link_policy_settings(uint16_t link_policy_settings);

    /* Controller & Baseband APIs */
    void set_event_mask(uint8_t* event_mask);
    void reset();
    void set_event_filter(uint8_t filter_type, uint8_t filter_condition_type, uint8_t* condition);
    void flush(uint16_t connection_handle);
    void read_pin_type();
    void write_pin_type(uint8_t pin_type);
    void read_stored_link_type(uint8_t* bd_addr, uint8_t read_all);
    void write_stored_link_key(uint8_t num_keys_to_write, uint8_t* bd_addr, uint8_t* link_key);
    void delete_stored_link_key(uint8_t* bd_addr, uint8_t delete_all);
    void write_local_name(const char* local_name);
    void write_connection_accept_timeout(uint16_t timeout);
    void write_page_timeout(uint16_t page_timeout);
    void write_scan_enable(uint8_t scan_enable);
    void write_auth_enable(uint8_t auth_enable);
    void write_class_of_device(uint32_t class_of_device);
    void read_voice_setting();
    void write_voice_setting(uint16_t voice_setting);
    void read_auto_flush_timeout(uint16_t connection_handle);
    void write_auto_flush_timeout(uint16_t connection_handle, uint16_t flush_timeout);
    void set_c2h_flow_control(uint8_t flow_control_enable);
    void host_buffer_size(uint16_t acl_packet_len, uint8_t sco_packet_len, uint16_t acl_packet_total_num, uint16_t sco_packet_total_num);
    void host_num_of_completed_packets(uint8_t num_handles, uint8_t* connection_handle, uint8_t* num_of_completed_packets);
    void write_inquiry_mode(uint8_t inquiry_mode);
    void write_extended_inquiry_response(uint8_t fec_required, uint8_t* extended_inquiry_response);
    void write_simple_pairing_mode(uint8_t simple_pairing_mode);

    /* Informational Parameters APIs */
    void read_local_version_info();
    void read_local_supported_commands();
    void read_local_supported_features();
    void read_local_extended_features(uint8_t page_number);
    void read_buffer_size();
    void read_bd_addr();
    void read_data_block_size();

private:
    void _assign_command(uint8_t* buf, uint8_t ogf, uint16_t ocf);

    QSerialPort& serialPort;
    QBtsnoop& btsnoop;
};

#endif // QHCI_H
