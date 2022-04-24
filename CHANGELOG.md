# Changelog

All notable changes to this project will be documented in this file.

# Asciiville-1.3.1r1 (2022-04-24)

* Install Asciiville Figlet Fonts in pyfiglet during postinstall
* Add `show_figlet_fonts` command
* Create armhf architecture Debian format package for Raspberry Pi installs
* Add -d option to indicate use of disk usage analyzer
* Add -X option to indicate use of current terminal window
* Fixup exit from asciimatics scripts
* MusicPlayerPlus and RoonCommandLine menu integration
* Support for w3m, mapscii, and setting default browser
* Add Figlet Fonts and use Figlet Text for menu headers
* Lolcat install during postinstall

# Asciiville-1.3.0r1 (2022-04-20)

Initial release with support for:

* The lightweight character based system monitor, `btop`
* The lightweight character based web browser, `lynx`
* The lightweight character based mail client, `neomutt` or `mutt`
* The lightweight character based file manager, `ranger`
* One or more terminal emulators running a command
* A tmux session
* A command line web search
* Command line character based Twitter client
* A network download/upload speed test
* The AAlib BB demo running in a tmux session (Debian based systems only)
* Character based ASCII art
* Display system info
* Display the Phase of the Moon
* Display a weather report
* Any character based client the user wishes to run
* One of several asciimatics animations optionally accompanied by audio

Thu Apr 21 18:12:30 2022 -0700 7df65d3 :
   Add Figlet Fonts and use Figlet Text as menu headers
Thu Apr 21 14:31:21 2022 -0700 fb95860 :
   Added RoonCommandLine menu integration
Thu Apr 21 13:30:09 2022 -0700 658469f :
   Fix test for wget in postinst
Thu Apr 21 13:18:29 2022 -0700 81d5325 :
   Lolcat install during postinstall, lolcat use toggle switches and menu
Thu Apr 21 12:05:29 2022 -0700 a84a623 :
   Added integration of mpcplus client and MusicPlayerPlus menus
Thu Apr 21 11:20:41 2022 -0700 d15efda :
   Add support for w3m, mapscii, and setting default browser
Wed Apr 20 18:34:10 2022 -0700 bba4f84 :
   Correct Moon url
Wed Apr 20 18:32:10 2022 -0700 fa0a01e :
   Initial release is v1.3.0r1
Wed Apr 20 17:09:37 2022 -0700 1b5faa7 :
   Relax required rpm packages, use mutt if neomutt unavailable
Wed Apr 20 16:24:54 2022 -0700 d24b11d :
   Update changelog and release notes in preparation for initial release
Wed Apr 20 16:04:18 2022 -0700 acf196a :
   Add 'moon' and 'weather' special command keywords, use wttr.in for these
Wed Apr 20 14:46:07 2022 -0700 e19ac22 :
   Update usage and README with new commands
Wed Apr 20 14:24:37 2022 -0700 e1cb535 :
   Fixup asciiart splash animation
Wed Apr 20 12:46:12 2022 -0700 97bf0b8 :
   Configure rainbowstream, add /usr/local/bin to PATH in postins to avoid warnings
Wed Apr 20 12:09:27 2022 -0700 5b00034 :
   Don't call update-menus in postinst
Wed Apr 20 12:05:41 2022 -0700 746bd8a :
   Install pip modules during postinst, add Twitter client
Wed Apr 20 10:07:34 2022 -0700 ef06f94 :
   Add ddgr command line web search
Wed Apr 20 09:04:43 2022 -0700 2aa98b0 :
   Update desktop file with languages and start asciiville in a terminal in interactive mode
Wed Apr 20 08:55:10 2022 -0700 e6c1ad3 :
   Moved configuration files into conf subdir
Tue Apr 19 23:12:13 2022 -0700 c35af82 :
   Typo in bin/ascinit
Tue Apr 19 22:54:58 2022 -0700 461a083 :
   Spiff up menus, add system info with neofetch, add rifle.conf
Tue Apr 19 19:40:28 2022 -0700 347d45c :
   Added my scrolling credit to asciiart
Tue Apr 19 19:20:56 2022 -0700 7dfd478 :
   Improve asciiart animation
Tue Apr 19 18:14:25 2022 -0700 ae649cf :
   Add license for art
Tue Apr 19 17:57:39 2022 -0700 3fd092e :
   Add asciiart to README
Tue Apr 19 17:56:25 2022 -0700 348a06f :
   Add art folder and asciiart asciimatics animation
Tue Apr 19 14:25:34 2022 -0700 133a71a :
   Add support for speedtest, cacaview, aalib, and bb demo in tmux
Tue Apr 19 10:49:16 2022 -0700 6d72db1 :
   Add asciinema to dependencies
Mon Apr 18 19:49:43 2022 -0700 3474ccf :
   Add several commands to asciiville, update man pages and README
Mon Apr 18 15:34:55 2022 -0700 c96f2a3 :
   Add lynx to dependencies
Mon Apr 18 15:01:31 2022 -0700 3fc034a :
   First pass at terminal command execution
Sun Apr 17 20:09:23 2022 -0700 2adf285 :
   Do not remove entries in .gitignore from dist dir
Sun Apr 17 19:58:24 2022 -0700 5b7ff08 :
   Don't use -v option to build-btop.sh
Sun Apr 17 19:55:26 2022 -0700 e22d0e8 :
   Use make distclean to clean btop
Sun Apr 17 19:49:40 2022 -0700 ef2515c :
   First pass at Asciiville commands
Sun Apr 17 18:46:01 2022 -0700 ad49bfc :
   Add tmux.conf
Sun Apr 17 12:02:21 2022 -0700 bcf44ac :
   Deleted screenshots for now
Sat Apr 16 18:01:48 2022 -0700 37e9e77 :
   Build btop, add man pages
Sat Apr 16 17:21:30 2022 -0700 5960bcf :
   Integrate btop build into packaging
Sat Apr 16 16:55:53 2022 -0700 30668d3 :
   First pass at README and packaging
Sat Apr 16 14:50:13 2022 -0700 43c6f51 :
   Initialize repo
Jakob P. Liljenberg, Thu Apr 14 14:45:06 2022 +0200 9abf849 :
   Merge pull request #313 from kz6fittycent/main
kz6fittycent, Wed Apr 13 18:45:18 2022 -0500 384f082 :
   Update snapcraft.yaml
Jakob P. Liljenberg, Wed Apr 13 11:42:04 2022 +0200 4bae3c4 :
   Update continuous-build-linux.yml
aristocratos, Wed Apr 13 11:19:47 2022 +0200 df205fb :
   v1.2.6 Bug fixes
aristocratos, Wed Apr 13 08:13:39 2022 +0200 2d93993 :
   Changed: Enter symbol to a more common variant
aristocratos, Wed Apr 6 13:03:49 2022 +0200 87579be :
   updated robin_hood.h to v3.11.5
aristocratos, Sat Mar 26 03:02:48 2022 +0100 9373d9d :
   Merge branch 'main' of github.com:aristocratos/btop
aristocratos, Sat Mar 26 03:00:54 2022 +0100 b0d5c03 :
   Fixed: Use cpu cores avarage temp if missing cpu package temp for FreeBSD
Jakob P. Liljenberg, Mon Mar 7 13:58:10 2022 +0100 d14af7f :
   Merge pull request #284 from mohi001/main
mohammad hashemy, Mon Mar 7 15:56:15 2022 +0330 c6c0870 :
   Fixed: Wrong memory unit when shorten and size is less than 10
aristocratos, Sun Mar 6 00:27:23 2022 +0100 76e26b0 :
   v1.2.5 Bug fixes
aristocratos, Fri Mar 4 20:26:04 2022 +0100 294b753 :
   Fixed: Mouse mappings for net box
aristocratos, Fri Mar 4 20:23:12 2022 +0100 7097c1c :
   Merge branch 'main' of github.com:aristocratos/btop
aristocratos, Fri Mar 4 20:22:46 2022 +0100 2b56926 :
   Fixed: Small ui fixes for mem and disks
Jakob P. Liljenberg, Fri Mar 4 20:21:33 2022 +0100 c0d54f5 :
   Merge pull request #275 from mrdotx/draw_fix
aristocratos, Fri Mar 4 19:40:01 2022 +0100 156f72f :
   Merge branch 'main' of github.com:aristocratos/btop
aristocratos, Fri Mar 4 19:39:06 2022 +0100 2779160 :
   Fixed: Fallback to less accurate UTF8 char count if conversion to wstring fails
