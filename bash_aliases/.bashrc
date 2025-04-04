# ~/.bashrc: executed by bash(1) for non-login shells.
# see /usr/share/doc/bash/examples/startup-files (in the package bash-doc)
# for examples
#
start=$(date +%s%N)

# If not running interactively, don't do anything
[ -z "$PS1" ] && return

# don't put duplicate lines in the history. See bash(1) for more options
# don't overwrite GNU Midnight Commander's setting of `ignorespace'.
HISTCONTROL=$HISTCONTROL${HISTCONTROL+,}ignoredups
# ... or force ignoredups and ignorespace
HISTCONTROL=ignoreboth

# append to the history file, don't overwrite it
shopt -s histappend

# for setting history length see HISTSIZE and HISTFILESIZE in bash(1)

# check the window size after each command and, if necessary,
# update the values of LINES and COLUMNS.
shopt -s checkwinsize

# make less more friendly for non-text input files, see lesspipe(1)
[ -x /usr/bin/lesspipe ] && eval "$(SHELL=/bin/sh lesspipe)"

# set variable identifying the chroot you work in (used in the prompt below)
if [ -z "$debian_chroot" ] && [ -r /etc/debian_chroot ]; then
    debian_chroot=$(cat /etc/debian_chroot)
fi

# set a fancy prompt (non-color, unless we know we "want" color)
case "$TERM" in
	xterm*|vt*|screen*) color_prompt=yes
	echo "terminal $TERM"
	;;
esac

# uncomment for a colored prompt, if the terminal has the capability; turned
# off by default to not distract the user: the focus in a terminal window
# should be on the output of commands, not on the prompt
#force_color_prompt=yes

if [ -n "$force_color_prompt" ]; then
	if [ -x /usr/bin/tput ] && tput setaf 1 >&/dev/null; then
	# We have color support; assume it's compliant with Ecma-48
	# (ISO/IEC-6429). (Lack of such support is extremely rare, and such
	# a case would tend to support setf rather than setaf.)
		color_prompt=yes
	else
		color_prompt=
	fi
fi

if [ "$color_prompt" = yes ]; then
	PS1='${debian_chroot:+($debian_chroot)}\[\033[01;32m\]\u@\h\[\033[00m\]:\[\033[01;36m\]\W\[\033[00m\]\$ '
else
    #PS1='${debian_chroot:+($debian_chroot)}\u@\h:\e[1;36m\w\e[m\$ '
	PS1='${debian_chroot:+($debian_chroot)}\u@\h:\w\$ '
fi
unset color_prompt force_color_prompt

# If this is an xterm set the title to user@host:dir
case "$TERM" in
xterm*|rxvt*)
	PS1="\[\e]0;${debian_chroot:+($debian_chroot)}\u@\h: \w\a\]$PS1"
	;;
	*)
;;
esac

# enable color support of ls and also add handy aliases
if [ -x /usr/bin/dircolors ]; then
#	test -r ~/.dircolors && eval "$(dircolors -b ~/.dircolors)" || eval "$(dircolors -b)"
#	alias ls='ls --color=none'
	alias ls='ls --color=auto'
	alias dir='dir --color=auto'
	alias vdir='vdir --color=auto'
	alias grep='grep --color=auto'
	alias fgrep='fgrep --color=auto'
	alias egrep='egrep --color=auto'
fi
shopt -s xpg_echo

if [ -f ~/git/Linux/bash_aliases/.bash_aliases ]; then
	. ~/git/Linux/bash_aliases/.bash_aliases
fi

if [ -f ~/git/Linux/bash_aliases/.vim_aliases ]; then
        . ~/git/Linux/bash_aliases/.vim_aliases
fi

# enable programmable completion features (you don't need to enable
# this, if it's already enabled in /etc/bash.bashrc and /etc/profile
# sources /etc/bash.bashrc).
if [ -f /etc/bash_completion ] && ! shopt -oq posix; then
	. /etc/bash_completion
fi

# enable emacs & vi mode
set -o vi
set -o emacs

#isWin=`uname | grep -i cygwin`
if [[ "$OS" == *Window* ]]; then
	echo setting windows aliases;
	if [ -f ~/git/Linux/bash_aliases/.cygwin_bash_aliases ]; then
		. ~/git/Linux/bash_aliases/.cygwin_bash_aliases
		if [ -f ~/private/.mycreds ]; then
			. ~/private/.mycreds
		fi
	fi
else
	echo setting linux aliases;
	if [ -f ~/git/Linux/bash_aliases/.linux_bash_aliases ]; then
		if [ -f ~/private/.mycreds ]; then
			. ~/git/Linux/bash_aliases/.linux_bash_aliases
			. ~/private/.mycreds
		fi
	fi
fi


end=$(date +%s%N)
diff=$((end-start))
if (( diff > 90000000 )); then
    printf "%s.%s seconds to load bash profile\n" "${diff:0: -9}" "${diff: -9:3}"
fi
