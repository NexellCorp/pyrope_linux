LOCAL_PATH:= $(call my-dir)
TEST_APP	:= $(LOCAL_PATH)

include $(TEST_APP)/reset_test/Android.mk
include $(TEST_APP)/test_cmd/Android.mk
include $(TEST_APP)/test_i2c/Android.mk
include $(TEST_APP)/test_lsocket/Android.mk
include $(TEST_APP)/test_ping/Android.mk
include $(TEST_APP)/test_pwm/Android.mk
include $(TEST_APP)/test_socket/Android.mk
include $(TEST_APP)/test_tms/Android.mk
include $(TEST_APP)/test_uart/Android.mk
include $(TEST_APP)/test_uevent/Android.mk
include $(TEST_APP)/test_eeprom/Android.mk
