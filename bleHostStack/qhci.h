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
    void send_cmd_inquiry(uint32_t inquiry_lap, uint8_t inquiry_len, uint8_t num_response);
    void send_cmd_inquiry_cancel();
    void send_cmd_create_connection(uint8_t* bd_addr, uint16_t packet_type, uint8_t page_scan_repetition_mode, uint16_t clock_offset, uint8_t allow_role_switch);
    void send_cmd_disconnect(uint16_t connection_handle, uint8_t reason);
    void send_cmd_create_connection_cancel(uint8_t* bd_addr);
    void send_cmd_accept_connection_request(uint8_t* bd_addr, uint8_t role);
    void send_cmd_reject_connection_request(uint8_t* bd_addr, uint8_t reason);
    void send_cmd_link_key_request_reply(uint8_t* bd_addr, uint8_t* link_key);
    void send_cmd_link_key_request_negative_reply(uint8_t* bd_addr);
    void send_cmd_pin_code_request_reply(uint8_t* bd_addr, uint8_t pin_code_len, uint8_t* pin_code);
    void send_cmd_pin_code_request_negative_reply(uint8_t* bd_addr);
    void send_cmd_change_connection_packet_type(uint16_t connection_handle, uint16_t packet_type);
    void send_cmd_auth_requested(uint16_t connection_handle);
    void send_cmd_set_connection_encryption(uint16_t connection_handle, uint8_t encryption_enable);
    void send_cmd_change_connection_link_key(uint16_t connection_handle);
    void send_cmd_link_key_selection(uint8_t key_flag);
    void send_cmd_read_remote_supported_features(uint16_t connection_handle);
    void send_cmd_read_remote_extended_features(uint16_t connection_handle, uint8_t page_number);
    void send_cmd_setup_sco_connection(uint16_t connection_handle, uint32_t transmit_bandwidth, uint32_t receive_bandwidth,
                                       uint16_t max_latency, uint16_t voice_setting, uint8_t retransmission_effort, uint16_t packet_type);
    void send_cmd_accept_sco_connection_request(uint16_t connection_handle, uint32_t transmit_bandwidth, uint32_t receive_bandwidth,
                                                uint16_t max_latency, uint16_t voice_setting, uint8_t retransmission_effort, uint16_t packet_type);
    void send_cmd_reject_sco_connection_request(uint8_t* bd_addr, uint8_t reason);
    void send_cmd_io_capability_request_reply(uint8_t* bd_addr, uint8_t io_capability, uint8_t oob_data_present, uint8_t auth_requirements);
    void send_cmd_io_capability_request_negative_reply(uint8_t* bd_addr, uint8_t reason);
    void send_cmd_user_confirm_request_reply(uint8_t* bd_addr);
    void send_cmd_user_confirm_request_negative_reply(uint8_t* bd_addr);
    void send_cmd_user_passkey_request_reply(uint8_t* bd_addr, uint32_t num_value);
    void send_cmd_user_passkey_request_negative_reply(uint8_t* bd_addr);

    /* Link Policy APIs */
    void send_cmd_hold_mode(uint16_t connection_handle, uint16_t max_interval, uint16_t min_interval);
    void send_cmd_sniff_mode(uint16_t connection_handle, uint16_t max_interval, uint16_t min_interval, uint16_t attempt, uint16_t timeout);
    void send_cmd_exit_sniff_mode(uint16_t connection_handle);
    void send_cmd_role_discovery(uint16_t connection_handle);
    void send_cmd_switch_role(uint16_t connection_handle, uint8_t role);
    void send_cmd_read_link_policy_settings(uint16_t connection_handle);
    void send_cmd_write_link_policy_settings(uint16_t connection_handle, uint16_t link_policy_settings);
    void send_cmd_read_default_link_policy_settings();
    void send_cmd_write_default_link_policy_settings(uint16_t link_policy_settings);

    /* Controller & Baseband APIs */
    void send_cmd_set_event_mask(uint8_t* event_mask);
    void send_cmd_reset();
    void send_cmd_set_event_filter(uint8_t filter_type, uint8_t filter_condition_type, uint8_t* condition);
    void send_cmd_flush(uint16_t connection_handle);
    void send_cmd_read_pin_type();
    void send_cmd_write_pin_type(uint8_t pin_type);
    void send_cmd_read_stored_link_type(uint8_t* bd_addr, uint8_t read_all);
    void send_cmd_write_stored_link_key(uint8_t num_keys_to_write, uint8_t* bd_addr, uint8_t* link_key);
    void send_cmd_delete_stored_link_key(uint8_t* bd_addr, uint8_t delete_all);
    void send_cmd_write_local_name(const char* local_name);
    void send_cmd_write_connection_accept_timeout(uint16_t timeout);
    void send_cmd_write_page_timeout(uint16_t page_timeout);
    void send_cmd_write_scan_enable(uint8_t scan_enable);
    void send_cmd_write_auth_enable(uint8_t auth_enable);
    void send_cmd_read_class_of_device();
    void send_cmd_write_class_of_device(uint8_t* class_of_device);
    void send_cmd_read_voice_setting();
    void send_cmd_write_voice_setting(uint16_t voice_setting);
    void send_cmd_read_auto_flush_timeout(uint16_t connection_handle);
    void send_cmd_write_auto_flush_timeout(uint16_t connection_handle, uint16_t flush_timeout);
    void send_cmd_set_c2h_flow_control(uint8_t flow_control_enable);
    void send_cmd_host_buffer_size(uint16_t acl_packet_len, uint8_t sco_packet_len, uint16_t acl_packet_total_num, uint16_t sco_packet_total_num);
    void send_cmd_host_num_of_completed_packets(uint8_t num_handles, uint8_t* connection_handle, uint8_t* num_of_completed_packets);
    void send_cmd_write_inquiry_mode(uint8_t inquiry_mode);
    void send_cmd_write_extended_inquiry_response(uint8_t fec_required, uint8_t* extended_inquiry_response);
    void send_cmd_write_simple_pairing_mode(uint8_t simple_pairing_mode);

    /* Informational Parameters APIs */
    void send_cmd_read_local_version_info();
    void send_cmd_read_local_supported_commands();
    void send_cmd_read_local_supported_features();
    void send_cmd_read_local_extended_features(uint8_t page_number);
    void send_cmd_read_buffer_size();
    void send_cmd_read_bd_addr();
    void send_cmd_read_data_block_size();

    /* LE Commands APIs */
    void send_cmd_le_read_buffer_size();
    void send_cmd_le_set_advertising_parameters(uint8_t* parameters);
    void send_cmd_le_set_advertising_data(uint8_t* data);
    void send_cmd_le_set_advertising_enable(uint8_t enable);

    void recv(uint8_t* data, uint16_t len);
    void recv_evt(uint8_t* data, uint16_t len);
    void recv_acl(uint8_t* data, uint16_t len);
    void recv_sco(uint8_t* data, uint16_t len);
    void recv_evt_command_complete(uint8_t* data, uint16_t len);

private:
    void _assign_cmd(uint8_t* buf, uint8_t ogf, uint16_t ocf);

    QSerialPort& serialPort;
    QBtsnoop& btsnoop;
};

#endif // QHCI_H