Jakob P. Liljenberg, Thu Mar 3 17:53:26 2022 +0100 f0ded7a :
   Merge pull request #279 from pallebone/PNewTheme
Peter, Sun Feb 27 12:49:13 2022 -0500 2536a67 :
   Delete HotPurpleTrafficLight.theme
Peter, Sun Feb 27 09:48:31 2022 -0500 0ccc19a :
   Add files via upload
Peter, Sun Feb 27 09:46:00 2022 -0500 82f7294 :
   Add files via upload
Peter, Sun Feb 27 09:36:34 2022 -0500 12d7ba9 :
   Add files via upload
aristocratos, Sun Feb 27 14:24:39 2022 +0100 a98b031 :
   v1.2.4 Bug fixes
mrdotx, Thu Feb 24 11:55:16 2022 +0100 ccfbf18 :
   fix: title_left symbol between auto and zero in the net box is not displayed
Jakob P. Liljenberg, Wed Feb 23 16:52:48 2022 +0100 e793370 :
   Merge pull request #274 from NMrocks/main
NMrocks, Wed Feb 23 21:19:44 2022 +0530 8dd587d :
   Fixed the "cpu responsive" string in btop_config.cpp as well
NMrocks, Wed Feb 23 20:04:46 2022 +0530 b4334a5 :
   Fixed a string in the options menu
Jakob P. Liljenberg, Mon Feb 21 19:37:15 2022 +0100 d3a8421 :
   Merge pull request #273 from mohi001/main
mohammad hashemy, Mon Feb 21 20:45:38 2022 +0330 6823130 :
   add "g" and "G" to vim keys
aristocratos, Sun Feb 20 00:27:03 2022 +0100 7b3d996 :
   Changed: Restrict command line for processes to 1000 characters to fix utf8 conversion errors
aristocratos, Thu Feb 17 22:28:10 2022 +0100 4e7a212 :
   Fixed: Ignore duplicate disks with same mountpoint
aristocratos, Wed Feb 16 08:06:24 2022 +0100 f7860f9 :
   Optimization: Proc::draw()
aristocratos, Tue Feb 15 18:39:36 2022 +0100 1797913 :
   v1.2.3 Bug fixes
aristocratos, Tue Feb 15 07:11:22 2022 +0100 2aeae24 :
   Fixed: string to wstring conversion crash when string is too big
aristocratos, Tue Feb 15 06:58:22 2022 +0100 58846af :
   Fixed: Process tree not redrawing properly
aristocratos, Mon Feb 14 22:21:50 2022 +0100 c88169f :
   Makefile supress find test error
aristocratos, Mon Feb 14 22:19:26 2022 +0100 f630a8d :
   Makefile fixed fallback for outdated find
aristocratos, Mon Feb 14 22:01:21 2022 +0100 49b415d :
   Makefile progress accuracy
aristocratos, Mon Feb 14 12:45:17 2022 +0100 2fa800e :
   Changed: floating_humanizer() now shows fractionals when shortened < 10
Jakob P. Liljenberg, Mon Feb 14 09:19:15 2022 +0100 4a94c83 :
   Merge pull request #266 from bmwiedemann/sort
Jakob P. Liljenberg, Sun Feb 13 19:43:27 2022 +0100 5ceb967 :
   btop.cfg -> btop.conf
Bernhard M. Wiedemann, Sun Feb 13 03:51:13 2022 +0100 f9a7cdb :
   Sort input file list
aristocratos, Sun Feb 13 00:59:00 2022 +0100 18457d1 :
   v1.2.2 Bug fixes
aristocratos, Sun Feb 13 00:51:35 2022 +0100 edece28 :
   Updated example config file
aristocratos, Sun Feb 13 00:44:55 2022 +0100 36a3c7f :
   Added: Clarification on signal screen that number can be manually entered
aristocratos, Sun Feb 13 00:33:20 2022 +0100 baabbe7 :
   Added: Toggle for showing free disk space for privileged or normal users
aristocratos, Sat Feb 12 22:27:11 2022 +0100 d804d3a :
   Changed: Reverted uncolor() back to using regex
aristocratos, Fri Feb 11 20:02:21 2022 +0100 fc15000 :
   v1.2.1 Bug fixes
aristocratos, Fri Feb 11 19:46:29 2022 +0100 ac5ad87 :
   Fixed: Battery meter draw fix
aristocratos, Fri Feb 11 19:30:06 2022 +0100 31555d8 :
   Fixed: Alignment of columns in proc box when wide UTF8 characters are used
aristocratos, Thu Feb 10 08:46:39 2022 +0100 951423d :
   Added: Percentage progress to Makefile
aristocratos, Wed Feb 9 22:30:53 2022 +0100 8e81bf2 :
   Removed: Unnecessary counter for atomic_lock
aristocratos, Wed Feb 9 22:08:27 2022 +0100 08f3957 :
   Fixed: Fx::uncolor() optimization
aristocratos, Wed Feb 9 21:58:41 2022 +0100 897769a :
   Fixed: Logic of Fx::uncolor()
aristocratos, Wed Feb 9 15:45:28 2022 +0100 932bb15 :
   Fixed: Wrong type for max() in msgBox
aristocratos, Wed Feb 9 15:42:09 2022 +0100 c77aee2 :
   Fixed: Terminal resize warning getting stuck
aristocratos, Wed Feb 9 15:39:50 2022 +0100 ec4bc68 :
   Description correction
aristocratos, Wed Feb 9 15:38:48 2022 +0100 1f4b724 :
   Fixed: Text alignment for popup boxes
aristocratos, Wed Feb 9 15:38:05 2022 +0100 3b9144d :
   Fixed: Fx::uncolor not removing all escapes
Jakob P. Liljenberg, Tue Feb 8 11:01:53 2022 +0100 59a1868 :
   Merge pull request #259 from NavigationHazard/main
NavigationHazard, Tue Feb 8 10:36:21 2022 +0100 3887389 :
   Added or and if statement
NavigationHazard, Tue Feb 8 01:24:28 2022 +0100 eebcb08 :
   Best of Both Worlds. Both ways are workings now. Using enter and then moving with arrows or just using arrow down.
NavigationHazard, Sun Feb 6 18:32:35 2022 +0100 0790520 :
   Added Request - Arrow only after use of "f" in Process search
NavigationHazard, Sun Feb 6 18:29:33 2022 +0100 4c5e48c :
   Added Request https://github.com/aristocratos/btop/issues/234
Jakob P. Liljenberg, Mon Jan 31 17:07:06 2022 +0100 5229625 :
   Merge pull request #254 from sebix/patch-1
Sebastian, Mon Jan 31 17:04:13 2022 +0100 f033c57 :
   README: Update openSUSE install section
Jakob P. Liljenberg, Sat Jan 29 10:59:27 2022 +0100 53d2c6e :
   Fixed: bad value for theme[used_end]
Jakob P. Liljenberg, Sat Jan 29 10:51:40 2022 +0100 c7493a3 :
   Merge pull request #253 from kz6fittycent/main
kz6fittycent, Fri Jan 28 17:28:55 2022 -0600 9adaa8f :
   Snap now autoconnects most interfaces
kz6fittycent, Tue Jan 18 21:46:40 2022 -0600 3a4dbc3 :
   Update snapcraft.yaml
aristocratos, Sun Jan 16 14:19:31 2022 +0100 22061fa :
   v1.2.0 FreeBSD Support
aristocratos, Sun Jan 16 13:49:06 2022 +0100 1816f9d :
   Merge branch 'main' of github.com:aristocratos/btop
aristocratos, Sun Jan 16 13:39:50 2022 +0100 989427f :
   Added: Option for base 10 bytes/bits
aristocratos, Sun Jan 16 12:45:27 2022 +0100 f46989a :
   Fixed (freebsd): Process cpu usage, sorting and warnings
aristocratos, Sun Jan 16 12:43:24 2022 +0100 0c18393 :
   Changed: process-per-core defaults to false
Jakob P. Liljenberg, Thu Jan 13 07:52:34 2022 +0100 fcc247b :
   Update bug_report.md
aristocratos, Wed Jan 12 20:49:27 2022 +0100 64cdb44 :
   Fixed: Net speed rollover for 32-bit
Jakob P. Liljenberg, Wed Jan 12 12:20:51 2022 +0100 b6e36a7 :
   Merge pull request #236 from marcoradocchia/main
