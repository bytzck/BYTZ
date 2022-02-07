packages:=boost libevent openssl cmake bls-dash gmp backtrace zeromq

qt_native_packages = native_protobuf
qt_packages = qrencode protobuf zlib

qrencode_packages = qrencode

qt_linux_packages:=qt expat libxcb xcb_proto libXau xproto freetype fontconfig libxkbcommon
qt_android_packages=qt

qt_darwin_packages=qt
qt_mingw32_packages=qt

wallet_packages=bdb

upnp_packages=miniupnpc

darwin_native_packages = native_biplist native_ds_store native_mac_alias

#$(host_arch)_$(host_os)_native_packages += native_b2

ifneq ($(build_os),darwin)
darwin_native_packages += native_libtapi native_cctools native_cdrkit native_libdmg-hfsplus
endif
