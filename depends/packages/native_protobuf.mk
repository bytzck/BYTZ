package=native_protobuf
$(package)_version=$(protobuf_version)
$(package)_download_path=$(protobuf_download_path)
$(package)_file_name=$(protobuf_file_name)
$(package)_sha256_hash=$(protobuf_sha256_hash)

define $(package)_set_vars
$(package)_config_opts=--disable-shared
endef

define $(package)_config_cmds
  $($(package)_autoconf)
endef

define $(package)_build_cmds
  $(MAKE) -C src protoc
endef

define $(package)_stage_cmds
  $(MAKE) -C src DESTDIR=$($(package)_staging_dir) install-strip
endef

define $(package)_postprocess_cmds
  rm -rf lib include
endef