Marco Radocchia, Wed Jan 12 11:34:14 2022 +0100 60223db :
   Added theme gruvbox_material_dark
Jos Dehaes, Wed Jan 12 09:02:40 2022 +0100 7c7bd10 :
   Merge pull request #238 from joske/main
Jos Dehaes, Wed Jan 12 00:04:26 2022 +0100 3d7bb52 :
   Squashed commit of the following:
Marco Radocchia, Mon Jan 10 14:20:03 2022 +0100 3ea3aaa :
   Added theme gruvbox_material_dark
aristocratos, Thu Dec 30 11:26:23 2021 +0100 56deec4 :
   Fixed: Account for system rolling over net speeds in Net::collect()
aristocratos, Wed Dec 29 14:40:32 2021 +0100 2171789 :
   v1.1.4 Bug fixes
Jakob P. Liljenberg, Wed Dec 29 14:21:13 2021 +0100 03d6dfc :
   Merge pull request #209 from mariogrip/fix-missing-copyright-src
Jakob P. Liljenberg, Wed Dec 29 14:20:49 2021 +0100 c5fd59f :
   Merge pull request #208 from mariogrip/fix-spelling-mistakes
Jakob P. Liljenberg, Wed Dec 29 14:12:40 2021 +0100 3cc19a5 :
   Merge pull request #227 from pg83/main
Anton Samokhvalov, Wed Dec 29 15:47:48 2021 +0300 9faeeab :
   quick_exit for Darwin
Anton Samokhvalov, Wed Dec 29 15:46:15 2021 +0300 c1f540e :
   Revert "no memory leak"
Anton Samokhvalov, Wed Dec 29 15:23:37 2021 +0300 fe66d52 :
   no memory leak
Anton Samokhvalov, Wed Dec 29 13:29:43 2021 +0300 35dccb1 :
   in_avail() can always be zero, it is a optimization opportunity only
aristocratos, Mon Dec 27 10:55:35 2021 +0100 b4e801a :
   Merge branch 'main' of github.com:aristocratos/btop
aristocratos, Mon Dec 27 10:55:17 2021 +0100 5453e82 :
   Changed: From rng::sort() to rng::stable_sort() for more stability
Jos Dehaes, Tue Dec 21 23:24:58 2021 +0100 2ee32bd :
   Merge pull request #222 from joske/main
Jos Dehaes, Tue Dec 21 23:15:02 2021 +0100 96ac114 :
   fix CPU temp fallback on macOS
Jakob P. Liljenberg, Tue Dec 21 18:21:18 2021 +0100 d4f5112 :
   Merge pull request #218 from stwnt/main
stwnt, Sun Dec 19 23:53:01 2021 +0100 0e2517a :
   Create dependency files in build directory when compiling.
Marius Gripsgard, Mon Dec 13 13:58:44 2021 +0100 7a1c88a :
   Fix some missing copyright in src files found by debian lintian
Marius Gripsgard, Mon Dec 13 13:45:22 2021 +0100 de27c58 :
   Fix spelling mistakes caught by debian lintian
aristocratos, Sat Dec 11 18:22:08 2021 +0100 a6d27c1 :
   v1.1.3 New themes + bugfixes
aristocratos, Sat Dec 11 01:06:27 2021 +0100 13a29c3 :
   Fixed: Suspected possibility of very rare stall in Input::clear()
aristocratos, Thu Dec 9 07:10:56 2021 +0100 609dfcc :
   Fixed: Fixed network graph scale int rollover
Jakob P. Liljenberg, Tue Dec 7 23:05:50 2021 +0100 41be7f8 :
   Merge pull request #189 from vtmx/main
vtmx, Tue Dec 7 18:35:32 2021 -0300 b015aa6 :
   Added theme OneDark
aristocratos, Thu Dec 2 21:26:10 2021 +0100 43c4190 :
   Merge pull request #184 from joske/main
Jos Dehaes, Sun Nov 28 21:50:49 2021 +0100 bcc0acf :
   #168 - try again with TC0C smc key
aristocratos, Thu Dec 2 13:01:05 2021 +0100 7d20ab3 :
   Merge pull request #178 from nevack/nevack/macos-homebrew
Dzmitry Neviadomski, Thu Dec 2 14:54:49 2021 +0300 1997c7a :
   Update README.md
aristocratos, Thu Dec 2 09:06:58 2021 +0100 defc419 :
   Merge pull request #180 from pietryszak/gruvbox_dark_v2
Paweł Pietryszak, Thu Dec 2 01:56:14 2021 +0100 5aef22c :
   Create gruvbox_dark_v2.theme
Dzmitry Neviadomski, Thu Dec 2 01:29:58 2021 +0300 7bee001 :
   [macOS] Add Homebrew installation instructions.
aristocratos, Wed Dec 1 15:06:25 2021 +0100 02db6a2 :
   Merge pull request #175 from AlphaNecron/patch-1
Nguyen Thanh Quang, Wed Dec 1 13:09:30 2021 +0700 7f7795d :
   Added `ayu`
kz6fittycent, Sun Nov 28 13:35:08 2021 -0600 c918826 :
   Update snapcraft.yaml
aristocratos, Sat Nov 27 18:52:10 2021 +0100 48cbe68 :
   v1.1.2 Bug fixes
aristocratos, Sat Nov 27 18:24:40 2021 +0100 9465e9b :
   Fixed: coretemp ordering
aristocratos, Sat Nov 27 18:20:30 2021 +0100 1bce1be :
   Merge branch 'sensor-fix'
aristocratos, Sat Nov 27 17:59:46 2021 +0100 50928fc :
   Enabled more verbosity and disabled currently failing builds
aristocratos, Sat Nov 27 17:38:29 2021 +0100 297cb69 :
   Fixed: UTF-8 detection on macos
aristocratos, Sat Nov 27 16:22:37 2021 +0100 538ec20 :
   Fixed: Wrong unit for net_upload and net_download in config menu
aristocratos, Wed Nov 24 21:58:21 2021 +0100 953216f :
   Disabled unused mouse_released event
aristocratos, Mon Nov 22 21:43:40 2021 +0100 e1074cd :
   Mem::collect optimization
Krzysztof Filipek, Wed Nov 17 00:08:05 2021 +0100 5fb484a :
   Fix coretemp iteration due to missing temp9_input (i9-9940X)
Jos Dehaes, Tue Nov 16 21:27:40 2021 +0100 e2668c6 :
   Merge pull request #154 from mgradowski/main
Mikołaj Gradowski, Tue Nov 16 20:40:49 2021 +0100 6ab49d2 :
   fix SISEGV on Mojave
aristocratos, Tue Nov 16 08:41:16 2021 +0100 557bcf5 :
   v1.1.1 OSX build fixes
aristocratos, Tue Nov 16 07:58:42 2021 +0100 19894fe :
   bug_report added instruction for lldb
aristocratos, Tue Nov 16 07:34:09 2021 +0100 7373d24 :
   Continuous build macos switch to BigSur
aristocratos, Tue Nov 16 07:16:55 2021 +0100 cb6ad14 :
   Added support for partial static linking on OSX
aristocratos, Mon Nov 15 21:58:09 2021 +0100 d721c4b :
   Fixed link
aristocratos, Sun Nov 14 01:30:27 2021 +0100 044927b :
   Fixed broken links
aristocratos, Sun Nov 14 01:23:12 2021 +0100 125e3c0 :
   v1.1.0 OSX Support
aristocratos, Sat Nov 13 23:56:29 2021 +0100 0ed8ad7 :
   Fix OSX build on main branch
aristocratos, Sat Nov 13 23:31:01 2021 +0100 c0e17a6 :
   Squashed commit of the following:
aristocratos, Sat Nov 13 21:25:08 2021 +0100 4926acd :
   Makefile OSX and freebsd fixes
aristocratos, Sat Nov 13 21:19:18 2021 +0100 7ca4940 :
   Added OSX fixes for upcoming merge
aristocratos, Tue Nov 9 10:03:37 2021 +0100 81df582 :
   v1.0.24 Bug fixes
aristocratos, Mon Nov 8 20:35:16 2021 +0100 41f1e9e :
   Fixed: SIGINT not cleaning up on exit
aristocratos, Mon Nov 8 19:39:16 2021 +0100 eb19bb3 :
   Fixed: Restore all escape seq mouse modes on exit
aristocratos, Mon Nov 8 19:29:02 2021 +0100 e717606 :
   Changed: Collection ordering
