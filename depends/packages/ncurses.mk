package=ncurses
$(package)_version=5.9
$(package)_download_path=https://invisible-mirror.net/archives/ncurses/
$(package)_file_name=ncurses-$($(package)_version).tar.gz
$(package)_sha256_hash=9046298fb440324c9d4135ecea7879ffed8546dd1b58e59430ea07a4633f563b
$(package)_patches=ncurses-5.9-gcc-5.patch

define $(package)_preprocess_cmds
   patch -p1 < $($(package)_patch_dir)/ncurses-5.9-gcc-5.patch
endef

define $(package)_config_cmds
  $($(package)_autoconf) --without-cxx --with-shared --enable-pc-files --with-termlib
endef

define $(package)_build_cmds
  $(MAKE)
endef


define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
