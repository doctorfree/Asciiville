Name: Asciiville
Version:    %{_version}
Release:    %{_release}
BuildArch:  x86_64
Requires: util-linux, python3, python3-pip, gnome-terminal, tilix, xfce4-terminal, cool-retro-term, tmux, mplayer, w3m, ranger, jq, asciinema, speedtest-cli, neofetch, figlet, ImageMagick, dconf, libjpeg-turbo, libpng, cmatrix, gnupg, pandoc
URL:        https://github.com/doctorfree/Asciiville
Vendor:     Doctorwhen's Bodacious Laboratory
Packager:   ronaldrecord@gmail.com
License     : MIT
Summary     : ASCII Art and Utilities

%global __os_install_post %{nil}

%description

%prep

%build

%install
cp -a %{_sourcedir}/usr %{buildroot}/usr

%pre

%post
export PATH=${PATH}:/usr/local/bin
python3_inst=`type -p python3`
if [ "${python3_inst}" ]
then
  PYTHON="python3"
else
  PYTHON="python"
fi
${PYTHON} -m pip install asciimatics
${PYTHON} -m pip install rainbowstream
npm_inst=`type -p npm`
if [ "${npm_inst}" ]
then
  npm install -g mapscii
fi

lol_inst=`type -p lolcat`
if [ ! "${lol_inst}" ]
then
  gem_inst=`type -p gem`
  if [ "${gem_inst}" ]
  then
    wget_inst=`type -p wget`
    if [ "${wget_inst}" ]
    then
      unzip_inst=`type -p unzip`
      if [ "${unzip_inst}" ]
      then
        wget --quiet https://github.com/busyloop/lolcat/archive/master.zip
        unzip -qq master.zip
        cd lolcat-master/bin
        gem install lolcat
        cd ../..
        rm -f master.zip
        rm -rf lolcat-master
      fi
    fi
  fi
fi

# Link our Figlet Fonts into default Figlet Font directory
# FIGLET_DIR="/usr/share/figlet"
# FIGLET_NEW="/usr/share/figlet-fonts"
# if [ -d ${FIGLET_NEW} ]
# then
#   if [ ! -d ${FIGLET_DIR} ]
#   then
#     mkdir -p ${FIGLET_DIR}
#   fi
#   cd ${FIGLET_NEW}
#   for font in *
#   do
#     if [ -f ${FIGLET_DIR}/"${font}" ]
#     then
#       continue
#     else
#       ln -s ${FIGLET_NEW}/"${font}" ${FIGLET_DIR}/"${font}" 
#     fi
#   done
# fi

FIGLET_DIR="/usr/share/figlet-fonts"
FIGLET_ZIP="figlet-fonts.zip"
zip_inst=`type -p zip`
if [ "${zip_inst}" ]
then
  pyfig_inst=`type -p pyfiglet`
  if [ "${pyfig_inst}" ]
  then
    if [ -d ${FIGLET_DIR} ]
    then
      cd ${FIGLET_DIR}
      zip -q ${FIGLET_ZIP} *.flf
      pyfiglet -L ${FIGLET_ZIP}
      rm -f ${FIGLET_ZIP}
    fi
  fi
fi

%preun

# FIGLET_DIR="/usr/share/figlet"
# FIGLET_NEW="/usr/share/figlet-fonts"
# if [ -d ${FIGLET_DIR} ]
# then
#   cd ${FIGLET_DIR}
#   for font in ${FIGLET_NEW}/*
#   do
#     if [ "${font}" == "${FIGLET_NEW}/*" ]
#     then
#       continue
#     else
#       b=`basename "${font}"`
#       if [ -L "$b" ]
#       then
#         readlink "$b" | grep ${FIGLET_NEW} > /dev/null && rm -f "$b"
#       else
#         continue
#       fi
#     fi
#   done
# fi

%files
/usr
%exclude %dir /usr/share/man/man1
%exclude %dir /usr/share/man
%exclude %dir /usr/share/doc
%exclude %dir /usr/share/menu
%exclude %dir /usr/share
%exclude %dir /usr/bin

%changelog