aristocratos, Mon Nov 8 18:58:25 2021 +0100 0201b5f :
   v1.0.23 Bug fixes
aristocratos, Mon Nov 8 18:48:11 2021 +0100 c133997 :
   Fixed: Inconsistent behaviour of "q" key in the menus
aristocratos, Mon Nov 8 12:46:18 2021 +0100 e4ac3d9 :
   Fixed: Stall when clearing input queue on exit and queue is >1
aristocratos, Thu Nov 4 12:44:11 2021 +0100 a6dba19 :
   Fixed: Vim keys menu lists selection
aristocratos, Thu Nov 4 12:36:23 2021 +0100 9af8e00 :
   Fixed: Config parser missing first value when not including version header
aristocratos, Wed Nov 3 22:11:31 2021 +0100 4516922 :
   v1.0.22 Fixed 32-bit
aristocratos, Wed Nov 3 22:01:49 2021 +0100 2107f70 :
   Updated README.md
aristocratos, Wed Nov 3 21:01:34 2021 +0100 1569980 :
   Fixed: Bad values for disks and network on 32-bit
aristocratos, Wed Nov 3 16:36:45 2021 +0100 f083c0a :
   Merge pull request #135 from lu4/patch-1
Denis Yaremov, Wed Nov 3 14:04:31 2021 +0200 77244dd :
   Update to Readme.md
aristocratos, Tue Nov 2 21:18:44 2021 +0100 6383ef8 :
   v1.0.21 Security and bug fixes
aristocratos, Tue Nov 2 21:10:41 2021 +0100 0f566ae :
   Fixed: Security issue when running with SUID bit set
aristocratos, Sat Oct 30 18:17:18 2021 +0200 98ae5e8 :
   Updated README.md
aristocratos, Wed Oct 27 20:19:43 2021 +0200 a6fd835 :
   Added: / as alternative bind for filter
aristocratos, Tue Oct 26 23:50:28 2021 +0200 9ecea2c :
   Fixed: Removed extra spaces in cpu name
aristocratos, Tue Oct 26 23:41:40 2021 +0200 587005f :
   v1.0.20 Bug fixes
aristocratos, Tue Oct 26 22:56:19 2021 +0200 36c74fb :
   Changed: Regex for Fx::uncolor() changed to string search and replace
aristocratos, Tue Oct 26 17:31:37 2021 +0200 ce2a279 :
   Merge branch 'main' of github.com:aristocratos/btop
aristocratos, Tue Oct 26 17:31:13 2021 +0200 1e374b1 :
   Removed bad code
aristocratos, Tue Oct 26 13:56:04 2021 +0200 383932a :
   Merge pull request #114 from adnanpri/main
CS Adnan, Tue Oct 26 11:26:52 2021 +0600 73a0ad7 :
   remove log warning + extra if for k10temp
CS Adnan, Mon Oct 25 00:17:54 2021 +0600 602cef8 :
   add support for AMD Ryzen 5000 temp sensor
aristocratos, Sun Oct 24 01:16:15 2021 +0200 496283e :
   Moved flags not needing to be tested
aristocratos, Thu Oct 21 16:54:18 2021 +0200 a1ba019 :
   v1.0.19 Improved build system
aristocratos, Thu Oct 21 08:16:52 2021 +0200 a046f75 :
   Fixed: Makefile now tests compiler flag compatibility
aristocratos, Tue Oct 19 17:29:57 2021 +0200 a0ee404 :
   v1.0.18 Fix for build platform detection
aristocratos, Mon Oct 18 20:55:06 2021 +0200 f15c2fd :
   Fixed: Makefile g++ -dumpmachine failure to get platform on some distros
aristocratos, Mon Oct 18 18:56:27 2021 +0200 997bfd2 :
   v1.0.17 New features and bug fixes
aristocratos, Sun Oct 17 22:55:36 2021 +0200 7d89c75 :
   FIxed: Battery error if non existant battery named is entered
aristocratos, Sun Oct 17 22:48:47 2021 +0200 368ed27 :
   Merge pull request #98 from jan-guenter/svg-logo
aristocratos, Sun Oct 17 22:26:43 2021 +0200 4cacdf2 :
   Added: Displayed battery selectable in options menu
Jan Günter, Sun Oct 17 20:54:02 2021 +0200 ebcfd1c :
   added a SVG version of the logo recreated logo.png form logo.svg for optimized png data
aristocratos, Sun Oct 17 20:25:46 2021 +0200 dfef4fd :
   Update README.md
aristocratos, Sun Oct 17 17:51:23 2021 +0200 9da58e0 :
   Trigger on push to branch "main" not "master"
aristocratos, Sun Oct 17 17:48:31 2021 +0200 be9086c :
   Swap triggers around to see if it triggers...
aristocratos, Sun Oct 17 17:14:18 2021 +0200 7adbcbb :
   Merge pull request #97 from jan-guenter/battery-detection-issue-94
Jan Günter, Sun Oct 17 17:06:18 2021 +0200 22297f7 :
   added try catch block as suggested in review https://github.com/aristocratos/btop/pull/97#discussion_r730428842
Jan Günter, Sun Oct 17 16:31:25 2021 +0200 966c9f5 :
   improved battery detection - new considering all power supplies (instead of path name filtering) - only consider power supplies that are currently present - only consider power supplies of type Batter or UPS
aristocratos, Sun Oct 17 12:11:25 2021 +0200 2ee1333 :
   Changed: Removed microblaze platform from build targets because of issue with atomics
aristocratos, Sun Oct 17 01:59:07 2021 +0200 8a1211e :
   Merge pull request #92 from jan-guenter/musl-build
Jan Günter, Sun Oct 17 01:45:26 2021 +0200 8162707 :
   readability change request in review https://github.com/aristocratos/btop/pull/92#discussion_r730321422
Jan Günter, Sun Oct 17 01:02:37 2021 +0200 ac1b6aa :
   adopting review comments https://github.com/aristocratos/btop/pull/92#discussion_r730320662 https://github.com/aristocratos/btop/pull/92#discussion_r730320819
aristocratos, Sat Oct 16 23:52:06 2021 +0200 1601422 :
   Changed: Reverted mutexes back to custom atomic bool based locks
Jan Günter, Sat Oct 16 23:51:12 2021 +0200 d3c85da :
   removed temporary trigger for testing on branch
Jan Günter, Sat Oct 16 15:47:48 2021 +0200 347e135 :
   updated README.md to reflect Makefile changes
Jan Günter, Sat Oct 16 15:37:40 2021 +0200 bca3521 :
   updated makefile to add cf-protection for i*86 builds
Jan Günter, Sat Oct 16 15:16:44 2021 +0200 b83f58a :
   updated workflow - removed ARCH parameter in favor of auto deection - added STRIP flag
Jan Günter, Sat Oct 16 15:15:56 2021 +0200 7d4c2bd :
   updated Makefile - added platform and arch detection from CXX machine tuple - add PLATFORM_LC, a lowercase version of PLATFORM, for case insensitive comparison - added bool flag STRIP to force -s linker flag - turned linker warnings into errors in case of STATIC to fail in case of glibc static linking issues - bumped version to 1.3
Jan Günter, Sat Oct 16 13:03:09 2021 +0200 f053509 :
   changed workflow to build binaries using musl
Jan Günter, Sat Oct 16 13:02:06 2021 +0200 c8a7c0d :
   changed static build behavior restrict getpwuid usage only in static builds with glibc
Jan Günter, Sat Oct 16 13:00:23 2021 +0200 19ff463 :
   added yml definitions to .editorconfig
aristocratos, Sat Oct 16 11:14:11 2021 +0200 3e936bf :
   v1.0.16 Bug fixes
aristocratos, Sat Oct 16 10:36:59 2021 +0200 070691d :
   Merge pull request #88 from jlopezcur/fix_vim-keys-typo
Javier López Úbeda, Sat Oct 16 03:29:39 2021 +0200 084e231 :
   fix: when vim keys [k]ill should be [K]ill
aristocratos, Sat Oct 16 02:23:44 2021 +0200 e5ccf7d :
   Update bug_report.md
aristocratos, Fri Oct 15 08:32:37 2021 +0200 e20258e :
   Added: Try to restart secondary thread in case of stall and addiotional error checks for ifstream in Proc::collect()
aristocratos, Thu Oct 14 21:44:39 2021 +0200 832699f :
   Changed: Removed unnecessary async threads in Runner thread
