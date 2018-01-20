#
# Copyright (C) 2014-2015 wrtnode.com
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=spi-bridge-wq
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define Package/spi-bridge-wq
  SECTION:=spi-bridge-wq
  CATEGORY:=WRTnode
  TITLE:=WRTnode2r bridge between 7688 and stm32.
  DEPENDS:=+libpthread +libmosquitto-nossl +libcares
endef

define Package/spi-bridge-wq/description
  This package is a bridge connected between 7688 and stm32.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
	$(TARGET_CONFIGURE_OPTS) CFLAGS="$(TARGET_CFLAGS)"
endef

define Package/spi-bridge-wq/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/spi-bridge-wq $(1)/usr/bin/
endef

$(eval $(call BuildPackage,spi-bridge-wq))
