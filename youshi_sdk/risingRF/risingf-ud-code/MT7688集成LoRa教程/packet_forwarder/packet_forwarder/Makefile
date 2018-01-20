#
# Copyright (C) 2015-2017 risinghf
#

include $(TOPDIR)/rules.mk

PKG_NAME:=risinghf-pfw
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/risinghf-pfw
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=risinghf packet forwarder
	DEPENDS:=+librt +libpthread
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		AR="$(TARGET_AR)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS:="$(TARGET_LDFLAGS)" 
endef

define Package/risinghf-pfw/install
	$(INSTALL_DIR) $(1)/home/rxhf/packet_forwarder
	$(INSTALL_DIR) $(1)/home/rxhf/packet_forwarder/gps_pkt_fwd
	$(INSTALL_DIR) $(1)/home/rxhf/packet_forwarder/lora_pkt_fwd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/gps_pkt_fwd/gps_pkt_fwd $(1)/home/rxhf/packet_forwarder/gps_pkt_fwd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lora_pkt_fwd/lora_pkt_fwd $(1)/home/rxhf/packet_forwarder/lora_pkt_fwd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/util_ack/util_ack $(1)/home/rxhf/packet_forwarder
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/util_sink/util_sink $(1)/home/rxhf/packet_forwarder
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/util_tx_test/util_tx_test $(1)/home/rxhf/packet_forwarder
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/gps_pkt_fwd/global_conf.json $(1)/home/rxhf/packet_forwarder/gps_pkt_fwd
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/gps_pkt_fwd/local_conf.json $(1)/home/rxhf/packet_forwarder/gps_pkt_fwd
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/lora_pkt_fwd/global_conf.json $(1)/home/rxhf/packet_forwarder/lora_pkt_fwd
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/lora_pkt_fwd/local_conf.json $(1)/home/rxhf/packet_forwarder/lora_pkt_fwd
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/reset_lgw.sh $(1)/usr/bin
	$(INSTALL_BIN) ./files/test_loragw_reg $(1)/usr/bin
	$(INSTALL_BIN) ./files/test_loragw_spi $(1)/usr/bin
	$(INSTALL_BIN) ./files/util_spi_stress $(1)/usr/bin
endef

$(eval $(call BuildPackage,risinghf-pfw))