aristocratos, Thu Oct 14 09:22:14 2021 +0200 79a030c :
   Changed: Reverted thread mutex lock to atomic bool with wait and timeout
aristocratos, Wed Oct 13 20:54:36 2021 +0200 804fe60 :
   Fixed: Swapped from atomic bool spinlocks to mutexes to fix rare deadlock
aristocratos, Tue Oct 12 20:09:35 2021 +0200 a1bda5f :
   Fixed: atomic_wait() and atomic_lock{} use cpu pause instructions instead of thread sleep
aristocratos, Tue Oct 12 17:39:21 2021 +0200 903ab4c :
   v1.0.15 Vim keys
aristocratos, Tue Oct 12 17:34:52 2021 +0200 2df9b58 :
   Added: Toggle in options for enabling directional vim keys "h,j,k,l"
aristocratos, Tue Oct 12 16:56:47 2021 +0200 07145f9 :
   Fixed: Removed unneeded lock in Runner::run()
aristocratos, Tue Oct 12 16:49:10 2021 +0200 4b4bac7 :
   Changed: Limit atomic_wait() to 1000ms to fix rare stall
aristocratos, Fri Oct 8 09:18:31 2021 +0200 27c58df :
   Changelog v1.0.15
aristocratos, Wed Oct 6 18:06:05 2021 +0200 d96fdd7 :
   Fixed: Extra "root" partition when running in snap
kz6fittycent, Wed Oct 6 10:51:04 2021 -0500 6a129e2 :
   - ADDFLAGS="-D SNAPPED"
aristocratos, Wed Oct 6 17:25:22 2021 +0200 132f4e6 :
   Ignore tags and other branches
aristocratos, Wed Oct 6 17:16:49 2021 +0200 dee69b5 :
   News
aristocratos, Wed Oct 6 17:11:10 2021 +0200 2d013e8 :
   v1.0.14 Bug fixes and start of OSX development
aristocratos, Wed Oct 6 12:36:54 2021 +0200 3a20e55 :
   Update changelog
aristocratos, Wed Oct 6 11:25:10 2021 +0200 dbcd12b :
   Fixed: snap root disk and changed to compiler flags instead of env variables for detection
aristocratos, Wed Oct 6 10:54:19 2021 +0200 ad0b3d6 :
   Added notice about LDAP and static compilation
aristocratos, Wed Oct 6 10:47:24 2021 +0200 69c5eba :
   Fixed: Uid -> User fallback to getpwuid() if failure for non static builds
aristocratos, Wed Oct 6 08:19:33 2021 +0200 8c0b815 :
   Ignore changes to osx or freebsd
aristocratos, Tue Oct 5 21:06:19 2021 +0200 9cf7a77 :
   Upcoming in v1.0.14
aristocratos, Tue Oct 5 20:57:33 2021 +0200 721da8c :
   Fixed missing colon
aristocratos, Tue Oct 5 20:55:15 2021 +0200 acdefb7 :
   Change to only build if there are changes to source files or the build script itself
aristocratos, Tue Oct 5 20:30:56 2021 +0200 ef13446 :
   Merge pull request #72 from ShrirajHegde/main
Shriraj Hegde, Tue Oct 5 23:50:47 2021 +0530 8d4afa3 :
   Upload each architecture binary separately
Shriraj Hegde, Tue Oct 5 19:26:15 2021 +0530 222c387 :
   Fix riscv64 cross compiler package name Move git init command
Shriraj Hegde, Tue Oct 5 18:58:06 2021 +0530 d311651 :
   Fix git fix Stopping at filesystem boundary
Shriraj Hegde, Tue Oct 5 18:54:51 2021 +0530 0fedb9e :
   Add more architectures to Continuous build
Shriraj Hegde, Tue Oct 5 18:34:51 2021 +0530 7ab51da :
   create 'multiarch_bin' directory
Shriraj Hegde, Tue Oct 5 18:31:53 2021 +0530 8952694 :
   Fix git hash in binary name
Shriraj Hegde, Tue Oct 5 18:25:57 2021 +0530 2e058b1 :
   Store built binaries in 'multiarch_bin'
Shriraj Hegde, Tue Oct 5 18:23:12 2021 +0530 0b00228 :
   Add distclean after builds
Shriraj Hegde, Tue Oct 5 17:46:08 2021 +0530 c4f8cf2 :
   Add aarch64 to workflow
Shriraj Hegde, Tue Oct 5 17:31:10 2021 +0530 0c7357d :
   Fix README.md
Shriraj Hegde, Tue Oct 5 16:43:31 2021 +0530 c84dcf3 :
   Add Linux CI badge
Shriraj Hegde, Tue Oct 5 16:26:22 2021 +0530 4dd8567 :
   fix (GIT_DISCOVERY_ACROSS_FILESYSTEM not set)
Shriraj Hegde, Tue Oct 5 16:23:22 2021 +0530 28ab3d9 :
   Rename binary to git hash
Shriraj Hegde, Tue Oct 5 16:03:41 2021 +0530 72ed836 :
   Use ubuntu 21.04 container
Shriraj Hegde, Tue Oct 5 14:12:36 2021 +0530 50a434d :
   Add workflow for continuous build
Shriraj Hegde, Tue Oct 5 14:05:06 2021 +0530 76536c5 :
   Track .github directory
aristocratos, Tue Oct 5 09:18:04 2021 +0200 02fcb8c :
   Changed: Total system memory is checked at every update instead of once at start
aristocratos, Tue Oct 5 07:17:16 2021 +0200 a139869 :
   Merge pull request #69 from kz6fittycent/main
aristocratos, Tue Oct 5 07:15:39 2021 +0200 fea5f0f :
   Merge pull request #71 from ShrirajHegde/main
Shriraj Hegde, Tue Oct 5 00:27:15 2021 +0530 665a8b1 :
   Add 'info' option in help Might be helpful when more platforms are compatible
aristocratos, Sat Oct 2 22:43:49 2021 +0200 f9505a4 :
   v1.0.13 Better symbol compatibility
aristocratos, Sat Oct 2 22:39:45 2021 +0200 585bb7b :
   Changed: Graph empty symbol is now regular whitespace
aristocratos, Fri Oct 1 20:57:40 2021 +0200 6c77620 :
   Merge branch 'main' of github.com:aristocratos/btop
aristocratos, Fri Oct 1 20:46:05 2021 +0200 3a4a9d0 :
   v1.0.12 Bug fixes
aristocratos, Fri Oct 1 20:42:41 2021 +0200 5082ae5 :
   Merge pull request #55 from 13werwolf13/main
aristocratos, Fri Oct 1 15:24:41 2021 +0200 a15f961 :
   Updated Prerequisites
aristocratos, Thu Sep 30 22:56:14 2021 +0200 8d393b8 :
   Version > Release
aristocratos, Thu Sep 30 22:51:05 2021 +0200 ae7b8b7 :
   Version bump
aristocratos, Thu Sep 30 22:49:14 2021 +0200 304b9af :
   Fixed: Exception handling for faulty net download/upload speed
aristocratos, Thu Sep 30 22:48:03 2021 +0200 d226e61 :
   Fixed: Cpu percent formatting if over 10'000
aristocratos, Wed Sep 29 21:24:33 2021 +0200 a246c09 :
   v1.0.11 Bug fixes
aristocratos, Wed Sep 29 20:20:45 2021 +0200 cd065cf :
   Merge branch 'night-owl-theme'
kz6fittycent, Wed Sep 29 06:47:52 2021 -0500 78cf6a4 :
   Update snapcraft.yaml
kz6fittycent, Wed Sep 29 06:33:24 2021 -0500 98af3bb :
   Update snapcraft.yaml
aristocratos, Wed Sep 29 11:43:37 2021 +0200 a49b8f9 :
   Changed: Init will continue if UTF-8 locale are detected even if it fails to set the locale
aristocratos, Wed Sep 29 08:16:49 2021 +0200 c70667e :
   Fixed: UTF8 set LANG and LC_ALL to empty before UTF8 search and fixed empty error msg on exit before signal handler init
Дмитрий Марков, Wed Sep 29 09:50:16 2021 +0500 b08f473 :
   add opensuse repo
aristocratos, Tue Sep 28 21:54:51 2021 +0200 ab013b9 :
   Changed: Snap new / mountpoint detection
aristocratos, Tue Sep 28 21:01:50 2021 +0200 06cac67 :
   Fixed: Get real / mountpoint when running inside snap
