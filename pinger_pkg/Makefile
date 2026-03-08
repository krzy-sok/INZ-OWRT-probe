include $(TOPDIR)/rules.mk

# Name, version and release number
# The name and version of your package are used to define the variable to point to the build directory of your package: $(PKG_BUILD_DIR)
PKG_NAME:=pinger_pkg
PKG_VERSION:=1.0
PKG_RELEASE:=1

# Source settings (i.e. where to find the source codes)
# This is a custom variable, used below
SOURCE_DIR:=/home/flopper/INZ/probe-module/pinger_pkg/src
# FILES = $(SOURCE_DIR)/main.cpp $(SOURCE_DIR)/classes/host.hpp $(SOURCE_DIR)/classes/host.cpp $(SOURCE_DIR)/classes/ping_row.hpp $(SOURCE_DIR)/classes/ping_row.cpp
FILES = $(shell find $(SOURCE_DIR) -name '*.hpp' -o -name '*.cpp')
# FILES="$(FILES) $(shell find $(SOURCE_DIR) -name '*.cpp')"

include $(INCLUDE_DIR)/package.mk

# Package definition; instructs on how and where our package will appear in the overall configuration menu ('make menuconfig')
define Package/pinger_pkg
	SECTION:=examples
	CATEGORY:=Examples
	TITLE:=C++ OOp Pinging demo
	DEPENDS:=+libstdcpp
endef

# Package description; a more verbose description on what our package does
define Package/pinger_pkg/description
	Send ICMP packets and measure rtt for a list of hosts with possible mac flooding.
endef

# Package preparation instructions; create the build directory and copy the source code.
# The last command is necessary to ensure our preparation instructions remain compatible with the patching system.
define Build/Prepare
		mkdir -p $(PKG_BUILD_DIR)
		cp $(SOURCE_DIR)/* $(PKG_BUILD_DIR) -r
		$(Build/Patch)
endef

# Package build instructions; invoke the target-specific compiler to first compile the source file, and then to link the file into the final executable

# 		$(TARGET_GCC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/pinger_pkg.o -c $(PKG_BUILD_DIR)/arp-pinger.c
# 		$(TARGET_GCC) $(TARGET_LDFLAGS) -o $(PKG_BUILD_DIR)/$1 $(PKG_BUILD_DIR)/pinger_pkg.o
define Build/Compile
		$(TARGET_CXX) $(TARGET_CXXFLAGS) $(FILES) -o $(PKG_BUILD_DIR)/pinger_pkg $(TARGET_LIBS) -std=c++20
endef

# Package install instructions; create a directory inside the package to hold our executable, and then copy the executable we built previously into the folder
define Package/pinger_pkg/install
		$(INSTALL_DIR) $(1)/usr/bin
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/pinger_pkg $(1)/usr/bin
endef

# This command is always the last, it uses the definitions and variables we give above in order to get the job done
$(eval $(call BuildPackage,pinger_pkg))
