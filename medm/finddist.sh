#!/bin/sh
# Find out the Linux distribution
# Ideas stolen from here:
#   https://github.com/jeonghanlee/pkg_automation/blob/master/pkg_automation.bash

finddistinternal() {
  if [ -x /usr/bin/lsb_release ] ; then
    dist_id=$(lsb_release -is)
    dist_cn=$(lsb_release -cs)
    dist_rs=$(lsb_release -rs)
    echo $dist_id ${dist_cn} ${dist_rs}
    return 0
  fi
  if [ -r /usr/bin/lsb_release ] ; then
    #eval $(grep "^PRETTY_NAME=" /etc/os-release)
    eval $(grep "^ID=" /etc/os-release)
    eval $(grep "^VERSION_ID=" /etc/os-release)
    #echo ${PRETTY_NAME}
		echo $ID$VERSION_ID
    return 0
  fi
  echo "unknown"
	return 0
}

if [ "$1" = "USR_LIBS" ] ; then
  dist=$(finddistinternal)
	echo >&2 dist=$dist
  case "$dist" in
    [dD]ebian*9*)
			echo "Xm Xt Xmu X11 Xext"
     	echo >&2 Debian 9
       ;;
		Raspbian.*8)
			echo "Xm Xt Xmu X11 Xext"
     	echo >&2 "Raspbian.*8"
       ;;
		*)
			echo "Xm Xt Xp Xmu X11 Xext"
     	echo >&2 "Needs Xp"
       ;;
	esac
	exit 0
fi

echo $(finddistinternal)
exit 0