aristocratos, Tue Sep 28 16:58:08 2021 +0200 ff6d1d6 :
   Changed: atomic_wait to use while loop instead of atom.wait() because of random stalls
aristocratos, Tue Sep 28 16:37:02 2021 +0200 5fba94c :
   v1.0.10 Bug fixes
kz6fittycent, Tue Sep 28 06:38:22 2021 -0500 7bdfc6e :
   Update snapcraft.yaml
aristocratos, Tue Sep 28 13:08:45 2021 +0200 8296c1a :
   Changed: UTF8 detection search if LANG is set but fails to set
aristocratos, Tue Sep 28 09:26:50 2021 +0200 dc914aa :
   Fixed: Cpu percent cutting off above 1000 percent and added scaling with "k" prefix above 10'000
aristocratos, Tue Sep 28 09:05:16 2021 +0200 683c575 :
   Changed: Moved signal handler setup to later in init
aristocratos, Tue Sep 28 08:56:09 2021 +0200 975db8d :
   Merge branch 'main' of github.com:aristocratos/btop
aristocratos, Tue Sep 28 08:54:22 2021 +0200 1e84265 :
   Changed: Cpu temp values check for existing values
zack kourouma, Mon Sep 27 18:40:26 2021 -0700 a1fa532 :
   More readable inactive/disabled text
zack kourouma, Mon Sep 27 12:27:17 2021 -0700 73119d3 :
   Add night-owl theme
kz6fittycent, Mon Sep 27 09:17:54 2021 -0500 245fac8 :
   Update snapcraft.yaml
aristocratos, Mon Sep 27 13:00:04 2021 +0200 4f3db69 :
   Update bug_report.md
aristocratos, Mon Sep 27 12:51:04 2021 +0200 a09ad06 :
   Added: Check for empty percentage deques
aristocratos, Mon Sep 27 11:59:10 2021 +0200 01dfd27 :
   Changed: Moved check for valid terminal dimensions to before platform init
aristocratos, Mon Sep 27 11:01:56 2021 +0200 65c62ce :
   Changed: Stop second thread before updating terminal size variables
aristocratos, Mon Sep 27 09:18:06 2021 +0200 9233b3c :
   Merge branch 'main' of github.com:aristocratos/btop
aristocratos, Mon Sep 27 09:16:39 2021 +0200 d76f258 :
   Added: Wait for terminal size properties to be available at start
aristocratos, Mon Sep 27 07:15:57 2021 +0200 efed7f2 :
   Merge pull request #46 from kz6fittycent/main
aristocratos, Mon Sep 27 07:15:07 2021 +0200 aa7a042 :
   Merge branch 'main' into main
kz6fittycent, Sun Sep 26 21:14:28 2021 -0500 12ba324 :
   Swapping stage and build
kz6fittycent, Sun Sep 26 21:01:31 2021 -0500 5c4a98a :
   Build works
kz6fittycent, Sun Sep 26 20:17:57 2021 -0500 14f0021 :
   Attempt w/o build-packages
aristocratos, Sun Sep 26 23:35:21 2021 +0200 3ebd475 :
   Merge branch 'main' of github.com:aristocratos/btop
aristocratos, Sun Sep 26 23:34:39 2021 +0200 ced6dd6 :
   Changed to static build
kz6fittycent, Sun Sep 26 16:01:50 2021 -0500 44a50fc :
   Update snapcraft.yaml
aristocratos, Sun Sep 26 22:38:31 2021 +0200 e35ea1c :
   Merge pull request #43 from kz6fittycent/main
kz6fittycent, Sun Sep 26 15:25:25 2021 -0500 589c19d :
   Updated readme
aristocratos, Sun Sep 26 22:05:49 2021 +0200 4b1af4f :
   Fixed instructions for snap connections
kz6fittycent, Sun Sep 26 15:04:21 2021 -0500 1d4d2f8 :
   Update snapcraft.yaml
kz6fittycent, Sun Sep 26 15:01:44 2021 -0500 7594d49 :
   Update snapcraft.yaml
aristocratos, Sun Sep 26 21:12:46 2021 +0200 d7b5aa9 :
   Added repology status
aristocratos, Sun Sep 26 21:07:59 2021 +0200 9f87095 :
   Merge pull request #42 from kz6fittycent/main
kz6fittycent, Sun Sep 26 13:42:35 2021 -0500 3349fe8 :
   Update README.md
kz6fittycent, Sun Sep 26 12:54:58 2021 -0500 9156659 :
   Update snapcraft.yaml
kz6fittycent, Sun Sep 26 12:51:36 2021 -0500 5d486f3 :
   Update .gitignore
kz6fittycent, Sun Sep 26 12:46:27 2021 -0500 ca57a99 :
   Update README.md
aristocratos, Sun Sep 26 19:35:09 2021 +0200 2ed28fc :
   Changed: use_fstab for disks collection now defaults to true
aristocratos, Sun Sep 26 13:46:51 2021 +0200 68d88e6 :
   v1.0.9 Bug fixes
aristocratos, Sun Sep 26 10:14:38 2021 +0200 f8cc4c4 :
   Fixed: Freeze on cin.ignore()
aristocratos, Sun Sep 26 01:03:57 2021 +0200 528df4d :
   Added: ifstream check and try-catch for stod() in Tools::system_uptime()
aristocratos, Sun Sep 26 00:23:46 2021 +0200 5ae05f0 :
   v1.0.8 Bug fixes
aristocratos, Sun Sep 26 00:17:39 2021 +0200 771a200 :
   Changed: Only log tty name if known
aristocratos, Sun Sep 26 00:06:00 2021 +0200 4197f0b :
   Added entry for gdb backtrace in bug report
aristocratos, Sat Sep 25 23:45:59 2021 +0200 741778b :
   Fixed: Missing NULL check for ttyname
aristocratos, Sat Sep 25 11:57:20 2021 +0200 cb83453 :
   Added example for ADDFLAGS  when compiling
aristocratos, Sat Sep 25 11:48:33 2021 +0200 3b4a7c1 :
   Changed: Only look for g++-11 if CXX=g++
aristocratos, Sat Sep 25 11:35:50 2021 +0200 94feb4e :
   More fixes for UTF-8 detection
aristocratos, Sat Sep 25 00:10:18 2021 +0200 5273a62 :
   Fixed: Additional NULL checks in UTF-8 detection
kz6fittycent, Fri Sep 24 16:58:26 2021 -0500 3114c6c :
   Update snapcraft.yaml
aristocratos, Fri Sep 24 22:35:13 2021 +0200 ee8109e :
   v1.0.7 Bug fixes
aristocratos, Fri Sep 24 22:24:15 2021 +0200 2aca963 :
   Fixed: Scrollbar position incorrect in small lists and selection not working when filtering
aristocratos, Fri Sep 24 22:07:01 2021 +0200 a35a2c6 :
   Fixed: Cores not constrained to cpu box and core numbers above 100 cut off
aristocratos, Fri Sep 24 20:58:58 2021 +0200 37de946 :
   Fixed: Crash when opening menu at too small size
kz6fittycent, Thu Sep 23 15:21:46 2021 -0500 62e016f :
   Update snapcraft.yaml
kz6fittycent, Thu Sep 23 15:10:03 2021 -0500 abdc2f4 :
   Update snapcraft.yaml
kz6fittycent, Thu Sep 23 15:03:53 2021 -0500 93c8d1b :
   Update snapcraft.yaml
kz6fittycent, Thu Sep 23 14:59:18 2021 -0500 18f7e13 :
   Update snapcraft.yaml
kz6fittycent, Thu Sep 23 14:52:58 2021 -0500 f4afb04 :
   trying override build
aristocratos, Thu Sep 23 21:00:56 2021 +0200 78c417b :
   v1.0.6 Bug fixes
aristocratos, Thu Sep 23 12:44:46 2021 +0200 5e6d1ba :
   Fixed: Processes not completely hidden when collapsed in tree mode
aristocratos, Thu Sep 23 07:43:12 2021 +0200 afc2288 :
   Merge pull request #23 from CoryM/main
Cory Marshall, Wed Sep 22 22:16:11 2021 -0400 fcd27ab :
   Ignore hidden directories like .vscode
aristocratos, Wed Sep 22 19:32:00 2021 +0200 1f5aebe :
   Fixed: Check that getenv("LANG") is not NULL in UTF-8 check
aristocratos, Wed Sep 22 17:38:06 2021 +0200 bc0c109 :
   Changed wrong filename error.log > btop.log
aristocratos, Wed Sep 22 17:31:17 2021 +0200 2f27d9f :
   v1.0.5 Bug fixes
aristocratos, Wed Sep 22 17:27:25 2021 +0200 eedab30 :
   Fixed: UTF-8 check crashing if LANG was set to non existant locale
aristocratos, Wed Sep 22 12:55:39 2021 +0200 b818d01 :
   Fixed: Sizing constraints bug on start and boxes can be toggled from size error screen
aristocratos, Wed Sep 22 10:50:14 2021 +0200 299f564 :
   Merge pull request #17 from davised/main
aristocratos, Wed Sep 22 09:10:52 2021 +0200 078daf4 :
   Fixed: Load AVG sizing when hiding temperatures
Edward Davis, Tue Sep 21 13:05:03 2021 -0700 b9e470f :
   Add make install missing reset escape sequence.
aristocratos, Tue Sep 21 21:27:46 2021 +0200 5ce7240 :
   v1.0.4 Bug fixes
aristocratos, Tue Sep 21 21:24:58 2021 +0200 6d11c8b :
   Fixed: Use /proc/pid/statm if RSS memory from /proc/pid/stat is faulty
aristocratos, Tue Sep 21 18:57:16 2021 +0200 8288d7c :
   Added QUIET variable "true/false" to suppress most messages during build
aristocratos, Tue Sep 21 17:39:49 2021 +0200 19e9a18 :
   v1.0.3 Bug fixes
aristocratos, Tue Sep 21 17:21:42 2021 +0200 8d3e457 :
   Fixed: stoi 0 literal pointer to nullptr and added more clamping for gradient array access
aristocratos, Tue Sep 21 08:40:44 2021 +0200 fe08903 :
   Added binary and compiler questions to bug report
aristocratos, Mon Sep 20 18:32:08 2021 +0200 026a931 :
   v1.0.2 Bug fixes
aristocratos, Mon Sep 20 18:28:45 2021 +0200 bb57281 :
   Fixed: Options menu page selection alignment
aristocratos, Mon Sep 20 18:20:30 2021 +0200 af465b5 :
   Fixed: Menu size and preset size issues and added warnings for small terminal size
aristocratos, Mon Sep 20 17:23:34 2021 +0200 85f945a :
   Fixed: Color gradient array out of bounds, added clamp 0-100 for cpu percent values
aristocratos, Mon Sep 20 17:02:37 2021 +0200 feaf2cd :
   Log name
aristocratos, Sun Sep 19 20:14:54 2021 +0200 b552e75 :
   Added minimum kernel version for release binaries
aristocratos, Sun Sep 19 16:05:37 2021 +0200 d2ee28f :
   Added link to latest release
aristocratos, Sun Sep 19 06:23:02 2021 +0200 748095c :
   Changes in v1.0.2
aristocratos, Sun Sep 19 06:18:09 2021 +0200 f0d3cc5 :
   Fixed ARCH detection in Makefile
kz6fittycent, Sat Sep 18 17:58:20 2021 -0500 60f6c89 :
   Update snapcraft.yaml
kz6fittycent, Sat Sep 18 17:44:29 2021 -0500 bfa0e16 :
   Update snapcraft.yaml
kz6fittycent, Sat Sep 18 15:28:43 2021 -0500 0f81dc6 :
   Update snapcraft.yaml
kz6fittycent, Sat Sep 18 15:22:19 2021 -0500 b4eb23f :
   Update snapcraft.yaml
kz6fittycent, Sat Sep 18 15:03:11 2021 -0500 c83e807 :
   Update snapcraft.yaml
kz6fittycent, Sat Sep 18 14:51:56 2021 -0500 231646d :
   Update snapcraft.yaml
kz6fittycent, Sat Sep 18 14:39:17 2021 -0500 8bc4b15 :
   Update snapcraft.yaml
kz6fittycent, Sat Sep 18 14:30:16 2021 -0500 a8bd4cb :
   Update snapcraft.yaml
kz6fittycent, Sat Sep 18 14:19:50 2021 -0500 11a8435 :
   Update snapcraft.yaml
aristocratos, Sat Sep 18 20:31:47 2021 +0200 9319d0c :
   Missing comma...
aristocratos, Sat Sep 18 20:20:02 2021 +0200 91025a8 :
   v1.0.1 Bug fixes
kz6fittycent, Sat Sep 18 12:48:21 2021 -0500 1c4fbdc :
   Create snapcraft.yaml
aristocratos, Sat Sep 18 18:03:58 2021 +0200 8ea296d :
   Upcoming changes in v1.0.1
aristocratos, Sat Sep 18 17:59:45 2021 +0200 fca1d8c :
   Fixed UTF-8 check to include UTF8 and added thread started check before joining in clean_quit()
aristocratos, Sat Sep 18 17:33:02 2021 +0200 3b80ef9 :
   Command line options fixup
aristocratos, Sat Sep 18 17:30:27 2021 +0200 e170971 :
   Merge pull request #2 from purinchu/main
Michael Pyne, Sat Sep 18 10:58:03 2021 -0400 97726f3 :
   Fix documentation of --utf-force in README and --help.
aristocratos, Sat Sep 18 15:30:42 2021 +0200 480b421 :
   v1.0.0 First release
aristocratos, Sat Sep 18 15:26:56 2021 +0200 9b6f362 :
   Added preset selection as program argument
aristocratos, Sat Sep 18 14:42:53 2021 +0200 33884e2 :
   Fixed variables to support 32-bit and added preset selection
aristocratos, Sat Sep 18 02:13:35 2021 +0200 cffa303 :
   Added battery info in Cpu::draw()
aristocratos, Fri Sep 17 22:40:29 2021 +0200 07c468a :
   Testing
aristocratos, Fri Sep 17 22:08:39 2021 +0200 34fd7ec :
   Formatting
aristocratos, Fri Sep 17 21:50:14 2021 +0200 4ea204a :
   Formatting
aristocratos, Fri Sep 17 15:26:05 2021 +0200 ca3e0da :
   Merge branch 'main' of github.com:aristocratos/btop
aristocratos, Fri Sep 17 15:25:16 2021 +0200 c11b85b :
   v0.9.0 Test release
aristocratos, Fri Sep 17 14:58:00 2021 +0200 294c309 :
   v0.9.0 Test release
aristocratos, Fri Sep 17 14:25:54 2021 +0200 650df9a :
   Added collect for battery
aristocratos, Sun Sep 12 15:58:23 2021 +0200 8583a8a :
   Added menu system
aristocratos, Wed Sep 1 21:40:13 2021 +0200 db96a20 :
   Added menus and boxes for signal sending
aristocratos, Sat Aug 28 22:28:23 2021 +0200 5dcc1b7 :
   Fixed logo again...
aristocratos, Sat Aug 28 22:13:03 2021 +0200 d79fef3 :
   Fixed logo
aristocratos, Mon Aug 23 22:52:52 2021 +0200 214d896 :
   Moving stuff around
aristocratos, Mon Aug 23 17:35:27 2021 +0200 03f64c2 :
   Added btop_platform.cpp, moved btop_linux.cpp to btop_linux.hpp and updated robin_hood.h
aristocratos, Sun Aug 22 21:39:25 2021 +0200 b2e0707 :
   Added terminal size limit warnings and fixed net auto sizing
aristocratos, Sun Aug 22 16:04:01 2021 +0200 3f27fb2 :
   Added clock, updated makefile and switch to using semaphore to trigger _runner thread
aristocratos, Tue Aug 17 22:33:21 2021 +0200 8bd97b4 :
   Fixed memory leak and greatly reduced general memory usage
aristocratos, Sun Aug 15 23:20:55 2021 +0200 7ab765b :
   Added Net::collect(), Net::draw() and input mappings for net
aristocratos, Thu Aug 12 22:25:18 2021 +0200 ee073f6 :
   Added some comments and changed some defaults
aristocratos, Wed Aug 11 23:21:33 2021 +0200 ced3d47 :
   Removed bad goto and added 100 microseconds sleep in thread manager loop to spare some cpu cycles
aristocratos, Wed Aug 11 20:25:11 2021 +0200 f9ed675 :
   Added bitmask enums and fixed signed to unsigned comparisons
aristocratos, Wed Aug 11 08:00:13 2021 +0200 d56fe4c :
   Runner::_runner() async threads and bitmask loop
aristocratos, Tue Aug 10 20:20:33 2021 +0200 acb2083 :
   Added mem and disks
aristocratos, Wed Aug 4 00:11:50 2021 +0200 95db0ea :
   Formatting and fixes
aristocratos, Tue Aug 3 23:47:46 2021 +0200 102ed61 :
   Added cpu temperature functionality
aristocratos, Thu Jul 29 23:40:56 2021 +0200 e33b4b7 :
   Added Cpu::collect(), Cpu::get_cpuName() and Cpu::getHz()
aristocratos, Mon Jul 26 01:06:34 2021 +0200 3a4f334 :
   Added proc info box and changed Proc::collect() to reuse old data when changing filters or sorting etc.
aristocratos, Sat Jul 24 02:13:26 2021 +0200 9ee9f32 :
   Added functionality for text editing and slight modifications to default theme colors
aristocratos, Fri Jul 23 01:41:00 2021 +0200 f8005ae :
   Added mouse support and graphs for processes in proc box
aristocratos, Wed Jul 21 03:30:41 2021 +0200 5460ed5 :
   Removed gcc 10 instructions
aristocratos, Wed Jul 21 03:21:10 2021 +0200 4dab58a :
   removed duplicate
aristocratos, Wed Jul 21 03:17:34 2021 +0200 5d4e2ce :
   Loads...
aristocratos, Sun Jul 18 18:04:49 2021 +0200 77ef41d :
   Small changesd
aristocratos, Sun Jul 18 15:44:32 2021 +0200 0c1feb9 :
   Added Input::process for input actions and Runner:: namespace for multithreading collection and drawing
aristocratos, Thu Jul 15 23:49:16 2021 +0200 1121978 :
   Proc::draw() and SIGWINCH resizing handler
aristocratos, Mon Jul 5 22:18:58 2021 +0200 e3b297e :
   Proc::draw gradients
aristocratos, Sun Jul 4 22:02:31 2021 +0200 1ff9e21 :
   Added Draw::calcSizes() and Proc::draw()
aristocratos, Sun Jul 4 01:18:48 2021 +0200 ad58642 :
   Added Proc::_collect_details for process info box collection
aristocratos, Wed Jun 30 22:28:12 2021 +0200 3634633 :
   Proc::collect() optimization and started on Proc::_collect_details()
aristocratos, Sun Jun 27 22:13:32 2021 +0200 331665d :
   Fixed filtering for processes tree view
aristocratos, Sun Jun 27 01:19:57 2021 +0200 db3bf06 :
   proc/[pid]/stat collection improvement
aristocratos, Fri Jun 25 23:58:19 2021 +0200 36f0264 :
   Added theme loadfile function
aristocratos, Tue Jun 22 22:06:31 2021 +0200 b4d223c :
   Small optimization for collect
aristocratos, Tue Jun 22 19:19:14 2021 +0200 f3628a9 :
   added str_to_upper and str_to_lower
aristocratos, Mon Jun 21 22:52:55 2021 +0200 c222805 :
   Why not and or
aristocratos, Sun Jun 20 22:07:04 2021 +0200 3a783ae :
   Added themes and 16 color mode
aristocratos, Sun Jun 20 00:49:13 2021 +0200 61694f7 :
   More reorganizing
aristocratos, Sun Jun 20 00:04:02 2021 +0200 470bca6 :
   More efficient proc sorting
aristocratos, Sat Jun 19 22:48:31 2021 +0200 3e92a5e :
   More file reorganizations
aristocratos, Sat Jun 19 14:57:27 2021 +0200 d459d08 :
   File reorganization and more efficient build
aristocratos, Sun Jun 13 23:12:11 2021 +0200 ba481d0 :
   Added processes tree view
aristocratos, Sat Jun 12 18:49:27 2021 +0200 c4b55c7 :
   Added config file loader
aristocratos, Sat Jun 12 00:35:15 2021 +0200 63a286d :
   Added config file descriptions and config write function
aristocratos, Wed Jun 9 19:47:49 2021 +0200 10a8bfe :
   Added signal handler
aristocratos, Sun Jun 6 23:12:01 2021 +0200 dbb5a05 :
   Fixed broken uresize()
aristocratos, Sun Jun 6 22:49:24 2021 +0200 9667fe6 :
   Change Proc::collect to return a reference to Proc::current_procs instead of a new vector
aristocratos, Sun Jun 6 01:41:36 2021 +0200 deec8f2 :
   Config thread safety
aristocratos, Sat Jun 5 01:41:24 2021 +0200 43e3c4f :
   Optimizations and fixes
aristocratos, Wed Jun 2 21:33:26 2021 +0200 ce34cbb :
   Small improvements
aristocratos, Tue Jun 1 22:36:36 2021 +0200 9bf7da0 :
   Graph symbols to 5x5 2D vector
aristocratos, Mon May 31 21:47:41 2021 +0200 d5aa08b :
   Graph changes
aristocratos, Sun May 30 21:46:10 2021 +0200 46f098f :
   Corrections
aristocratos, Sun May 30 17:01:57 2021 +0200 4780035 :
   Fix Graph
aristocratos, Sun May 30 02:15:09 2021 +0200 3263e74 :
   Added Graph class
aristocratos, Sat May 29 02:32:36 2021 +0200 f424feb :
   Small changes
aristocratos, Thu May 27 22:29:36 2021 +0200 ecd4ef9 :
   Added Meter class
aristocratos, Wed May 26 16:23:29 2021 +0200 e7cbc28 :
   small
aristocratos, Sun May 23 22:25:07 2021 +0200 342f0af :
   Added error logger
aristocratos, Sun May 23 01:59:13 2021 +0200 b5e709d :
   Added init path setup and strf_time function
aristocratos, Sat May 22 02:13:56 2021 +0200 81f2284 :
   Proc optimization
aristocratos, Thu May 20 15:04:06 2021 +0200 881b90f :
   fixes
aristocratos, Thu May 20 15:03:33 2021 +0200 9a901a4 :
   ssplit() added ignore_remainder bool
aristocratos, Thu May 20 09:36:36 2021 +0200 d17c6d4 :
   Switch /proc/pid/stat to string parsing to avoid bad charaters in comm field
aristocratos, Wed May 19 23:21:56 2021 +0200 098a914 :
   Proc changes
aristocratos, Tue May 18 22:11:34 2021 +0200 806045c :
   Switched from map to unordered_map where possible
aristocratos, Tue May 18 01:16:22 2021 +0200 9b83753 :
   Switch from vectors to arrays when size is known
aristocratos, Sun May 16 22:58:16 2021 +0200 e040e6b :
   Added color gradients and function for drawing boxes
aristocratos, Sat May 15 13:24:24 2021 +0200 8364d85 :
   changed proc cache to be more effective
aristocratos, Sat May 15 02:56:04 2021 +0200 c007282 :
   added proc_info and p_cache structs instead of tuples
aristocratos, Fri May 14 18:54:37 2021 +0200 833d253 :
   Namespaces < Classes
aristocratos, Thu May 13 21:11:10 2021 +0200 05eb21d :
   Added threads, memory to processes
aristocratos, Thu May 13 03:12:46 2021 +0200 621534f :
   del
aristocratos, Thu May 13 03:11:29 2021 +0200 cfa944f :
   Processes sorting, filtering and cpu calculations
aristocratos, Mon May 10 23:46:41 2021 +0200 d1180d6 :
   Testing Processes class
aristocratos, Sun May 9 22:27:59 2021 +0200 73098b7 :
   Added
aristocratos, Sun May 9 22:25:41 2021 +0200 faa52b0 :
   Testing reading from proc
aristocratos, Sun May 9 00:18:51 2021 +0200 224ade2 :
   Refactoring and new functions
aristocratos, Sat May 8 20:37:36 2021 +0200 3b3a939 :
   File reorganization
aristocratos, Sat May 8 14:56:48 2021 +0200 55e2a3f :
   Split up code in multiple files
aristocratos, Sat May 8 02:38:51 2021 +0200 7ca94a1 :
   Refactoring
aristocratos, Thu May 6 23:14:20 2021 +0200 4850075 :
   ...
aristocratos, Thu May 6 23:12:18 2021 +0200 5f86b9f :
   ignore null extension
aristocratos, Thu May 6 23:09:11 2021 +0200 535b311 :
   Color functions changes
aristocratos, Thu May 6 20:45:13 2021 +0200 ce3eef6 :
   exclude btop binary
aristocratos, Thu May 6 20:32:03 2021 +0200 f4b6bff :
   Init
